/**************************************************************************
* Copyright (c)  ΢�߼�(WEILUOJI). All rights reserved.                   *
**************************************************************************/

#ifndef _SERDRV_H_
#define _SERDRV_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------
//������Ϣ
typedef struct _RXINFO
{
	LONG				nRead;
	LONG				nWrite;
//	LONG				nCounts;

	LONG				nLength;
	LPBYTE				pRxBuf;
	CRITICAL_SECTION	csRxBuf;	//���·���pRxBuf��ַ��Length��С


} RXINFO, *PRXINFO;

//����� �򿪽ṹ
typedef struct _SEROPEN_INFO
{
	//������
	HANDLE_THIS( _SEROPEN_INFO );

	//Driver����Ϣ
	LPVOID				pInfoDrv;

	//����
	LIST_UNIT			hListOpen;

	//���ʲ���
	DWORD				dwAccessCode;
	DWORD				dwShareMode;

	//���ü���
	WORD				wCntRead;
	WORD				wCntWrite;
	WORD				wCntWaitEvent;
	WORD				wCntIOCtl;
	WORD				wCntIntr;
	WORD				wXXX;

	//ͨ���¼�
	CRITICAL_SECTION	csCommEvt;
	HANDLE				hEvtComm;
	BOOL				fAbortEvt;	//��ֹͨ���¼�
	DWORD				dwEvtMask;
	DWORD				dwEvtData;

	//for Braden. ����ʹ��
	BOOL				fClose;

}SEROPEN_INFO, *PSEROPEN_INFO;

//����� ��Ϣ�ṹ
typedef struct _SERDRV_INFO
{
	//������
	HANDLE_THIS( _SERDRV_INFO );

	//���� ARCH
	POBJECT_ARCH		pObjDev;		//�豸����Ԫ
	PVTBL_ARCH_SER		pFunTbl;		//�豸����Ĳ���
    PVOID				pHeadDev;		//�豸�������Ϣ�ṹָ��

	//���д򿪵��б�
	LONG				nOpenCnt;
	LIST_UNIT			hListOpen;
	CRITICAL_SECTION	cshListOpen;
    PSEROPEN_INFO		pAccessOwner;

	BOOL				fOpenByData;	//for Braden. ����ʹ��

	//��������
	DCB					dcb;
	LONG				dwHWOffLim;	//Ӳ����͡�����ʣ��ռ䡱
	LONG				dwHWOnLim;	//Ӳ���㹻������ʣ��ռ䡱

	COMMTIMEOUTS		CommTmouts;

	//ͨ���¼�
	DWORD				dwEvtMask;		//���д򿪵��¼������ܺ�

	//��(COM_Read)����(�ж�)
	CRITICAL_SECTION	csRead;			//����COM_Read����
	HANDLE				hEvtRead;		//��ʱ�ĵȴ��¼�
	RXINFO				InfoRxBuf;		//�жϽ�����Ϣ
	
	CRITICAL_SECTION	csRx;			//���ı�Read��Write��fDataFull��ֵ

	//д(COM_Write)
	CRITICAL_SECTION	csWrite;		//����COM_Write����
	HANDLE				hEvtWrite;

	//��(�ж�)
	LPBYTE				pTxBuf;			//��: buffer
	DWORD				dwTxBytesPending;//��: ʣ��
	DWORD				dwTxBytes;		//��: �Ѿ�
	CRITICAL_SECTION	csTx;			// 

	//�շ� �ı�־
	DWORD				fFlow_RxBuf:1;	//���������ƣ���������ʣ��ռ䡱������ʱ�������Ӳ��֪ͨ�Է�
										//����������fOutX: ���յ�Xoffʱ�����ٷ����ݣ�ֱ����XonΪֹ
	DWORD				fSentXoff:1;	//�Ѿ�����Xoff��֪ͨ�Է�---( ���߶Է�ֹͣ���� )
	DWORD				fSentDtrOff:1;	//�Ѿ���Ӳ��Dtr��֪ͨ�Է�---( ���߶Է�ֹͣ���� )
	DWORD				fSentRtsOff:1;	//�Ѿ���Ӳ��Rts��֪ͨ�Է�---( ���߶Է�ֹͣ���� )

	DWORD				fStopXmit:1;	//��Ϊ�Է��������Ӳ��֪ͨ�������Ѿ�ֹͣ

	DWORD				fAbortRead:1;	//��ֹCOM_Read
//	DWORD				fAbortWrite:1;	//��ֹCOM_Write

	//DWORD				Reserved:28;
	DWORD                dwWritePerm; //д���,��Ϊ�ж��߳���Ҫ���û������ݷ�������ˣ��ж��̱߳�����û������ݽ��ж�д�����

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
