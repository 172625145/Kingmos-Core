/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/


/*****************************************************
文件说明：串口驱动---硬件无关层
版本号：  1.0.0
开发时期：2003-07-01
作者：    肖远钢
修改记录：
	2004-12-06, 增加了GetRxBufferSize 函数，有PDD层返回适合该硬件的size
			同时，去掉了对 RxBuffers的动态设置功能(IOCTL_SERIAL_SET_QUEUE_SIZE..)，有以下考虑：
			1。PDD层应该是通过充分的测试。
			2。对user来说，他们没有我们专业。
			3。简化对 临界段的操作，使代码效率更高。
			4. ser_HandleInterrupt 是在高优先级状态运行。可以不需要某些 临界段的操作
******************************************************/

//----------------------------------------------------
//
//文件解说：
//1.  流设备接口实现(参考drvCom) 和其注册(RegisterDevice_COM)
//2.  CreateFile的打开方式(参考COM_Open说明)
//3.  握手提示Buffer极限的大小与接收Buffer、Baudrate相关(参考ser_CalHWLim)
//4.  用户SetDCB时，用ser_GetFlagSetDCB判断是否对硬件有新设置(参考ser_GetFlagSetDCB 和 
//    IOCTL_SERIAL_SET_DCB)
//5.  "for Braden"是方便于FAX等使用(参考COM_Open说明)
//
//----------------------------------------------------

//root   include
#include <ewindows.h>
#include <edevice.h>	// for DEVICE_DRIVER  and RegisterDevice
#include <serbase.h>
#include <listunit.h>
//Private include
//#include <isr.h>		//for interrupt callback's defines "LPISR"
//driver  include
#include <sysintr.h>	//for INTR_* function
//#include <serbase.h>
#include <sertbl.h>
//current
#include "xhelp.h"
#include "ser_drv.h"

#include <devdrv.h>

/***************  全局区 定义， 声明 *****************/


//-----------------------------------------
//调试部分的定义
//-----------------------------------------

#undef	DEBUG_SERDRV	// for debug

#ifdef	DEBUG_SERDRV

//static	LONG	g_nCountRx_Debug;
//static	LONG	g_nCountRead_Debug;

void	dcb_Debug( LPDCB lpDCB, LPTSTR pszText );

#endif	//DEBUG_SERDRV

//-----------------------------------------
//正常的定义
//-----------------------------------------
//help function
static	void	ser_CalHWLim( PSERDRV_INFO pInfoDrv );
static	DWORD	ser_GetFlagSetDCB( DCB* lpDCBDes, DCB* lpDCBSet );
static	void	ser_GetFlow( PSERDRV_INFO pInfoDrv );
static	LPVOID	GetSerialObject(DWORD dwIDObj);

static	void	ser_NotifyCommEvent_Someone( PSEROPEN_INFO pInfoOpen, DWORD fdwEventMask );	//for braden
		VOID	ser_NotifyCommEvent(PVOID pHead, DWORD fdwEventMask);
static	DWORD	ser_WaitCommEvent(PSEROPEN_INFO pInfoOpen, LPDWORD pfdwEventMask, LPOVERLAPPED Unused);
static	VOID	ser_DoTxData( PSERDRV_INFO pInfoDrv );
		void	ser_HandleInterrupt( LPVOID pData );

DWORD	COM_Init( DWORD   dwParamDevice );
BOOL	COM_Deinit(DWORD pHead);
DWORD	COM_Open( DWORD  pHead, DWORD  dwAccessCode, DWORD dwShareMode );
BOOL	COM_Close(DWORD pHead);
DWORD	COM_Read( DWORD pHead, LPVOID lpBuffer, DWORD dwnLenRead );
DWORD	COM_Write(DWORD pHead, LPCVOID lpBuffer, DWORD  NumberOfBytes );
DWORD	COM_Seek( DWORD  pHead, LONG Position, DWORD Type );
VOID	COM_PowerUp( DWORD pHead );
VOID	COM_PowerDown( DWORD   pHead );
BOOL	COM_IOControl(DWORD pHead, DWORD dwCode, LPVOID pBufIn_, DWORD dwLenIn, LPVOID pBufOut_, DWORD dwLenOut, LPDWORD pdwActualOut);

/******************************************************/
//PRXINFO
#define GET_RX_DATA_LENGTH( lpRxBuf )  ( (lpRxBuf)->nWrite >= (lpRxBuf)->nRead ? ( (lpRxBuf)->nWrite - (lpRxBuf)->nRead ) : ( (lpRxBuf)->nLength - (lpRxBuf)->nRead + (lpRxBuf)->nWrite ) )
//#define GET_RX_DATA_LENGTH( lpRxBuf )  ( lpRxBuf->nWrite >= lpRxBuf->nRead ? ( lpRxBuf->nWrite - lpRxBuf->nRead ) : ( lpRxBuf->nLength - lpRxBuf->nRead + lpRxBuf->nWrite ) )

const DEVICE_DRIVER drvCom =
{
    COM_Init,
	COM_Deinit,
	COM_IOControl,
	COM_Open,
	COM_Close,
	COM_Read,
	COM_Write,
	COM_Seek,
	COM_PowerUp,
	COM_PowerDown
};

// ********************************************************************
// 声明：BOOL RegisterDevice_COM( DWORD nIndex )
// 参数：
//	IN nIndex-指定索引号
// 返回值：
//	成功，返回TRUE;失败，返回FALSE 
// 功能描述：注册设备驱动程序
// 引用: 
// ********************************************************************
BOOL RegisterDevice_COM( DWORD nIndex )
{
	HANDLE	hDev;

    hDev = RegisterDriver( "COM", nIndex, &drvCom, (LPVOID)nIndex );
	RETAILMSG( 1, (TEXT("\r\n\r\nRegisterDevice_COM__xyg: [COM%d] hDev=0x%x\r\n\r\n"), nIndex, hDev) );
	if( hDev )
		return TRUE;
	else
		return FALSE;
}


// ********************************************************************
// 声明：void	ser_HandleInterrupt( LPVOID  pHead )
// 参数：
//	IN pHead-驱动信息结构
// 返回值：
//	无 
// 功能描述：中断处理
// 引用: 
// ********************************************************************
void	ser_HandleInterrupt( LPVOID  pHead )
{
    PSERDRV_INFO	pInfoDrv = (PSERDRV_INFO)pHead;
    PVOID			pHeadDev;
    PVTBL_ARCH_SER	pFunTbl;
    DWORD			dwIntr;

	//------------------------- for INTR_RX begin
	char		XoffChar;
	char		XonChar;
	WORD		wXXXX;

    BOOL		RxDataAvail;
	LONG		nLenRx_Write;
	LPBYTE		pBufWrite;
	
	PRXINFO		lpInfoRxBuf;
	register	LONG*		pnRead;
	register	LONG*		pnWrite;
//	register	LONG*		pnCounts;
	register	LONG*		pnLength;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        return ;
	}
	//获取基本信息
	pHeadDev = pInfoDrv->pHeadDev;
	pFunTbl = pInfoDrv->pFunTbl;
	//获取 接收信息
	lpInfoRxBuf = &pInfoDrv->InfoRxBuf;
	pnRead		= &( lpInfoRxBuf->nRead );
	pnWrite		= &( lpInfoRxBuf->nWrite );
//	pnCounts    = &( lpInfoRxBuf->nCounts );
	pnLength	= &( lpInfoRxBuf->nLength );

	XoffChar = pInfoDrv->dcb.XoffChar;
	XonChar  = pInfoDrv->dcb.XonChar;
	//------------------------- for INTR_RX end

	if ( pInfoDrv->pAccessOwner )
	{
		INTR_CNT_INC(pInfoDrv->pAccessOwner);
	}
	
	//csEnter_RxResize( lpInfoRxBuf );	//功能： 防止 rxBuffer重新分配 和 中断重入
	dwIntr = INTR_NONE;
	RxDataAvail = FALSE;
	while( dwIntr=pFunTbl->lpFnIntrTypeQuery(pHeadDev) )
	{
		if( dwIntr & INTR_RX )
		{
			//2004-12-06, 不需要，看修改说明
			//防止 rxBuffer重新分配 和 中断重入
			//csEnter_RxResize( lpInfoRxBuf );
			//			
			//获取 剩余空间
			//csEnter_Rx(pInfoDrv);
			//			
         
			if( *pnRead == 0 )
			{   // 保留一个字节为COM_Read做是否有数据判断
				nLenRx_Write = *pnLength - *pnWrite - 1;
			}
			if( *pnRead > *pnWrite )
			{	// 保留一个字节为COM_Read做是否有数据判断
				nLenRx_Write = *pnRead - *pnWrite - 1;
			}
			else
			{
				nLenRx_Write = *pnLength - *pnWrite;
			}
			
/*			
			if( *pnCounts == *pnLength )
			{	//没有足够的空间
				nLenRx_Write = 0;
			}
			else
			{			
				if( *pnWrite < *pnRead )
				{
					nLenRx_Write = (*pnRead - *pnWrite);
				}
				else
				{
					nLenRx_Write = (*pnLength - *pnWrite);
				}
			}
*/			
			//2004-12-06, 不需要，看修改说明			
			//csLeave_Rx(pInfoDrv);

			//接收/丢弃 字符
			if( nLenRx_Write==0 )
			{
				//丢弃 字符
				BYTE    TempBuf[16];//16
				nLenRx_Write = 16;
				pFunTbl->lpFnRecv( pHeadDev, TempBuf, &nLenRx_Write );
				EdbgOutputDebugString( "dis[%d]", nLenRx_Write );
				nLenRx_Write = 0;
			}
			else
			{
				//接收 字符
				pBufWrite = lpInfoRxBuf->pRxBuf + (*pnWrite);
				pFunTbl->lpFnRecv( pHeadDev, pBufWrite, &nLenRx_Write );
				if( nLenRx_Write )
				{
					//
					//软件流控制---控制发送功能
					//1. 检测是否收到通知字符(Xoff/Xon)
					//2. 设置停止/继续发送的标志
					//3. Xoff/Xon不属于正常数据，要丢弃
					//
					if( pInfoDrv->dcb.fOutX )
					{
						register	LPBYTE		pBufWrite_tmp;
						LONG        nLenMove;
						
						pBufWrite_tmp = pBufWrite;
						nLenMove = nLenRx_Write;
						while( nLenMove>0 )
						{
							nLenMove --;
							if( *pBufWrite_tmp==XoffChar )
							{
								EdbgOutputDebugString( "Xoff\r\n" );

								//停止发送(because received a Xoff)
								pInfoDrv->fStopXmit = 1;

								//丢弃Xoff
								if( nLenMove )
									memmove( pBufWrite_tmp, pBufWrite_tmp+1, nLenMove );	//nLenRx_Write - nIndex - 1
								nLenRx_Write--;
							}
							else if( *pBufWrite_tmp==XonChar )
							{
								EdbgOutputDebugString( "Xon\r\n" );

								//继续发送(because received a Xon)
								pInfoDrv->fStopXmit = 0;
								//若收到Xon，则尝试发送数据
								dwIntr |= INTR_TX;

								//丢弃Xon
								if( nLenMove )
									memmove( pBufWrite_tmp, pBufWrite_tmp+1, nLenMove );
								nLenRx_Write--;
							}
							else
							{
								pBufWrite_tmp ++;
							}
						}//for  for ( nLenMove=0; nLenMove < nLenRx_Write; )
					}//for  if ( pInfoDrv->dcb.fOutX )

					//正常数据保存
					if( nLenRx_Write )
					{
						//设置 信息---nCounts and nWrite
						//2004-12-06, 不需要，看修改说明
						//csEnter_Rx( pInfoDrv );

						//g_nCountRx_Debug += nLenRx_Write;	//for debug
//						(*pnCounts)+= nLenRx_Write;
						(*pnWrite) += nLenRx_Write;
						(*pnWrite) %= (*pnLength);

						//csLeave_Rx( pInfoDrv );

						RxDataAvail = TRUE;
					}
				}
			}

			//
			//流控制---控制“接收剩余空间”功能
			//
			if( pInfoDrv->fFlow_RxBuf )
			{
				//RxBuf剩余空间
				//2004-12-06, 不需要，看修改说明
				//csEnter_Rx( pInfoDrv );
				                           
				nLenRx_Write = *pnLength - GET_RX_DATA_LENGTH(lpInfoRxBuf);//*pnCounts;
				//2004-12-06, 不需要，看修改说明
				//csLeave_Rx( pInfoDrv );

				//如果到达，硬件最低“接收剩余空间”，就硬件通知对方
				if( nLenRx_Write <= pInfoDrv->dwHWOffLim )
				{
					if( (pInfoDrv->dcb.fDtrControl==DTR_CONTROL_HANDSHAKE) && (!pInfoDrv->fSentDtrOff) )
					{
						pInfoDrv->fSentDtrOff = 1;
						pFunTbl->lpFnClearSingalDTR(pHeadDev);
					}
					if( (pInfoDrv->dcb.fRtsControl==RTS_CONTROL_HANDSHAKE) && (!pInfoDrv->fSentRtsOff) )
					{
						pInfoDrv->fSentRtsOff = 1;
						pFunTbl->lpFnClearSingalRTS(pHeadDev);
					}
				}

				//如果到达，软件最低“接收剩余空间”，就软件通知对方
				if( nLenRx_Write <= pInfoDrv->dcb.XoffLim )
				{
					if( pInfoDrv->dcb.fInX && !(pInfoDrv->fSentXoff) )
					{
						pInfoDrv->fSentXoff = 1;
						pFunTbl->lpFnXmitChar( pHeadDev, XoffChar);
					}
				}
			}
			//
			//2004-12-06, 不需要，看修改说明
			//csLeave_RxResize( lpInfoRxBuf );
			//
		}//for if ( dwIntr & INTR_RX ) 

		if ( dwIntr & INTR_TX )
		{
			ser_DoTxData( pInfoDrv );
		}
		if ( dwIntr & INTR_MODEM ) 
		{
			pFunTbl->lpFnIntrHandleModem(pHeadDev);
		}
		if ( dwIntr & INTR_LINE )
		{
			pFunTbl->lpFnIntrHandleLine(pHeadDev);
		}
	}
	//2004-12-06, 不需要，看修改说明
	//csLeave_RxResize( lpInfoRxBuf );
	
	//EV_RXCHAR
	if( RxDataAvail )
	{
		SetEvent(pInfoDrv->hEvtRead);
		ser_NotifyCommEvent( pInfoDrv, EV_RXCHAR );
	}
	
	if ( pInfoDrv->pAccessOwner )
	{
		INTR_CNT_DEC(pInfoDrv->pAccessOwner);
	}
}

// ********************************************************************
// 声明：DWORD	COM_Init( DWORD dwParamDevice )
// 参数：
//	IN dwParamDevice-初始化参数
// 返回值：
//	驱动信息结构
// 功能描述：驱动程序初始化
// 引用: 
// ********************************************************************
DWORD	COM_Init( DWORD dwParamDevice )
{
	BOOL			fSuccess;
    PSERDRV_INFO	pInfoDrv;

	////RETAILMSG( 1, (TEXT("COM_Init: [%d]\r\n"), dwParamDevice) );
	//分配结构
    pInfoDrv  =  (PSERDRV_INFO)HANDLE_ALLOC( sizeof(SERDRV_INFO) );
    if ( !pInfoDrv )
        return 0;
	HANDLE_INIT( pInfoDrv, sizeof(SERDRV_INFO) );
	fSuccess = FALSE;

 	//初始化---ARCH对象的信息
    pInfoDrv->pObjDev = (POBJECT_ARCH)GetSerialObject( dwParamDevice );
    if( !pInfoDrv->pObjDev || !(pInfoDrv->pObjDev->pFunTbl) )
	{
        HANDLE_FREE( pInfoDrv );
        return 0;
    }
	pInfoDrv->pFunTbl = (PVTBL_ARCH_SER)pInfoDrv->pObjDev->pFunTbl;

	//初始化---接收缓冲
    //2004-12-06 lilin , remove to 下面	
    //pInfoDrv->InfoRxBuf.nLength = RX_BUFFER_SIZE_MIN;
    //pInfoDrv->InfoRxBuf.pRxBuf  = (LPBYTE)malloc( pInfoDrv->InfoRxBuf.nLength );
    //

	//初始化---读写
    pInfoDrv->hEvtWrite   = CreateEvent(0, FALSE, FALSE, NULL);
    pInfoDrv->hEvtRead    = CreateEvent(0, FALSE, FALSE, NULL);
	csInit_Read( pInfoDrv );
    csInit_Write( pInfoDrv );
	csInit_RxResize( &pInfoDrv->InfoRxBuf );
	csInit_Rx( pInfoDrv );
	csInit_Tx( pInfoDrv );

	//初始化---打开链表
	csInit_hListOpen( pInfoDrv );
	List_InitHead( &pInfoDrv->hListOpen );

    //初始化---默认超时
    pInfoDrv->CommTmouts.ReadIntervalTimeout		  = READ_TIMEOUT;
    pInfoDrv->CommTmouts.ReadTotalTimeoutMultiplier = READ_TIMEOUT_MULTIPLIER;
    pInfoDrv->CommTmouts.ReadTotalTimeoutConstant	  = READ_TIMEOUT_CONSTANT;
    pInfoDrv->CommTmouts.WriteTotalTimeoutMultiplier= WRITE_TIMEOUT_MULTIPLIER;
    pInfoDrv->CommTmouts.WriteTotalTimeoutConstant  = WRITE_TIMEOUT_CONSTANT;

	//初始化---硬件
    pInfoDrv->pHeadDev = pInfoDrv->pFunTbl->lpFnInit(dwParamDevice, pInfoDrv, &pInfoDrv->dcb);
    if( !pInfoDrv->pHeadDev )
	{
		////RETAILMSG( 1, (TEXT("COM_Init: fail because   lpFnInit\r\n")) );
		goto EXIT_INIT;
    }
    //2004-12-06, lilin add
    pInfoDrv->InfoRxBuf.nLength = 2 * pInfoDrv->pFunTbl->lpGetRxBufferSize(pInfoDrv->pHeadDev);
    if( pInfoDrv->InfoRxBuf.nLength < RX_BUFFER_SIZE )
        pInfoDrv->InfoRxBuf.nLength = RX_BUFFER_SIZE;
    
    pInfoDrv->InfoRxBuf.pRxBuf = malloc( pInfoDrv->InfoRxBuf.nLength );
    if( !pInfoDrv->InfoRxBuf.pRxBuf )
	{
        goto EXIT_INIT;
    }
    //
	//pInfoDrv->pFunTbl->lpFnInitLast( pHWHead );//再给1次机会 硬件初始化

	//调试 的初始化
	//Debug_Mdd_init( pInfoDrv );//attention: used to deubg

	fSuccess = TRUE;
EXIT_INIT:
	if( !fSuccess )
	{
        COM_Deinit((DWORD)pInfoDrv);
		return 0;
	}
	else
	{
		////RETAILMSG( 1, (TEXT("\r\nCOM_Init: success----\r\n")) );
		return (DWORD)pInfoDrv;
	}
}

// ********************************************************************
// 声明：BOOL	COM_Deinit(DWORD pHead)
// 参数：
//	IN pHead-驱动信息结构
// 返回值：
//	成功返回TRUE
// 功能描述：驱动程序释放
// 引用: 
// ********************************************************************
BOOL	COM_Deinit(DWORD pHead)
{
	PSERDRV_INFO	pInfoDrv = (PSERDRV_INFO)pHead;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError(ERROR_INVALID_HANDLE);
        return (FALSE);
	}

    //退出 中断处理线程---移到Arch层
	//{
	//	pInfoDrv->fExitISR = TRUE;
	//	SetEvent( pInfoDrv->hEvtISR );
	//	CloseHandle( pInfoDrv->hThrdISR );
	//}

	//关闭端口 COM_Close
	if( pInfoDrv->nOpenCnt ) 
	{
        PSEROPEN_INFO	pInfoOpen;
        PLIST_UNIT		pUnit;
		PLIST_UNIT		pUnitHeader;

		pUnitHeader = &pInfoDrv->hListOpen;
		pUnit = pUnitHeader->pNext;
        while( pUnit!=pUnitHeader )
		{
			pInfoOpen = LIST_CONTAINER( pUnit, SEROPEN_INFO, hListOpen);
			pUnit = pUnit->pNext;

			COM_Close((DWORD)pInfoOpen);
		}
	}
	
	//释放所有资源
	csDelete_Read( pInfoDrv );
	csDelete_Write( pInfoDrv );
	csDelete_Tx( pInfoDrv );
	csDelete_RxResize( &pInfoDrv->InfoRxBuf );
	csDelete_Rx( pInfoDrv );
    if ( pInfoDrv->hEvtRead )
        CloseHandle(pInfoDrv->hEvtRead);
    if ( pInfoDrv->hEvtWrite )
        CloseHandle(pInfoDrv->hEvtWrite);
    if ( pInfoDrv->InfoRxBuf.pRxBuf )
        free(pInfoDrv->InfoRxBuf.pRxBuf);

	//释放 ARCH
    pInfoDrv->pFunTbl->lpFnDeinit(pInfoDrv->pHeadDev);

	//释放 自己
    HANDLE_FREE( pInfoDrv );

    return (TRUE);
}


//----------------------------------------------------
//说明：
//	1. RW 独占---以 RW 方式打开，只能1次------------------------属于正常情况
//	2. 0  允许--- 0 方式可以在任何情况打开，用于IOControl操作---属于正常情况
//	3. D  优先---允许 D 方式打开1次，无论是否有 RW 方式打开的，并通知其它 RW 方式打开的功能关闭（退出Com_read/com_write, 不用通知事件给Waitcommevent）
//				 并在 D 方式关闭时，取消其它 RW 方式打开的功能限制
//	4. D  排斥---以 D 方式打开后，就再不能以 RW 方式打开
//
// ********************************************************************
// 声明：DWORD	COM_Open( DWORD pHead, DWORD dwAccessCode, DWORD dwShareMode )
// 参数：
//	IN pHead-驱动信息结构
//	IN dwAccessCode-访问方式
//	IN dwShareMode-共享方式
// 返回值：
//	返回打开信息结构
// 功能描述：打开设备
// 引用: 
// ********************************************************************
DWORD	COM_Open( DWORD pHead, DWORD dwAccessCode, DWORD dwShareMode )
{
    PSERDRV_INFO	pInfoDrv = (PSERDRV_INFO)pHead;
    PSEROPEN_INFO   pInfoOpen;
    PVTBL_ARCH_SER	pFunTbl;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError(ERROR_INVALID_HANDLE);        
        return 0;
	}
	// 检查权限和参数
	if( dwAccessCode & GENERIC_DATA )	//for Braden
	{
		if( pInfoDrv->fOpenByData )
		{
			SetLastError(ERROR_INVALID_ACCESS);        
			return 0;
		}
	}
	else
	{
		//pInfoDrv->pAccessOwner: 保存 RW 或 D 方式打开的句柄
		if( (dwAccessCode & (GENERIC_READ | GENERIC_WRITE)) && pInfoDrv->pAccessOwner ) 
		{
			SetLastError(ERROR_INVALID_ACCESS);        
			return 0;
		}
	}

	//分配结构，保存参数
    pInfoOpen = (PSEROPEN_INFO)HANDLE_ALLOC( sizeof(SEROPEN_INFO) );
    if( !pInfoOpen ) 
	{
		return 0;
	}
	HANDLE_INIT( pInfoOpen, sizeof(SEROPEN_INFO) );
    pInfoOpen->dwAccessCode = dwAccessCode;
    pInfoOpen->dwShareMode = dwShareMode;

	//初始化---通信事件 ser_WaitCommEvent
	pInfoOpen->hEvtComm = CreateEvent(NULL, FALSE, FALSE, NULL);
    pInfoOpen->dwEvtMask = 0;
    pInfoOpen->dwEvtData = 0;
    pInfoOpen->fAbortEvt = 0;
	csInit_CommEvt( pInfoOpen );
	
	csEnter_hListOpen( pInfoDrv );
	//初始化---硬件（在第一次打开时）
    if( !pInfoDrv->nOpenCnt )
	{
		pFunTbl = pInfoDrv->pFunTbl;

		//初始化---流控制
		pInfoDrv->fSentXoff = 0;
		pInfoDrv->fStopXmit = 0;

		//初始化---DCB配置
		memset( &pInfoDrv->dcb, 0, sizeof(DCB) );
        pInfoDrv->dcb.DCBlength  = sizeof(DCB);
        pInfoDrv->dcb.BaudRate   = CBR_9600; //CBR_115200  
        pInfoDrv->dcb.fBinary    = TRUE;

        pInfoDrv->dcb.fParity    = FALSE;
        pInfoDrv->dcb.ByteSize   = 8;
        pInfoDrv->dcb.Parity     = NOPARITY;
        pInfoDrv->dcb.StopBits   = ONESTOPBIT;

        pInfoDrv->dcb.fDtrControl= DTR_CONTROL_ENABLE;
        pInfoDrv->dcb.fRtsControl= RTS_CONTROL_ENABLE;
        pInfoDrv->dcb.XonLim     = 512;//send Xon,  if RxBuffer剩余空间 >= DCB.XonLim
        pInfoDrv->dcb.XoffLim    = 200;//send Xoff, if RxBuffer剩余空间 <= DCB.XoffLim

        pInfoDrv->dcb.XonChar    = X_ON_CHAR;
        pInfoDrv->dcb.XoffChar   = X_OFF_CHAR;
        pInfoDrv->dcb.ErrorChar  = ERROR_CHAR;
        pInfoDrv->dcb.EofChar    = E_OF_CHAR;
        pInfoDrv->dcb.EvtChar    = EVENT_CHAR;
		//获取流控制
		ser_GetFlow( pInfoDrv );
		//设置DCB到硬件
		//if( !pFunTbl->lpFnSetDCB( pInfoDrv->pHeadDev, &(pInfoDrv->dcb), SETDCB_ALL ) )
		//	goto EXIT_OPEN;

		//设置硬件握手的极限值
		ser_CalHWLim( pInfoDrv );

		//打开硬件
		if ( !pFunTbl->lpFnOpen(pInfoDrv->pHeadDev) )	//此中会进行 lpFnSetDCB 调用
            goto EXIT_OPEN;

		//清除硬件
        pFunTbl->lpFnPurgeComm( pInfoDrv->pHeadDev, PURGE_RXCLEAR );

		//清除RxBuffer
		csEnter_Rx( pInfoDrv );
		pInfoDrv->InfoRxBuf.nWrite	= 0;
		pInfoDrv->InfoRxBuf.nRead	= 0;
//		pInfoDrv->InfoRxBuf.nCounts = 0;
		//memset( pInfoDrv->InfoRxBuf.pRxBuf, 0, pInfoDrv->InfoRxBuf.nLength );
		csLeave_Rx( pInfoDrv );

		//清除TxInfo
		csEnter_Tx( pInfoDrv );
		pInfoDrv->dwTxBytes = 0;
		pInfoDrv->dwTxBytesPending = 0;
		pInfoDrv->pTxBuf = NULL;
		csLeave_Tx( pInfoDrv );
		
		//RxBuf_dbg_init( pInfoDrv );//for debug
    }

    ++(pInfoDrv->nOpenCnt);
	//关联 pInfoOpen 和 pInfoDrv
    pInfoOpen->pInfoDrv = pInfoDrv;  // pointer back to our parent
	if( dwAccessCode & GENERIC_DATA )	//for Braden
	{
		PSEROPEN_INFO	pInfoOpenNormal;

		pInfoDrv->fOpenByData = TRUE;
		//
		pInfoOpenNormal = pInfoDrv->pAccessOwner;
		if( pInfoOpenNormal )
		{
			pInfoOpenNormal->fClose = TRUE;
			SetEvent( pInfoDrv->hEvtRead );
			SetEvent( pInfoDrv->hEvtWrite );
		}
		else
		{
			pInfoDrv->pAccessOwner = pInfoOpen;
		}
	}
    else if( dwAccessCode & (GENERIC_READ | GENERIC_WRITE) ) 
	{
		pInfoDrv->pAccessOwner = pInfoOpen;
    }

	List_InsertTail(&pInfoDrv->hListOpen, &pInfoOpen->hListOpen);
	csLeave_hListOpen( pInfoDrv );

	//dcb_Debug( &pInfoDrv->dcb, "COM_Open" );
	//RETAILMSG( 1, (TEXT("\r\nCOM_Open: return success----\r\n")) );
    return (DWORD)(pInfoOpen);
	
EXIT_OPEN :
	csLeave_hListOpen( pInfoDrv );

    if ( pInfoOpen->hEvtComm )
	{
        CloseHandle(pInfoOpen->hEvtComm);
	}
	csDelete_CommEvt( pInfoOpen );

    HANDLE_FREE( pInfoOpen );
    SetLastError(ERROR_OPEN_FAILED);
    return 0;
}

// ********************************************************************
// 声明：BOOL	COM_Close(DWORD pHead)
// 参数：
//	IN pHead-打开信息结构
// 返回值：
//	成功返回TRUE
// 功能描述：关闭设备
// 引用: 
// ********************************************************************
//Close入口
BOOL	COM_Close(DWORD pHead)
{
	PSEROPEN_INFO	pInfoOpen= (PSEROPEN_INFO)pHead;
    PSERDRV_INFO	pInfoDrv;
    PVTBL_ARCH_SER	pFunTbl;
	int				i;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoOpen) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
	}
	pInfoDrv = (PSERDRV_INFO)pInfoOpen->pInfoDrv;
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
	}
	pFunTbl = pInfoDrv->pFunTbl;
	
	//
	//释放所有的调用
	//
	
	//判断当前是否被释放完
	csEnter_hListOpen( pInfoDrv );
    if( pInfoDrv->nOpenCnt<=0 )
	{
		////EdbgOutputDebugString("===COM_Close: SetLastError ");
        SetLastError( ERROR_INVALID_HANDLE );
        csLeave_hListOpen( pInfoDrv );
		return FALSE;
	}
	--(pInfoDrv->nOpenCnt);
	
	//退出--- WaitCommEvent 
	pInfoOpen->fAbortEvt = 1;
	pInfoOpen->dwEvtMask = 0;
	for( i=0; i<pInfoOpen->wCntWaitEvent; i++ )
	{
		//csEnter_CommEvt( pInfoOpen );
		SetEvent(pInfoOpen->hEvtComm);
		//csLeave_CommEvt( pInfoOpen );
	}
	Sleep(2);
	
	//退出--- ReadFile
	pInfoDrv->fAbortRead = 1;
	if ( pInfoOpen->dwAccessCode & (GENERIC_READ) )
	{
		for( i=0; i<pInfoOpen->wCntRead; i++ )
			SetEvent(pInfoDrv->hEvtRead);
	}
	Sleep(2);
	
	//退出--- WriteFile
	if ( pInfoOpen->dwAccessCode & (GENERIC_WRITE) )
	{
		for( i=0; i<pInfoOpen->wCntWrite; i++ )
			SetEvent(pInfoDrv->hEvtWrite);
	}
	//
	Sleep(2);
	
	//退出--- 打开链表
	List_RemoveUnit(&pInfoOpen->hListOpen);
	
	//关闭硬件
	if( pInfoDrv->nOpenCnt == 0 )
	{
		pFunTbl->lpFnClose(pInfoDrv->pHeadDev);
		//
		//if ( pInfoDrv->pObjDev->BindFlags & THREAD_AT_OPEN )
		//{
		//}
		if( pInfoOpen->dwAccessCode & GENERIC_DATA )//for braden
			pInfoDrv->fOpenByData = FALSE;
	}
	else if( pInfoOpen->dwAccessCode & GENERIC_DATA )//for braden
	{
		PSEROPEN_INFO	pInfoOpen;
		PLIST_UNIT		pUnit;
		PLIST_UNIT		pUnitHeader;
		
		pInfoDrv->fOpenByData = FALSE;
		
		pUnitHeader = &pInfoDrv->hListOpen;
		pUnit = pUnitHeader->pNext;
		while( pUnit!=pUnitHeader )
		{
			pInfoOpen = LIST_CONTAINER( pUnit, SEROPEN_INFO, hListOpen);
			pUnit = pUnit->pNext;
			
			if( pInfoOpen->dwAccessCode & (GENERIC_READ | GENERIC_WRITE) )
			{
				pInfoOpen->fClose = FALSE;
				ser_NotifyCommEvent_Someone( pInfoOpen, EV_POWER );
			}
		}
	}
	
	if( pInfoOpen==pInfoDrv->pAccessOwner ) 
	{
		pInfoDrv->pAccessOwner = NULL;
	}
	
	//等待所有的退出
	i = 0;
	while( i<600 && ((pInfoOpen->wCntRead) || (pInfoOpen->wCntWrite) || (pInfoOpen->wCntIOCtl) || 
						  (pInfoOpen->wCntWaitEvent) || (pInfoOpen->wCntIntr)) )
	{
		i ++;
		Sleep( 1 );
	}
	
	//释放资源
	if ( pInfoOpen->hEvtComm )
	{
		CloseHandle(pInfoOpen->hEvtComm);
	}
	csDelete_CommEvt( pInfoOpen );
	//释放
	HANDLE_FREE( pInfoOpen );

	csLeave_hListOpen( pInfoDrv );
	return TRUE;
}

// ********************************************************************
// 声明：DWORD	COM_Read( DWORD pHead, LPVOID lpBuffer, DWORD dwnLenRead )
// 参数：
//	IN pHead-打开信息结构
//	IN lpBuffer-读数据的BUFFER
//	IN dwnLenRead-读数据的BUFFER的长度
// 返回值：
//	返回读取数据的长度
// 功能描述：读设备
// 引用: 
// ********************************************************************
DWORD	COM_Read( DWORD pHead, LPVOID lpBuffer, DWORD dwnLenRead )
{
    PSEROPEN_INFO   pInfoOpen = (PSEROPEN_INFO)pHead;
    PSERDRV_INFO	pInfoDrv;
    PVOID           pHeadDev;
    PVTBL_ARCH_SER	pFunTbl;

	LPCOMMTIMEOUTS	pTmout;
	DWORD           dwTicks;
    DWORD           dwTimeout;
    DWORD           dwIntervalTimeout;
    DWORD           dwTotalTimeout;
    DWORD           dwTimeSpent;
	
	LPBYTE			pBufRead;
	LONG			nLenRead;
    DWORD           nBytesRead;
    LONG			nLenRx_Read;
	
	PRXINFO			lpInfoRxBuf;
	register LONG*	pnRead;
	register LONG*	pnWrite;
//	register LONG*	pnCounts;
	register LONG*	pnLength;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoOpen) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return (DWORD)-1;
	}
	pInfoDrv = (PSERDRV_INFO)pInfoOpen->pInfoDrv;
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return (DWORD)-1;
	}
	//请打开设备才能操作
	if( !pInfoDrv->nOpenCnt )
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return (FALSE);
	}
	// 检查权限和参数
    if( !(pInfoOpen->dwAccessCode & GENERIC_READ) )
	{
        SetLastError (ERROR_INVALID_ACCESS);
        return (DWORD)-1;
    }
	if( lpBuffer==NULL )
	{
        SetLastError (ERROR_INVALID_PARAMETER);
        return (DWORD)-1;
    }
	if( dwnLenRead == 0 )
	{
        return 0;
    }
	if( pInfoOpen->fClose )	//for Braden
	{
		return 0;
	}
	pBufRead = (LPBYTE)lpBuffer;
	nLenRead = (LONG)dwnLenRead;

	pHeadDev = pInfoDrv->pHeadDev;
	pFunTbl  = pInfoDrv->pFunTbl;
	pTmout   = &pInfoDrv->CommTmouts;

	lpInfoRxBuf = &pInfoDrv->InfoRxBuf;
	pnRead		= &( lpInfoRxBuf->nRead );
	pnWrite		= &( lpInfoRxBuf->nWrite );
//	pnCounts    = &( lpInfoRxBuf->nCounts );
	pnLength	= &( lpInfoRxBuf->nLength );

	nBytesRead = 0;

	//进入读
	csEnter_Read( pInfoDrv );
    READ_CNT_INC(pInfoOpen);
	//准备
    pInfoDrv->fAbortRead = 0;
	ResetEvent(pInfoDrv->hEvtRead);
    //超时的计算
	dwTotalTimeout = pTmout->ReadTotalTimeoutConstant;
    if( pTmout->ReadTotalTimeoutMultiplier!=MAXDWORD ) 
        dwTotalTimeout += (pTmout->ReadTotalTimeoutMultiplier * nLenRead);
    dwIntervalTimeout  = pTmout->ReadIntervalTimeout;
	dwTimeSpent = 0;

	//读数据
	while( nLenRead )
	{
		//判断RxBuffer中是否有数据
		csEnter_Rx(pInfoDrv);
		
		nLenRx_Read = GET_RX_DATA_LENGTH( lpInfoRxBuf );	
	
		// lilin 2004-12-08 remove
		//csLeave_Rx(pInfoDrv);
		//
		if( nLenRx_Read > nLenRead )
			nLenRx_Read = nLenRead;

		//如果有数据
		if( nLenRx_Read )
		{
			//读取数据，从RxBuffer中
			// lilin 2004-12-08 remove
            //csEnter_Rx(pInfoDrv);
			//

			memcpy( pBufRead, lpInfoRxBuf->pRxBuf+*pnRead, nLenRx_Read );
//			(*pnCounts)-= nLenRx_Read;
			(*pnRead)  += nLenRx_Read;
			(*pnRead)  %= (*pnLength);
			//g_nCountRead_Debug += nLenRx_Read;	//for debug
			//
            csLeave_Rx(pInfoDrv);

			//更新信息
			pBufRead   += nLenRx_Read;
			nBytesRead += nLenRx_Read;
			nLenRead   -= nLenRx_Read;
		}
		else
		{
			// lilin 2004-12-08 add
            csLeave_Rx(pInfoDrv);
            
			//计算超时的 将要等待的时间dwTimeout
			if( dwTotalTimeout==0 )
			{
				if( dwIntervalTimeout==MAXDWORD )
				{
					//EdbgOutputDebugString( "exit: dwIntervalTimeout==MAXDWORD\r\n" );
					break;	//(I==max, M==0/max, C==0): 无论收到char与否，应立即返回
				}
				else		//(I==any, M==0/max, C==0): 一定要等到 Byte，之后在间隔时间内 超时
				{
					if ( !nBytesRead )
						dwTimeout = INFINITE;			//用所有时间 等 第一个字符
					else
					{
						if( dwTimeSpent>=dwIntervalTimeout )
						{
							//EdbgOutputDebugString( "exit: time out at dwTotalTimeout==0\r\n" );
							break;
						}
						dwTimeout = dwIntervalTimeout;	//用间隔时间 等 以后的字符
					}
				}
			}
			else
			{
                if ( dwTimeSpent >= dwTotalTimeout )//超时退出
				{
					//EdbgOutputDebugString( "exit: time out\r\n" );
					break;
				}

                dwTimeout = dwTotalTimeout - dwTimeSpent;			//默认 用所有时间 等 第一个字符
                if ( nBytesRead && (dwIntervalTimeout!=0) )	
				{
					if( dwTimeout > dwIntervalTimeout )
						dwTimeout = dwIntervalTimeout;			//用间隔时间 等 以后的字符
                }
			}
 
            dwTicks = GetTickCount();
			//等待数据事件：接收中断、PurgeComm、COM_Close
            if( WaitForSingleObject(pInfoDrv->hEvtRead, dwTimeout)==WAIT_TIMEOUT )
			{
				//EdbgOutputDebugString( "exit: time out at wait no data\r\n" );
                break;
            }
            dwTimeSpent += (GetTickCount() - dwTicks);

			//是否退出
            if( pInfoDrv->fAbortRead || pInfoOpen->fClose )
			{
				//EdbgOutputDebugString( "exit: time out at fAbortRead/fClose\r\n" );
                break;
            }
			//是否退出
            if( !pInfoDrv->nOpenCnt )
			{
				//EdbgOutputDebugString( "exit: time out at nOpenCnt\r\n" );
                SetLastError(ERROR_INVALID_HANDLE);
				nBytesRead = (DWORD)-1;
                break;
            }
        }

		//
		//解开流控制---控制“接收剩余空间”功能
		//
		if( pInfoDrv->fFlow_RxBuf && (pInfoDrv->fSentXoff || pInfoDrv->fSentDtrOff || pInfoDrv->fSentRtsOff) )
		{
			//RxBuf剩余空间
			csEnter_Rx(pInfoDrv);
			nLenRx_Read = *pnLength - GET_RX_DATA_LENGTH(lpInfoRxBuf);//*pnCounts;
			csLeave_Rx(pInfoDrv);

			//如果“接收剩余空间”足够大，软件通知对方继续发送
			if( pInfoDrv->fSentXoff && (nLenRx_Read >= pInfoDrv->dcb.XonLim) )
			{
				pInfoDrv->fSentXoff = 0;
				pFunTbl->lpFnXmitChar( pHeadDev, pInfoDrv->dcb.XonChar);
			}
			//如果“接收剩余空间”足够大，硬件通知对方继续发送
			if( nLenRx_Read >= pInfoDrv->dwHWOnLim )
			{
				//if( pInfoDrv->fSentDtrOff && (pInfoDrv->dcb.fDtrControl==DTR_CONTROL_HANDSHAKE) )
				if( pInfoDrv->fSentDtrOff )
				{
					pInfoDrv->fSentDtrOff = 0;
					pFunTbl->lpFnSetSingalDTR(pHeadDev);
				}
				//if( pInfoDrv->fSentRtsOff && (pInfoDrv->dcb.fRtsControl==RTS_CONTROL_HANDSHAKE) )
				if( pInfoDrv->fSentRtsOff )
				{
					pInfoDrv->fSentRtsOff = 0;
					pFunTbl->lpFnSetSingalRTS(pHeadDev);
				}
			}
		}
    }//for while loop

    READ_CNT_DEC(pInfoOpen);
	csLeave_Read( pInfoDrv );

	////EdbgOutputDebugString( "COM_Read: return " );
	return(nBytesRead);
}

// ********************************************************************
// 声明：DWORD	COM_Write(DWORD pHead, LPCVOID lpBuffer, DWORD  NumberOfBytes )
// 参数：
//	IN pHead-打开信息结构
//	IN lpBuffer-写数据的BUFFER
//	IN NumberOfBytes-写数据的BUFFER的长度
// 返回值：
//	返回写数据的长度
// 功能描述：写设备
// 引用: 
// ********************************************************************
DWORD	COM_Write(DWORD pHead, LPCVOID lpBuffer, DWORD  NumberOfBytes )
{
	PSEROPEN_INFO	pInfoOpen = (PSEROPEN_INFO)pHead;
	PSERDRV_INFO	pInfoDrv;
	DWORD			dwRet;

	LPBYTE			pBufWrite = (LPBYTE)lpBuffer;

	LPCOMMTIMEOUTS	pTmout;
	DWORD			dwTotalTimeout;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoOpen) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return (DWORD)-1;
	}
	pInfoDrv = (PSERDRV_INFO)pInfoOpen->pInfoDrv;
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return (DWORD)-1;
	}
	//请打开设备才能操作
	if( !pInfoDrv->nOpenCnt )
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return (FALSE);
	}
	// 检查权限和参数
	if ( !(pInfoOpen->dwAccessCode & GENERIC_WRITE) )
	{
		SetLastError (ERROR_INVALID_ACCESS);
		return (DWORD)-1;
	}
	if( pBufWrite==NULL )
	{
		SetLastError (ERROR_INVALID_PARAMETER);
		return (DWORD)-1;
	}
	if( NumberOfBytes==0 )
	{
		return 0;
	}
	if( pInfoOpen->fClose )	//for Braden
	{
		return 0;
	}

	//进入发送 
	csEnter_Write( pInfoDrv );
    WRITE_CNT_INC(pInfoOpen);
	//
    //pInfoDrv->fAbortWrite = 0;
	ResetEvent( pInfoDrv->hEvtWrite );
	//准备
	csEnter_Tx( pInfoDrv );
	pInfoDrv->dwTxBytes = 0;
	pInfoDrv->dwTxBytesPending = NumberOfBytes;
	pInfoDrv->pTxBuf = pBufWrite;
	pInfoDrv->dwWritePerm = GetCurrentPermissions();
//	RETAILMSG( 1, ( "pInfoDrv->dwWritePerm=0x%x.\r\n", pInfoDrv->dwWritePerm ) );
	csLeave_Tx( pInfoDrv );

	//开始发送
	ser_DoTxData( pInfoDrv );
	//计时
	pTmout = &pInfoDrv->CommTmouts;
	dwTotalTimeout = pTmout->WriteTotalTimeoutConstant;
	if( pTmout->WriteTotalTimeoutMultiplier!=MAXDWORD )
		dwTotalTimeout += (pTmout->WriteTotalTimeoutMultiplier * NumberOfBytes);
	if ( !dwTotalTimeout )
		dwTotalTimeout = INFINITE;
	//等待
	WaitForSingleObject(pInfoDrv->hEvtWrite, dwTotalTimeout);
	if ( !pInfoDrv->nOpenCnt )
	{
		SetLastError(ERROR_INVALID_HANDLE);
		dwRet = (DWORD)-1;
		dwRet = pInfoDrv->dwTxBytes;
	}
	else
	{
		dwRet = pInfoDrv->dwTxBytes;
	}

	//结束 退出
	csEnter_Tx( pInfoDrv );
	pInfoDrv->pTxBuf	= NULL;
	pInfoDrv->dwTxBytes			= 0;
	pInfoDrv->dwTxBytesPending	= 0;
	pInfoDrv->dwWritePerm = 0;
	csLeave_Tx( pInfoDrv );

	//通信事件EV_TXEMPTY
	//if( pInfoDrv->fAbortWrite==0 )
	if( dwRet==NumberOfBytes )
		ser_NotifyCommEvent(pInfoDrv, EV_TXEMPTY);	//attention here
	//硬件设置
	if( pInfoDrv->dcb.fRtsControl == RTS_CONTROL_TOGGLE ) 
		pInfoDrv->pFunTbl->lpFnClearSingalRTS(pInfoDrv->pHeadDev);

	WRITE_CNT_DEC(pInfoOpen);
	csLeave_Write( pInfoDrv );

	////EdbgOutputDebugString( "===COM_Write_last: NumTotal=%d, BytesTx= %d , BytesTxPending= %d ", NumberOfBytes, pInfoDrv->TxBytes, pInfoDrv->dwTxBytesPending );
	return ( dwRet );
}

// ********************************************************************
// 声明：DWORD	COM_Seek( DWORD  pHead, LONG    Position, DWORD   Type )
// 参数：
//	IN pHead-打开信息结构
//	IN Position-不支持
//	IN Type-不支持
// 返回值：
//	返回-1
// 功能描述：SEEK设备
// 引用: 
// ********************************************************************
DWORD	COM_Seek( DWORD  pHead, LONG    Position, DWORD   Type )
{    return (DWORD)-1;	}


// ********************************************************************
// 声明：VOID	COM_PowerUp( DWORD pHead )
// 参数：
//	IN pHead-驱动信息结构
// 返回值：
//	无
// 功能描述：电源UP
// 引用: 
// ********************************************************************
//Powerup入口
VOID	COM_PowerUp( DWORD pHead )
{
	PSERDRV_INFO  pInfoDrv    = (PSERDRV_INFO)pHead;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        return ;
	}
	////EdbgOutputDebugString("===COM_PowerUp");
	pInfoDrv->pFunTbl->lpFnPowerOn( pInfoDrv->pHeadDev );
}
// ********************************************************************
// 声明：VOID	COM_PowerDown( DWORD   pHead )
// 参数：
//	IN pHead-驱动信息结构
// 返回值：
//	无
// 功能描述：电源DOWN
// 引用: 
// ********************************************************************
VOID	COM_PowerDown( DWORD   pHead )
{
	PSERDRV_INFO     pInfoDrv = (PSERDRV_INFO)pHead;

	////EdbgOutputDebugString("===COM_PowerDown");
	//句柄安全检查
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        return ;
	}
	pInfoDrv->pFunTbl->lpFnPowerOff( pInfoDrv->pHeadDev );
}

//----------------------------------------------------
typedef	struct	_PARAM_CLEARCOMMERR
{
	DWORD	dwErrors;
	COMSTAT	comStat;

}PARAM_CLEARCOMMERR;

// ********************************************************************
// 声明：BOOL	COM_IOControl(DWORD pHead, DWORD dwCode, LPVOID pBufIn_, DWORD dwLenIn, LPVOID pBufOut_, DWORD dwLenOut, LPDWORD pdwActualOut)
// 参数：
//	IN pHead-打开信息结构
//	IN dwCode-IO控制码
//	IN pBufIn_-输入BUFFER
//	IN dwLenIn-输入BUFFER的长度
//	OUT pBufOut_-输出BUFFER
//	IN dwLenOut-输出BUFFER的长度
//	OUT pdwActualOut-输出BUFFER实际获取数据的长度
// 返回值：
//	成功返回TRUE
// 功能描述：IOControl设备
// 引用: 
// ********************************************************************
BOOL	COM_IOControl(DWORD pHead, DWORD dwCode, LPVOID pBufIn_, DWORD dwLenIn, LPVOID pBufOut_, DWORD dwLenOut, LPDWORD pdwActualOut)
{
	PSEROPEN_INFO	pInfoOpen= (PSEROPEN_INFO)pHead;
	PSERDRV_INFO    pInfoDrv;
	PVTBL_ARCH_SER	pFunTbl;
	PVOID           pHeadDev;
	DWORD           dwFlags;
	PBYTE			pBufIn = (PBYTE)pBufIn_;
	PBYTE			pBufOut = (PBYTE)pBufOut_;

	DWORD			dwErr;
	DWORD			dwActualOut;

	//句柄安全检查
	if( !HANDLE_CHECK(pInfoOpen) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return (ULONG)-1;
	}
	pInfoDrv = (PSERDRV_INFO)pInfoOpen->pInfoDrv;
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return (ULONG)-1;
	}
	//请打开设备才能操作
	if( !pInfoDrv->nOpenCnt )
	{
		SetLastError (ERROR_INVALID_HANDLE);
		return (FALSE);
	}
	//
	if( pInfoOpen->fClose )	//for Braden
	{
		//added by xyg_2004_11_09-begin
		if( dwCode==IOCTL_SERIAL_WAIT_ON_MASK )
		{
			if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) )
			{
				dwErr = ERROR_INVALID_PARAMETER;
			}
			else
			{
				*( (DWORD *)pBufOut )= EV_POWER;
				if( pdwActualOut )
				{
					*pdwActualOut = sizeof(DWORD);
				}
				return TRUE;
			}
		}
		//added by xyg_2004_11_09-end
		return FALSE;
	}
	//检查访问权限---除了下面部分的code，其他的都需要“读写权限”
	if ( !( (dwCode == IOCTL_SERIAL_GET_WAIT_MASK) ||
			(dwCode == IOCTL_SERIAL_SET_WAIT_MASK) ||
			(dwCode == IOCTL_SERIAL_WAIT_ON_MASK) ||
			(dwCode == IOCTL_SERIAL_GET_MODEMSTATUS) ||
			(dwCode == IOCTL_SERIAL_GET_PROPERTIES) ||
			(dwCode == IOCTL_SERIAL_GET_TIMEOUTS) 
		  ) )
	{
		if( !(pInfoOpen->dwAccessCode & (GENERIC_READ | GENERIC_WRITE) ) ) 
		{
			SetLastError (ERROR_INVALID_ACCESS);
			return (FALSE);
		}
		
	}

	IOCTL_CNT_INC( pInfoOpen );
	//
	pHeadDev = pInfoDrv->pHeadDev;
	pFunTbl = pInfoDrv->pFunTbl;

	//RETAILMSG( 1, (TEXT("\r\n COM_IOCtl: code=%d, begin\r\n"), dwCode) );
	dwErr = ERROR_SUCCESS;
	switch ( dwCode )
	{
	//通信状态：Get
	case IOCTL_SERIAL_GET_COMMSTATUS :
		if ( (dwLenOut < sizeof(PARAM_CLEARCOMMERR)) || (NULL == pBufOut) ) 
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			PARAM_CLEARCOMMERR*		lpParamGet;
			LPDWORD					lpErrors;
			LPCOMSTAT				lpStat;

			//获取参数和初始化
			lpParamGet = (PARAM_CLEARCOMMERR*)pBufOut;
			lpErrors = &lpParamGet->dwErrors;
			lpStat   = &lpParamGet->comStat;
			memset( lpStat, 0, sizeof(COMSTAT) );

			//获取 ARCH信息
			*lpErrors = pFunTbl->lpFnGetComStat( pHeadDev, lpStat );

			//获取 本层信息
			lpStat->fXoffSent = pInfoDrv->fSentXoff;			//是否发送了Xoff
			lpStat->fXoffHold = pInfoDrv->fStopXmit;			//发送因为收到Xoff而停止

			csEnter_Rx(pInfoDrv);
			lpStat->cbInQue   = GET_RX_DATA_LENGTH(&pInfoDrv->InfoRxBuf);// pInfoDrv->InfoRxBuf.nCounts;
			csLeave_Rx(pInfoDrv);

			csEnter_Tx( pInfoDrv );
			lpStat->cbOutQue  = pInfoDrv->dwTxBytesPending;
			csLeave_Tx( pInfoDrv );
		}
		break;
	//Modem状态：Get  (Dsr/Cts/Ring/Rlsd)
	case IOCTL_SERIAL_GET_MODEMSTATUS :
		if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			//*(DWORD *)pBufOut = 0;
			pFunTbl->lpFnGetCommModemStatus(pHeadDev, (LPDWORD)pBufOut);
			dwActualOut = sizeof(DWORD);
		}
		break;

	//发送字符
	case IOCTL_SERIAL_IMMEDIATE_CHAR :
		if( (dwLenIn < sizeof(BYTE)) || (NULL==pBufIn) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			pFunTbl->lpFnXmitChar( pHeadDev, *pBufIn );
		}
		break;

	//软件握手：
	//	设置标志致使停止/继续发送，相当于发送时，接收到Xoff/Xon字符
	case IOCTL_SERIAL_SET_XOFF :
		if( pInfoDrv->dcb.fOutX )
			pInfoDrv->fStopXmit = 1;
		break;
	case IOCTL_SERIAL_SET_XON :
		if( pInfoDrv->dcb.fOutX )
			pInfoDrv->fStopXmit = 0;
		break;

	//硬件信号设置---break
	case IOCTL_SERIAL_SET_BREAK_ON :
		pFunTbl->lpFnSetBreak(pHeadDev);
		break;
	case IOCTL_SERIAL_SET_BREAK_OFF :
		pFunTbl->lpFnClearBreak(pHeadDev);
		break;

	//硬件信号设置---DTR
	case IOCTL_SERIAL_SET_DTR :
	case IOCTL_SERIAL_CLR_DTR :
		if ( pInfoDrv->dcb.fDtrControl == DTR_CONTROL_HANDSHAKE )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			if( dwCode==IOCTL_SERIAL_SET_DTR )
			{
				pFunTbl->lpFnSetSingalDTR(pHeadDev);
			}
			else
			{
				pFunTbl->lpFnClearSingalDTR(pHeadDev);
			}
		}
		break;

	//硬件信号设置---RTS
	case IOCTL_SERIAL_SET_RTS :
	case IOCTL_SERIAL_CLR_RTS :
		if( pInfoDrv->dcb.fRtsControl==RTS_CONTROL_HANDSHAKE )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			if( dwCode==IOCTL_SERIAL_SET_RTS )
			{
				pFunTbl->lpFnSetSingalRTS(pHeadDev);
			}
			else
			{
				pFunTbl->lpFnClearSingalRTS(pHeadDev);
			}
		}
		break;

	//通信事件操作：Get
	case IOCTL_SERIAL_GET_WAIT_MASK :
		if( (dwLenOut < sizeof(DWORD)) || (NULL==pBufOut) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			*(DWORD *)pBufOut = pInfoOpen->dwEvtMask;
			dwActualOut = sizeof(DWORD);
		}
		break;
	//通信事件操作：Set
	case IOCTL_SERIAL_SET_WAIT_MASK :
		if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			DWORD			dwEvtWait;

			//通知所有 waitcommevent函数 退出
			dwEvtWait = *(DWORD *)pBufIn;
			pInfoOpen->fAbortEvt = 1;
			SetEvent(pInfoOpen->hEvtComm);

			//设置 pInfoOpen的事件掩码
			csEnter_CommEvt( pInfoOpen );
			pInfoOpen->dwEvtMask = dwEvtWait;
			csLeave_CommEvt( pInfoOpen );
			
			//更新 pInfoDrv上所有Open者的掩码
			if( pInfoDrv->nOpenCnt > 1 )
			{
				PSEROPEN_INFO   pTmpInfoOpen;
				DWORD			dwFlags;
				PLIST_UNIT		pUnit;
				PLIST_UNIT		pUnitHeader;

				dwFlags = dwEvtWait;
				//csEnter_hListOpen( pInfoDrv );

				pUnitHeader = &pInfoDrv->hListOpen;
				pUnit = pUnitHeader->pNext;
				while( pUnit!=pUnitHeader )
				{
					pTmpInfoOpen = LIST_CONTAINER( pUnit, SEROPEN_INFO, hListOpen );
					pUnit = pUnit->pNext;    // advance to next 

					dwFlags |= pTmpInfoOpen->dwEvtMask;
				}
				pInfoDrv->dwEvtMask = dwFlags;//所有打开的

				//csLeave_hListOpen( pInfoDrv );
			}
			else
			{
				pInfoDrv->dwEvtMask = dwEvtWait;//所有打开的
			}
		}
		break;
	//通信事件操作：Wait
	case IOCTL_SERIAL_WAIT_ON_MASK :
		if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			dwErr = ser_WaitCommEvent(pInfoOpen, (DWORD *)pBufOut, NULL);
			dwActualOut = sizeof(DWORD);
		}
		break;


	//串口驱动程序的属性：
	case IOCTL_SERIAL_GET_PROPERTIES :
		if ( (dwLenOut < sizeof(COMMPROP)) || (NULL == pBufOut) ) 
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			LPCOMMPROP	lpCmmProp;

			lpCmmProp = (LPCOMMPROP)pBufOut;
			pFunTbl->lpFnGetCommProperties(pHeadDev, lpCmmProp);  
			lpCmmProp->dwMaxTxQueue      = 0;
			lpCmmProp->dwMaxRxQueue      = RX_BUFFER_SIZE_MAX;
			lpCmmProp->dwCurrentTxQueue  = 0;
			lpCmmProp->dwCurrentRxQueue  = pInfoDrv->InfoRxBuf.nLength;

			dwActualOut = sizeof(COMMPROP);
		}
		break;

	//读写超时的设置
	case IOCTL_SERIAL_SET_TIMEOUTS :
		////EdbgOutputDebugString (" IOCTL_SERIAL_SET_TIMEOUTS");
		if ( (dwLenIn < sizeof(COMMTIMEOUTS)) || (NULL == pBufIn) ) 
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			memcpy( &(pInfoDrv->CommTmouts), (COMMTIMEOUTS *)pBufIn, dwLenIn );
		}
		break;
	//读写超时的获取
	case IOCTL_SERIAL_GET_TIMEOUTS :
		if ( (dwLenOut < sizeof(COMMTIMEOUTS)) || (NULL == pBufOut) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			memcpy((LPCOMMTIMEOUTS)pBufOut, &(pInfoDrv->CommTmouts), sizeof(COMMTIMEOUTS));
			dwActualOut = sizeof(COMMTIMEOUTS);
		}
		break;

	//清除操作：
	case IOCTL_SERIAL_PURGE :
		if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			dwFlags = *((PDWORD) pBufIn);

			//硬件 purge
			pFunTbl->lpFnPurgeComm(pHeadDev,dwFlags);
			//软件接收 purge
			if ( dwFlags & PURGE_RXCLEAR ) 
			{
				//接收缓冲 purge
				csEnter_Rx(pInfoDrv);
				//g_nCountRx_Debug = 0;	//for debug
				//g_nCountRead_Debug = 0;	//for debug
				// lilin 2004-12-08 
				//	pInfoDrv->InfoRxBuf.nWrite	= 0;
				//	pInfoDrv->InfoRxBuf.nRead	= 0;
				//	pInfoDrv->InfoRxBuf.nCounts = 0;
				pInfoDrv->InfoRxBuf.nRead = pInfoDrv->InfoRxBuf.nWrite;
				//
				//memset( pInfoDrv->InfoRxBuf.pRxBuf, 0, pInfoDrv->InfoRxBuf.nLength );
				csLeave_Rx(pInfoDrv);
				
				//清除流控制
				if( pInfoDrv->fFlow_RxBuf )
				{
					//if ( pInfoDrv->dcb.fInX && pInfoDrv->fSentXoff ) 
					if( pInfoDrv->fSentXoff ) 
					{
						//发送Xon，告诉对方继续发送
						pInfoDrv->fSentXoff = 0;
						pFunTbl->lpFnXmitChar( pHeadDev, pInfoDrv->dcb.XonChar );   
					}
					//if ( pInfoDrv->fSentDtrOff && (pInfoDrv->dcb.fDtrControl==DTR_CONTROL_HANDSHAKE) ) 
					if ( pInfoDrv->fSentDtrOff ) 
					{
						//设置Dtr，告诉对方继续发送
						pInfoDrv->fSentDtrOff = 0;
						pFunTbl->lpFnSetSingalDTR( pHeadDev );
					}
					//if ( pInfoDrv->fSentRtsOff && (pInfoDrv->dcb.fRtsControl==RTS_CONTROL_HANDSHAKE) )
					if ( pInfoDrv->fSentRtsOff )
					{
						//设置Rts，告诉对方继续发送
						pInfoDrv->fSentRtsOff = 0;
						pFunTbl->lpFnSetSingalRTS( pHeadDev );
					}
				}
			}
			//软件发送 purge
			if ( dwFlags & (PURGE_TXCLEAR) )
			{
				//发送信息 purge
				csEnter_Tx( pInfoDrv );
				pInfoDrv->pTxBuf	= NULL;
				//pInfoDrv->dwTxBytes		= 0;
				pInfoDrv->dwTxBytesPending	= 0;
				csLeave_Tx( pInfoDrv );

				//清除流控制
				pInfoDrv->fStopXmit = 0;
			}

			//终止读写操作
			if ( dwFlags & PURGE_RXABORT )
			{
				pInfoDrv->fAbortRead = 1;
				SetEvent(pInfoDrv->hEvtRead);
			}
			if ( dwFlags & PURGE_TXABORT )
			{
				//pInfoDrv->fAbortWrite = 1;
				SetEvent(pInfoDrv->hEvtWrite);
			}
		}
		break;
/*  2004-12-06, 没有必要， 看修改说明
	//收发缓冲buffer大小
	case IOCTL_SERIAL_SET_QUEUE_SIZE : 
		if ( ( dwLenIn < 2*sizeof(DWORD) ) || (NULL == pBufIn) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			PRXINFO	lpInfoRxBuf;
			LONG	dwLenAlloc;
			LPBYTE	pAlloc;
			
			//判断分配信息
			dwLenAlloc = *(LONG*)pBufIn;
			lpInfoRxBuf= &pInfoDrv->InfoRxBuf;
			if( (dwLenAlloc>=RX_BUFFER_SIZE_MIN_x) && (dwLenAlloc!=lpInfoRxBuf->nLength) )
			{
				csEnter_RxResize( &pInfoDrv->InfoRxBuf );
				csEnter_Rx(pInfoDrv);

				//开始分配
				if( lpInfoRxBuf->pRxBuf )
				{
					pAlloc = realloc( lpInfoRxBuf->pRxBuf, dwLenAlloc );
				}
				else
				{
					pAlloc = malloc( dwLenAlloc );
				}
				//重新设置 收缓冲buffer信息
				if( !pAlloc )
				{
					dwErr = ERROR_NOT_ENOUGH_MEMORY;
				}
				else
				{
					lpInfoRxBuf->pRxBuf	 = pAlloc;
					lpInfoRxBuf->nLength = dwLenAlloc;
					lpInfoRxBuf->nWrite	 = 0;
					lpInfoRxBuf->nRead	 = 0;
					lpInfoRxBuf->nCounts = 0;

					//设置硬件握手的极限值
					ser_CalHWLim( pInfoDrv );
				}
				csLeave_Rx(pInfoDrv);
				csLeave_RxResize( &pInfoDrv->InfoRxBuf );
			}
		}
		break;
*/
	//获取DCB
	case IOCTL_SERIAL_GET_DCB :
		if( (dwLenOut < sizeof(DCB)) || (NULL == pBufOut) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			memcpy((char *)pBufOut, (char *)&(pInfoDrv->dcb), sizeof(DCB));
			dwActualOut = sizeof(DCB);
		}
		break;
	//设置DCB
	case IOCTL_SERIAL_SET_DCB :
		if ( (dwLenIn < sizeof(DCB)) || (NULL == pBufIn) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			DCB*	lpDCBSet;
			DCB*	lpDCBDes;
			DWORD	dwFlagSetDCB;

			lpDCBSet = (DCB *)pBufIn;
			lpDCBDes = &pInfoDrv->dcb;
			dwFlagSetDCB = ser_GetFlagSetDCB( lpDCBDes, lpDCBSet );
			if( pFunTbl->lpFnSetDCB( pInfoDrv->pHeadDev, lpDCBSet, dwFlagSetDCB ) )
			{
				memcpy( lpDCBDes, lpDCBSet, sizeof(DCB) );
				ser_GetFlow( pInfoDrv );
				//设置硬件握手的极限值
				ser_CalHWLim( pInfoDrv );
			}
			else
			{
				dwErr = ERROR_INVALID_PARAMETER;
			}
		}
		break;

	//红外功能
	case IOCTL_SERIAL_ENABLE_IR :
		if( !pFunTbl->lpFnIREnable(pHeadDev, pInfoDrv->dcb.BaudRate) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		break;
	case IOCTL_SERIAL_DISABLE_IR :
		pFunTbl->lpFnIRDisable(pHeadDev);
		break;

	//ARCH 默认处理
	default :
        if( (pFunTbl->lpFnIOControl == NULL) ||
			 (pFunTbl->lpFnIOControl(pHeadDev,dwCode,pBufIn,dwLenIn,pBufOut,dwLenOut,pdwActualOut) == FALSE) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		break;
    }

	IOCTL_CNT_DEC( pInfoOpen );

	//RETAILMSG( 1, (TEXT("\r\nCOM_IOCtl: return=%d\r\n"), dwErr) );
	if( dwErr==ERROR_SUCCESS )
	{
		if( pdwActualOut )
		{
			*pdwActualOut = dwActualOut;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


// ********************************************************************
// 声明：VOID	ser_DoTxData( PSERDRV_INFO pInfoDrv )
// 参数：
//	IN pInfoDrv-驱动信息结构
// 返回值：
//	无 
// 功能描述：发送数据，由中断和COM_Write调用
// 引用: 
// ********************************************************************
VOID	ser_DoTxData( PSERDRV_INFO pInfoDrv )
{
    PVTBL_ARCH_SER	pFunTbl = pInfoDrv->pFunTbl;
    PVOID			pHeadDev = pInfoDrv->pHeadDev;
	DWORD			BytesTx;

	//发送的 完成/终止
	//if ( pInfoDrv->dwTxBytesPending==0 || pInfoDrv->fAbortWrite )
	if ( pInfoDrv->dwTxBytesPending==0 )
	{
		//关闭硬件的发送
		pFunTbl->lpFnIntrHandleTx(pHeadDev);
		//通知COM_Write
		SetEvent(pInfoDrv->hEvtWrite);
	}
	else
	{
		if ( pInfoDrv->dcb.fRtsControl == RTS_CONTROL_TOGGLE ) 
			pFunTbl->lpFnSetSingalRTS(pHeadDev);

		//软件握手---判断是否可以发送数据(由对方的Xoff/Xon来控制)
		if(	!pInfoDrv->fStopXmit || pInfoDrv->dcb.fTXContinueOnXoff )
		{
			DWORD            dwOldPerm;
			
//			RETAILMSG( 1,  ("dwWritePerm=0x%x.\r\n", pInfoDrv->dwWritePerm ) );
			
			//INTERRUPTS_OFF();
			//INTR_Disable( pInfoDrv->pObjDev->dwIntID );
			csEnter_Tx( pInfoDrv );


			BytesTx  = pInfoDrv->dwTxBytesPending;
			
			dwOldPerm = SetProcPermissions( pInfoDrv->dwWritePerm );
			pFunTbl->lpFnSend( pHeadDev, pInfoDrv->pTxBuf+pInfoDrv->dwTxBytes, &BytesTx );	
			SetProcPermissions( dwOldPerm );
			
			pInfoDrv->dwTxBytes += BytesTx;
			pInfoDrv->dwTxBytesPending -= BytesTx;

			csLeave_Tx( pInfoDrv );
			//INTERRUPTS_ON();
		}
	}

}

// ********************************************************************
// 声明：void	ser_NotifyCommEvent(PVOID pHead, DWORD fdwEventMask)
// 参数：
//	IN pHead-驱动信息结构
//	IN fdwEventMask-事件掩码
// 返回值：
//	无 
// 功能描述：通知事件
// 引用: 
// ********************************************************************
//如果在PDD或MDD中，发生事件fdwEventMask，则将其保存到每个打开的句柄中，并通知WaitCommEvent
//
void	ser_NotifyCommEvent(PVOID pHead, DWORD fdwEventMask)
{
    PSERDRV_INFO	pInfoDrv = (PSERDRV_INFO)pHead;
    PSEROPEN_INFO	pInfoOpen;
    PLIST_UNIT		pUnit;
    PLIST_UNIT		pUnitHeader;
	
    if ( !pInfoDrv->nOpenCnt ) 
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return;
    }
	if ( pInfoDrv->dwEvtMask & fdwEventMask )//for all opens
	{
		//通知到 每个Open中的 WaitCommEvent
		pUnitHeader = &pInfoDrv->hListOpen;
        pUnit = pUnitHeader->pNext;
		while( pUnit!=pUnitHeader )
		{
			pInfoOpen = LIST_CONTAINER( pUnit, SEROPEN_INFO, hListOpen);
			pUnit = pUnit->pNext;

			if( !pInfoOpen->fClose )//for braden.
			{
				csEnter_CommEvt( pInfoOpen );
				if ( pInfoOpen->dwEvtMask & fdwEventMask )//通知
				{
					pInfoOpen->dwEvtData |= (fdwEventMask & pInfoOpen->dwEvtMask);
					//RETAILMSG( 1, (TEXT("Notify: [0x%X]\r\n"), (fdwEventMask & pInfoOpen->dwEvtMask)) );
					SetEvent(pInfoOpen->hEvtComm);
				}
				csLeave_CommEvt( pInfoOpen );
			}
		}//while
    }//if
}

// ********************************************************************
// 声明：VOID	ser_NotifyCommEvent_Someone( PSEROPEN_INFO pInfoOpen, DWORD fdwEventMask )
// 参数：
//	IN pInfoOpen-打开信息结构
//	IN fdwEventMask-事件掩码
// 返回值：
//	无 
// 功能描述：通知事件（for braden）
// 引用: 
// ********************************************************************
//专门用于COM_Close时，通知其他的打开者。for braden
VOID	ser_NotifyCommEvent_Someone( PSEROPEN_INFO pInfoOpen, DWORD fdwEventMask )	//for braden
{
	csEnter_CommEvt( pInfoOpen );
	if( pInfoOpen->dwEvtMask & fdwEventMask )//通知
	{
		pInfoOpen->dwEvtData |= (fdwEventMask & pInfoOpen->dwEvtMask);
		SetEvent(pInfoOpen->hEvtComm);
	}
	csLeave_CommEvt( pInfoOpen );
}

// ********************************************************************
// 声明：DWORD	ser_WaitCommEvent(PSEROPEN_INFO pInfoOpen, LPDWORD pfdwEventMask, LPOVERLAPPED Unused)
// 参数：
//	IN pInfoOpen-打开信息结构
//	IN fdwEventMask-事件掩码
//	IN Unused-无用的参数
// 返回值：
//	 返回错误数值
// 功能描述：等待事件
// 引用: 
// ********************************************************************
//调用：WaitCommEvent-->COM_DeviceIOControl-->ser_WaitCommEvent
//如果在PDD或MDD中发生事件，由ser_NotifyCommEvent通知，则保存事件并返回
DWORD	ser_WaitCommEvent(PSEROPEN_INFO pInfoOpen, LPDWORD pfdwEventMask, LPOVERLAPPED Unused)
{
    PSERDRV_INFO	pInfoDrv = (PSERDRV_INFO)pInfoOpen->pInfoDrv;
    DWORD			dwEventData;

    ////EdbgOutputDebugString("ser_WaitCommEvent is called ");
	*pfdwEventMask = 0;
    if ( !pInfoDrv || !pInfoDrv->nOpenCnt )
	{
        return ERROR_INVALID_HANDLE;
    }
    //不需要等待事件，立即返回
    if ( !pInfoOpen->dwEvtMask )
	{
		return ERROR_INVALID_PARAMETER;
    }
	//
    WAITEVENT_CNT_INC(pInfoOpen);
	//复位退出标志
    pInfoOpen->fAbortEvt = 0;

    while( pInfoDrv->nOpenCnt ) 
	{
		csEnter_CommEvt( pInfoOpen );
		//取出事件
        dwEventData = pInfoOpen->dwEvtData;
		pInfoOpen->dwEvtData = 0;
		////RETAILMSG( 1, (TEXT("drv_WaitCommEvent: [0x%X]\r\n"), dwEventData) );

		//若要等EV_RXCHAR，则---有数据，就有事件
		if( pInfoOpen->dwEvtMask & EV_RXCHAR )
		{
//			if( pInfoDrv->InfoRxBuf.nCounts==0 )
			if( GET_RX_DATA_LENGTH(&pInfoDrv->InfoRxBuf) == 0 )
				dwEventData &= ~EV_RXCHAR;	//如果没有数据，就没有事件
			else
				dwEventData |= EV_RXCHAR;	//如果有数据，就有事件
		}

		//等到了要等的事件，则退出告诉AP
        if( dwEventData & pInfoOpen->dwEvtMask ) 
		{
            *pfdwEventMask = (dwEventData & pInfoOpen->dwEvtMask);
			////RETAILMSG( 1, (TEXT("drv_WaitCommEvent: exit[%X], get=[%X] 等到了要等的事件\r\n"), dwEventData, *pfdwEventMask) );
			csLeave_CommEvt( pInfoOpen );
            break;
		}
		else
		{
			csLeave_CommEvt( pInfoOpen );
		}

        //等待串口的事件
        WaitForSingleObject(pInfoOpen->hEvtComm, (DWORD)-1);

        //当调COM_Close(、SetCommMask)时
        if( pInfoOpen->fAbortEvt )//
		{
            *pfdwEventMask = 0;
            break;
        }
    }

    WAITEVENT_CNT_DEC(pInfoOpen);
	if ( !pInfoDrv->nOpenCnt ) 
	{
        *pfdwEventMask = 0;
        return ERROR_INVALID_HANDLE;
    }
	else 
	{
        return ERROR_SUCCESS;
    }
}

// ********************************************************************
// 声明：void	ser_CalHWLim( PSERDRV_INFO pInfoDrv )
// 参数：
//	IN pInfoDrv-驱动信息结构
// 返回值：
//	无
// 功能描述：握手提示Buffer极限的大小与接收Buffer、Baudrate相关
// 引用: 
// ********************************************************************
void	ser_CalHWLim( PSERDRV_INFO pInfoDrv )
{
	pInfoDrv->dwHWOffLim	= HANDSHAKEOFF_CONST + pInfoDrv->dcb.BaudRate / CBR_9600 + pInfoDrv->InfoRxBuf.nLength / 1024;
	if( pInfoDrv->dwHWOffLim > HANDSHAKEOFF_MAX )
		pInfoDrv->dwHWOffLim = HANDSHAKEOFF_MAX;
	pInfoDrv->dwHWOnLim	= pInfoDrv->dwHWOffLim + 128;//(3 * pInfoDrv->dwHWOffLim)/2;
	RETAILMSG( 1, (TEXT("COM_handshake: Off_Lim=%d, On_Lim=%d\r\n"), pInfoDrv->dwHWOffLim, pInfoDrv->dwHWOnLim) );
}

// ********************************************************************
// 声明：void	ser_CalHWLim( PSERDRV_INFO pInfoDrv )
// 参数：
//	IN lpDCBDes-源DCB
//	IN lpDCBSet-目的DCB
// 返回值：
//	返回设置的标志
// 功能描述：判断是否对硬件有新设置
// 引用: 
// ********************************************************************
DWORD	ser_GetFlagSetDCB( DCB* lpDCBDes, DCB* lpDCBSet )
{
	DWORD	dwFlagSetDCB;

	dwFlagSetDCB = SETDCB_NONE;
	if( lpDCBDes->BaudRate!=lpDCBSet->BaudRate )
		dwFlagSetDCB |= SETDCB_BAUD;
	if( lpDCBDes->ByteSize!=lpDCBSet->ByteSize )
		dwFlagSetDCB |= SETDCB_BYTESIZE;
	if( lpDCBDes->Parity!=lpDCBSet->Parity ||lpDCBDes->fParity!=lpDCBSet->fParity )
		dwFlagSetDCB |= SETDCB_PARITY;
	if( lpDCBDes->StopBits!=lpDCBSet->StopBits )
		dwFlagSetDCB |= SETDCB_STOPBITS;
	
	//RETAILMSG( 1, (TEXT("Flag_SetDCB: [%x]\r\n"), dwFlagSetDCB) );
	return dwFlagSetDCB;
}

// ********************************************************************
// 声明：void	ser_GetFlow( PSERDRV_INFO pInfoDrv )
// 参数：
//	IN pInfoDrv-驱动信息结构
// 返回值：
//	无
// 功能描述：如果接收缓冲Buffer不够，是否采用流控制提示对方，告诉对方不要继续发送数据
// 引用: 
// ********************************************************************
void	ser_GetFlow( PSERDRV_INFO pInfoDrv )
{
	DCB*	lpDCB;
	lpDCB = &pInfoDrv->dcb;
	if( lpDCB->fInX || (lpDCB->fDtrControl==DTR_CONTROL_HANDSHAKE) ||(lpDCB->fRtsControl==RTS_CONTROL_HANDSHAKE) )
		pInfoDrv->fFlow_RxBuf = 1;
	else
		pInfoDrv->fFlow_RxBuf = 0;
}




extern OBJECT_ARCH SerObj;		//xyg: serial port
extern OBJECT_ARCH SerCardObj;	//
extern OBJECT_ARCH udcObj;		//

// ********************************************************************
// 声明：LPVOID	GetSerialObject(DWORD dwIDObj)
// 参数：
//	IN dwIDObj-PDD对象的标志
// 返回值：
//	成功返回TRUE
// 功能描述：检索PDD对象
// 引用: 
// ********************************************************************
LPVOID	GetSerialObject(DWORD dwIDObj)
{
	//
	if( (dwIDObj==ID_COM1)||(dwIDObj==ID_COM2) )
	{
		return &SerObj;
	}
	//
#if 0	
	if( dwIDObj>=ID_COM_MIN_CARD )
	{
		return &SerCardObj;
	}
#endif	
	//

//#ifdef	HW_PCB_VERSION_C
//	if( dwIDObj==ID_COM5 )
//	{
		//RETAILMSG( 1, (TEXT("\r\n\r\nxyg: obj=%x\r\n\r\n\r\n"), &udcObj) );
//		return &udcObj;
//	}
//#endif

	//
	return NULL;
}


//----------------------------------------------------
#ifdef	DEBUG_SERDRV
//调试函数
void	dcb_Debug( LPDCB lpDCB, LPTSTR pszText )
{
	//RETAILMSG( 1, (TEXT("\r\n\r\n    ---  [%s] dcb info--- begin \r\n"), pszText ) );

	//RETAILMSG( 1, (TEXT("BaudRate=%d, ByteSize=%d, Parity=%d, StopBits=%d\r\n"),      lpDCB->BaudRate, lpDCB->ByteSize, lpDCB->Parity, lpDCB->StopBits) );
	//RETAILMSG( 1, (TEXT("send flow: fOutxCtsFlow=%x, fOutxDsrFlow=%x, fOutX=%x\r\n"), lpDCB->fOutxCtsFlow, lpDCB->fOutxDsrFlow, lpDCB->fOutX) );
	//RETAILMSG( 1, (TEXT("recv flow: fRtsControl=%x, fDtrControl=%x, fInX=%x\r\n"),    lpDCB->fRtsControl, lpDCB->fDtrControl, lpDCB->fInX) );
	//RETAILMSG( 1, (TEXT("other: fDsrSensitivity=%x, fTXContinueOnXoff=%x, fParity=%x, fErrorChar=%x, fNull=%x, fAbortOnError=%x\r\n"), lpDCB->fDsrSensitivity, lpDCB->fTXContinueOnXoff, lpDCB->fParity, lpDCB->fErrorChar, lpDCB->fNull, lpDCB->fAbortOnError) );
	//RETAILMSG( 1, (TEXT("value: XoffLim=%d, XonLim=%d, XoffChar=%d, XonChar=%d, ErrorChar=%d, EofChar=%d, EvtChar=%d\r\n"), lpDCB->XoffLim, lpDCB->XonLim, lpDCB->XoffChar, lpDCB->XonChar, lpDCB->ErrorChar, lpDCB->EofChar, lpDCB->EvtChar) );

	//RETAILMSG( 1, (TEXT("\r\n\r\n    ---  dcb info--- end \r\n") ) );
}
#endif	//DEBUG_SERDRV
