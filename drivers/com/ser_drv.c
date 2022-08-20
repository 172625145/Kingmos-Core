/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/


/*****************************************************
�ļ�˵������������---Ӳ���޹ز�
�汾�ţ�  1.0.0
����ʱ�ڣ�2003-07-01
���ߣ�    ФԶ��
�޸ļ�¼��
	2004-12-06, ������GetRxBufferSize ��������PDD�㷵���ʺϸ�Ӳ����size
			ͬʱ��ȥ���˶� RxBuffers�Ķ�̬���ù���(IOCTL_SERIAL_SET_QUEUE_SIZE..)�������¿��ǣ�
			1��PDD��Ӧ����ͨ����ֵĲ��ԡ�
			2����user��˵������û������רҵ��
			3���򻯶� �ٽ�εĲ�����ʹ����Ч�ʸ��ߡ�
			4. ser_HandleInterrupt ���ڸ����ȼ�״̬���С����Բ���ҪĳЩ �ٽ�εĲ���
******************************************************/

//----------------------------------------------------
//
//�ļ���˵��
//1.  ���豸�ӿ�ʵ��(�ο�drvCom) ����ע��(RegisterDevice_COM)
//2.  CreateFile�Ĵ򿪷�ʽ(�ο�COM_Open˵��)
//3.  ������ʾBuffer���޵Ĵ�С�����Buffer��Baudrate���(�ο�ser_CalHWLim)
//4.  �û�SetDCBʱ����ser_GetFlagSetDCB�ж��Ƿ��Ӳ����������(�ο�ser_GetFlagSetDCB �� 
//    IOCTL_SERIAL_SET_DCB)
//5.  "for Braden"�Ƿ�����FAX��ʹ��(�ο�COM_Open˵��)
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

/***************  ȫ���� ���壬 ���� *****************/


//-----------------------------------------
//���Բ��ֵĶ���
//-----------------------------------------

#undef	DEBUG_SERDRV	// for debug

#ifdef	DEBUG_SERDRV

//static	LONG	g_nCountRx_Debug;
//static	LONG	g_nCountRead_Debug;

void	dcb_Debug( LPDCB lpDCB, LPTSTR pszText );

#endif	//DEBUG_SERDRV

//-----------------------------------------
//�����Ķ���
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
// ������BOOL RegisterDevice_COM( DWORD nIndex )
// ������
//	IN nIndex-ָ��������
// ����ֵ��
//	�ɹ�������TRUE;ʧ�ܣ�����FALSE 
// ����������ע���豸��������
// ����: 
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
// ������void	ser_HandleInterrupt( LPVOID  pHead )
// ������
//	IN pHead-������Ϣ�ṹ
// ����ֵ��
//	�� 
// �����������жϴ���
// ����: 
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

	//�����ȫ���
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        return ;
	}
	//��ȡ������Ϣ
	pHeadDev = pInfoDrv->pHeadDev;
	pFunTbl = pInfoDrv->pFunTbl;
	//��ȡ ������Ϣ
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
	
	//csEnter_RxResize( lpInfoRxBuf );	//���ܣ� ��ֹ rxBuffer���·��� �� �ж�����
	dwIntr = INTR_NONE;
	RxDataAvail = FALSE;
	while( dwIntr=pFunTbl->lpFnIntrTypeQuery(pHeadDev) )
	{
		if( dwIntr & INTR_RX )
		{
			//2004-12-06, ����Ҫ�����޸�˵��
			//��ֹ rxBuffer���·��� �� �ж�����
			//csEnter_RxResize( lpInfoRxBuf );
			//			
			//��ȡ ʣ��ռ�
			//csEnter_Rx(pInfoDrv);
			//			
         
			if( *pnRead == 0 )
			{   // ����һ���ֽ�ΪCOM_Read���Ƿ��������ж�
				nLenRx_Write = *pnLength - *pnWrite - 1;
			}
			if( *pnRead > *pnWrite )
			{	// ����һ���ֽ�ΪCOM_Read���Ƿ��������ж�
				nLenRx_Write = *pnRead - *pnWrite - 1;
			}
			else
			{
				nLenRx_Write = *pnLength - *pnWrite;
			}
			
/*			
			if( *pnCounts == *pnLength )
			{	//û���㹻�Ŀռ�
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
			//2004-12-06, ����Ҫ�����޸�˵��			
			//csLeave_Rx(pInfoDrv);

			//����/���� �ַ�
			if( nLenRx_Write==0 )
			{
				//���� �ַ�
				BYTE    TempBuf[16];//16
				nLenRx_Write = 16;
				pFunTbl->lpFnRecv( pHeadDev, TempBuf, &nLenRx_Write );
				EdbgOutputDebugString( "dis[%d]", nLenRx_Write );
				nLenRx_Write = 0;
			}
			else
			{
				//���� �ַ�
				pBufWrite = lpInfoRxBuf->pRxBuf + (*pnWrite);
				pFunTbl->lpFnRecv( pHeadDev, pBufWrite, &nLenRx_Write );
				if( nLenRx_Write )
				{
					//
					//���������---���Ʒ��͹���
					//1. ����Ƿ��յ�֪ͨ�ַ�(Xoff/Xon)
					//2. ����ֹͣ/�������͵ı�־
					//3. Xoff/Xon�������������ݣ�Ҫ����
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

								//ֹͣ����(because received a Xoff)
								pInfoDrv->fStopXmit = 1;

								//����Xoff
								if( nLenMove )
									memmove( pBufWrite_tmp, pBufWrite_tmp+1, nLenMove );	//nLenRx_Write - nIndex - 1
								nLenRx_Write--;
							}
							else if( *pBufWrite_tmp==XonChar )
							{
								EdbgOutputDebugString( "Xon\r\n" );

								//��������(because received a Xon)
								pInfoDrv->fStopXmit = 0;
								//���յ�Xon�����Է�������
								dwIntr |= INTR_TX;

								//����Xon
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

					//�������ݱ���
					if( nLenRx_Write )
					{
						//���� ��Ϣ---nCounts and nWrite
						//2004-12-06, ����Ҫ�����޸�˵��
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
			//������---���ơ�����ʣ��ռ䡱����
			//
			if( pInfoDrv->fFlow_RxBuf )
			{
				//RxBufʣ��ռ�
				//2004-12-06, ����Ҫ�����޸�˵��
				//csEnter_Rx( pInfoDrv );
				                           
				nLenRx_Write = *pnLength - GET_RX_DATA_LENGTH(lpInfoRxBuf);//*pnCounts;
				//2004-12-06, ����Ҫ�����޸�˵��
				//csLeave_Rx( pInfoDrv );

				//������Ӳ����͡�����ʣ��ռ䡱����Ӳ��֪ͨ�Է�
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

				//�����������͡�����ʣ��ռ䡱�������֪ͨ�Է�
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
			//2004-12-06, ����Ҫ�����޸�˵��
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
	//2004-12-06, ����Ҫ�����޸�˵��
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
// ������DWORD	COM_Init( DWORD dwParamDevice )
// ������
//	IN dwParamDevice-��ʼ������
// ����ֵ��
//	������Ϣ�ṹ
// �������������������ʼ��
// ����: 
// ********************************************************************
DWORD	COM_Init( DWORD dwParamDevice )
{
	BOOL			fSuccess;
    PSERDRV_INFO	pInfoDrv;

	////RETAILMSG( 1, (TEXT("COM_Init: [%d]\r\n"), dwParamDevice) );
	//����ṹ
    pInfoDrv  =  (PSERDRV_INFO)HANDLE_ALLOC( sizeof(SERDRV_INFO) );
    if ( !pInfoDrv )
        return 0;
	HANDLE_INIT( pInfoDrv, sizeof(SERDRV_INFO) );
	fSuccess = FALSE;

 	//��ʼ��---ARCH�������Ϣ
    pInfoDrv->pObjDev = (POBJECT_ARCH)GetSerialObject( dwParamDevice );
    if( !pInfoDrv->pObjDev || !(pInfoDrv->pObjDev->pFunTbl) )
	{
        HANDLE_FREE( pInfoDrv );
        return 0;
    }
	pInfoDrv->pFunTbl = (PVTBL_ARCH_SER)pInfoDrv->pObjDev->pFunTbl;

	//��ʼ��---���ջ���
    //2004-12-06 lilin , remove to ����	
    //pInfoDrv->InfoRxBuf.nLength = RX_BUFFER_SIZE_MIN;
    //pInfoDrv->InfoRxBuf.pRxBuf  = (LPBYTE)malloc( pInfoDrv->InfoRxBuf.nLength );
    //

	//��ʼ��---��д
    pInfoDrv->hEvtWrite   = CreateEvent(0, FALSE, FALSE, NULL);
    pInfoDrv->hEvtRead    = CreateEvent(0, FALSE, FALSE, NULL);
	csInit_Read( pInfoDrv );
    csInit_Write( pInfoDrv );
	csInit_RxResize( &pInfoDrv->InfoRxBuf );
	csInit_Rx( pInfoDrv );
	csInit_Tx( pInfoDrv );

	//��ʼ��---������
	csInit_hListOpen( pInfoDrv );
	List_InitHead( &pInfoDrv->hListOpen );

    //��ʼ��---Ĭ�ϳ�ʱ
    pInfoDrv->CommTmouts.ReadIntervalTimeout		  = READ_TIMEOUT;
    pInfoDrv->CommTmouts.ReadTotalTimeoutMultiplier = READ_TIMEOUT_MULTIPLIER;
    pInfoDrv->CommTmouts.ReadTotalTimeoutConstant	  = READ_TIMEOUT_CONSTANT;
    pInfoDrv->CommTmouts.WriteTotalTimeoutMultiplier= WRITE_TIMEOUT_MULTIPLIER;
    pInfoDrv->CommTmouts.WriteTotalTimeoutConstant  = WRITE_TIMEOUT_CONSTANT;

	//��ʼ��---Ӳ��
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
	//pInfoDrv->pFunTbl->lpFnInitLast( pHWHead );//�ٸ�1�λ��� Ӳ����ʼ��

	//���� �ĳ�ʼ��
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
// ������BOOL	COM_Deinit(DWORD pHead)
// ������
//	IN pHead-������Ϣ�ṹ
// ����ֵ��
//	�ɹ�����TRUE
// �������������������ͷ�
// ����: 
// ********************************************************************
BOOL	COM_Deinit(DWORD pHead)
{
	PSERDRV_INFO	pInfoDrv = (PSERDRV_INFO)pHead;

	//�����ȫ���
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError(ERROR_INVALID_HANDLE);
        return (FALSE);
	}

    //�˳� �жϴ����߳�---�Ƶ�Arch��
	//{
	//	pInfoDrv->fExitISR = TRUE;
	//	SetEvent( pInfoDrv->hEvtISR );
	//	CloseHandle( pInfoDrv->hThrdISR );
	//}

	//�رն˿� COM_Close
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
	
	//�ͷ�������Դ
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

	//�ͷ� ARCH
    pInfoDrv->pFunTbl->lpFnDeinit(pInfoDrv->pHeadDev);

	//�ͷ� �Լ�
    HANDLE_FREE( pInfoDrv );

    return (TRUE);
}


//----------------------------------------------------
//˵����
//	1. RW ��ռ---�� RW ��ʽ�򿪣�ֻ��1��------------------------�����������
//	2. 0  ����--- 0 ��ʽ�������κ�����򿪣�����IOControl����---�����������
//	3. D  ����---���� D ��ʽ��1�Σ������Ƿ��� RW ��ʽ�򿪵ģ���֪ͨ���� RW ��ʽ�򿪵Ĺ��ܹرգ��˳�Com_read/com_write, ����֪ͨ�¼���Waitcommevent��
//				 ���� D ��ʽ�ر�ʱ��ȡ������ RW ��ʽ�򿪵Ĺ�������
//	4. D  �ų�---�� D ��ʽ�򿪺󣬾��ٲ����� RW ��ʽ��
//
// ********************************************************************
// ������DWORD	COM_Open( DWORD pHead, DWORD dwAccessCode, DWORD dwShareMode )
// ������
//	IN pHead-������Ϣ�ṹ
//	IN dwAccessCode-���ʷ�ʽ
//	IN dwShareMode-����ʽ
// ����ֵ��
//	���ش���Ϣ�ṹ
// �������������豸
// ����: 
// ********************************************************************
DWORD	COM_Open( DWORD pHead, DWORD dwAccessCode, DWORD dwShareMode )
{
    PSERDRV_INFO	pInfoDrv = (PSERDRV_INFO)pHead;
    PSEROPEN_INFO   pInfoOpen;
    PVTBL_ARCH_SER	pFunTbl;

	//�����ȫ���
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        SetLastError(ERROR_INVALID_HANDLE);        
        return 0;
	}
	// ���Ȩ�޺Ͳ���
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
		//pInfoDrv->pAccessOwner: ���� RW �� D ��ʽ�򿪵ľ��
		if( (dwAccessCode & (GENERIC_READ | GENERIC_WRITE)) && pInfoDrv->pAccessOwner ) 
		{
			SetLastError(ERROR_INVALID_ACCESS);        
			return 0;
		}
	}

	//����ṹ���������
    pInfoOpen = (PSEROPEN_INFO)HANDLE_ALLOC( sizeof(SEROPEN_INFO) );
    if( !pInfoOpen ) 
	{
		return 0;
	}
	HANDLE_INIT( pInfoOpen, sizeof(SEROPEN_INFO) );
    pInfoOpen->dwAccessCode = dwAccessCode;
    pInfoOpen->dwShareMode = dwShareMode;

	//��ʼ��---ͨ���¼� ser_WaitCommEvent
	pInfoOpen->hEvtComm = CreateEvent(NULL, FALSE, FALSE, NULL);
    pInfoOpen->dwEvtMask = 0;
    pInfoOpen->dwEvtData = 0;
    pInfoOpen->fAbortEvt = 0;
	csInit_CommEvt( pInfoOpen );
	
	csEnter_hListOpen( pInfoDrv );
	//��ʼ��---Ӳ�����ڵ�һ�δ�ʱ��
    if( !pInfoDrv->nOpenCnt )
	{
		pFunTbl = pInfoDrv->pFunTbl;

		//��ʼ��---������
		pInfoDrv->fSentXoff = 0;
		pInfoDrv->fStopXmit = 0;

		//��ʼ��---DCB����
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
        pInfoDrv->dcb.XonLim     = 512;//send Xon,  if RxBufferʣ��ռ� >= DCB.XonLim
        pInfoDrv->dcb.XoffLim    = 200;//send Xoff, if RxBufferʣ��ռ� <= DCB.XoffLim

        pInfoDrv->dcb.XonChar    = X_ON_CHAR;
        pInfoDrv->dcb.XoffChar   = X_OFF_CHAR;
        pInfoDrv->dcb.ErrorChar  = ERROR_CHAR;
        pInfoDrv->dcb.EofChar    = E_OF_CHAR;
        pInfoDrv->dcb.EvtChar    = EVENT_CHAR;
		//��ȡ������
		ser_GetFlow( pInfoDrv );
		//����DCB��Ӳ��
		//if( !pFunTbl->lpFnSetDCB( pInfoDrv->pHeadDev, &(pInfoDrv->dcb), SETDCB_ALL ) )
		//	goto EXIT_OPEN;

		//����Ӳ�����ֵļ���ֵ
		ser_CalHWLim( pInfoDrv );

		//��Ӳ��
		if ( !pFunTbl->lpFnOpen(pInfoDrv->pHeadDev) )	//���л���� lpFnSetDCB ����
            goto EXIT_OPEN;

		//���Ӳ��
        pFunTbl->lpFnPurgeComm( pInfoDrv->pHeadDev, PURGE_RXCLEAR );

		//���RxBuffer
		csEnter_Rx( pInfoDrv );
		pInfoDrv->InfoRxBuf.nWrite	= 0;
		pInfoDrv->InfoRxBuf.nRead	= 0;
//		pInfoDrv->InfoRxBuf.nCounts = 0;
		//memset( pInfoDrv->InfoRxBuf.pRxBuf, 0, pInfoDrv->InfoRxBuf.nLength );
		csLeave_Rx( pInfoDrv );

		//���TxInfo
		csEnter_Tx( pInfoDrv );
		pInfoDrv->dwTxBytes = 0;
		pInfoDrv->dwTxBytesPending = 0;
		pInfoDrv->pTxBuf = NULL;
		csLeave_Tx( pInfoDrv );
		
		//RxBuf_dbg_init( pInfoDrv );//for debug
    }

    ++(pInfoDrv->nOpenCnt);
	//���� pInfoOpen �� pInfoDrv
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
// ������BOOL	COM_Close(DWORD pHead)
// ������
//	IN pHead-����Ϣ�ṹ
// ����ֵ��
//	�ɹ�����TRUE
// �����������ر��豸
// ����: 
// ********************************************************************
//Close���
BOOL	COM_Close(DWORD pHead)
{
	PSEROPEN_INFO	pInfoOpen= (PSEROPEN_INFO)pHead;
    PSERDRV_INFO	pInfoDrv;
    PVTBL_ARCH_SER	pFunTbl;
	int				i;

	//�����ȫ���
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
	//�ͷ����еĵ���
	//
	
	//�жϵ�ǰ�Ƿ��ͷ���
	csEnter_hListOpen( pInfoDrv );
    if( pInfoDrv->nOpenCnt<=0 )
	{
		////EdbgOutputDebugString("===COM_Close: SetLastError ");
        SetLastError( ERROR_INVALID_HANDLE );
        csLeave_hListOpen( pInfoDrv );
		return FALSE;
	}
	--(pInfoDrv->nOpenCnt);
	
	//�˳�--- WaitCommEvent 
	pInfoOpen->fAbortEvt = 1;
	pInfoOpen->dwEvtMask = 0;
	for( i=0; i<pInfoOpen->wCntWaitEvent; i++ )
	{
		//csEnter_CommEvt( pInfoOpen );
		SetEvent(pInfoOpen->hEvtComm);
		//csLeave_CommEvt( pInfoOpen );
	}
	Sleep(2);
	
	//�˳�--- ReadFile
	pInfoDrv->fAbortRead = 1;
	if ( pInfoOpen->dwAccessCode & (GENERIC_READ) )
	{
		for( i=0; i<pInfoOpen->wCntRead; i++ )
			SetEvent(pInfoDrv->hEvtRead);
	}
	Sleep(2);
	
	//�˳�--- WriteFile
	if ( pInfoOpen->dwAccessCode & (GENERIC_WRITE) )
	{
		for( i=0; i<pInfoOpen->wCntWrite; i++ )
			SetEvent(pInfoDrv->hEvtWrite);
	}
	//
	Sleep(2);
	
	//�˳�--- ������
	List_RemoveUnit(&pInfoOpen->hListOpen);
	
	//�ر�Ӳ��
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
	
	//�ȴ����е��˳�
	i = 0;
	while( i<600 && ((pInfoOpen->wCntRead) || (pInfoOpen->wCntWrite) || (pInfoOpen->wCntIOCtl) || 
						  (pInfoOpen->wCntWaitEvent) || (pInfoOpen->wCntIntr)) )
	{
		i ++;
		Sleep( 1 );
	}
	
	//�ͷ���Դ
	if ( pInfoOpen->hEvtComm )
	{
		CloseHandle(pInfoOpen->hEvtComm);
	}
	csDelete_CommEvt( pInfoOpen );
	//�ͷ�
	HANDLE_FREE( pInfoOpen );

	csLeave_hListOpen( pInfoDrv );
	return TRUE;
}

// ********************************************************************
// ������DWORD	COM_Read( DWORD pHead, LPVOID lpBuffer, DWORD dwnLenRead )
// ������
//	IN pHead-����Ϣ�ṹ
//	IN lpBuffer-�����ݵ�BUFFER
//	IN dwnLenRead-�����ݵ�BUFFER�ĳ���
// ����ֵ��
//	���ض�ȡ���ݵĳ���
// �������������豸
// ����: 
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

	//�����ȫ���
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
	//����豸���ܲ���
	if( !pInfoDrv->nOpenCnt )
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return (FALSE);
	}
	// ���Ȩ�޺Ͳ���
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

	//�����
	csEnter_Read( pInfoDrv );
    READ_CNT_INC(pInfoOpen);
	//׼��
    pInfoDrv->fAbortRead = 0;
	ResetEvent(pInfoDrv->hEvtRead);
    //��ʱ�ļ���
	dwTotalTimeout = pTmout->ReadTotalTimeoutConstant;
    if( pTmout->ReadTotalTimeoutMultiplier!=MAXDWORD ) 
        dwTotalTimeout += (pTmout->ReadTotalTimeoutMultiplier * nLenRead);
    dwIntervalTimeout  = pTmout->ReadIntervalTimeout;
	dwTimeSpent = 0;

	//������
	while( nLenRead )
	{
		//�ж�RxBuffer���Ƿ�������
		csEnter_Rx(pInfoDrv);
		
		nLenRx_Read = GET_RX_DATA_LENGTH( lpInfoRxBuf );	
	
		// lilin 2004-12-08 remove
		//csLeave_Rx(pInfoDrv);
		//
		if( nLenRx_Read > nLenRead )
			nLenRx_Read = nLenRead;

		//���������
		if( nLenRx_Read )
		{
			//��ȡ���ݣ���RxBuffer��
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

			//������Ϣ
			pBufRead   += nLenRx_Read;
			nBytesRead += nLenRx_Read;
			nLenRead   -= nLenRx_Read;
		}
		else
		{
			// lilin 2004-12-08 add
            csLeave_Rx(pInfoDrv);
            
			//���㳬ʱ�� ��Ҫ�ȴ���ʱ��dwTimeout
			if( dwTotalTimeout==0 )
			{
				if( dwIntervalTimeout==MAXDWORD )
				{
					//EdbgOutputDebugString( "exit: dwIntervalTimeout==MAXDWORD\r\n" );
					break;	//(I==max, M==0/max, C==0): �����յ�char���Ӧ��������
				}
				else		//(I==any, M==0/max, C==0): һ��Ҫ�ȵ� Byte��֮���ڼ��ʱ���� ��ʱ
				{
					if ( !nBytesRead )
						dwTimeout = INFINITE;			//������ʱ�� �� ��һ���ַ�
					else
					{
						if( dwTimeSpent>=dwIntervalTimeout )
						{
							//EdbgOutputDebugString( "exit: time out at dwTotalTimeout==0\r\n" );
							break;
						}
						dwTimeout = dwIntervalTimeout;	//�ü��ʱ�� �� �Ժ���ַ�
					}
				}
			}
			else
			{
                if ( dwTimeSpent >= dwTotalTimeout )//��ʱ�˳�
				{
					//EdbgOutputDebugString( "exit: time out\r\n" );
					break;
				}

                dwTimeout = dwTotalTimeout - dwTimeSpent;			//Ĭ�� ������ʱ�� �� ��һ���ַ�
                if ( nBytesRead && (dwIntervalTimeout!=0) )	
				{
					if( dwTimeout > dwIntervalTimeout )
						dwTimeout = dwIntervalTimeout;			//�ü��ʱ�� �� �Ժ���ַ�
                }
			}
 
            dwTicks = GetTickCount();
			//�ȴ������¼��������жϡ�PurgeComm��COM_Close
            if( WaitForSingleObject(pInfoDrv->hEvtRead, dwTimeout)==WAIT_TIMEOUT )
			{
				//EdbgOutputDebugString( "exit: time out at wait no data\r\n" );
                break;
            }
            dwTimeSpent += (GetTickCount() - dwTicks);

			//�Ƿ��˳�
            if( pInfoDrv->fAbortRead || pInfoOpen->fClose )
			{
				//EdbgOutputDebugString( "exit: time out at fAbortRead/fClose\r\n" );
                break;
            }
			//�Ƿ��˳�
            if( !pInfoDrv->nOpenCnt )
			{
				//EdbgOutputDebugString( "exit: time out at nOpenCnt\r\n" );
                SetLastError(ERROR_INVALID_HANDLE);
				nBytesRead = (DWORD)-1;
                break;
            }
        }

		//
		//�⿪������---���ơ�����ʣ��ռ䡱����
		//
		if( pInfoDrv->fFlow_RxBuf && (pInfoDrv->fSentXoff || pInfoDrv->fSentDtrOff || pInfoDrv->fSentRtsOff) )
		{
			//RxBufʣ��ռ�
			csEnter_Rx(pInfoDrv);
			nLenRx_Read = *pnLength - GET_RX_DATA_LENGTH(lpInfoRxBuf);//*pnCounts;
			csLeave_Rx(pInfoDrv);

			//���������ʣ��ռ䡱�㹻�����֪ͨ�Է���������
			if( pInfoDrv->fSentXoff && (nLenRx_Read >= pInfoDrv->dcb.XonLim) )
			{
				pInfoDrv->fSentXoff = 0;
				pFunTbl->lpFnXmitChar( pHeadDev, pInfoDrv->dcb.XonChar);
			}
			//���������ʣ��ռ䡱�㹻��Ӳ��֪ͨ�Է���������
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
// ������DWORD	COM_Write(DWORD pHead, LPCVOID lpBuffer, DWORD  NumberOfBytes )
// ������
//	IN pHead-����Ϣ�ṹ
//	IN lpBuffer-д���ݵ�BUFFER
//	IN NumberOfBytes-д���ݵ�BUFFER�ĳ���
// ����ֵ��
//	����д���ݵĳ���
// ����������д�豸
// ����: 
// ********************************************************************
DWORD	COM_Write(DWORD pHead, LPCVOID lpBuffer, DWORD  NumberOfBytes )
{
	PSEROPEN_INFO	pInfoOpen = (PSEROPEN_INFO)pHead;
	PSERDRV_INFO	pInfoDrv;
	DWORD			dwRet;

	LPBYTE			pBufWrite = (LPBYTE)lpBuffer;

	LPCOMMTIMEOUTS	pTmout;
	DWORD			dwTotalTimeout;

	//�����ȫ���
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
	//����豸���ܲ���
	if( !pInfoDrv->nOpenCnt )
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return (FALSE);
	}
	// ���Ȩ�޺Ͳ���
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

	//���뷢�� 
	csEnter_Write( pInfoDrv );
    WRITE_CNT_INC(pInfoOpen);
	//
    //pInfoDrv->fAbortWrite = 0;
	ResetEvent( pInfoDrv->hEvtWrite );
	//׼��
	csEnter_Tx( pInfoDrv );
	pInfoDrv->dwTxBytes = 0;
	pInfoDrv->dwTxBytesPending = NumberOfBytes;
	pInfoDrv->pTxBuf = pBufWrite;
	pInfoDrv->dwWritePerm = GetCurrentPermissions();
//	RETAILMSG( 1, ( "pInfoDrv->dwWritePerm=0x%x.\r\n", pInfoDrv->dwWritePerm ) );
	csLeave_Tx( pInfoDrv );

	//��ʼ����
	ser_DoTxData( pInfoDrv );
	//��ʱ
	pTmout = &pInfoDrv->CommTmouts;
	dwTotalTimeout = pTmout->WriteTotalTimeoutConstant;
	if( pTmout->WriteTotalTimeoutMultiplier!=MAXDWORD )
		dwTotalTimeout += (pTmout->WriteTotalTimeoutMultiplier * NumberOfBytes);
	if ( !dwTotalTimeout )
		dwTotalTimeout = INFINITE;
	//�ȴ�
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

	//���� �˳�
	csEnter_Tx( pInfoDrv );
	pInfoDrv->pTxBuf	= NULL;
	pInfoDrv->dwTxBytes			= 0;
	pInfoDrv->dwTxBytesPending	= 0;
	pInfoDrv->dwWritePerm = 0;
	csLeave_Tx( pInfoDrv );

	//ͨ���¼�EV_TXEMPTY
	//if( pInfoDrv->fAbortWrite==0 )
	if( dwRet==NumberOfBytes )
		ser_NotifyCommEvent(pInfoDrv, EV_TXEMPTY);	//attention here
	//Ӳ������
	if( pInfoDrv->dcb.fRtsControl == RTS_CONTROL_TOGGLE ) 
		pInfoDrv->pFunTbl->lpFnClearSingalRTS(pInfoDrv->pHeadDev);

	WRITE_CNT_DEC(pInfoOpen);
	csLeave_Write( pInfoDrv );

	////EdbgOutputDebugString( "===COM_Write_last: NumTotal=%d, BytesTx= %d , BytesTxPending= %d ", NumberOfBytes, pInfoDrv->TxBytes, pInfoDrv->dwTxBytesPending );
	return ( dwRet );
}

// ********************************************************************
// ������DWORD	COM_Seek( DWORD  pHead, LONG    Position, DWORD   Type )
// ������
//	IN pHead-����Ϣ�ṹ
//	IN Position-��֧��
//	IN Type-��֧��
// ����ֵ��
//	����-1
// ����������SEEK�豸
// ����: 
// ********************************************************************
DWORD	COM_Seek( DWORD  pHead, LONG    Position, DWORD   Type )
{    return (DWORD)-1;	}


// ********************************************************************
// ������VOID	COM_PowerUp( DWORD pHead )
// ������
//	IN pHead-������Ϣ�ṹ
// ����ֵ��
//	��
// ������������ԴUP
// ����: 
// ********************************************************************
//Powerup���
VOID	COM_PowerUp( DWORD pHead )
{
	PSERDRV_INFO  pInfoDrv    = (PSERDRV_INFO)pHead;

	//�����ȫ���
	if( !HANDLE_CHECK(pInfoDrv) )
	{
        return ;
	}
	////EdbgOutputDebugString("===COM_PowerUp");
	pInfoDrv->pFunTbl->lpFnPowerOn( pInfoDrv->pHeadDev );
}
// ********************************************************************
// ������VOID	COM_PowerDown( DWORD   pHead )
// ������
//	IN pHead-������Ϣ�ṹ
// ����ֵ��
//	��
// ������������ԴDOWN
// ����: 
// ********************************************************************
VOID	COM_PowerDown( DWORD   pHead )
{
	PSERDRV_INFO     pInfoDrv = (PSERDRV_INFO)pHead;

	////EdbgOutputDebugString("===COM_PowerDown");
	//�����ȫ���
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
// ������BOOL	COM_IOControl(DWORD pHead, DWORD dwCode, LPVOID pBufIn_, DWORD dwLenIn, LPVOID pBufOut_, DWORD dwLenOut, LPDWORD pdwActualOut)
// ������
//	IN pHead-����Ϣ�ṹ
//	IN dwCode-IO������
//	IN pBufIn_-����BUFFER
//	IN dwLenIn-����BUFFER�ĳ���
//	OUT pBufOut_-���BUFFER
//	IN dwLenOut-���BUFFER�ĳ���
//	OUT pdwActualOut-���BUFFERʵ�ʻ�ȡ���ݵĳ���
// ����ֵ��
//	�ɹ�����TRUE
// ����������IOControl�豸
// ����: 
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

	//�����ȫ���
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
	//����豸���ܲ���
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
	//������Ȩ��---�������沿�ֵ�code�������Ķ���Ҫ����дȨ�ޡ�
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
	//ͨ��״̬��Get
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

			//��ȡ�����ͳ�ʼ��
			lpParamGet = (PARAM_CLEARCOMMERR*)pBufOut;
			lpErrors = &lpParamGet->dwErrors;
			lpStat   = &lpParamGet->comStat;
			memset( lpStat, 0, sizeof(COMSTAT) );

			//��ȡ ARCH��Ϣ
			*lpErrors = pFunTbl->lpFnGetComStat( pHeadDev, lpStat );

			//��ȡ ������Ϣ
			lpStat->fXoffSent = pInfoDrv->fSentXoff;			//�Ƿ�����Xoff
			lpStat->fXoffHold = pInfoDrv->fStopXmit;			//������Ϊ�յ�Xoff��ֹͣ

			csEnter_Rx(pInfoDrv);
			lpStat->cbInQue   = GET_RX_DATA_LENGTH(&pInfoDrv->InfoRxBuf);// pInfoDrv->InfoRxBuf.nCounts;
			csLeave_Rx(pInfoDrv);

			csEnter_Tx( pInfoDrv );
			lpStat->cbOutQue  = pInfoDrv->dwTxBytesPending;
			csLeave_Tx( pInfoDrv );
		}
		break;
	//Modem״̬��Get  (Dsr/Cts/Ring/Rlsd)
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

	//�����ַ�
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

	//������֣�
	//	���ñ�־��ʹֹͣ/�������ͣ��൱�ڷ���ʱ�����յ�Xoff/Xon�ַ�
	case IOCTL_SERIAL_SET_XOFF :
		if( pInfoDrv->dcb.fOutX )
			pInfoDrv->fStopXmit = 1;
		break;
	case IOCTL_SERIAL_SET_XON :
		if( pInfoDrv->dcb.fOutX )
			pInfoDrv->fStopXmit = 0;
		break;

	//Ӳ���ź�����---break
	case IOCTL_SERIAL_SET_BREAK_ON :
		pFunTbl->lpFnSetBreak(pHeadDev);
		break;
	case IOCTL_SERIAL_SET_BREAK_OFF :
		pFunTbl->lpFnClearBreak(pHeadDev);
		break;

	//Ӳ���ź�����---DTR
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

	//Ӳ���ź�����---RTS
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

	//ͨ���¼�������Get
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
	//ͨ���¼�������Set
	case IOCTL_SERIAL_SET_WAIT_MASK :
		if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			DWORD			dwEvtWait;

			//֪ͨ���� waitcommevent���� �˳�
			dwEvtWait = *(DWORD *)pBufIn;
			pInfoOpen->fAbortEvt = 1;
			SetEvent(pInfoOpen->hEvtComm);

			//���� pInfoOpen���¼�����
			csEnter_CommEvt( pInfoOpen );
			pInfoOpen->dwEvtMask = dwEvtWait;
			csLeave_CommEvt( pInfoOpen );
			
			//���� pInfoDrv������Open�ߵ�����
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
				pInfoDrv->dwEvtMask = dwFlags;//���д򿪵�

				//csLeave_hListOpen( pInfoDrv );
			}
			else
			{
				pInfoDrv->dwEvtMask = dwEvtWait;//���д򿪵�
			}
		}
		break;
	//ͨ���¼�������Wait
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


	//����������������ԣ�
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

	//��д��ʱ������
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
	//��д��ʱ�Ļ�ȡ
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

	//���������
	case IOCTL_SERIAL_PURGE :
		if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		else
		{
			dwFlags = *((PDWORD) pBufIn);

			//Ӳ�� purge
			pFunTbl->lpFnPurgeComm(pHeadDev,dwFlags);
			//������� purge
			if ( dwFlags & PURGE_RXCLEAR ) 
			{
				//���ջ��� purge
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
				
				//���������
				if( pInfoDrv->fFlow_RxBuf )
				{
					//if ( pInfoDrv->dcb.fInX && pInfoDrv->fSentXoff ) 
					if( pInfoDrv->fSentXoff ) 
					{
						//����Xon�����߶Է���������
						pInfoDrv->fSentXoff = 0;
						pFunTbl->lpFnXmitChar( pHeadDev, pInfoDrv->dcb.XonChar );   
					}
					//if ( pInfoDrv->fSentDtrOff && (pInfoDrv->dcb.fDtrControl==DTR_CONTROL_HANDSHAKE) ) 
					if ( pInfoDrv->fSentDtrOff ) 
					{
						//����Dtr�����߶Է���������
						pInfoDrv->fSentDtrOff = 0;
						pFunTbl->lpFnSetSingalDTR( pHeadDev );
					}
					//if ( pInfoDrv->fSentRtsOff && (pInfoDrv->dcb.fRtsControl==RTS_CONTROL_HANDSHAKE) )
					if ( pInfoDrv->fSentRtsOff )
					{
						//����Rts�����߶Է���������
						pInfoDrv->fSentRtsOff = 0;
						pFunTbl->lpFnSetSingalRTS( pHeadDev );
					}
				}
			}
			//������� purge
			if ( dwFlags & (PURGE_TXCLEAR) )
			{
				//������Ϣ purge
				csEnter_Tx( pInfoDrv );
				pInfoDrv->pTxBuf	= NULL;
				//pInfoDrv->dwTxBytes		= 0;
				pInfoDrv->dwTxBytesPending	= 0;
				csLeave_Tx( pInfoDrv );

				//���������
				pInfoDrv->fStopXmit = 0;
			}

			//��ֹ��д����
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
/*  2004-12-06, û�б�Ҫ�� ���޸�˵��
	//�շ�����buffer��С
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
			
			//�жϷ�����Ϣ
			dwLenAlloc = *(LONG*)pBufIn;
			lpInfoRxBuf= &pInfoDrv->InfoRxBuf;
			if( (dwLenAlloc>=RX_BUFFER_SIZE_MIN_x) && (dwLenAlloc!=lpInfoRxBuf->nLength) )
			{
				csEnter_RxResize( &pInfoDrv->InfoRxBuf );
				csEnter_Rx(pInfoDrv);

				//��ʼ����
				if( lpInfoRxBuf->pRxBuf )
				{
					pAlloc = realloc( lpInfoRxBuf->pRxBuf, dwLenAlloc );
				}
				else
				{
					pAlloc = malloc( dwLenAlloc );
				}
				//�������� �ջ���buffer��Ϣ
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

					//����Ӳ�����ֵļ���ֵ
					ser_CalHWLim( pInfoDrv );
				}
				csLeave_Rx(pInfoDrv);
				csLeave_RxResize( &pInfoDrv->InfoRxBuf );
			}
		}
		break;
*/
	//��ȡDCB
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
	//����DCB
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
				//����Ӳ�����ֵļ���ֵ
				ser_CalHWLim( pInfoDrv );
			}
			else
			{
				dwErr = ERROR_INVALID_PARAMETER;
			}
		}
		break;

	//���⹦��
	case IOCTL_SERIAL_ENABLE_IR :
		if( !pFunTbl->lpFnIREnable(pHeadDev, pInfoDrv->dcb.BaudRate) )
		{
			dwErr = ERROR_INVALID_PARAMETER;
		}
		break;
	case IOCTL_SERIAL_DISABLE_IR :
		pFunTbl->lpFnIRDisable(pHeadDev);
		break;

	//ARCH Ĭ�ϴ���
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
// ������VOID	ser_DoTxData( PSERDRV_INFO pInfoDrv )
// ������
//	IN pInfoDrv-������Ϣ�ṹ
// ����ֵ��
//	�� 
// �����������������ݣ����жϺ�COM_Write����
// ����: 
// ********************************************************************
VOID	ser_DoTxData( PSERDRV_INFO pInfoDrv )
{
    PVTBL_ARCH_SER	pFunTbl = pInfoDrv->pFunTbl;
    PVOID			pHeadDev = pInfoDrv->pHeadDev;
	DWORD			BytesTx;

	//���͵� ���/��ֹ
	//if ( pInfoDrv->dwTxBytesPending==0 || pInfoDrv->fAbortWrite )
	if ( pInfoDrv->dwTxBytesPending==0 )
	{
		//�ر�Ӳ���ķ���
		pFunTbl->lpFnIntrHandleTx(pHeadDev);
		//֪ͨCOM_Write
		SetEvent(pInfoDrv->hEvtWrite);
	}
	else
	{
		if ( pInfoDrv->dcb.fRtsControl == RTS_CONTROL_TOGGLE ) 
			pFunTbl->lpFnSetSingalRTS(pHeadDev);

		//�������---�ж��Ƿ���Է�������(�ɶԷ���Xoff/Xon������)
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
// ������void	ser_NotifyCommEvent(PVOID pHead, DWORD fdwEventMask)
// ������
//	IN pHead-������Ϣ�ṹ
//	IN fdwEventMask-�¼�����
// ����ֵ��
//	�� 
// ����������֪ͨ�¼�
// ����: 
// ********************************************************************
//�����PDD��MDD�У������¼�fdwEventMask�����䱣�浽ÿ���򿪵ľ���У���֪ͨWaitCommEvent
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
		//֪ͨ�� ÿ��Open�е� WaitCommEvent
		pUnitHeader = &pInfoDrv->hListOpen;
        pUnit = pUnitHeader->pNext;
		while( pUnit!=pUnitHeader )
		{
			pInfoOpen = LIST_CONTAINER( pUnit, SEROPEN_INFO, hListOpen);
			pUnit = pUnit->pNext;

			if( !pInfoOpen->fClose )//for braden.
			{
				csEnter_CommEvt( pInfoOpen );
				if ( pInfoOpen->dwEvtMask & fdwEventMask )//֪ͨ
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
// ������VOID	ser_NotifyCommEvent_Someone( PSEROPEN_INFO pInfoOpen, DWORD fdwEventMask )
// ������
//	IN pInfoOpen-����Ϣ�ṹ
//	IN fdwEventMask-�¼�����
// ����ֵ��
//	�� 
// ����������֪ͨ�¼���for braden��
// ����: 
// ********************************************************************
//ר������COM_Closeʱ��֪ͨ�����Ĵ��ߡ�for braden
VOID	ser_NotifyCommEvent_Someone( PSEROPEN_INFO pInfoOpen, DWORD fdwEventMask )	//for braden
{
	csEnter_CommEvt( pInfoOpen );
	if( pInfoOpen->dwEvtMask & fdwEventMask )//֪ͨ
	{
		pInfoOpen->dwEvtData |= (fdwEventMask & pInfoOpen->dwEvtMask);
		SetEvent(pInfoOpen->hEvtComm);
	}
	csLeave_CommEvt( pInfoOpen );
}

// ********************************************************************
// ������DWORD	ser_WaitCommEvent(PSEROPEN_INFO pInfoOpen, LPDWORD pfdwEventMask, LPOVERLAPPED Unused)
// ������
//	IN pInfoOpen-����Ϣ�ṹ
//	IN fdwEventMask-�¼�����
//	IN Unused-���õĲ���
// ����ֵ��
//	 ���ش�����ֵ
// �����������ȴ��¼�
// ����: 
// ********************************************************************
//���ã�WaitCommEvent-->COM_DeviceIOControl-->ser_WaitCommEvent
//�����PDD��MDD�з����¼�����ser_NotifyCommEvent֪ͨ���򱣴��¼�������
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
    //����Ҫ�ȴ��¼�����������
    if ( !pInfoOpen->dwEvtMask )
	{
		return ERROR_INVALID_PARAMETER;
    }
	//
    WAITEVENT_CNT_INC(pInfoOpen);
	//��λ�˳���־
    pInfoOpen->fAbortEvt = 0;

    while( pInfoDrv->nOpenCnt ) 
	{
		csEnter_CommEvt( pInfoOpen );
		//ȡ���¼�
        dwEventData = pInfoOpen->dwEvtData;
		pInfoOpen->dwEvtData = 0;
		////RETAILMSG( 1, (TEXT("drv_WaitCommEvent: [0x%X]\r\n"), dwEventData) );

		//��Ҫ��EV_RXCHAR����---�����ݣ������¼�
		if( pInfoOpen->dwEvtMask & EV_RXCHAR )
		{
//			if( pInfoDrv->InfoRxBuf.nCounts==0 )
			if( GET_RX_DATA_LENGTH(&pInfoDrv->InfoRxBuf) == 0 )
				dwEventData &= ~EV_RXCHAR;	//���û�����ݣ���û���¼�
			else
				dwEventData |= EV_RXCHAR;	//��������ݣ������¼�
		}

		//�ȵ���Ҫ�ȵ��¼������˳�����AP
        if( dwEventData & pInfoOpen->dwEvtMask ) 
		{
            *pfdwEventMask = (dwEventData & pInfoOpen->dwEvtMask);
			////RETAILMSG( 1, (TEXT("drv_WaitCommEvent: exit[%X], get=[%X] �ȵ���Ҫ�ȵ��¼�\r\n"), dwEventData, *pfdwEventMask) );
			csLeave_CommEvt( pInfoOpen );
            break;
		}
		else
		{
			csLeave_CommEvt( pInfoOpen );
		}

        //�ȴ����ڵ��¼�
        WaitForSingleObject(pInfoOpen->hEvtComm, (DWORD)-1);

        //����COM_Close(��SetCommMask)ʱ
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
// ������void	ser_CalHWLim( PSERDRV_INFO pInfoDrv )
// ������
//	IN pInfoDrv-������Ϣ�ṹ
// ����ֵ��
//	��
// ����������������ʾBuffer���޵Ĵ�С�����Buffer��Baudrate���
// ����: 
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
// ������void	ser_CalHWLim( PSERDRV_INFO pInfoDrv )
// ������
//	IN lpDCBDes-ԴDCB
//	IN lpDCBSet-Ŀ��DCB
// ����ֵ��
//	�������õı�־
// �����������ж��Ƿ��Ӳ����������
// ����: 
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
// ������void	ser_GetFlow( PSERDRV_INFO pInfoDrv )
// ������
//	IN pInfoDrv-������Ϣ�ṹ
// ����ֵ��
//	��
// ����������������ջ���Buffer�������Ƿ������������ʾ�Է������߶Է���Ҫ������������
// ����: 
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
// ������LPVOID	GetSerialObject(DWORD dwIDObj)
// ������
//	IN dwIDObj-PDD����ı�־
// ����ֵ��
//	�ɹ�����TRUE
// ��������������PDD����
// ����: 
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
//���Ժ���
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
