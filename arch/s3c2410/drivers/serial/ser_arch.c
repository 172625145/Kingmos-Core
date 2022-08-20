/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/


/*****************************************************
�ļ�˵������������---Ӳ����ز�
�汾�ţ�  1.0.0
����ʱ�ڣ�2003-07-01
���ߣ�    ФԶ��
�޸ļ�¼��
2004-03-18�����Ǵ��������ǵģ�����ͬ���ӵĵ��Դ��� ����ص�
******************************************************/

//root include
#include <ewindows.h>

//arm\ebook_  include
#include <s2410.h>
#include <oalintr.H>
#include <serbaud.h>	//add by xyg
//driver  include
#include <sertbl.h>		//add by xyg
//current

//added by xyg 2004-09-28
//#include "ser_rwbuf.h"
#include "drv_glob.h"
#include "ser_arch.h"	//add by xyg
//#ifndef CANCEL_XYG_SER_RX
//static DRIVER_GLOBALS * lpGlobal =(DRIVER_GLOBALS *)DRIVER_GLOBALS_PHYSICAL_MEMORY_START;
//#endif


/***************  ȫ���� ���壬 ���� *****************/


//-----------------------------------------
//���Բ��ֵĶ���
//-----------------------------------------

//
//#define	DEBUG_SERARCH	// 

//#define xyg_ser_sub_mask

//
#ifdef	DEBUG_SERARCH

//
static	PSERARCH_INFO	g_pDebugSerArch = NULL;

//���Ժ���
#define	DEBUGTO_NONE		0
#define	DEBUGTO_SERIAL		1
#define	DEBUGTO_DESKTOP		2
extern	DWORD	SerDebug_Enable( DWORD dwFlag );
extern	void	SerDebug_CleanScreen( );
extern	void	OEM_WriteDebugByte_ToWnd(unsigned char c);

extern	void	dcb_Debug( LPDCB lpDCB, LPTSTR pszText );
static	void	Debug_Arch( PVOID pHead );

#endif

//-----------------------------------------
//�����Ķ���
//-----------------------------------------

#define		UART_DEBUGPORT		UART3_BASE_VIRTUAL

//-----------------------------------------
extern	void	msWait(unsigned msVal);
extern	void	usWait(unsigned usVal);
extern	void	ser_NotifyCommEvent(PVOID pHead, ULONG fdwEventMask);
extern	void	ser_HandleInterrupt( LPVOID pData );

//-----------------------------------------
//wait interrupt
static	DWORD	WINAPI	ser_WaitInterrupt( DWORD  pHead );
static	void	Debug_Arch( PVOID pHead );
//help fun
static	BOOL	ser_InitInfoOfHardware( PSERARCH_INFO pInfoSerArch, ULONG Identifier );
static	void	ser_CheckLine(PSERARCH_INFO pInfoSerArch, volatile PS2410_UART_REG pUART);
static	void	ser_OpenMode( PSERARCH_INFO pInfoSerArch );
static	void	ser_CloseMode( PSERARCH_INFO pInfoSerArch );
static	BOOL	ser_CheckDCB( LPDCB lpDCB, PSERARCH_INFO pInfoSerArch );
static	BOOL	ser_CheckFlowOff( PSERARCH_INFO pInfoSerArch );

//init, deinit
static	PVOID	SerArch_Init( ULONG Identifier, PVOID pHeadDrv, PVOID lpDCB );
static	VOID	SerArch_InitLast( PVOID   pHead );
static	BOOL	SerArch_Deinit( PVOID   pHead );
static	ULONG	SerArch_Close( PVOID   pHead );
static	BOOL	SerArch_Open( PVOID pHead );
static	BOOL	SerArch_PowerOff(PVOID pHead);
static	BOOL	SerArch_PowerOn(PVOID pHead);
static	BOOL	SerArch_IOControl(PVOID pHead, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);
//
static	BOOL	SerArch_IREnable(PVOID pHead,ULONG BaudRate);
static	BOOL	SerArch_IRDisable(PVOID pHead);
//
static	ULONG	SerArch_Recv( PVOID pHead, LPBYTE pBufDes, LONG* pnLenBuf );
static	ULONG	SerArch_Send( PVOID pHead, LPBYTE pBufSrc, LONG* pnLenBuf );
static	BOOL	SerArch_XMitChar(PVOID pHead, UCHAR ComChar);
//
static	DWORD	SerArch_IntrTypeQuery( PVOID pHead );
static	DWORD	SerArch_IntrHandleModem(PVOID pHead);
static	VOID	SerArch_IntrHandleLine(PVOID pHead);
static	VOID	SerArch_IntrHandleTx(PVOID pHead);
//
static	VOID	SerArch_ClearPinDTR( PVOID pHead );
static	VOID	SerArch_SetPinDTR( PVOID pHead );
static	VOID	SerArch_ClearPinRTS( PVOID pHead );
static	VOID	SerArch_SetPinRTS( PVOID pHead );
//
static	VOID	SerArch_ClearBreak( PVOID pHead );
static	VOID	SerArch_SetBreak( PVOID pHead );
//
static	VOID	SerArch_PurgeComm(PVOID pHead,DWORD fdwAction);
//
static	ULONG	SerArch_GetComStat(PVOID pHead, LPCOMSTAT lpStat);
static	VOID	SerArch_GetCommProperties(PVOID pHead,LPCOMMPROP pCommProp);
static	VOID	SerArch_GetModemStatus(PVOID pHead, LPDWORD pModemStatus);
static	BOOL	SerArch_SetDCB( PVOID pHead, LPDCB lpDCB, DWORD dwFlagSetDCB );

//-----------------------------------------
//inline function
static	WORD	ser_ReadMSR( PSERARCH_INFO pInfoSerArch );

#if defined( HW_PCB_VERSION_C )
static	void	ser_SelectSIR( PSERARCH_INFO pInfoSerArch );
#endif

/******************************************************/

//-----------------------------------------
// ********************************************************************
// ������WORD	ser_ReadMSR( PSERARCH_INFO pInfoSerArch )
// ������
//	IN pGPIO-GPIO�ĵ�ַ
// ����ֵ��
//	����MODEM��״̬
// ������������ȡMODEM��״̬
// ����: 
// ********************************************************************
WORD	ser_ReadMSR( PSERARCH_INFO pInfoSerArch )
{
	volatile	WORD	msr_ret;
	ULONG       msr_val;
	
	//��Ӳ��
	//msr_ret = 0;
	//if( !pGPIO->gplr.gp14 )
	//{
	//	msr_ret |= MS_CTS_ON;
	//}
	//if( !pGPIO->gplr.gp13 )
	//{
	//	msr_ret |= MS_DSR_ON;
	//	msr_ret |= MS_RLSD_ON;
	//}
	msr_ret = MS_DSR_ON;
	msr_val = INREG(pInfoSerArch,rUMSTAT);
	if (msr_val & COM2410_MSR_CTS)
	{
		msr_ret |= MS_CTS_ON;
	}

	return msr_ret;
}

// ********************************************************************
// ������VOID ClearPendingInts( PVOID   pHead )
// ������
//	IN PVOID pHead - ָ��PDD���ȫ�ֽṹ����
// ����ֵ��
//	��
// �������������üĴ�����ʹ��δ֪״̬Ϊ��֪״̬
// ���ã��� SL_Init( ) �е���
// ********************************************************************
VOID ClearPendingInts( PVOID   pHead )
{
	volatile PSERARCH_INFO   pInfoSerArch   = (PSERARCH_INFO)pHead;
	volatile UINT32 tmpReg;  //2004-12-06�� lilin add volatile, ��ֹ�������Ż�

	//RETAILMSG(1,(TEXT("ClearPendingInts\n")));
	SETREG(pInfoSerArch,rUFCON,0x6);    // tx, rx fifo reset

	tmpReg = INREG(pInfoSerArch,rUERSTAT);		//���ж�֮�󣬸�λ���

	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bTxINT | pInfoSerArch->bRxINT | pInfoSerArch->bErrINT);	//���SUBSRCPND�д��ڵ� Tx,Rx,Err�ж�
	//RETAILMSG(1,(TEXT("**After  ClearSubINTPnd:%x\n"),pInfoSerArch->pIrqCtrlAddr->rSUBSRCPND));
	ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);	//���SRCPND�д����ж�
	//RETAILMSG(1,(TEXT("**SRCPND:%x\n"),pInfoSerArch->UART_INTSRCPND));
	//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//{
	//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;	//#define   BIT_UART0       ( 0x1 << 28 )
	//}
	//RETAILMSG(1,(TEXT("pInfoSerArch->UART_INTPND --- :%x\n"),pInfoSerArch->UART_INTPND));

	tmpReg = INREG(pInfoSerArch,rUERSTAT);		//���ж�֮�󣬸�λ���

}

#if defined( HW_PCB_VERSION_C )
// ********************************************************************
// ������void	ser_SelectSIR( PSERARCH_INFO pInfoSerArch )
// ������
//	IN pInfoSerArch-ARCH��Ϣ
// ����ֵ��
//	��
// ����������ѡ����⹦��
// ����: 
// ********************************************************************
//Select SIR function: set param, open power, open recv and send register
void	ser_SelectSIR( PSERARCH_INFO pInfoSerArch )
{
	//����ģʽ
	WRITE_BITFIELD(struct hscr0Bits,&pInfoSerArch->pHSSP->hscr0,itr,0);
	WRITE_BITFIELD(struct utcr4Bits,&pInfoSerArch->pUART->utcr4,hse,1);
	WRITE_BITFIELD(struct utcr4Bits,&pInfoSerArch->pUART->utcr4,lpm,0);
	//�򿪵�Դ
	pInfoSerArch->pGPIO->gpdr.gp15 = 1; // ��� ����
	pInfoSerArch->pGPIO->gpsr.gp15 = 1; // �򿪵�Դ
	{	DWORD	iDlay;	for( iDlay=0; iDlay<0xA000; iDlay++ ) ;	}
	//��Ӳ����  ��
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,rxe,1);
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,txe,0);
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,rie,1);
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,tie,1);
}
#endif

//-----------------------------------------

// ********************************************************************
// ������void	ser_CheckLine( PSERARCH_INFO pInfoSerArch, volatile PS2410_UART_REG pUART)
// ������
//	IN pInfoSerArch-ARCH��Ϣ
//	IN pUART-UART�ĵ�ַ
// ����ֵ��
//	��
// ������������鴮�ڵ�״̬
// ����: 
// ********************************************************************
//check line status, if error occurs, then notify and clear status
void	ser_CheckLine( PSERARCH_INFO pInfoSerArch, volatile PS2410_UART_REG pUART)
{
	ULONG LineEvents;
	ULONG LineStatus;
//	int nCntTry;	// Deleted By Liujun:Invalid
	UCHAR cCharRx;
	
	//OEM_WriteDebugByte_ToWnd( 'z' );
	//EdbgOutputDebugString("\r\ne_1: sr0=%x, sr1=%x, cr0=%x\r\n", pUART->utsr0, pUART->utsr1, pUART->utcr0 );
	//���ִ���
	LineEvents = 0;
	LineStatus = 0;
//	nCntTry = 0;

	//RETAILMSG(1,TEXT("\r\n SerArch_CheckLine() ---- Debug_Arch \r\n"));
	//Debug_Arch( pInfoSerArch );

//    while( nCntTry++<100 )	// Delete!
	{
		LineStatus = pUART->rUERSTAT;//INREG(pInfoSerArch,rUERSTAT);	//ȡ�Ĵ���״ֵ̬
		//RETAILMSG(1,(TEXT("Line Status Register:%x"),LineStatus));
		//  �жϿ���״̬	[��� | ��ż | ֡] ����
		if ( LineStatus & (COM2410_LSR_OE | COM2410_LSR_PE | COM2410_LSR_FE)) 
		{
			if ( LineStatus & COM2410_LSR_OE )	//�������
			{
				//When overrun error occurs, S2410 rURXH must be read.
				pInfoSerArch->CommErrors |= CE_OVERRUN;
				LineEvents |= EV_ERR;
				//RETAILMSG(1,TEXT("\r\n CLine_ov"));
			}
			if ( LineStatus & COM2410_LSR_PE )	//��ż����
			{
				pInfoSerArch->CommErrors |= CE_RXPARITY;
				LineEvents |= EV_ERR;
				//RETAILMSG(1,TEXT("\r\n CLine_pp"));
			}
			if ( LineStatus & COM2410_LSR_FE )	//֡����
			{
				pInfoSerArch->CommErrors |= CE_FRAME;
				LineEvents |= EV_ERR;
				//RETAILMSG(1,TEXT("\r\n CLine_fr"));
			}
			cCharRx = *(pInfoSerArch->pUFRXH);	// Read the error data
		}
//		else
//		{
//			break;
//		}
	}

	if ( LineStatus & COM2410_LSR_BI )	//�жϴ���
	{
		LineEvents |= EV_BREAK;
		pInfoSerArch->CommErrors |= CE_BREAK;
		//EdbgOutputDebugString("\r\nEV_BREAK\r\n" );
	}
	
	//EdbgOutputDebugString( ",d=%d", nDiscard );
	//EdbgOutputDebugString("\r\ne_2: sr0=%x, sr1=%x, cr0=%x\r\n", pUART->utsr0, pUART->utsr1, pUART->utcr0 );
	if( LineEvents )
	{
		//֪ͨWaitCommEvent
		pInfoSerArch->lpfnEventNotify(pInfoSerArch->pHeadDrv, LineEvents);
	}

	//���Pending�����ж�
	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bErrINT );
	ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
#ifdef xyg_ser_sub_mask
	//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//{
	//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
	//}
	//
	//EnINT(pInfoSerArch, pInfoSerArch->bINT); //2004-12-06 lilin remove,Ӧ����oemintr.c��
	EnSubINT(pInfoSerArch, (pInfoSerArch->bRxINT|pInfoSerArch->bErrINT) );
	//EnSubINT(pInfoSerArch, (pInfoSerArch->bErrINT) );
#endif

	//RETAILMSG(1,TEXT("\r\n SerArch_CheckLine() ---- Debug_Arch OVER\r\n"));
	//Debug_Arch( pInfoSerArch );
}

// ********************************************************************
// ������void	ser_OpenMode( PSERARCH_INFO pInfoSerArch )
// ������
//	IN pInfoSerArch-ARCH��Ϣ
// ����ֵ��
//	��
// �����������ṹ��ʼ�� �� Ӳ����ʼ����
// ����: 
// ********************************************************************
void	ser_OpenMode( PSERARCH_INFO pInfoSerArch )
{
	volatile	PS2410_UART_REG		pUART;//UART-registers

	//��ʼ��������Ϣ�ı���
	pInfoSerArch->fPowerOff  = 0;
	pInfoSerArch->msrChange  = 0;
    pInfoSerArch->CommErrors = 0;
	pInfoSerArch->FlowOffDSR = 0;
	pInfoSerArch->FlowOffCTS = 0;
	pInfoSerArch->RestartTxForFlow = 0;
	//pInfoSerArch->fWaitTxim  = 0;
	//
	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	{
		pInfoSerArch->ModemStatus = ser_ReadMSR( pInfoSerArch );
	}
	//else
	//{
	//	pInfoSerArch->ModemStatus = ( MS_CTS_ON | MS_DSR_ON | MS_RLSD_ON );
	//}

	//
	//Ӳ���Ĳ������ú�״̬���( ע�⣺֮ǰ�Ѿ�������HWSetDCB )
	//
	pUART = pInfoSerArch->pUART;

	// ���ò�����
//	SerArch_SetDCB( pInfoSerArch, pInfoSerArch->lpDCB, SETDCB_ALL );	// �Ƶ�����

	//OUTREG(pInfoSerArch,rUCON,0x2c5);	// Select Clock,Tx and Rx Interrupt Type,Transmit and Receive Mode.
	OUTREG(pInfoSerArch,rUCON,0x02c5);	// Select Clock,Tx and Rx Interrupt Type,Transmit and Receive Mode.
	OUTREG(pInfoSerArch,rUFCON,0x6);		// Reset FIFO
	//OUTREG(pInfoSerArch,rUFCON,0x41);	// FIFO enable : tx-4bytes, rx-4bytes
	OUTREG(pInfoSerArch,rUFCON,0x01);	// FIFO enable : tx-4bytes, rx-4bytes
	//OUTREG(pInfoSerArch,rUMCON,0x00);	// Disable auto flow control.
	OUTREG(pInfoSerArch,rUMCON,0x10);	// Enable auto flow control.
	OUTREG(pInfoSerArch,rULCON,0x3);		// Normal mode, N81

	// ���ò�����
	SerArch_SetDCB( pInfoSerArch, pInfoSerArch->lpDCB, SETDCB_ALL );

	RETAILMSG(1,(TEXT("\n*******************************\n")));
	RETAILMSG(1,(TEXT("**rUFCON:%x\n"),pUART->rUFCON));
	RETAILMSG(1,(TEXT("**rUCON:%x\n"),pUART->rUCON));
	RETAILMSG(1,(TEXT("\n*******************************\n")));

	// �ж���Ч
	EnINT(pInfoSerArch, pInfoSerArch->bINT);
	//RETAILMSG(1,(TEXT("\r\n  [%x]   \r\n"),pInfoSerArch->pIrqCtrlAddr->rINTMSK));
	//RETAILMSG(1,(TEXT("Before EnSubInt function,INTSUBMSK:%x\n"),pInfoSerArch->pIrqCtrlAddr->rINTSUBMSK));
	EnSubINT(pInfoSerArch, (pInfoSerArch->bRxINT|pInfoSerArch->bErrINT));
	//EnSubINT(pInfoSerArch, pInfoSerArch->bRxINT);
	//EnSubINT(pInfoSerArch, pInfoSerArch->bErrINT);

	ser_CheckLine( pInfoSerArch, pInfoSerArch->pUART );
	ClearPendingInts( pInfoSerArch );

/*#ifndef CANCEL_XYG_SER_RX
	EnterCriticalSection(&(pInfoSerArch->csBufRW));
	RWBuf_SetZero( pInfoSerArch->lpRWBuf, TRUE );
	LeaveCriticalSection(&(pInfoSerArch->csBufRW));
#endif
*/
	return ;
}

// ********************************************************************
// ������VOID S2410_SetIrDAIOP( PVOID  pHead )
// ������
//	IN PVOID  pHead - PDD��ȫ�ֽṹ����
// ����ֵ��
//	��
// �������������� IO �˿ڣ�ʹ�����ܹ���Чʹ��
// ���ã���SL_Init( )�е���
// ********************************************************************
VOID S2410_SetIrDAIOP( PVOID  pHead )
{
	volatile PSERARCH_INFO   pInfoSerArch   = (PSERARCH_INFO)pHead;

	switch( pInfoSerArch->dwIndex )
	{
	case ID_COM1:
		pInfoSerArch->pGPIO->rGPHCON &= ~(0x3<<0 | 0x3<<2 | 0x3<<4 | 0x3<<6); // clear uart 0 - rx, tx, rts, cts
		pInfoSerArch->pGPIO->rGPHCON |= (0x2<<0 | 0x2<<2 | 0x2<<4 | 0x2<<6); 
		pInfoSerArch->pGPIO->rGPHUP  |= (0x1<<0 | 0x1<<1 | 0x1<<2 | 0x1<<3);
		break;
	case ID_COM2:
		pInfoSerArch->pGPIO->rGPHCON &= ~((3 << 8) | (3 << 10));	// Configure GPH2 and GHP3 for UART1 Tx and Rx, respectively.
		pInfoSerArch->pGPIO->rGPHCON |=  ((2 << 8) | (2 << 10));	// Enable GHP3 for RXD0 and GHP2 for TXD0
		pInfoSerArch->pGPIO->rGPHUP |= (1 << 4) | (1 << 5);						// pull-up functio
		break;
	}
}

// ********************************************************************
// ������void	ser_CloseMode( PSERARCH_INFO pInfoSerArch )
// ������
//	IN pInfoSerArch-ARCH��Ϣ
// ����ֵ��
//	��
// ����������Ӳ���ر�
// ����: 
// ********************************************************************
//close hardware function
void	ser_CloseMode( PSERARCH_INFO pInfoSerArch )
{
/*
#ifndef CANCEL_XYG_SER_RX
	EnterCriticalSection(&(pInfoSerArch->csBufRW));
	RWBuf_SetZero( pInfoSerArch->lpRWBuf, FALSE );
	LeaveCriticalSection(&(pInfoSerArch->csBufRW));
#endif
*/

	//EnterCriticalSection(&(pInfoSerArch->RegCritSec));

	//�ر�MODEM����
	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	{
		SerArch_ClearPinRTS( pInfoSerArch );
	}

	// Disable all interrupts and clear MCR.
	DisEnSubINT(pInfoSerArch, (pInfoSerArch->bRxINT|pInfoSerArch->bTxINT|pInfoSerArch->bErrINT));
	pInfoSerArch->fSW_EnTxINT = FALSE;

	// This routhine for auto detect.
	S2410_SetIrDAIOP(pInfoSerArch);	//����Ӳ���ָ�����ʼ��״̬

	//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
}

//-----------------------------------------
// ********************************************************************
// ������PVOID	SerArch_Init( ULONG Identifier, PVOID pHeadDrv, PVOID lpDCB )
// ������
//	IN Identifier-�ϲ㴫�ݵĲ���
//	IN pHeadDrv-�ϲ㴫�ݵĲ���
//	IN lpDCB-�ϲ㴫�ݵĲ���
// ����ֵ��
//	��
// ����������Ӳ����ʼ��
// ����: 
// ********************************************************************
PVOID	SerArch_Init( ULONG Identifier, PVOID pHeadDrv, PVOID lpDCB )
{
    PSERARCH_INFO	pInfoSerArch;

    RETAILMSG(1,TEXT("\r\n++++++++++SerArch_Init Start!++++++++++++\r\n"));
	//����ṹ
    pInfoSerArch = (PSERARCH_INFO)malloc( sizeof(SERARCH_INFO) );
	if( pInfoSerArch==NULL )
		return NULL;
	memset( pInfoSerArch, 0, sizeof(SERARCH_INFO) );

	//��ʼ��---
	InitializeCriticalSection(&(pInfoSerArch->csTransmit));
	//InitializeCriticalSection(&(pInfoSerArch->RegCritSec));

	//��ʼ��---Ӳ����Ϣ
	if( ser_InitInfoOfHardware(pInfoSerArch, Identifier) )//��֤Ӳ������
	{
		//��ʼ��---�ϲ���Ϣ
		pInfoSerArch->pHeadDrv = pHeadDrv;
	    pInfoSerArch->lpfnEventNotify = ser_NotifyCommEvent;
		//��ʼ��---Ӳ��
		//if( pInfoSerArch->dwIoBase!=UART_DEBUGPORT )//���ԵĴ���
		{
			ser_CloseMode( pInfoSerArch );
		}

		//��ʼ��---Ӳ��
		pInfoSerArch->dwOpenCount= 0;
		pInfoSerArch->lpDCB    = (DCB*)lpDCB;
		//pInfoSerArch->fWaitTxim  = 0;
		pInfoSerArch->hEvtXMit   = CreateEvent(0, FALSE, FALSE, NULL);

		
		//��ʼ��---�ж�ISR
		pInfoSerArch->hEvtISR = CreateEvent( NULL, FALSE, FALSE, NULL );
		ClearPendingInts( pInfoSerArch );  // 2004-12-06, lilin add , ������е�״̬/pending
		INTR_Init( pInfoSerArch->dwISR, pInfoSerArch->hEvtISR, NULL, 0 );//pInfoDrv->pObjDev->dwIntIDע���ж�
		INTR_Disable( pInfoSerArch->dwISR );//pInfoDrv->pObjDev->dwIntIDע���ж�
		//ResetEvent( pInfoSerArch->hEvtISR );	// 2004-12-06, remove, û�б�Ҫ

		//if( pInfoSerArch->pObjDev->BindFlags==THREAD_AT_INIT )
		pInfoSerArch->hThrdISR = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ser_WaitInterrupt, (LPVOID)pInfoSerArch, 0, &pInfoSerArch->dwThrdISR );
		if( !pInfoSerArch->hThrdISR )
		{
			SerArch_Deinit( pInfoSerArch );
			return NULL;
		}
		SetThreadPriority( pInfoSerArch->hThrdISR, THREAD_PRIORITY_TIME_CRITICAL );
		//Sleep( 100 ); //2004-12-06 û�б�Ҫ
		RETAILMSG(1,TEXT("\r\nSerArch_Init() over\r\n"));
		
		//SetThreadPolicy( pInfoSerArch->hThrdISR, STP_ROTATION );//2004-03-18

		//
#ifdef	DEBUG_SERARCH
		g_pDebugSerArch = pInfoSerArch;
#endif

#ifdef	DEBUG_SERARCH
		RETAILMSG(1,TEXT("\r\n SerArch_Init() ---- Debug_Arch \r\n"));
	Debug_Arch( pInfoSerArch );
#endif
		return pInfoSerArch;
	}

	free( pInfoSerArch );
	return NULL;
}

// ********************************************************************
// ������DWORD	WINAPI	ser_WaitInterrupt( DWORD  pHead )
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	0
// ����������Ӳ���жϷ�������
// ����: 
// ********************************************************************
//interrupt service route
static DRIVER_GLOBALS * lpGlobal =(DRIVER_GLOBALS *)DRIVER_GLOBALS_PHYSICAL_MEMORY_START;

DWORD	WINAPI	ser_WaitInterrupt( DWORD  pHead )
{
    PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;

	//Sleep( 10 ); // 2004-12-06, lilin remove, û�б�Ҫ
	//SetThreadPriority( pInfoSerArch->hThrdISR, THREAD_PRIORITY_TIME_CRITICAL );	// 2004-12-06, lilin remove, ��CreateThread�Ѿ�����
	while( !pInfoSerArch->fExitISR )
	{
		WaitForSingleObject( pInfoSerArch->hEvtISR, INFINITE );
		if( pInfoSerArch->fExitISR )
			break;

		//if( lpGlobal->dwCurTickCount - lpGlobal->dwIntTickCount > 1 )
		//{
			//RETAILMSG( 1, ( "diff=%d.\r\n",  lpGlobal->dwCurTickCount - lpGlobal->dwIntTickCount ) );
		//}


		ser_HandleInterrupt( pInfoSerArch->pHeadDrv );
		INTR_Done( pInfoSerArch->dwISR );
	}
	return 0;
}

// ********************************************************************
// ������BOOL	ser_InitInfoOfHardware( PSERARCH_INFO pInfoSerArch, ULONG Identifier )
// ������
//	IN pInfoSerArch-ARCH��Ϣ
//	IN Identifier-�ϲ㴫�ݵĲ���
// ����ֵ��
//	�ɹ�����TRUE
// �����������˺�����ʼ������Ӳ����ַ�͹���(��ȡע��� ����ֱ�ӳ�ʼ��ָ��)����֤Ӳ������
// ����: 
// ********************************************************************
//init hardware information
BOOL	ser_InitInfoOfHardware( PSERARCH_INFO pInfoSerArch, ULONG Identifier )
{
	//ָ��Ӳ����ָ��͹���
	switch( Identifier )
	{
	case ID_COM1://
		//����Ӳ��������Ϣ
		pInfoSerArch->dwIndex= ID_COM1;
		pInfoSerArch->dwIoBase= UART0_BASE;//2004-03-18
		pInfoSerArch->dwISR  = SYSINTR_SERIAL;

		//pInfoSerArch->dwComFun	= COM_UART;
//		pInfoSerArch->pBaudTbl= (BAUDTBL *)&LS_BaudTable;

		//pInfoSerArch->dwOpt |= OPT_MODEM;//2004-03-18

		pInfoSerArch->bINT		= BIT_UART0;	//#define   BIT_UART0       ( 0x1 << 28 )
		pInfoSerArch->bTxINT	= INTSUB_TXD0;	//#define	INTSUB_TXD0		( 0x1 << 1  )
		pInfoSerArch->bRxINT	= INTSUB_RXD0;	//#define 	INTSUB_RXD0		( 0x1 << 0  )
		pInfoSerArch->bErrINT	= INTSUB_ERR0;	//#define	INTSUB_ERR0		( 0x1 << 2  )

/*
#ifndef CANCEL_XYG_SER_RX
		pInfoSerArch->lpRWBuf = &lpGlobal->stSerRx0;
		InitializeCriticalSection(&(pInfoSerArch->csBufRW));
#endif
*/
		break;

	case ID_COM2:
		//����Ӳ��������Ϣ
		pInfoSerArch->dwIndex= ID_COM2;
		pInfoSerArch->dwIoBase= UART1_BASE;//2004-03-18
		pInfoSerArch->dwISR  = SYSINTR_IR;

		//pInfoSerArch->dwComFun	= COM_UART;
//		pInfoSerArch->pBaudTbl= (BAUDTBL *)&LS_BaudTable;

		//pInfoSerArch->dwOpt |= OPT_MODEM;//2004-03-18

		pInfoSerArch->bINT		= BIT_UART1;	//#define   BIT_UART0       ( 0x1 << 28 )
		pInfoSerArch->bTxINT	= INTSUB_TXD1;	//#define	INTSUB_TXD0		( 0x1 << 1  )
		pInfoSerArch->bRxINT	= INTSUB_RXD1;	//#define 	INTSUB_RXD0		( 0x1 << 0  )
		pInfoSerArch->bErrINT	= INTSUB_ERR1;	//#define	INTSUB_ERR0		( 0x1 << 2  )

/*
#ifndef CANCEL_XYG_SER_RX
		pInfoSerArch->lpRWBuf = &lpGlobal->stSerRx1;
		InitializeCriticalSection(&(pInfoSerArch->csBufRW));
#endif
*/
		break;

	default ://�����Ӳ�������ڣ���ȡ��
		return FALSE;
	}

	//�豸����
	pInfoSerArch->CommProp.wPacketLength      = 0xffff;
	pInfoSerArch->CommProp.wPacketVersion     = 0xffff;
	pInfoSerArch->CommProp.dwServiceMask      = SP_SERIALCOMM;
	pInfoSerArch->CommProp.dwMaxBaud          = BAUD_115200;
	pInfoSerArch->CommProp.dwProvSubType      = PST_RS232;
	pInfoSerArch->CommProp.dwSettableBaud     =
		BAUD_075  | BAUD_110 | BAUD_150 | BAUD_300 | BAUD_600 |
		BAUD_1200 | BAUD_1800 | BAUD_2400 | BAUD_4800 | BAUD_7200 | 
		BAUD_9600 | BAUD_14400 | BAUD_19200  | BAUD_38400 | 
		BAUD_56K | BAUD_128K | BAUD_115200 | BAUD_57600;
	pInfoSerArch->CommProp.wSettableData      =	DATABITS_7 | DATABITS_8;
	pInfoSerArch->CommProp.wSettableStopParity=	PARITY_NONE | PARITY_ODD | PARITY_EVEN;
	pInfoSerArch->CommProp.dwProvCapabilities =	
		PCF_SETXCHAR |PCF_SPECIALCHARS |PCF_XONXOFF |PCF_PARITY_CHECK
		|PCF_INTTIMEOUTS |PCF_TOTALTIMEOUTS;
	pInfoSerArch->CommProp.dwSettableParams   =
		SP_BAUD | SP_DATABITS | SP_HANDSHAKING | SP_PARITY | 
		SP_PARITY_CHECK | SP_RLSD | SP_STOPBITS;
	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	{
		//��� Ӳ������
		//pInfoSerArch->CommProp.dwProvCapabilities |= PCF_DTRDSR |PCF_RLSD |PCF_RTSCTS;
		pInfoSerArch->CommProp.dwProvCapabilities |= PCF_RTSCTS;
	}

	//�󶨵������ַ
	pInfoSerArch->pUART = (volatile PS2410_UART_REG)pInfoSerArch->dwIoBase;
	pInfoSerArch->pGPIO = (volatile IOPreg *)IOP_BASE;
	pInfoSerArch->pIrqCtrlAddr = (volatile INTreg*)INT_BASE;

	// ���ݼĴ���
	pInfoSerArch->pUFTXH = (volatile unsigned char *)&(pInfoSerArch->pUART->rUTXH);	//����Ĵ���
	pInfoSerArch->pUFRXH = (volatile unsigned char *)&(pInfoSerArch->pUART->rURXH);	//���ռĴ���

	// �����жϼĴ���
	pInfoSerArch->UART_INTMASK		= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rINTMSK);
	pInfoSerArch->UART_INTSUBMASK	= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rINTSUBMSK);
	pInfoSerArch->UART_INTPND		= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rINTPND);
	pInfoSerArch->UART_INTSRCPND		= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rSRCPND);
	pInfoSerArch->UART_INTSUBSRCPND	= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rSUBSRCPND);

	//osm2 timer: �ɹر�״̬������ʼ��
	//WRITE_BITFIELD(struct icregBits,&pInfoSerArch->pIrqCtrlAddr->icmr,osmr2,0);
	//WRITE_BITFIELD(struct icregBits,&(pInfoSerArch->pIrqCtrlAddr)->iclr,osmr2,0);
	//IOW_REG_BITWRITE (struct matchRegBits, &(pInfoSerArch->pOSTReg)->ossr,m2,1);
	//WRITE_BITFIELD(struct matchRegBits, &(pInfoSerArch->pOSTReg)->oier,m2,0);

	pInfoSerArch->pBaudTbl		= (BAUDTBL *)&LS_BaudTable;//����baud��һ��
	pInfoSerArch->dwSizeTbl		= BAUD_TABLE_SIZE;//����baud��һ��

	return TRUE;
}

//reserved for the last init hardware
VOID	SerArch_InitLast( PVOID   pHead )
{
}

// ********************************************************************
// ������BOOL	SerArch_Deinit( PVOID   pHead )
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	�ɹ�����TRUE
// ���������������ͷ�
// ����: 
// ********************************************************************
//the entry of deinit
BOOL	SerArch_Deinit( PVOID   pHead )
{
    PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

    if(!pInfoSerArch)
	{
        return(FALSE);
	}

    if(pInfoSerArch->dwOpenCount)
	{
		SerArch_Close(pHead);
	}
	//
    DeleteCriticalSection(&(pInfoSerArch->csTransmit));
	if( pInfoSerArch->hEvtXMit )
		CloseHandle( pInfoSerArch->hEvtXMit );
	//
	pInfoSerArch->fExitISR = TRUE;
	SetEvent( pInfoSerArch->hEvtISR );
	Sleep( 10 );
	if( pInfoSerArch->hEvtISR )
		CloseHandle( pInfoSerArch->hEvtISR );
	if( pInfoSerArch->hThrdISR )
		CloseHandle( pInfoSerArch->hThrdISR );

    free(pInfoSerArch);
    return(TRUE);
}

//-----------------------------------------
// ********************************************************************
// ������ULONG	SerArch_Close( PVOID   pHead )
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	0
// �������������ڹر�
// ����: 
// ********************************************************************
ULONG	SerArch_Close( PVOID   pHead )
{
    PSERARCH_INFO   pInfoSerArch = (PSERARCH_INFO)pHead;
    ULONG  uTries;
    
    if( pInfoSerArch->dwOpenCount )
    {
        pInfoSerArch->dwOpenCount--;

        // while we are still transmitting, sleep.
		while((pInfoSerArch->pUART->rUFSTAT & SER2410_FIFOSTAT_MASK) & (uTries++ < 100))
		{	// TxFifo not empty..
			Sleep(10);
		}

		//close hardware
		if( pInfoSerArch->dwOpenCount==0 )
		{
			//RETAILMSG( 1, (TEXT("xyg call ser_CloseMode\r\n")) );
			//if( pInfoSerArch->dwIoBase!=UART_DEBUGPORT )//because it is debug serial
				ser_CloseMode( pInfoSerArch );
		}
	}

//#ifdef	DEBUG_SERARCH
//	SerDebug_Enable( DEBUGTO_SERIAL );
//#endif
    return(0);
}

// ********************************************************************
// ������static void SerSetOutputMode( PSERARCH_INFO pInfoSerArch, BOOL UseIR )
// ������
//	IN PSERARCH_INFO pInfoSerArch - PDD��ȫ�ֽṹ
//	IN BOOL UseIR - �����ߣ�NONE��
// ����ֵ��
//	��
// �����������Ƿ�ʹ�ú����ߣ�û�к����߹��ܣ�
// ���ã��� SerArch_Init( ) �е���
// ********************************************************************
static void SerSetOutputMode( PSERARCH_INFO pInfoSerArch, BOOL UseIR )
{
	volatile	PSERARCH_INFO   pHWHead2   = (PSERARCH_INFO)pInfoSerArch;

	if (UseIR)
	{
		CLEARREG(pHWHead2,rULCON,SER2410_IRMODE_MASK);   // Infra-red mode enable.
	}
	else
	{
		CLEARREG(pHWHead2,rULCON,SER2410_IRMODE_MASK);   // Infra-red mode disable.
	}
}

// ********************************************************************
// ������BOOL	SerArch_Open( PVOID pHead )
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	0
// �������������ڴ�
// ����: 
// ********************************************************************
BOOL	SerArch_Open( PVOID pHead )
{
    PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

//	SerDebug_Enable( DEBUGTO_DESKTOP );
#ifdef	DEBUG_SERARCH
	//SerDebug_CleanScreen( );
#endif

	EdbgOutputDebugString( "SerArch_Open begin\r\n");
	//RETAILMSG( 1, (TEXT("\r\n ....  SerArch_Open begin ....\r\n")));
    if( pInfoSerArch->dwOpenCount )
	{
        return(FALSE);
	}
    pInfoSerArch->dwOpenCount++;

	SerSetOutputMode( pInfoSerArch, FALSE );	//����ʹ�ô���ģʽ�����κ�����
	//
	ser_OpenMode( pInfoSerArch );

	RETAILMSG( 1, (TEXT("\r\n ....  SerArch_Open end ....\r\n")));
#ifdef	DEBUG_SERARCH
		RETAILMSG(1,TEXT("\r\n SerArch_Open() ---- Debug_Arch \r\n"));
	Debug_Arch( pInfoSerArch );
#endif

	return(TRUE);
}

// ********************************************************************
// ������BOOL	SerArch_PowerOff(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	TRUE
// �������������ڹص�Դ
// ����: 
// ********************************************************************
BOOL	SerArch_PowerOff(PVOID pHead)
{
	PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

	if( pInfoSerArch->dwOpenCount )
	{
		//if( pInfoSerArch->dwOpt & OPT_MODEM )
		//{
		//	//pInfoSerArch->DTRPin = pInfoSerArch->pGPIO->gplr.gp10;
		//	//pInfoSerArch->RTSPin = pInfoSerArch->pGPIO->gplr.gp11;
		//}
		pInfoSerArch->fPowerOff = 1;
	}
	ser_CloseMode( pInfoSerArch );

	return TRUE;
}

// ********************************************************************
// ������BOOL	SerArch_PowerOn(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	TRUE
// �������������ڿ���Դ
// ����: 
// ********************************************************************
BOOL	SerArch_PowerOn(PVOID pHead)
{
	PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

	if( pInfoSerArch->fPowerOff )
	{
		ser_OpenMode( pInfoSerArch );
		//
		//if( pInfoSerArch->dwOpt & OPT_MODEM )
		//{
		//	pInfoSerArch->pGPIO->gpsr.gp10 = pInfoSerArch->DTRPin;
		//	pInfoSerArch->pGPIO->gpsr.gp11 = pInfoSerArch->RTSPin;
		//}
	}
	return TRUE;
}

//------------------------------------------------------------------------------
// ********************************************************************
// ������VOID	SerArch_PurgeComm(PVOID pHead,DWORD fdwAction)
// ������
//	IN pHead-ARCH��Ϣ
//	IN fdwAction-����
// ����ֵ��
//	��
// ����������PURGE����
// ����: 
// ********************************************************************
VOID	SerArch_PurgeComm(PVOID pHead,DWORD fdwAction)
{
	PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;
	//volatile PS2410_UART_REG pUART;

	//pUART = pInfoSerArch->pUART;
	//if( (fdwAction & PURGE_RXCLEAR) || (fdwAction & PURGE_RXABORT) )
	//{
	//	WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,rie,0);
	//	WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,rxe,0);
	//	WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,rxe,1);
	//	WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,rie,1);
	//}
	//if( (fdwAction & PURGE_TXCLEAR) || (fdwAction & PURGE_TXABORT) )
	//{
	//	WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,txe, 0);
	//	WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,txe, 1);
	//}

	if (fdwAction & (PURGE_TXABORT | PURGE_TXCLEAR))
	{
		SETREG(pInfoSerArch,rUFCON,0x4);	//Tx FIFO reset
		CLEARREG(pInfoSerArch,rUFCON,0x4);	//Tx FIFO Normal
	}
	if (fdwAction & (PURGE_RXABORT | PURGE_RXCLEAR))
	{
		SETREG(pInfoSerArch,rUFCON,0x2);	//Rx FIFO reset
		CLEARREG(pInfoSerArch,rUFCON,0x2);	//Rx FIFO Normal		;
	}

//#ifdef	DEBUG_SERARCH
//	Debug_Arch( );
//#endif
}
// ********************************************************************
// ������VOID SerArch_ClearPinDTR(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ����������clear modem pin
// ����: 
// ********************************************************************
VOID SerArch_ClearPinDTR(PVOID pHead)
{
	//PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	//{
	//	//pInfoSerArch->pGPIO->gpsr.gp10 = 1;
	//	EnterCriticalSection(&(pInfoSerArch->RegCritSec));
	//	{
	//		// Low active pin.
	//		//*(pInfoSerArch->rDTRport) |= (1<<(pInfoSerArch->DtrPortNum));
	//	}
	//	LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
	//}
}
// ********************************************************************
// ������VOID SerArch_SetPinDTR(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ����������set modem pin
// ����: 
// ********************************************************************
VOID SerArch_SetPinDTR(PVOID pHead)
{
	//PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	//{
	//	//pInfoSerArch->pGPIO->gpcr.gp10 = 1;
	//	EnterCriticalSection(&(pInfoSerArch->RegCritSec));
	//	{
	//		// Low active
	//		*(pInfoSerArch->rDTRport) &= ~(1<<(pInfoSerArch->DtrPortNum));
	//	}
	//	LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
	//}
}
// ********************************************************************
// ������VOID SerArch_ClearPinRTS(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ����������clear modem pin
// ����: 
// ********************************************************************
VOID SerArch_ClearPinRTS(PVOID pHead)
{
	PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	{
		//pInfoSerArch->pGPIO->gpsr.gp11 = 1;
		CLEARREG(pInfoSerArch, rUMCON, SER2410_RTS);
	}
}
// ********************************************************************
// ������VOID SerArch_SetPinRTS(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ����������set modem pin
// ����: 
// ********************************************************************
VOID SerArch_SetPinRTS(PVOID pHead)
{
	PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	{
		//pInfoSerArch->pGPIO->gpcr.gp11 = 1;
		//EnterCriticalSection(&(pInfoSerArch->RegCritSec));
		{
			SETREG(pInfoSerArch, rUMCON, SER2410_RTS);
		}
		//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
	}
}

// ********************************************************************
// ������VOID SerArch_ClearBreak(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ����������clear break pin
// ����: 
// ********************************************************************
VOID SerArch_ClearBreak(PVOID pHead)
{
	PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;

	//pInfoSerArch->pUART->utcr3.brk = 0;
	//EnterCriticalSection(&(pInfoSerArch->RegCritSec));
	{
		CLEARREG(pInfoSerArch,rUCON,BS_SEND);
	}
	//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
}

// ********************************************************************
// ������VOID	SerArch_SetBreak(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ����������set break pin
// ����: 
// ********************************************************************
//���ܣ����±��ز��ܷ�������(ԭ�� pUART->utsr0==0  and  pUART->utsr1==0 )
VOID	SerArch_SetBreak(PVOID pHead)
{
	PSERARCH_INFO   pInfoSerArch=(PSERARCH_INFO)pHead;

	EnterCriticalSection(&(pInfoSerArch->csTransmit)); // This will stop xmit action
	//EnterCriticalSection(&(pInfoSerArch->RegCritSec));
	{
		SETREG(pInfoSerArch,rUCON,BS_SEND);
	}
	//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
	LeaveCriticalSection(&(pInfoSerArch->csTransmit));
}

//-----------------------------------------
// ********************************************************************
// ������BOOL	ser_CheckDCB( LPDCB lpDCB, PSERARCH_INFO pInfoSerArch )
// ������
//	IN lpDCB-DCB��Ϣ
//	IN pInfoSerArch-ARCH��Ϣ
// ����ֵ��
//	��
// ����������check dcb wether is validate
// ����: 
// ********************************************************************
BOOL	ser_CheckDCB( LPDCB lpDCB, PSERARCH_INFO pInfoSerArch )
{
	PBAUDTBL	pTbl;
	DWORD		dwSizeTbl;
	DWORD		index;

	DWORD		BaudRate;
	DWORD		ByteSize;
	DWORD		Parity;
	DWORD		StopBits;
	
	//
	BaudRate = lpDCB->BaudRate;
	ByteSize = lpDCB->ByteSize;
	Parity   = lpDCB->Parity  ;
	StopBits = lpDCB->StopBits;

	pTbl = pInfoSerArch->pBaudTbl;
	dwSizeTbl = pInfoSerArch->dwSizeTbl;
	for( index=0; index<dwSizeTbl; index++ )
	{
		if( BaudRate==pTbl[index].dwBaud )
			break;
	}
	if( index<dwSizeTbl )//baud
	{
		if( ByteSize==8 || ByteSize==7 )//
		{
			if( Parity==NOPARITY || Parity==EVENPARITY || Parity==ODDPARITY )//
			{
				if( StopBits==TWOSTOPBITS || StopBits==ONESTOPBIT )//
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

// ********************************************************************
// ������BOOL	SerArch_SetDCB( PVOID pHead, LPDCB lpDCB, DWORD dwFlagSetDCB )
// ������
//	IN pHead-ARCH��Ϣ
//	IN lpDCB-DCB��Ϣ
//	IN dwFlagSetDCB-����DCB�ı�־
// ����ֵ��
//	�ɹ�����TRUE
// �������������óɹ��Ĳ��ֲ���lpDCB��ʧ�ܵĲ��ֲ���ԭĬ��ֵ
// ����: 
// ********************************************************************
//set dcb info into hardware/function
BOOL	SerArch_SetDCB( PVOID pHead, LPDCB lpDCB, DWORD dwFlagSetDCB )
{
	PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	volatile	PS2410_UART_REG	pUART;//UART-registers
	BOOL		bRet;

	if(pInfoSerArch->dwOpenCount)
	{
		pUART = pInfoSerArch->pUART;
		//
		if( ser_CheckDCB(lpDCB, pInfoSerArch) )
		{
			//EnterCriticalSection(&(pInfoSerArch->RegCritSec));
			//baudrate
			if( dwFlagSetDCB & SETDCB_BAUD )
			{
				RETAILMSG(1,(TEXT("\r\n���ò����ʺ�:%d\r\n"),lpDCB->BaudRate));
				OUTREG(pInfoSerArch,rUBRDIV,( (int)( S2410PCLK / (lpDCB->BaudRate * 16) ) -1));
				RETAILMSG(1,(TEXT("\r\n���ò����ʺ�:%x\r\n"),pInfoSerArch->pUART->rUBRDIV));
			}
			//ByteSize
			if( dwFlagSetDCB & SETDCB_BYTESIZE )
			{
				UINT32 lcr;
				bRet = TRUE;
				//if( ByteSize==8 || ByteSize==7 )
				//WRITE_BITFIELD(struct utcr0Bits, &pUART->utcr0, dss, (lpDCB->ByteSize==8 ? 1 : 0));
				{
					lcr = (UINT32)INREG(pInfoSerArch,rULCON);
					lcr &= ~SER2410_DATABIT_MASK;
					
					switch ( lpDCB->ByteSize ) 
					{
					case 5:
						lcr |= 0;//SERIAL_5_DATA;
						break;
					case 6:
						lcr |= 1;//SERIAL_6_DATA;
						break;
					case 7:
						lcr |= 2;//SERIAL_7_DATA;
						break;
					case 8:
						lcr |= 3;//SERIAL_8_DATA;
						break;
					default:
						bRet = FALSE;
						break;
					}
					if (bRet)
						OUTREG(pInfoSerArch,rULCON,lcr);	//��������λ��Ӳ��
				}
			}
			//Parity
			if( dwFlagSetDCB & SETDCB_PARITY )
			{
				UINT32 lcr;
				bRet = TRUE;
				//if( Parity==NOPARITY || Parity==EVENPARITY || Parity==ODDPARITY )
				//WRITE_BITFIELD(struct utcr0Bits, &pUART->utcr0, oes, (lpDCB->Parity==EVENPARITY ? 1 : 0));
				//WRITE_BITFIELD(struct utcr0Bits, &pUART->utcr0, pe,  (lpDCB->fParity));//lpDCB->Parity==NOPARITY ? 0 : 1
				{
					lcr = (UINT32)INREG(pInfoSerArch,rULCON);      
					lcr &= ~SER2410_PARITY_MASK;	//0x38
					
					switch ( lpDCB->Parity )
					{
					case ODDPARITY:
						lcr |= 0x20;	//SERIAL_ODD_PARITY;
						break;
					case EVENPARITY:
						lcr |= 0x28;	//SERIAL_EVEN_PARITY;
						break;
					case MARKPARITY:
						lcr |= 0x30;	//SERIAL_MARK_PARITY;
						break;
					case SPACEPARITY:
						lcr |= 0x38;	//SERIAL_SPACE_PARITY;
						break;
					case NOPARITY:
						lcr |= 0;		//SERIAL_NONE_PARITY;
						break;
					default:
						bRet = FALSE;
						break;
					}
					if (bRet)
						OUTREG(pInfoSerArch,rULCON,lcr) ;
				}
			}
			//StopBits
			if( dwFlagSetDCB & SETDCB_STOPBITS )
			{
				UINT32 lcr;
				bRet = TRUE;
				//if( StopBits==TWOSTOPBITS || StopBits==ONESTOPBIT )
				//WRITE_BITFIELD(struct utcr0Bits, &pUART->utcr0, sbs, (lpDCB->StopBits==TWOSTOPBITS ? 1 : 0));
				{
					lcr = INREG(pInfoSerArch,rULCON);
					lcr &= ~SER2410_STOPBIT_MASK;	//0x4
					
					switch ( lpDCB->StopBits ) 
					{
					case ONESTOPBIT :
						lcr |= 0;//SERIAL_1_STOP ;
						break;
					case ONE5STOPBITS :
						//lcr |= SERIAL_1_5_STOP ;
						//break;
					case TWOSTOPBITS :
						lcr |= 4;//SERIAL_2_STOP ;
						break;
					default:
						bRet = FALSE;
						break;
					}
					if (bRet)
						OUTREG(pInfoSerArch,rULCON,lcr);
				}
			}
			//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
			//RETAILMSG( 1, (TEXT( "\r\nser_SetBaud: baud=%d, pUart=0x[%x]\r\n" ), lpDCB->BaudRate, pUART));

			//�жϲ�����Ӳ���ź�---disable, enable, or nothing
			//if( pInfoSerArch->dwOpt & OPT_MODEM )
			{
				if ( lpDCB->fDtrControl == DTR_CONTROL_DISABLE ) 
					SerArch_ClearPinDTR( pInfoSerArch );
				else if ( lpDCB->fDtrControl == DTR_CONTROL_ENABLE ) 
					SerArch_SetPinDTR( pInfoSerArch );
				if ( lpDCB->fRtsControl == RTS_CONTROL_DISABLE ) 
					SerArch_ClearPinRTS( pInfoSerArch );
				else if ( lpDCB->fRtsControl == RTS_CONTROL_ENABLE ) 
					SerArch_SetPinRTS( pInfoSerArch );
			}
			//dcb_Debug( lpDCB, "SerArch_SetDCB" );
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

//-----------------------------------------
// ********************************************************************
// ������ULONG	SerArch_GetComStat(PVOID pHead, LPCOMSTAT lpStat)
// ������
//	IN pHead-ARCH��Ϣ
//	OUT lpStat-����״̬
// ����ֵ��
//	���ش�����Ϣ
// ������������ȡMODEM״̬
// ����: 
// ********************************************************************
ULONG	SerArch_GetComStat(PVOID pHead, LPCOMSTAT lpStat)
{
	PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	ULONG			RetVal;
	
	RetVal = pInfoSerArch->CommErrors;
	pInfoSerArch->CommErrors = 0;//Clear old errors each time
	//
	if (lpStat)
	{
		lpStat->fDsrHold  = pInfoSerArch->FlowOffDSR;
		lpStat->fCtsHold  = pInfoSerArch->FlowOffCTS;
		//pInfoSerArch->CommStat = 0;
		//pInfoSerArch->CommStat.fEof = 0;
	}
	return RetVal;
}
// ********************************************************************
// ������VOID	SerArch_GetCommProperties(PVOID pHead,LPCOMMPROP pCommProp)
// ������
//	IN pHead-ARCH��Ϣ
//	OUT pCommProp-����������Ϣ
// ����ֵ��
//	��
// ������������ȡ������Ϣ
// ����: 
// ********************************************************************
VOID	SerArch_GetCommProperties(PVOID pHead,LPCOMMPROP pCommProp)
{
	*pCommProp = ((PSERARCH_INFO)pHead)->CommProp;
}
// ********************************************************************
// ������VOID SerArch_GetModemStatus(PVOID pHead, LPDWORD pModemStatus)
// ������
//	IN pHead-ARCH��Ϣ
//	OUT pModemStatus-����modem״̬��Ϣ
// ����ֵ��
//	��
// ������������ȡmodem״̬��Ϣ
// ����: 
// ********************************************************************
VOID SerArch_GetModemStatus(PVOID pHead, LPDWORD pModemStatus)
{
	PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;

	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	{
		pInfoSerArch->ModemStatus = ser_ReadMSR( pInfoSerArch );
	}
	*pModemStatus = pInfoSerArch->ModemStatus;
	//EdbgOutputDebugString( "mState=%x, *pModemStatus=%x\r\n", ((PSERARCH_INFO)pHead)->ModemStatus, *pModemStatus );
}
// ********************************************************************
// ������BOOL	SerArch_XMitChar(PVOID pHead, UCHAR ComChar)
// ������
//	IN pHead-ARCH��Ϣ
//	IN ComChar-�����͵��ַ�
// ����ֵ��
//	��
// ������������������1���ַ�
// ����: 
// ********************************************************************
BOOL	SerArch_XMitChar(PVOID pHead, UCHAR ComChar)
{
	PSERARCH_INFO pInfoSerArch = (PSERARCH_INFO)pHead;
	volatile PS2410_UART_REG pUART;
	ULONG       rFifoStat, TxFifoCnt;

	//pUART = pInfoSerArch->pUART;
    EnterCriticalSection(&(pInfoSerArch->csTransmit));
	while( 1 )
	{
		//EnterCriticalSection(&(pInfoSerArch->RegCritSec));
		// Write the character if we can
		rFifoStat = INREG(pInfoSerArch,rUFSTAT);
		TxFifoCnt = (rFifoStat & SER2410_FIFOCNT_MASK_TX) >> 4;
		
		if (!(rFifoStat & SER2410_FIFOFULL_TX) && (TxFifoCnt < (SER2410_FIFO_DEPTH_TX-1))) 
		{
			// FIFO is empty, send this character
			//OUTB(pInfoSerArch, pData, ComChar);
			OUTREG(pInfoSerArch,rUTXH,ComChar);	// ����Ĵ���
			// Make sure we release the register critical section
			//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
			break;
		}
		
		// If we couldn't write the data yet, then wait for a
		// TXINTR to come in and try it again.
		
		// Enable xmit intr.
		//EnINT(pInfoSerArch, pInfoSerArch->bINT);
		//EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT | pInfoSerArch->bRxINT | pInfoSerArch->bErrINT); // canceled 
		// 2004-12-06 lilin modify, ���������˳������Ҫ��
		//EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
		//pInfoSerArch->fSW_EnTxINT = TRUE;
		pInfoSerArch->fSW_EnTxINT = TRUE;
		EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);		
		//
		//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
		
		// Wait until the txintr has signalled.
		
		WaitForSingleObject(pInfoSerArch->hEvtXMit, (ULONG)1000);
#if 0
		if( pUART->utsr1.tnf )
		{
			(pUART->utdr.data) = ComChar;

			//pInfoSerArch->CommErrors &= ~CE_TXFULL;
			//pInfoSerArch->CommStat.fTxim = 0;
			break;
		}
		else
		{
			//struct utsr0Bits utsr0;
			//ULONG	time;
			
			//���Ӳ��
			//utsr0 = pUART->utsr0;
			//if( (utsr0.eif) || (utsr0.rbb) || (utsr0.reb) )
			//{
			//	ser_CheckLine( pInfoSerArch, pUART );
			//}
			//pInfoSerArch->CommErrors |= CE_TXFULL;
			//pInfoSerArch->CommStat.fTxim = 1;

			WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,tie,1);
			//EdbgOutputDebugString( "wait for xmit char\r\n" );
			WaitForSingleObject( pInfoSerArch->hEvtXMit, 400 );
			//time = 0;
			//while( !pInfoSerArch->fWaitTxim )//by xyg
			//{
			//	usWait( 1 );
			//	time ++;
			//	if( time>400000 )//1s timeout
			//		break;
			//}
			//pInfoSerArch->fWaitTxim = 0;
		}
#endif
	}
	LeaveCriticalSection(&(pInfoSerArch->csTransmit));

	return(TRUE);
}

//------------------------------------------------------------------------------
// ********************************************************************
// ������DWORD	SerArch_IntrTypeQuery(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ������������ѯ�ж�����
// ����: 
// ********************************************************************
DWORD	SerArch_IntrTypeQuery(PVOID pHead)
{
    PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
    DWORD			interrupts	= INTR_NONE;
	volatile	ULONG	IntSubPndVal;//=0;

//	RETAILMSG(1,(TEXT("\r\n Ser_Query \r\n")));
	IntSubPndVal  = *(pInfoSerArch->UART_INTSUBSRCPND);
	//��ѯ ����״̬---��� | ��ż | ֡] ����
	if(IntSubPndVal & (pInfoSerArch->bErrINT) )
	{
		interrupts = INTR_LINE;  // Error status
	}
	else
	{
		//��ѯ ����״̬
/*
#ifndef CANCEL_XYG_SER_RX
		if( pInfoSerArch->lpRWBuf->dwCntRW )
		{
			interrupts = INTR_RX;    // Received valid data.
		}
#else
*/

#if 1
		volatile	ULONG	rFifoStat;
		rFifoStat = INREG(pInfoSerArch,rUFSTAT);
		if( ((rFifoStat & 0x0f) > 0) || (rFifoStat & 0x100) )
		{
			interrupts = INTR_RX;    // Received valid data.
		}
#else
		if(IntSubPndVal & (pInfoSerArch->bRxINT) )
		{
			//RETAILMSG(1,(TEXT("SerArch_IntrTypeQuery:RxINT\n")));
			//RETAILMSG(1,(TEXT("\r\n Ser_Query rx \r\n")));
			interrupts = INTR_RX;    // Received valid data.
		}
#endif
//#endif
		//��ѯ ����״̬
		else if((IntSubPndVal & (pInfoSerArch->bTxINT)) && pInfoSerArch->fSW_EnTxINT )
		{
			//RETAILMSG(1,(TEXT("SerArch_IntrTypeQuery:TxINT\n")));
			interrupts |= INTR_TX;	// Tx.
		}
	}

	//��ѯ �Ƿ�����Ӳ������ Ҫ���������
	if( pInfoSerArch->RestartTxForFlow )
    {
        interrupts |= INTR_TX;
		pInfoSerArch->RestartTxForFlow = 0;
    }

	//��ѯ MODEM״̬
	//if( pInfoSerArch->dwOpt & OPT_MODEM )
    if( interrupts & INTR_TX )
	{
		ULONG       msr_val;
		volatile	WORD	msr_ret;
		WORD	msrChange;
		
		//��Ӳ��
		msr_ret = MS_DSR_ON;
		msr_val = INREG(pInfoSerArch,rUMSTAT);
		if (msr_val & COM2410_MSR_CTS)
		{
			msr_ret |= MS_CTS_ON;
		}
		//msr_ret = ser_ReadMSR( pInfoSerArch );
		//
		if( msrChange = (pInfoSerArch->ModemStatus ^ msr_ret) )
		{
			//����״̬
			pInfoSerArch->ModemStatus = msr_ret;
			pInfoSerArch->msrChange = msrChange;

			interrupts |= INTR_MODEM;
		}
	}
	
	//��ѯ OSM2�ж�״̬
//	irq |= HW_SA11x0GetOSM2( pHead );

    return(interrupts);
}

// ********************************************************************
// ������VOID SerArch_IntrHandleTx(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// �����������������ж�
// ����: 
// ********************************************************************
VOID SerArch_IntrHandleTx(PVOID pHead)
{
	PSERARCH_INFO   pInfoSerArch  = (PSERARCH_INFO)pHead;

	//////EdbgOutputDebugString("SerArch_IntrHandleTxHandler: set tie=0, only do this");
	EnterCriticalSection(&(pInfoSerArch->csTransmit));
	//EnterCriticalSection(&(pInfoSerArch->RegCritSec));

	//ֹͣ �����ж�
	//DisEnINT(pInfoSerArch, pInfoSerArch->bINT);
	DisEnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
	pInfoSerArch->fSW_EnTxINT = FALSE;
	//���Pending�����ж�
	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bTxINT );
	ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
	//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//{
	//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
	//}

	//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
	LeaveCriticalSection(&(pInfoSerArch->csTransmit));
}

// ********************************************************************
// ������VOID	SerArch_IntrHandleLine(PVOID pHead)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ��������������״̬�ж�
// ����: 
// ********************************************************************
VOID	SerArch_IntrHandleLine(PVOID pHead)
{
	PSERARCH_INFO   pInfoSerArch  = (PSERARCH_INFO)pHead;

	//EdbgOutputDebugString("INTR_LINE\r\n");
	//���Ӳ��
	ser_CheckLine( pInfoSerArch, pInfoSerArch->pUART );
}

// ********************************************************************
// ������DWORD	SerArch_IntrHandleModem( PVOID pHead )
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	��
// ��������������MODEM�ж�
// ����: 
// ********************************************************************
DWORD	SerArch_IntrHandleModem( PVOID pHead )
{
	PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	BOOL			fFlowChangeToOn = FALSE;

	//�����Ƿ�ı�
	if(	pInfoSerArch->dwOpenCount )
	{
		DWORD	dwMdmEvt;
		WORD	msrChange;
		
		dwMdmEvt = 0;
		msrChange = pInfoSerArch->msrChange;
		if( msrChange & MS_CTS_ON )
			dwMdmEvt |= EV_CTS;
		if( msrChange & MS_DSR_ON )	//MS_RLSD_ON
		{
			dwMdmEvt |= EV_DSR;
			dwMdmEvt |= EV_RLSD;
		}
		//״̬  �����ı�
		if( dwMdmEvt )
			pInfoSerArch->lpfnEventNotify( pInfoSerArch->pHeadDrv, dwMdmEvt );

		//�ָ�
		if( pInfoSerArch->FlowOffDSR && (pInfoSerArch->ModemStatus & MS_DSR_ON) )
		{
			pInfoSerArch->FlowOffDSR = 0;
			fFlowChangeToOn = TRUE;
		}
		if( pInfoSerArch->FlowOffCTS && (pInfoSerArch->ModemStatus & MS_CTS_ON) )
		{
			pInfoSerArch->FlowOffCTS = 0;
			fFlowChangeToOn = TRUE;
		}
		if( fFlowChangeToOn )
		{
			//EdbgOutputDebugString( "Modem Intr Handler: set tie=1\r\n" );
			//WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,tie,1);
			//EnINT(pInfoSerArch, pInfoSerArch->bINT);
			//EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT | pInfoSerArch->bRxINT | pInfoSerArch->bErrINT);

			// 2004-12-06 lilin modify, ���������˳������Ҫ��
			//EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
			//pInfoSerArch->fSW_EnTxINT = TRUE;
			pInfoSerArch->fSW_EnTxINT = TRUE;
			EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
			//
			

			pInfoSerArch->RestartTxForFlow = 1;
		}
	}
	EdbgOutputDebugString( "INTR_MODEM: mState=%x\r\n", pInfoSerArch->ModemStatus );
	return pInfoSerArch->ModemStatus;
}

/*#ifndef CANCEL_XYG_SER_RX

// ********************************************************************
// ������ULONG	SerArch_Recv( PVOID pHead, LPBYTE pBufDes, LONG* pnLenBuf )
// ������
//	IN pHead-ARCH��Ϣ
//	IN pBufDes-��ȡ������
//	IN pnLenBuf-ָ��pBufDes�ĳ��Ⱥ�ʵ�ʻ�ȡ���ݵĳ���
// ����ֵ��
//	��
// ������������������ж�
// ����: 
// ********************************************************************
ULONG	SerArch_Recv( PVOID pHead, LPBYTE pBufRecv, LONG* pnLenBuf )
{
    PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	DCB*			lpDCB;
    LONG			nLenRecv;
	LPBYTE			pBufEnd;

    BOOL			fRXFlag;
    UCHAR			cCharRx;
	
	//
	nLenRecv = RWBuf_ReadBuf( pInfoSerArch->lpRWBuf, pBufRecv, *pnLenBuf );
	*pnLenBuf = nLenRecv;
	if( !nLenRecv )
	{
		return 0;
	}
	//RETAILMSG(1,(TEXT("\r\n===========SerArch_Recv()===========\r\n")));
	lpDCB = pInfoSerArch->lpDCB;
	//�ǲ��ǰ빤�豸
	//if( pInfoSerArch->dwComFun == COM_SIR )
	//{
	//	EnterCriticalSection(&(pInfoSerArch->csTransmit));
	//}

	fRXFlag = FALSE;
	pBufEnd = pBufRecv + nLenRecv;
    while( pBufRecv<pBufEnd )
    {
		//��ȡ����
		cCharRx = *pBufRecv;
		pBufRecv ++;
		//RETAILMSG(1,(TEXT("\r\n [ %x ] \r\n"), cCharRx));
		
		//����Dsr���У���Dsr�ǹصģ� �� ��������
		if( lpDCB->fDsrSensitivity && !(pInfoSerArch->ModemStatus & MS_DSR_ON) )
		{
			//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: ����Dsr���У���Dsr�ǹصģ��� ��������\r\n")) );
			continue;
		}
		else if (!cCharRx && lpDCB->fNull)//���ַ�  �ǲ���Ҫ����
		{
			//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: �������ַ�\r\n")) );
			continue;
		}
		else
		{
			//�����¼��ַ�EvtChar��֪ͨWaitCommEvent
			if (cCharRx == lpDCB->EvtChar)
				fRXFlag = TRUE;
		}// for else <normal operation>
    }// for while

    if( fRXFlag )
	{
		//�����¼��ַ�EvtChar��֪ͨWaitCommEvent
		pInfoSerArch->lpfnEventNotify(pInfoSerArch->pHeadDrv, EV_RXFLAG);
	}

	//�ǲ��ǰ빤�豸
	//if( pInfoSerArch->dwComFun == COM_SIR )
	//{
	//	LeaveCriticalSection(&(pInfoSerArch->csTransmit));
	//}

	//EdbgOutputDebugString( "read=%d\r\n", (*pnLenBuf) );
	return 0;
}

//#else
*/

// ********************************************************************
// ������ULONG	SerArch_Recv( PVOID pHead, LPBYTE pBufDes, LONG* pnLenBuf )
// ������
//	IN pHead-ARCH��Ϣ
//	IN pBufDes-��ȡ������
//	IN pnLenBuf-ָ��pBufDes�ĳ��Ⱥ�ʵ�ʻ�ȡ���ݵĳ���
// ����ֵ��
//	��
// ������������������ж�
// ����: 
// ********************************************************************

ULONG	SerArch_Recv( PVOID pHead, LPBYTE pBufDes, LONG* pnLenBuf )
{
    PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	volatile	PS2410_UART_REG	pUART;	//UART
	DCB*			lpDCB;
    LPBYTE			pBufRecv;
    LONG			nLenRecv;

    BOOL			fRXFlag;
    UCHAR			cCharRx;
	ULONG			rFifoStat;
	//BOOL			fNeedClearRTS;
	ULONG IntSubPndVal;//=0;
	
	//RETAILMSG(1,(TEXT("\r\n===========SerArch_Recv()===========\r\n")));
	lpDCB = pInfoSerArch->lpDCB;
	pUART = pInfoSerArch->pUART;
	//�ǲ��ǰ빤�豸
	//if( pInfoSerArch->dwComFun == COM_SIR )
	//{
	//	EnterCriticalSection(&(pInfoSerArch->csTransmit));
	//}


//	RETAILMSG(1,(TEXT("\r\n Ser_Query \r\n")));
		
	//fNeedClearRTS = TRUE;
	fRXFlag = FALSE;
	pBufRecv = pBufDes;
	nLenRecv = *pnLenBuf;
    while( nLenRecv )
    {
		//utsr0 = pUART->utsr0;
		IntSubPndVal  = *(pInfoSerArch->UART_INTSUBSRCPND);
		//��ѯ ����״̬---��� | ��ż | ֡] ����
		if(IntSubPndVal & (pInfoSerArch->bErrINT) )
		{
		//	//OEM_WriteDebugByte_ToWnd( 'e' );
			ser_CheckLine( pInfoSerArch, pUART );
		}
		//
		rFifoStat = pUART->rUFSTAT;//INREG(pInfoSerArch,rUFSTAT);
		//if( ((rFifoStat & 0x100) || ((rFifoStat & 0x0f) >= 12)) && fNeedClearRTS )
		//{
		//	CLEARREG(pInfoSerArch, rUMCON, SER2410_RTS);
		//	fNeedClearRTS = FALSE;
		//	//RETAILMSG(1,(TEXT("\r\n==SerArch_Recv(): CLEAR RTS=\r\n")));
		//}
		if( (rFifoStat & 0x100) || (rFifoStat & 0x0f) )
		{
			//��ȡ����
			cCharRx = *(pInfoSerArch->pUFRXH);
			//RETAILMSG(1,(TEXT("\r\n [ %x ] \r\n"), cCharRx));
	
			//����Dsr���У���Dsr�ǹصģ� �� ��������
			if( lpDCB->fDsrSensitivity && !(pInfoSerArch->ModemStatus & MS_DSR_ON) )
			{
				//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: ����Dsr���У���Dsr�ǹصģ��� ��������\r\n")) );
				continue;
			}
			else if (!cCharRx && lpDCB->fNull)//���ַ�  �ǲ���Ҫ����
			{
				//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: �������ַ�\r\n")) );
				continue;
			}
			else
			{
				//if( utsr1.pre )
				//{
				//	//���ִ���(fErrorChar && fParity)�Ƿ������ErrorChar
				//	if( lpDCB->fErrorChar && lpDCB->fParity)
				//		cCharRx = lpDCB->ErrorChar;
				//	else
				//		continue;
				//}
				//else
				{
					//�����¼��ַ�EvtChar��֪ͨWaitCommEvent
					if (cCharRx == lpDCB->EvtChar)
                        fRXFlag = TRUE;
					//���֣�֪ͨ
					//if (cCharRx == lpDCB->EofChar)
					//	pInfoSerArch->CommStat.fEof = 1;
				}	
				
				*pBufRecv = cCharRx;
				pBufRecv ++;
				nLenRecv --;
			}// for else <normal operation>
		}// for if( utsr1.rne )
		else
		{
#if 0
			int iii;
			for( iii=0; iii<90000; iii++ )
			{
				rFifoStat = pUART->rUFSTAT;//INREG(pInfoSerArch,rUFSTAT);
				if( (rFifoStat & 0x100) || (rFifoStat & 0x0f) )
				{
					break;
				}
			}
			if( iii>=90000 )
			{
				//usWait( 90 );
				break;
			}
#else
			break;
#endif
		}
    }// for while
	//IOW_REG_BITWRITE(struct utsr0Bits,&pUART->utsr0,rid,1);
	//���Pending�����ж�
	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bRxINT ); //| pInfoSerArch->bErrINT
	//if ( *(pInfoSerArch->UART_INTSUBSRCPND) & ( pInfoSerArch->bRxINT | pInfoSerArch->bErrINT ) )
	//{
	//	;
	//}
	//else
	{
		ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
	//	if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//	{
	//		(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
	//	}
	}
	//if( !fNeedClearRTS )
	//{
	//	SETREG(pInfoSerArch, rUMCON, SER2410_RTS);
	//}
#ifdef xyg_ser_sub_mask	
	//���� �����ж�
	//EnINT(pInfoSerArch, pInfoSerArch->bINT);	//2004-12-06 lilin remove,Ӧ����oemintr.c��
	EnSubINT(pInfoSerArch, (pInfoSerArch->bRxINT|pInfoSerArch->bErrINT));
#endif

//	Debug_Arch();
    if( fRXFlag )
	{
		//�����¼��ַ�EvtChar��֪ͨWaitCommEvent
		pInfoSerArch->lpfnEventNotify(pInfoSerArch->pHeadDrv, EV_RXFLAG);
	}

	//�ǲ��ǰ빤�豸
	//if( pInfoSerArch->dwComFun == COM_SIR )
	//{
	//	LeaveCriticalSection(&(pInfoSerArch->csTransmit));
	//}

	(*pnLenBuf) = (LONG)(pBufRecv - pBufDes);
	//EdbgOutputDebugString( "read=%d\r\n", (*pnLenBuf) );
	return 0;
}

//#endif*/

// ********************************************************************
// ������BOOL	ser_CheckFlowOff( PSERARCH_INFO pInfoSerArch )
// ������
//	IN pInfoSerArch-ARCH��Ϣ
// ����ֵ��
//	�ɹ�����TRUE
// �������������������
// ����: 
// ********************************************************************
//check wether flow switch is validate
BOOL	ser_CheckFlowOff( PSERARCH_INFO pInfoSerArch )
{
	pInfoSerArch->FlowOffCTS = 0;
	pInfoSerArch->FlowOffDSR = 0;
	if( !(pInfoSerArch->ModemStatus & MS_CTS_ON) && pInfoSerArch->lpDCB->fOutxCtsFlow )
		pInfoSerArch->FlowOffCTS = 1;
	if( !(pInfoSerArch->ModemStatus & MS_DSR_ON) && pInfoSerArch->lpDCB->fOutxDsrFlow )
		pInfoSerArch->FlowOffDSR = 1;
	return( pInfoSerArch->FlowOffDSR || pInfoSerArch->FlowOffCTS );
}

// ********************************************************************
// ������ULONG	SerArch_Send( PVOID pHead, LPBYTE pBufSrc, LONG* pnLenBuf )
// ������
//	IN pHead-ARCH��Ϣ
//	IN pBufSrc-�������ݵ�BUFFER
//	IN pnLenBuf-�������ݵ�BUFFER�ĳ���
// ����ֵ��
//	�ɹ����ط��͵ĸ���
// ������������������
// ����: 
// ********************************************************************
ULONG	SerArch_Send( PVOID pHead, LPBYTE pBufSrc, LONG* pnLenBuf )
{
	volatile PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	LPBYTE			pBufSrc_B;
	DWORD			nLenSend;
	ULONG        rFifoStat, TxFifoCnt;
	unsigned int tmpreg;
	//unsigned int FifoModeReg;

	//Ӳ������---�ж��Ƿ���Է�������(�ɶԷ���Dsr/Cts������)
	//if( !*pnLenBuf || ((pInfoSerArch->dwOpt & OPT_MODEM) && ser_CheckFlowOff(pInfoSerArch)) )
	if( !*pnLenBuf || ser_CheckFlowOff(pInfoSerArch) )
	{
		//EdbgOutputDebugString( "Ӳ������ xyg --fail...\r\n" );
		*pnLenBuf = 0;
		//WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,tie,0);

		//ֹͣ �����ж�
		DisEnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
		pInfoSerArch->fSW_EnTxINT = FALSE;
		//���Pending�����ж�
		ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bTxINT );
		ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
		//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
		//{
		//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
		//}

		return 0;
	}

	//׼����������
	pBufSrc_B= pBufSrc;
	//nLenSend = *pnLenBuf;

	//��ʼ��������
	EnterCriticalSection(&(pInfoSerArch->csTransmit));
	//FifoModeReg = INREG(pInfoSerArch, rUFCON);
	rFifoStat = INREG(pInfoSerArch,rUFSTAT);
	TxFifoCnt = ((rFifoStat & SER2410_FIFOCNT_MASK_TX) >> 4);
	if (!(rFifoStat & SER2410_FIFOFULL_TX) && (TxFifoCnt < (SER2410_FIFO_DEPTH_TX-1))) 
	{
		nLenSend = (unsigned char)(SER2410_FIFO_DEPTH_TX-TxFifoCnt);
		if( nLenSend>*pnLenBuf )
		{
			nLenSend=*pnLenBuf;
		}
	}
	else
	{
		nLenSend = 0;
	}

	//RETAILMSG(1,(TEXT("\r\nnLenSend = [ %d ]\r\n"), nLenSend));
	while( nLenSend )
	{
		//���Ӳ���Ƿ��������������״̬
		//utsr0 = pUART->utsr0;
		//if( (utsr0.eif) || (utsr0.rbb) || (utsr0.reb) )
		//	ser_CheckLine(pInfoSerArch, pUART);
		//�ж�Ӳ���Ƿ�æ
		//if (FifoModeReg&0x1) // FIFO Mode enabled.
		{
			tmpreg = INREG(pInfoSerArch, rUFSTAT);
			//tmpreg & 0x200 == 1  ->  Fifo full -> waiting...
			//tmpreg & 0xf0 == 0 -> There is no data to send -> break loop.
			//RETAILMSG(1,(TEXT("\r\ntmpreg = [ %d ]\r\n"), tmpreg));
			if ( (tmpreg & 0x200) == 0x200 )
			{
				// //RETAILMSG(DEBUGMODE, (TEXT("SL_TxInt :  fifo full : tmpreg = %x \r\n"), tmpreg));
				break;
			}
		}
		//else
		//{
		//	tmpreg = INREG(pInfoSerArch, rUTRSTAT);
		//	if ( !(tmpreg & 0x2) )
		//		break;
		//}
		//��������
		*(pInfoSerArch->pUFTXH) = *pBufSrc;	//��������
		pBufSrc ++;
		nLenSend--;
	}

	// �ڴ������֮�������FIFOģʽ������Ҫ�����ƺ���
	//if (FifoModeReg & 0x1) // FIFO Mode enabled.
#if 0	// ȡ������ĵȴ�
	{
		unsigned int cnt = 0;
		while ( 1 )
		{
			//#define INREG(pInfo, reg) (pInfo->s2410SerReg->reg)
			tmpreg = INREG(pInfoSerArch, rUFSTAT);	//��ȡ״ֵ̬��ѭ�������ж�
			// //RETAILMSG(DEBUGMODE, (TEXT("SL_TxInt : Waiting till tx buffer empty :  tmpreg = %x \r\n"), tmpreg));
			if ( (tmpreg & 0xf0) == 0) break; // waitint to empty the tx buffer empty...
			if ( cnt++ > 600000 )
			{
				SETREG(pInfoSerArch,rUFCON,0x4);    // tx, rx fifo reset
				break;
			}
		}
	}
#endif

	//WRITE_BITFIELD(struct utcr3Bits,&pUART->utcr3,tie,1);	//
	//���Pending�����ж�
	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bTxINT ); //| pInfoSerArch->bErrINT
	ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
	//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//{
	//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
	//}
	//���� �����ж�
	//EnINT(pInfoSerArch, pInfoSerArch->bINT);

	// 2004-12-06 lilin modify, ���������˳������Ҫ��	
	//EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
	//pInfoSerArch->fSW_EnTxINT = TRUE;
	pInfoSerArch->fSW_EnTxINT = TRUE;
	EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
	//
	

	LeaveCriticalSection(&(pInfoSerArch->csTransmit));
	
	(*pnLenBuf) = (LONG)(pBufSrc - pBufSrc_B);
	//RETAILMSG( 1,(TEXT("\r\nTX=[ %d ]\r\n"), *pnLenBuf));
	return 0;
}

// ********************************************************************
// ������BOOL	SerArch_IOControl(PVOID pHead, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
// ������
//	IN pHead-ARCH��Ϣ
//	IN dwCode-IO������
//	IN pBufIn_-����BUFFER
//	IN dwLenIn-����BUFFER�ĳ���
//	OUT pBufOut_-���BUFFER
//	IN dwLenOut-���BUFFER�ĳ���
//	OUT pdwActualOut-���BUFFERʵ�ʻ�ȡ���ݵĳ���
// ����ֵ��
//	�ɹ�����TRUE
// ����������IOControl����
// ����: 
// ********************************************************************
BOOL	SerArch_IOControl(PVOID pHead, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
#ifdef	DEBUG_SERARCH
	BOOL RetVal = TRUE;
	switch (dwCode)
	{
	case 31:
		if ( (dwLenIn<sizeof(DWORD)) || (NULL == pBufIn) )
			RetVal = FALSE;
//		else
//			SerDebug_Enable( (DWORD)(*pBufIn) );
		break;
	case 32:
		Debug_Arch( pHead );
		break;
	case 33:
		//SerDebug_CleanScreen( );
		Debug_Arch(pHead );
		break;
	case 34:
		break;

	default:
		RetVal = FALSE;
		break;
	}
	return(RetVal);
#else
	return FALSE;
#endif
}

//-----------------------------------------
// ********************************************************************
// ������BOOL	SerArch_IREnable(PVOID pHead,ULONG BaudRate)
// ������
//	IN pHead-ARCH��Ϣ
//	IN BaudRate-������
// ����ֵ��
//	�ɹ�����TRUE
// �����������������⹦�ܣ������ò�����
// ����: 
// ********************************************************************
BOOL	SerArch_IREnable(PVOID pHead,ULONG BaudRate)
{
	return FALSE;
}

// ********************************************************************
// ������BOOL	SerArch_IREnable(PVOID pHead,ULONG BaudRate)
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	�ɹ�����TRUE
// ����������ȡ�����⹦��
// ����: 
// ********************************************************************
BOOL	SerArch_IRDisable(PVOID pHead)
{
	return FALSE;
}

//-----------------------------------------
//for debug
#ifdef	DEBUG_SERARCH

//���Ժ���
void	Debug_Arch( PVOID pHead )
{
	volatile	PSERARCH_INFO   pInfoSerArch   = (PSERARCH_INFO)pHead;
	volatile	PS2410_UART_REG	pUART = pInfoSerArch->pUART;	//UART
#if 0
	volatile struct uart *pUart; // xyg: connected to hardware register---UART

 	volatile IOPreg *pGpio;
	pUart = (volatile PS2410_UART_REG)UART3_BASE_VIRTUAL;
	pGpio = (volatile IOPreg *)GPIO_BASE_VIRTUAL;
	//EdbgOutputDebugString("xyg: serial version bbbbb--------\r\n");
	EdbgOutputDebugString( "b_uart: s0=%x, s1=%x, c0=%x, c1=%x, c2=%x, c3=%x\r\n", pUart->utsr0, pUart->utsr1, pUart->utcr0, pUart->utcr1, pUart->utcr2, pUart->utcr3 );
	EdbgOutputDebugString( "gpio: DTR=%x, RTS=%x, DSR=%x, CTS=%x\r\n", pGpio->gplr.gp10, pGpio->gplr.gp11, pGpio->gplr.gp13, pGpio->gplr.gp14 );
#endif
	//sr0 = &pUART->utsr0;
	//EdbgOutputDebugString( _T("s0: rbb=%d, reb=%d, eif=%d, tfs=%d, rfs=%d, rid=%d"), sr0->rbb, sr0->reb, sr0->eif, sr0->tfs, sr0->rfs, sr0->rid );
	//sr1 = &pUART->utsr1;        
	//EdbgOutputDebugString( _T("s1: rne=%d, ror=%d, pre=%d, tnf=%d, tby=%d"), sr1->rne, sr1->ror, sr1->pre, sr1->tnf, sr1->tby );
	
	//cr3 = &pUART->utcr3;
	//EdbgOutputDebugString( TEXT("c3: rxe=%d rie=%d,  txe=%d, tie=%d"), cr3->rxe, cr3->rie, cr3->txe, cr3->tie );
	
	//cr4 = &pUART->utcr4;
	//EdbgOutputDebugString( TEXT("c4: hse=%d, lpm=%d"), cr4->hse, cr4->lpm );
	
	//EdbgOutputDebugString( TEXT("hscr0: itr=%x"), pHSSP->hscr0.itr );
	//EdbgOutputDebugString( TEXT("PPC: ppdr=%x, ppsr=%x, ppar=%x,  psdr=%x,  ppfr=%x, hscr2=%x"), pPPC->ppdr, pPPC->ppsr, pPPC->ppar, pPPC->psdr, pPPC->ppfr, pPPC->hscr2 );
	//EdbgOutputDebugString( TEXT("PPC ppdr_1: txd1=%d, txd2=%d, txd3=%d"), pPPC->ppdr.txd1, pPPC->ppdr.txd2, pPPC->ppdr.txd3 );
	//EdbgOutputDebugString( TEXT("PPC ppdr_2: rxd1=%d, rxd2=%d, rxd3=%d"), pPPC->ppdr.rxd1, pPPC->ppdr.rxd2, pPPC->ppdr.rxd3 );
	//EdbgOutputDebugString( TEXT("PPC ppsr_1: txd1=%d, txd2=%d, txd3=%d"), pPPC->ppsr.txd1, pPPC->ppsr.txd2, pPPC->ppsr.txd3 );
	//EdbgOutputDebugString( TEXT("PPC ppsr_2: rxd1=%d, rxd2=%d, rxd3=%d"), pPPC->ppsr.rxd1, pPPC->ppsr.rxd2, pPPC->ppsr.rxd3 );

	RETAILMSG(1,(TEXT("\r\n <<< debug register begin serial \r\n")));

	RETAILMSG(1,(TEXT("\r\n:rULCON [ %x ]\r\n"),pUART->rULCON));
	RETAILMSG(1,(TEXT("\r\n:rUCON [ %x ]\r\n"),pUART->rUCON));
	RETAILMSG(1,(TEXT("\r\n:rUMCON [ %x ]\r\n"),pUART->rUMCON));
	RETAILMSG(1,(TEXT("\r\n:rUTRSTAT [ %x ]\r\n"),pUART->rUTRSTAT));
	RETAILMSG(1,(TEXT("\r\n:rUERSTAT [ %x ]\r\n"),pUART->rUERSTAT));
	RETAILMSG(1,(TEXT("\r\n:rUFSTAT [ %x ]\r\n"),pUART->rUFSTAT));
	RETAILMSG(1,(TEXT("\r\n:rUMSTAT [ %x ]\r\n"),pUART->rUMSTAT));
	RETAILMSG(1,(TEXT("\r\n:rUTXH [ %x ]\r\n"),pUART->rUTXH));
	RETAILMSG(1,(TEXT("\r\n:rURXH [ %x ]\r\n"),pUART->rURXH));
	RETAILMSG(1,(TEXT("\r\n:rUBRDIV [ %x ]\r\n"),pUART->rUBRDIV));


	RETAILMSG(1,(TEXT("\r\n+++++++++++++++++++++++++++++\r\n")));

	RETAILMSG(1,(TEXT("\r\n:UART_INTMASK [ %x ]\r\n"),(*pInfoSerArch->UART_INTMASK)));
	RETAILMSG(1,(TEXT("\r\n:UART_INTSUBMASK [ %x ]\r\n"),(*pInfoSerArch->UART_INTSUBMASK)));
	RETAILMSG(1,(TEXT("\r\n:UART_INTPND [ %x ]\r\n"),(*pInfoSerArch->UART_INTPND)));
	RETAILMSG(1,(TEXT("\r\n:UART_INTSRCPND [ %x ]\r\n"),(*pInfoSerArch->UART_INTSRCPND)));
	RETAILMSG(1,(TEXT("\r\n:UART_INTSUBSRCPND [ %x ]\r\n"),(*pInfoSerArch->UART_INTSUBSRCPND)));

	RETAILMSG(1,(TEXT("\r\n >>> debug register end serial \r\n")));


}

#endif


// ********************************************************************
// ������static ULONG SerArch_GetRxBufferSize( PVOID pHead )
// ������
//	IN pHead-ARCH��Ϣ
// ����ֵ��
//	�ɹ�������Ҫ��buffer��С
// ����������
//	�õ�PDD��Ҫ��buffer��С������ 16550 UARTS��������Ҫ����0,
//	��Ϊ��Ҫ����Сbuffer ��2048
// ����: 
// ********************************************************************
// 
static ULONG SerArch_GetRxBufferSize( PVOID pHead )
{
	return 1024*8;
}
//-----------------------------------------

static	VTBL_ARCH_SER	SerVTbl =
{
	//ͨ�� ����
	SerArch_Init,
	SerArch_InitLast,
	SerArch_Deinit,
	SerArch_Open,
	SerArch_Close,
	SerArch_PowerOff,
	SerArch_PowerOn,
	SerArch_IOControl,
	//�շ� ����
	SerArch_Recv,
	SerArch_Send,
	SerArch_XMitChar,
	//�ж� ����
	SerArch_IntrTypeQuery,
	SerArch_IntrHandleModem,
	SerArch_IntrHandleLine,
	SerArch_IntrHandleTx,

	//IR�����Ĺ��� ����
	SerArch_IREnable,
	SerArch_IRDisable,

	//Pin Singal�Ĺ��� ����
	SerArch_ClearPinDTR,
	SerArch_SetPinDTR,
	SerArch_ClearPinRTS,
	SerArch_SetPinRTS,

	//Break�Ĺ��� ����
	SerArch_ClearBreak,
	SerArch_SetBreak,
	
	//Purge�Ĺ��� ����
	SerArch_PurgeComm,
	
	//״̬���ԵĹ��� ����---for ClearCommError
	SerArch_GetComStat,
	//״̬���ԵĹ��� ����---for GetCommModemStatus
	SerArch_GetModemStatus,
	//״̬���ԵĹ��� ����---for GetCommProperties
	SerArch_GetCommProperties,	
	//״̬���ԵĹ��� ����---for SetCommState
	SerArch_SetDCB,
	SerArch_GetRxBufferSize
};

OBJECT_ARCH	SerObj =
{
	SYSINTR_SERIAL,
	(LPVOID)&SerVTbl
};


