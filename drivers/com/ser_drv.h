/**************************************************************************
* Copyright (c)  微逻辑(WEILUOJI). All rights reserved.                   *
**************************************************************************/

#ifndef _SERDRV_H_
#define _SERDRV_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------
//接收信息
typedef struct _RXINFO
{
	LONG				nRead;
	LONG				nWrite;
//	LONG				nCounts;

	LONG				nLength;
	LPBYTE				pRxBuf;
	CRITICAL_SECTION	csRxBuf;	//重新分配pRxBuf地址和Length大小


} RXINFO, *PRXINFO;

//软件层 打开结构
typedef struct _SEROPEN_INFO
{
	//句柄检查
	HANDLE_THIS( _SEROPEN_INFO );

	//Driver层信息
	LPVOID				pInfoDrv;

	//链表
	LIST_UNIT			hListOpen;

	//访问参数
	DWORD				dwAccessCode;
	DWORD				dwShareMode;

	//调用计数
	WORD				wCntRead;
	WORD				wCntWrite;
	WORD				wCntWaitEvent;
	WORD				wCntIOCtl;
	WORD				wCntIntr;
	WORD				wXXX;

	//通信事件
	CRITICAL_SECTION	csCommEvt;
	HANDLE				hEvtComm;
	BOOL				fAbortEvt;	//终止通信事件
	DWORD				dwEvtMask;
	DWORD				dwEvtData;

	//for Braden. 特殊使用
	BOOL				fClose;

}SEROPEN_INFO, *PSEROPEN_INFO;

//软件层 信息结构
typedef struct _SERDRV_INFO
{
	//句柄检查
	HANDLE_THIS( _SERDRV_INFO );

	//关联 ARCH
	POBJECT_ARCH		pObjDev;		//设备对象单元
	PVTBL_ARCH_SER		pFunTbl;		//设备对象的操作
    PVOID				pHeadDev;		//设备对象的信息结构指针

	//所有打开的列表
	LONG				nOpenCnt;
	LIST_UNIT			hListOpen;
	CRITICAL_SECTION	cshListOpen;
    PSEROPEN_INFO		pAccessOwner;

	BOOL				fOpenByData;	//for Braden. 特殊使用

	//操作属性
	DCB					dcb;
	LONG				dwHWOffLim;	//硬件最低“接收剩余空间”
	LONG				dwHWOnLim;	//硬件足够“接收剩余空间”

	COMMTIMEOUTS		CommTmouts;

	//通信事件
	DWORD				dwEvtMask;		//所有打开的事件掩码总和

	//读(COM_Read)和收(中断)
	CRITICAL_SECTION	csRead;			//所有COM_Read互斥
	HANDLE				hEvtRead;		//超时的等待事件
	RXINFO				InfoRxBuf;		//中断接受信息
	
	CRITICAL_SECTION	csRx;			//将改变Read、Write、fDataFull的值

	//写(COM_Write)
	CRITICAL_SECTION	csWrite;		//所有COM_Write互斥
	HANDLE				hEvtWrite;

	//发(中断)
	LPBYTE				pTxBuf;			//发: buffer
	DWORD				dwTxBytesPending;//发: 剩余
	DWORD				dwTxBytes;		//发: 已经
	CRITICAL_SECTION	csTx;			// 

	//收发 的标志
	DWORD				fFlow_RxBuf:1;	//接收流控制：当“接收剩余空间”到极限时，软件或硬件通知对方
										//发送流控制fOutX: 当收到Xoff时，不再发数据，直到收Xon为止
	DWORD				fSentXoff:1;	//已经发送Xoff，通知对方---( 告诉对方停止发送 )
	DWORD				fSentDtrOff:1;	//已经用硬件Dtr，通知对方---( 告诉对方停止发送 )
	DWORD				fSentRtsOff:1;	//已经用硬件Rts，通知对方---( 告诉对方停止发送 )

	DWORD				fStopXmit:1;	//因为对方的软件或硬件通知，发送已经停止

	DWORD				fAbortRead:1;	//终止COM_Read
//	DWORD				fAbortWrite:1;	//终止COM_Write

	//DWORD				Reserved:28;
	DWORD                dwWritePerm; //写许可,因为中断线程需要将用户的数据发出，因此，中断线程必须对用户的数据进行读写的许可

} SERDRV_INFO, *PSERDRV_INFO;

//--------------------------------------------------------------------------------
#define RX_BUFFER_SIZE_MIN				2048
#define RX_BUFFER_SIZE_MIN_x			400
#define RX_BUFFER_SIZE_MAX				0
#define HANDSHAKEOFF_CONST				120
#define HANDSHAKEOFF_MAX				512

#define E_OF_CHAR						0xd
#define ERROR_CHAR						0xd
#define BREAK_CHAR						0xd
#define EVENT_CHAR						0xd
#define X_ON_CHAR						0x11
#define X_OFF_CHAR						0x13

//COMMTIMEOUT PARAM
#define READ_TIMEOUT					250
#define READ_TIMEOUT_MULTIPLIER			10
#define READ_TIMEOUT_CONSTANT			50   //100
#define WRITE_TIMEOUT_MULTIPLIER		10	//
#define WRITE_TIMEOUT_CONSTANT			100
//
#define READ_CNT_INC(pOpenHead)			(pOpenHead->wCntRead ++)//InterlockedIncrement(&pOpenHead->dwUsedCount)
#define READ_CNT_DEC(pOpenHead)			(pOpenHead->wCntRead --)//InterlockedIncrement(&pOpenHead->dwUsedCount)

#define WRITE_CNT_INC(pOpenHead)		(pOpenHead->wCntWrite ++)//InterlockedIncrement(&pOpenHead->dwUsedCount)
#define WRITE_CNT_DEC(pOpenHead)		(pOpenHead->wCntWrite --)//InterlockedIncrement(&pOpenHead->dwUsedCount)

#define IOCTL_CNT_INC(pOpenHead)		(pOpenHead->wCntIOCtl ++)//InterlockedIncrement(&pOpenHead->dwUsedCount)
#define IOCTL_CNT_DEC(pOpenHead)		(pOpenHead->wCntIOCtl --)//InterlockedIncrement(&pOpenHead->dwUsedCount)

#define WAITEVENT_CNT_INC(pOpenHead)	(pOpenHead->wCntWaitEvent ++)//InterlockedIncrement(&pOpenHead->dwUsedCount)
#define WAITEVENT_CNT_DEC(pOpenHead)	(pOpenHead->wCntWaitEvent --)//InterlockedIncrement(&pOpenHead->dwUsedCount)

#define INTR_CNT_INC(pOpenHead)			(pOpenHead->wCntIntr ++)//InterlockedIncrement(&pOpenHead->dwUsedCount)
#define INTR_CNT_DEC(pOpenHead)			(pOpenHead->wCntIntr --)//InterlockedIncrement(&pOpenHead->dwUsedCount)
//
#define csInit_hListOpen(pSH)	InitializeCriticalSection (&(pSH->cshListOpen))
#define csDelete_hListOpen(pSH)	DeleteCriticalSection (&(pSH->cshListOpen))
#define csEnter_hListOpen(pSH)	EnterCriticalSection (&(pSH->cshListOpen))
#define csLeave_hListOpen(pSH)	LeaveCriticalSection (&(pSH->cshListOpen))
//
#define csInit_Read(pSH)		InitializeCriticalSection (&(pSH->csRead))
#define csDelete_Read(pSH)		DeleteCriticalSection (&(pSH->csRead))
#define csEnter_Read(pSH)		EnterCriticalSection (&(pSH->csRead))
#define csLeave_Read(pSH)		LeaveCriticalSection (&(pSH->csRead))
//
#define csInit_Write(pSH)		InitializeCriticalSection (&(pSH->csWrite))
#define csDelete_Write(pSH)		DeleteCriticalSection (&(pSH->csWrite))
#define csEnter_Write(pSH)		EnterCriticalSection (&(pSH->csWrite))
#define csLeave_Write(pSH)		LeaveCriticalSection (&(pSH->csWrite))
//
#define csInit_CommEvt(pSH)		InitializeCriticalSection (&(pSH->csCommEvt))
#define csDelete_CommEvt(pSH)	DeleteCriticalSection (&(pSH->csCommEvt))
#define csEnter_CommEvt(pSH)	EnterCriticalSection (&(pSH->csCommEvt))
#define csLeave_CommEvt(pSH)	LeaveCriticalSection (&(pSH->csCommEvt))
//
#define csInit_Rx(pSH)			InitializeCriticalSection (&(pSH->csRx))
#define csDelete_Rx(pSH)		DeleteCriticalSection (&(pSH->csRx))
#define csEnter_Rx(pSH)			EnterCriticalSection (&(pSH->csRx))
#define csLeave_Rx(pSH)			LeaveCriticalSection (&(pSH->csRx))
//
#define csInit_Tx(pSH)			InitializeCriticalSection (&(pSH->csTx))
#define csDelete_Tx(pSH)		DeleteCriticalSection (&(pSH->csTx))
#define csEnter_Tx(pSH)			EnterCriticalSection (&(pSH->csTx))
#define csLeave_Tx(pSH)			LeaveCriticalSection (&(pSH->csTx))
//
//
#define csInit_RxResize(pRx)	InitializeCriticalSection (&((pRx)->csRxBuf))
#define csDelete_RxResize(pRx)	DeleteCriticalSection (&((pRx)->csRxBuf))
#define csEnter_RxResize(pRx)	EnterCriticalSection (&((pRx)->csRxBuf))
#define csLeave_RxResize(pRx)	LeaveCriticalSection (&((pRx)->csRxBuf))

#ifdef __cplusplus
}	
#endif

#endif  //_SERDRV_H_
