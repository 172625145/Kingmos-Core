/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/


/*****************************************************
文件说明：串口驱动---硬件相关层
版本号：  1.0.0
开发时期：2003-07-01
作者：    肖远钢
修改记录：
2004-03-18：凡是带有这个标记的，跟不同板子的调试串口 是相关的
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


/***************  全局区 定义， 声明 *****************/


//-----------------------------------------
//调试部分的定义
//-----------------------------------------

//
//#define	DEBUG_SERARCH	// 

//#define xyg_ser_sub_mask

//
#ifdef	DEBUG_SERARCH

//
static	PSERARCH_INFO	g_pDebugSerArch = NULL;

//调试函数
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
//正常的定义
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
// 声明：WORD	ser_ReadMSR( PSERARCH_INFO pInfoSerArch )
// 参数：
//	IN pGPIO-GPIO的地址
// 返回值：
//	返回MODEM的状态
// 功能描述：读取MODEM的状态
// 引用: 
// ********************************************************************
WORD	ser_ReadMSR( PSERARCH_INFO pInfoSerArch )
{
	volatile	WORD	msr_ret;
	ULONG       msr_val;
	
	//读硬件
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
// 声明：VOID ClearPendingInts( PVOID   pHead )
// 参数：
//	IN PVOID pHead - 指向PDD层的全局结构变量
// 返回值：
//	无
// 功能描述：设置寄存器，使由未知状态为已知状态
// 引用：在 SL_Init( ) 中调用
// ********************************************************************
VOID ClearPendingInts( PVOID   pHead )
{
	volatile PSERARCH_INFO   pInfoSerArch   = (PSERARCH_INFO)pHead;
	volatile UINT32 tmpReg;  //2004-12-06， lilin add volatile, 防止编译器优化

	//RETAILMSG(1,(TEXT("ClearPendingInts\n")));
	SETREG(pInfoSerArch,rUFCON,0x6);    // tx, rx fifo reset

	tmpReg = INREG(pInfoSerArch,rUERSTAT);		//进行读之后，该位清除

	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bTxINT | pInfoSerArch->bRxINT | pInfoSerArch->bErrINT);	//清除SUBSRCPND中串口的 Tx,Rx,Err中断
	//RETAILMSG(1,(TEXT("**After  ClearSubINTPnd:%x\n"),pInfoSerArch->pIrqCtrlAddr->rSUBSRCPND));
	ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);	//清除SRCPND中串口中断
	//RETAILMSG(1,(TEXT("**SRCPND:%x\n"),pInfoSerArch->UART_INTSRCPND));
	//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//{
	//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;	//#define   BIT_UART0       ( 0x1 << 28 )
	//}
	//RETAILMSG(1,(TEXT("pInfoSerArch->UART_INTPND --- :%x\n"),pInfoSerArch->UART_INTPND));

	tmpReg = INREG(pInfoSerArch,rUERSTAT);		//进行读之后，该位清除

}

#if defined( HW_PCB_VERSION_C )
// ********************************************************************
// 声明：void	ser_SelectSIR( PSERARCH_INFO pInfoSerArch )
// 参数：
//	IN pInfoSerArch-ARCH信息
// 返回值：
//	无
// 功能描述：选择红外功能
// 引用: 
// ********************************************************************
//Select SIR function: set param, open power, open recv and send register
void	ser_SelectSIR( PSERARCH_INFO pInfoSerArch )
{
	//设置模式
	WRITE_BITFIELD(struct hscr0Bits,&pInfoSerArch->pHSSP->hscr0,itr,0);
	WRITE_BITFIELD(struct utcr4Bits,&pInfoSerArch->pUART->utcr4,hse,1);
	WRITE_BITFIELD(struct utcr4Bits,&pInfoSerArch->pUART->utcr4,lpm,0);
	//打开电源
	pInfoSerArch->pGPIO->gpdr.gp15 = 1; // 输出 方向
	pInfoSerArch->pGPIO->gpsr.gp15 = 1; // 打开电源
	{	DWORD	iDlay;	for( iDlay=0; iDlay<0xA000; iDlay++ ) ;	}
	//打开硬件的  收
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,rxe,1);
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,txe,0);
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,rie,1);
	WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,tie,1);
}
#endif

//-----------------------------------------

// ********************************************************************
// 声明：void	ser_CheckLine( PSERARCH_INFO pInfoSerArch, volatile PS2410_UART_REG pUART)
// 参数：
//	IN pInfoSerArch-ARCH信息
//	IN pUART-UART的地址
// 返回值：
//	无
// 功能描述：检查串口的状态
// 引用: 
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
	//出现错误
	LineEvents = 0;
	LineStatus = 0;
//	nCntTry = 0;

	//RETAILMSG(1,TEXT("\r\n SerArch_CheckLine() ---- Debug_Arch \r\n"));
	//Debug_Arch( pInfoSerArch );

//    while( nCntTry++<100 )	// Delete!
	{
		LineStatus = pUART->rUERSTAT;//INREG(pInfoSerArch,rUERSTAT);	//取寄存器状态值
		//RETAILMSG(1,(TEXT("Line Status Register:%x"),LineStatus));
		//  判断控制状态	[溢出 | 奇偶 | 帧] 错误
		if ( LineStatus & (COM2410_LSR_OE | COM2410_LSR_PE | COM2410_LSR_FE)) 
		{
			if ( LineStatus & COM2410_LSR_OE )	//溢出错误
			{
				//When overrun error occurs, S2410 rURXH must be read.
				pInfoSerArch->CommErrors |= CE_OVERRUN;
				LineEvents |= EV_ERR;
				//RETAILMSG(1,TEXT("\r\n CLine_ov"));
			}
			if ( LineStatus & COM2410_LSR_PE )	//奇偶错误
			{
				pInfoSerArch->CommErrors |= CE_RXPARITY;
				LineEvents |= EV_ERR;
				//RETAILMSG(1,TEXT("\r\n CLine_pp"));
			}
			if ( LineStatus & COM2410_LSR_FE )	//帧错误
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

	if ( LineStatus & COM2410_LSR_BI )	//中断错误
	{
		LineEvents |= EV_BREAK;
		pInfoSerArch->CommErrors |= CE_BREAK;
		//EdbgOutputDebugString("\r\nEV_BREAK\r\n" );
	}
	
	//EdbgOutputDebugString( ",d=%d", nDiscard );
	//EdbgOutputDebugString("\r\ne_2: sr0=%x, sr1=%x, cr0=%x\r\n", pUART->utsr0, pUART->utsr1, pUART->utcr0 );
	if( LineEvents )
	{
		//通知WaitCommEvent
		pInfoSerArch->lpfnEventNotify(pInfoSerArch->pHeadDrv, LineEvents);
	}

	//清除Pending串口中断
	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bErrINT );
	ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
#ifdef xyg_ser_sub_mask
	//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//{
	//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
	//}
	//
	//EnINT(pInfoSerArch, pInfoSerArch->bINT); //2004-12-06 lilin remove,应该在oemintr.c打开
	EnSubINT(pInfoSerArch, (pInfoSerArch->bRxINT|pInfoSerArch->bErrINT) );
	//EnSubINT(pInfoSerArch, (pInfoSerArch->bErrINT) );
#endif

	//RETAILMSG(1,TEXT("\r\n SerArch_CheckLine() ---- Debug_Arch OVER\r\n"));
	//Debug_Arch( pInfoSerArch );
}

// ********************************************************************
// 声明：void	ser_OpenMode( PSERARCH_INFO pInfoSerArch )
// 参数：
//	IN pInfoSerArch-ARCH信息
// 返回值：
//	无
// 功能描述：结构初始化 和 硬件初始化打开
// 引用: 
// ********************************************************************
void	ser_OpenMode( PSERARCH_INFO pInfoSerArch )
{
	volatile	PS2410_UART_REG		pUART;//UART-registers

	//初始化操作信息的变量
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
	//硬件的参数设置和状态清除( 注意：之前已经调用了HWSetDCB )
	//
	pUART = pInfoSerArch->pUART;

	// 设置波特率
//	SerArch_SetDCB( pInfoSerArch, pInfoSerArch->lpDCB, SETDCB_ALL );	// 移到下面

	//OUTREG(pInfoSerArch,rUCON,0x2c5);	// Select Clock,Tx and Rx Interrupt Type,Transmit and Receive Mode.
	OUTREG(pInfoSerArch,rUCON,0x02c5);	// Select Clock,Tx and Rx Interrupt Type,Transmit and Receive Mode.
	OUTREG(pInfoSerArch,rUFCON,0x6);		// Reset FIFO
	//OUTREG(pInfoSerArch,rUFCON,0x41);	// FIFO enable : tx-4bytes, rx-4bytes
	OUTREG(pInfoSerArch,rUFCON,0x01);	// FIFO enable : tx-4bytes, rx-4bytes
	//OUTREG(pInfoSerArch,rUMCON,0x00);	// Disable auto flow control.
	OUTREG(pInfoSerArch,rUMCON,0x10);	// Enable auto flow control.
	OUTREG(pInfoSerArch,rULCON,0x3);		// Normal mode, N81

	// 设置波特率
	SerArch_SetDCB( pInfoSerArch, pInfoSerArch->lpDCB, SETDCB_ALL );

	RETAILMSG(1,(TEXT("\n*******************************\n")));
	RETAILMSG(1,(TEXT("**rUFCON:%x\n"),pUART->rUFCON));
	RETAILMSG(1,(TEXT("**rUCON:%x\n"),pUART->rUCON));
	RETAILMSG(1,(TEXT("\n*******************************\n")));

	// 中断有效
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
// 声明：VOID S2410_SetIrDAIOP( PVOID  pHead )
// 参数：
//	IN PVOID  pHead - PDD层全局结构变量
// 返回值：
//	无
// 功能描述：设置 IO 端口，使串口能够有效使用
// 引用：在SL_Init( )中调用
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
// 声明：void	ser_CloseMode( PSERARCH_INFO pInfoSerArch )
// 参数：
//	IN pInfoSerArch-ARCH信息
// 返回值：
//	无
// 功能描述：硬件关闭
// 引用: 
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

	//关闭MODEM引脚
	//if( pInfoSerArch->dwOpt & OPT_MODEM )
	{
		SerArch_ClearPinRTS( pInfoSerArch );
	}

	// Disable all interrupts and clear MCR.
	DisEnSubINT(pInfoSerArch, (pInfoSerArch->bRxINT|pInfoSerArch->bTxINT|pInfoSerArch->bErrINT));
	pInfoSerArch->fSW_EnTxINT = FALSE;

	// This routhine for auto detect.
	S2410_SetIrDAIOP(pInfoSerArch);	//设置硬件恢复到初始化状态

	//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
}

//-----------------------------------------
// ********************************************************************
// 声明：PVOID	SerArch_Init( ULONG Identifier, PVOID pHeadDrv, PVOID lpDCB )
// 参数：
//	IN Identifier-上层传递的参数
//	IN pHeadDrv-上层传递的参数
//	IN lpDCB-上层传递的参数
// 返回值：
//	无
// 功能描述：硬件初始化
// 引用: 
// ********************************************************************
PVOID	SerArch_Init( ULONG Identifier, PVOID pHeadDrv, PVOID lpDCB )
{
    PSERARCH_INFO	pInfoSerArch;

    RETAILMSG(1,TEXT("\r\n++++++++++SerArch_Init Start!++++++++++++\r\n"));
	//分配结构
    pInfoSerArch = (PSERARCH_INFO)malloc( sizeof(SERARCH_INFO) );
	if( pInfoSerArch==NULL )
		return NULL;
	memset( pInfoSerArch, 0, sizeof(SERARCH_INFO) );

	//初始化---
	InitializeCriticalSection(&(pInfoSerArch->csTransmit));
	//InitializeCriticalSection(&(pInfoSerArch->RegCritSec));

	//初始化---硬件信息
	if( ser_InitInfoOfHardware(pInfoSerArch, Identifier) )//保证硬件存在
	{
		//初始化---上层信息
		pInfoSerArch->pHeadDrv = pHeadDrv;
	    pInfoSerArch->lpfnEventNotify = ser_NotifyCommEvent;
		//初始化---硬件
		//if( pInfoSerArch->dwIoBase!=UART_DEBUGPORT )//调试的串口
		{
			ser_CloseMode( pInfoSerArch );
		}

		//初始化---硬件
		pInfoSerArch->dwOpenCount= 0;
		pInfoSerArch->lpDCB    = (DCB*)lpDCB;
		//pInfoSerArch->fWaitTxim  = 0;
		pInfoSerArch->hEvtXMit   = CreateEvent(0, FALSE, FALSE, NULL);

		
		//初始化---中断ISR
		pInfoSerArch->hEvtISR = CreateEvent( NULL, FALSE, FALSE, NULL );
		ClearPendingInts( pInfoSerArch );  // 2004-12-06, lilin add , 清除所有的状态/pending
		INTR_Init( pInfoSerArch->dwISR, pInfoSerArch->hEvtISR, NULL, 0 );//pInfoDrv->pObjDev->dwIntID注册中断
		INTR_Disable( pInfoSerArch->dwISR );//pInfoDrv->pObjDev->dwIntID注册中断
		//ResetEvent( pInfoSerArch->hEvtISR );	// 2004-12-06, remove, 没有必要

		//if( pInfoSerArch->pObjDev->BindFlags==THREAD_AT_INIT )
		pInfoSerArch->hThrdISR = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ser_WaitInterrupt, (LPVOID)pInfoSerArch, 0, &pInfoSerArch->dwThrdISR );
		if( !pInfoSerArch->hThrdISR )
		{
			SerArch_Deinit( pInfoSerArch );
			return NULL;
		}
		SetThreadPriority( pInfoSerArch->hThrdISR, THREAD_PRIORITY_TIME_CRITICAL );
		//Sleep( 100 ); //2004-12-06 没有必要
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
// 声明：DWORD	WINAPI	ser_WaitInterrupt( DWORD  pHead )
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	0
// 功能描述：硬件中断服务例程
// 引用: 
// ********************************************************************
//interrupt service route
static DRIVER_GLOBALS * lpGlobal =(DRIVER_GLOBALS *)DRIVER_GLOBALS_PHYSICAL_MEMORY_START;

DWORD	WINAPI	ser_WaitInterrupt( DWORD  pHead )
{
    PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;

	//Sleep( 10 ); // 2004-12-06, lilin remove, 没有必要
	//SetThreadPriority( pInfoSerArch->hThrdISR, THREAD_PRIORITY_TIME_CRITICAL );	// 2004-12-06, lilin remove, 在CreateThread已经设置
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
// 声明：BOOL	ser_InitInfoOfHardware( PSERARCH_INFO pInfoSerArch, ULONG Identifier )
// 参数：
//	IN pInfoSerArch-ARCH信息
//	IN Identifier-上层传递的参数
// 返回值：
//	成功返回TRUE
// 功能描述：此函数初始化串口硬件地址和功能(读取注册表 或者直接初始化指定)；保证硬件存在
// 引用: 
// ********************************************************************
//init hardware information
BOOL	ser_InitInfoOfHardware( PSERARCH_INFO pInfoSerArch, ULONG Identifier )
{
	//指定硬件的指针和功能
	switch( Identifier )
	{
	case ID_COM1://
		//串口硬件功能信息
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
		//串口硬件功能信息
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

	default ://如果该硬件不存在，则取消
		return FALSE;
	}

	//设备属性
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
		//添加 硬件握手
		//pInfoSerArch->CommProp.dwProvCapabilities |= PCF_DTRDSR |PCF_RLSD |PCF_RTSCTS;
		pInfoSerArch->CommProp.dwProvCapabilities |= PCF_RTSCTS;
	}

	//绑定到虚拟地址
	pInfoSerArch->pUART = (volatile PS2410_UART_REG)pInfoSerArch->dwIoBase;
	pInfoSerArch->pGPIO = (volatile IOPreg *)IOP_BASE;
	pInfoSerArch->pIrqCtrlAddr = (volatile INTreg*)INT_BASE;

	// 数据寄存器
	pInfoSerArch->pUFTXH = (volatile unsigned char *)&(pInfoSerArch->pUART->rUTXH);	//传输寄存器
	pInfoSerArch->pUFRXH = (volatile unsigned char *)&(pInfoSerArch->pUART->rURXH);	//接收寄存器

	// 关联中断寄存器
	pInfoSerArch->UART_INTMASK		= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rINTMSK);
	pInfoSerArch->UART_INTSUBMASK	= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rINTSUBMSK);
	pInfoSerArch->UART_INTPND		= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rINTPND);
	pInfoSerArch->UART_INTSRCPND		= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rSRCPND);
	pInfoSerArch->UART_INTSUBSRCPND	= (volatile unsigned int *)&(pInfoSerArch->pIrqCtrlAddr->rSUBSRCPND);

	//osm2 timer: 成关闭状态，并初始化
	//WRITE_BITFIELD(struct icregBits,&pInfoSerArch->pIrqCtrlAddr->icmr,osmr2,0);
	//WRITE_BITFIELD(struct icregBits,&(pInfoSerArch->pIrqCtrlAddr)->iclr,osmr2,0);
	//IOW_REG_BITWRITE (struct matchRegBits, &(pInfoSerArch->pOSTReg)->ossr,m2,1);
	//WRITE_BITFIELD(struct matchRegBits, &(pInfoSerArch->pOSTReg)->oier,m2,0);

	pInfoSerArch->pBaudTbl		= (BAUDTBL *)&LS_BaudTable;//各个baud都一样
	pInfoSerArch->dwSizeTbl		= BAUD_TABLE_SIZE;//各个baud都一样

	return TRUE;
}

//reserved for the last init hardware
VOID	SerArch_InitLast( PVOID   pHead )
{
}

// ********************************************************************
// 声明：BOOL	SerArch_Deinit( PVOID   pHead )
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	成功返回TRUE
// 功能描述：串口释放
// 引用: 
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
// 声明：ULONG	SerArch_Close( PVOID   pHead )
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	0
// 功能描述：串口关闭
// 引用: 
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
// 声明：static void SerSetOutputMode( PSERARCH_INFO pInfoSerArch, BOOL UseIR )
// 参数：
//	IN PSERARCH_INFO pInfoSerArch - PDD层全局结构
//	IN BOOL UseIR - 红外线（NONE）
// 返回值：
//	无
// 功能描述：是否使用红外线（没有红外线功能）
// 引用：由 SerArch_Init( ) 中调用
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
// 声明：BOOL	SerArch_Open( PVOID pHead )
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	0
// 功能描述：串口打开
// 引用: 
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

	SerSetOutputMode( pInfoSerArch, FALSE );	//设置使用串口模式：屏蔽红外线
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
// 声明：BOOL	SerArch_PowerOff(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	TRUE
// 功能描述：串口关电源
// 引用: 
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
// 声明：BOOL	SerArch_PowerOn(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	TRUE
// 功能描述：串口开电源
// 引用: 
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
// 声明：VOID	SerArch_PurgeComm(PVOID pHead,DWORD fdwAction)
// 参数：
//	IN pHead-ARCH信息
//	IN fdwAction-操作
// 返回值：
//	无
// 功能描述：PURGE串口
// 引用: 
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
// 声明：VOID SerArch_ClearPinDTR(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：clear modem pin
// 引用: 
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
// 声明：VOID SerArch_SetPinDTR(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：set modem pin
// 引用: 
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
// 声明：VOID SerArch_ClearPinRTS(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：clear modem pin
// 引用: 
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
// 声明：VOID SerArch_SetPinRTS(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：set modem pin
// 引用: 
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
// 声明：VOID SerArch_ClearBreak(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：clear break pin
// 引用: 
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
// 声明：VOID	SerArch_SetBreak(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：set break pin
// 引用: 
// ********************************************************************
//功能：导致本地不能发送数据(原因： pUART->utsr0==0  and  pUART->utsr1==0 )
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
// 声明：BOOL	ser_CheckDCB( LPDCB lpDCB, PSERARCH_INFO pInfoSerArch )
// 参数：
//	IN lpDCB-DCB信息
//	IN pInfoSerArch-ARCH信息
// 返回值：
//	无
// 功能描述：check dcb wether is validate
// 引用: 
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
// 声明：BOOL	SerArch_SetDCB( PVOID pHead, LPDCB lpDCB, DWORD dwFlagSetDCB )
// 参数：
//	IN pHead-ARCH信息
//	IN lpDCB-DCB信息
//	IN dwFlagSetDCB-设置DCB的标志
// 返回值：
//	成功返回TRUE
// 功能描述：设置成功的部分采用lpDCB，失败的部分采用原默认值
// 引用: 
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
				RETAILMSG(1,(TEXT("\r\n设置波特率后:%d\r\n"),lpDCB->BaudRate));
				OUTREG(pInfoSerArch,rUBRDIV,( (int)( S2410PCLK / (lpDCB->BaudRate * 16) ) -1));
				RETAILMSG(1,(TEXT("\r\n设置波特率后:%x\r\n"),pInfoSerArch->pUART->rUBRDIV));
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
						OUTREG(pInfoSerArch,rULCON,lcr);	//设置数据位至硬件
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

			//判断并设置硬件信号---disable, enable, or nothing
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
// 声明：ULONG	SerArch_GetComStat(PVOID pHead, LPCOMSTAT lpStat)
// 参数：
//	IN pHead-ARCH信息
//	OUT lpStat-保存状态
// 返回值：
//	返回错误信息
// 功能描述：获取MODEM状态
// 引用: 
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
// 声明：VOID	SerArch_GetCommProperties(PVOID pHead,LPCOMMPROP pCommProp)
// 参数：
//	IN pHead-ARCH信息
//	OUT pCommProp-保存属性信息
// 返回值：
//	无
// 功能描述：获取属性信息
// 引用: 
// ********************************************************************
VOID	SerArch_GetCommProperties(PVOID pHead,LPCOMMPROP pCommProp)
{
	*pCommProp = ((PSERARCH_INFO)pHead)->CommProp;
}
// ********************************************************************
// 声明：VOID SerArch_GetModemStatus(PVOID pHead, LPDWORD pModemStatus)
// 参数：
//	IN pHead-ARCH信息
//	OUT pModemStatus-保存modem状态信息
// 返回值：
//	无
// 功能描述：获取modem状态信息
// 引用: 
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
// 声明：BOOL	SerArch_XMitChar(PVOID pHead, UCHAR ComChar)
// 参数：
//	IN pHead-ARCH信息
//	IN ComChar-待发送的字符
// 返回值：
//	无
// 功能描述：立即发送1个字符
// 引用: 
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
			OUTREG(pInfoSerArch,rUTXH,ComChar);	// 传输寄存器
			// Make sure we release the register critical section
			//LeaveCriticalSection(&(pInfoSerArch->RegCritSec));
			break;
		}
		
		// If we couldn't write the data yet, then wait for a
		// TXINTR to come in and try it again.
		
		// Enable xmit intr.
		//EnINT(pInfoSerArch, pInfoSerArch->bINT);
		//EnSubINT(pInfoSerArch, pInfoSerArch->bTxINT | pInfoSerArch->bRxINT | pInfoSerArch->bErrINT); // canceled 
		// 2004-12-06 lilin modify, 以下两句的顺序是重要的
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
			
			//检查硬件
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
// 声明：DWORD	SerArch_IntrTypeQuery(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：查询中断类型
// 引用: 
// ********************************************************************
DWORD	SerArch_IntrTypeQuery(PVOID pHead)
{
    PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
    DWORD			interrupts	= INTR_NONE;
	volatile	ULONG	IntSubPndVal;//=0;

//	RETAILMSG(1,(TEXT("\r\n Ser_Query \r\n")));
	IntSubPndVal  = *(pInfoSerArch->UART_INTSUBSRCPND);
	//查询 出错状态---溢出 | 奇偶 | 帧] 错误
	if(IntSubPndVal & (pInfoSerArch->bErrINT) )
	{
		interrupts = INTR_LINE;  // Error status
	}
	else
	{
		//查询 接收状态
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
		//查询 发送状态
		else if((IntSubPndVal & (pInfoSerArch->bTxINT)) && pInfoSerArch->fSW_EnTxINT )
		{
			//RETAILMSG(1,(TEXT("SerArch_IntrTypeQuery:TxINT\n")));
			interrupts |= INTR_TX;	// Tx.
		}
	}

	//查询 是否由于硬件握手 要求继续发送
	if( pInfoSerArch->RestartTxForFlow )
    {
        interrupts |= INTR_TX;
		pInfoSerArch->RestartTxForFlow = 0;
    }

	//查询 MODEM状态
	//if( pInfoSerArch->dwOpt & OPT_MODEM )
    if( interrupts & INTR_TX )
	{
		ULONG       msr_val;
		volatile	WORD	msr_ret;
		WORD	msrChange;
		
		//读硬件
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
			//保存状态
			pInfoSerArch->ModemStatus = msr_ret;
			pInfoSerArch->msrChange = msrChange;

			interrupts |= INTR_MODEM;
		}
	}
	
	//查询 OSM2中断状态
//	irq |= HW_SA11x0GetOSM2( pHead );

    return(interrupts);
}

// ********************************************************************
// 声明：VOID SerArch_IntrHandleTx(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：处理发送中断
// 引用: 
// ********************************************************************
VOID SerArch_IntrHandleTx(PVOID pHead)
{
	PSERARCH_INFO   pInfoSerArch  = (PSERARCH_INFO)pHead;

	//////EdbgOutputDebugString("SerArch_IntrHandleTxHandler: set tie=0, only do this");
	EnterCriticalSection(&(pInfoSerArch->csTransmit));
	//EnterCriticalSection(&(pInfoSerArch->RegCritSec));

	//停止 发送中断
	//DisEnINT(pInfoSerArch, pInfoSerArch->bINT);
	DisEnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
	pInfoSerArch->fSW_EnTxINT = FALSE;
	//清除Pending串口中断
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
// 声明：VOID	SerArch_IntrHandleLine(PVOID pHead)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：处理状态中断
// 引用: 
// ********************************************************************
VOID	SerArch_IntrHandleLine(PVOID pHead)
{
	PSERARCH_INFO   pInfoSerArch  = (PSERARCH_INFO)pHead;

	//EdbgOutputDebugString("INTR_LINE\r\n");
	//检查硬件
	ser_CheckLine( pInfoSerArch, pInfoSerArch->pUART );
}

// ********************************************************************
// 声明：DWORD	SerArch_IntrHandleModem( PVOID pHead )
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	无
// 功能描述：处理MODEM中断
// 引用: 
// ********************************************************************
DWORD	SerArch_IntrHandleModem( PVOID pHead )
{
	PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	BOOL			fFlowChangeToOn = FALSE;

	//分析是否改变
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
		//状态  发生改变
		if( dwMdmEvt )
			pInfoSerArch->lpfnEventNotify( pInfoSerArch->pHeadDrv, dwMdmEvt );

		//恢复
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

			// 2004-12-06 lilin modify, 以下两句的顺序是重要的
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
// 声明：ULONG	SerArch_Recv( PVOID pHead, LPBYTE pBufDes, LONG* pnLenBuf )
// 参数：
//	IN pHead-ARCH信息
//	IN pBufDes-获取的数据
//	IN pnLenBuf-指定pBufDes的长度和实际获取数据的长度
// 返回值：
//	无
// 功能描述：处理接收中断
// 引用: 
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
	//是不是半工设备
	//if( pInfoSerArch->dwComFun == COM_SIR )
	//{
	//	EnterCriticalSection(&(pInfoSerArch->csTransmit));
	//}

	fRXFlag = FALSE;
	pBufEnd = pBufRecv + nLenRecv;
    while( pBufRecv<pBufEnd )
    {
		//读取数据
		cCharRx = *pBufRecv;
		pBufRecv ++;
		//RETAILMSG(1,(TEXT("\r\n [ %x ] \r\n"), cCharRx));
		
		//若是Dsr敏感，且Dsr是关的， 则 忽略数据
		if( lpDCB->fDsrSensitivity && !(pInfoSerArch->ModemStatus & MS_DSR_ON) )
		{
			//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: 若是Dsr敏感，且Dsr是关的，则 忽略数据\r\n")) );
			continue;
		}
		else if (!cCharRx && lpDCB->fNull)//空字符  是不是要丢弃
		{
			//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: 丢弃空字符\r\n")) );
			continue;
		}
		else
		{
			//出现事件字符EvtChar，通知WaitCommEvent
			if (cCharRx == lpDCB->EvtChar)
				fRXFlag = TRUE;
		}// for else <normal operation>
    }// for while

    if( fRXFlag )
	{
		//出现事件字符EvtChar，通知WaitCommEvent
		pInfoSerArch->lpfnEventNotify(pInfoSerArch->pHeadDrv, EV_RXFLAG);
	}

	//是不是半工设备
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
// 声明：ULONG	SerArch_Recv( PVOID pHead, LPBYTE pBufDes, LONG* pnLenBuf )
// 参数：
//	IN pHead-ARCH信息
//	IN pBufDes-获取的数据
//	IN pnLenBuf-指定pBufDes的长度和实际获取数据的长度
// 返回值：
//	无
// 功能描述：处理接收中断
// 引用: 
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
	//是不是半工设备
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
		//查询 出错状态---溢出 | 奇偶 | 帧] 错误
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
			//读取数据
			cCharRx = *(pInfoSerArch->pUFRXH);
			//RETAILMSG(1,(TEXT("\r\n [ %x ] \r\n"), cCharRx));
	
			//若是Dsr敏感，且Dsr是关的， 则 忽略数据
			if( lpDCB->fDsrSensitivity && !(pInfoSerArch->ModemStatus & MS_DSR_ON) )
			{
				//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: 若是Dsr敏感，且Dsr是关的，则 忽略数据\r\n")) );
				continue;
			}
			else if (!cCharRx && lpDCB->fNull)//空字符  是不是要丢弃
			{
				//RETAILMSG( 1, (TEXT("\r\nxyg_Recv: 丢弃空字符\r\n")) );
				continue;
			}
			else
			{
				//if( utsr1.pre )
				//{
				//	//出现错误，(fErrorChar && fParity)是否替代成ErrorChar
				//	if( lpDCB->fErrorChar && lpDCB->fParity)
				//		cCharRx = lpDCB->ErrorChar;
				//	else
				//		continue;
				//}
				//else
				{
					//出现事件字符EvtChar，通知WaitCommEvent
					if (cCharRx == lpDCB->EvtChar)
                        fRXFlag = TRUE;
					//出现，通知
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
	//清除Pending串口中断
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
	//引发 接收中断
	//EnINT(pInfoSerArch, pInfoSerArch->bINT);	//2004-12-06 lilin remove,应该在oemintr.c打开
	EnSubINT(pInfoSerArch, (pInfoSerArch->bRxINT|pInfoSerArch->bErrINT));
#endif

//	Debug_Arch();
    if( fRXFlag )
	{
		//出现事件字符EvtChar，通知WaitCommEvent
		pInfoSerArch->lpfnEventNotify(pInfoSerArch->pHeadDrv, EV_RXFLAG);
	}

	//是不是半工设备
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
// 声明：BOOL	ser_CheckFlowOff( PSERARCH_INFO pInfoSerArch )
// 参数：
//	IN pInfoSerArch-ARCH信息
// 返回值：
//	成功返回TRUE
// 功能描述：检查流控制
// 引用: 
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
// 声明：ULONG	SerArch_Send( PVOID pHead, LPBYTE pBufSrc, LONG* pnLenBuf )
// 参数：
//	IN pHead-ARCH信息
//	IN pBufSrc-发送数据的BUFFER
//	IN pnLenBuf-发送数据的BUFFER的长度
// 返回值：
//	成功返回发送的个数
// 功能描述：发送数据
// 引用: 
// ********************************************************************
ULONG	SerArch_Send( PVOID pHead, LPBYTE pBufSrc, LONG* pnLenBuf )
{
	volatile PSERARCH_INFO	pInfoSerArch = (PSERARCH_INFO)pHead;
	LPBYTE			pBufSrc_B;
	DWORD			nLenSend;
	ULONG        rFifoStat, TxFifoCnt;
	unsigned int tmpreg;
	//unsigned int FifoModeReg;

	//硬件握手---判断是否可以发送数据(由对方的Dsr/Cts来控制)
	//if( !*pnLenBuf || ((pInfoSerArch->dwOpt & OPT_MODEM) && ser_CheckFlowOff(pInfoSerArch)) )
	if( !*pnLenBuf || ser_CheckFlowOff(pInfoSerArch) )
	{
		//EdbgOutputDebugString( "硬件握手 xyg --fail...\r\n" );
		*pnLenBuf = 0;
		//WRITE_BITFIELD(struct utcr3Bits,&pInfoSerArch->pUART->utcr3,tie,0);

		//停止 发送中断
		DisEnSubINT(pInfoSerArch, pInfoSerArch->bTxINT);
		pInfoSerArch->fSW_EnTxINT = FALSE;
		//清除Pending串口中断
		ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bTxINT );
		ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
		//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
		//{
		//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
		//}

		return 0;
	}

	//准备发送数据
	pBufSrc_B= pBufSrc;
	//nLenSend = *pnLenBuf;

	//开始发送数据
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
		//检查硬件是否出错，并进入正常状态
		//utsr0 = pUART->utsr0;
		//if( (utsr0.eif) || (utsr0.rbb) || (utsr0.reb) )
		//	ser_CheckLine(pInfoSerArch, pUART);
		//判断硬件是否忙
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
		//传输数据
		*(pInfoSerArch->pUFTXH) = *pBufSrc;	//发送数据
		pBufSrc ++;
		nLenSend--;
	}

	// 在传输完毕之后，如果是FIFO模式，则需要进行善后处理
	//if (FifoModeReg & 0x1) // FIFO Mode enabled.
#if 0	// 取消下面的等待
	{
		unsigned int cnt = 0;
		while ( 1 )
		{
			//#define INREG(pInfo, reg) (pInfo->s2410SerReg->reg)
			tmpreg = INREG(pInfoSerArch, rUFSTAT);	//获取状态值，循环进行判断
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
	//清除Pending串口中断
	ClearSubINTPnd(pInfoSerArch, pInfoSerArch->bTxINT ); //| pInfoSerArch->bErrINT
	ClearINTPnd(pInfoSerArch, pInfoSerArch->bINT);
	//if ((*pInfoSerArch->UART_INTPND) & pInfoSerArch->bINT )
	//{
	//	(*pInfoSerArch->UART_INTPND)		|= pInfoSerArch->bINT;
	//}
	//引发 发送中断
	//EnINT(pInfoSerArch, pInfoSerArch->bINT);

	// 2004-12-06 lilin modify, 以下两句的顺序是重要的	
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
// 声明：BOOL	SerArch_IOControl(PVOID pHead, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
// 参数：
//	IN pHead-ARCH信息
//	IN dwCode-IO控制码
//	IN pBufIn_-输入BUFFER
//	IN dwLenIn-输入BUFFER的长度
//	OUT pBufOut_-输出BUFFER
//	IN dwLenOut-输出BUFFER的长度
//	OUT pdwActualOut-输出BUFFER实际获取数据的长度
// 返回值：
//	成功返回TRUE
// 功能描述：IOControl操作
// 引用: 
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
// 声明：BOOL	SerArch_IREnable(PVOID pHead,ULONG BaudRate)
// 参数：
//	IN pHead-ARCH信息
//	IN BaudRate-波特率
// 返回值：
//	成功返回TRUE
// 功能描述：启动红外功能，并设置波特率
// 引用: 
// ********************************************************************
BOOL	SerArch_IREnable(PVOID pHead,ULONG BaudRate)
{
	return FALSE;
}

// ********************************************************************
// 声明：BOOL	SerArch_IREnable(PVOID pHead,ULONG BaudRate)
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	成功返回TRUE
// 功能描述：取消红外功能
// 引用: 
// ********************************************************************
BOOL	SerArch_IRDisable(PVOID pHead)
{
	return FALSE;
}

//-----------------------------------------
//for debug
#ifdef	DEBUG_SERARCH

//调试函数
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
// 声明：static ULONG SerArch_GetRxBufferSize( PVOID pHead )
// 参数：
//	IN pHead-ARCH信息
// 返回值：
//	成功返回需要的buffer大小
// 功能描述：
//	得到PDD需要的buffer大小，对于 16550 UARTS，仅仅需要返回0,
//	因为需要的最小buffer 是2048
// 引用: 
// ********************************************************************
// 
static ULONG SerArch_GetRxBufferSize( PVOID pHead )
{
	return 1024*8;
}
//-----------------------------------------

static	VTBL_ARCH_SER	SerVTbl =
{
	//通常 函数
	SerArch_Init,
	SerArch_InitLast,
	SerArch_Deinit,
	SerArch_Open,
	SerArch_Close,
	SerArch_PowerOff,
	SerArch_PowerOn,
	SerArch_IOControl,
	//收发 函数
	SerArch_Recv,
	SerArch_Send,
	SerArch_XMitChar,
	//中断 函数
	SerArch_IntrTypeQuery,
	SerArch_IntrHandleModem,
	SerArch_IntrHandleLine,
	SerArch_IntrHandleTx,

	//IR启动的功能 函数
	SerArch_IREnable,
	SerArch_IRDisable,

	//Pin Singal的功能 函数
	SerArch_ClearPinDTR,
	SerArch_SetPinDTR,
	SerArch_ClearPinRTS,
	SerArch_SetPinRTS,

	//Break的功能 函数
	SerArch_ClearBreak,
	SerArch_SetBreak,
	
	//Purge的功能 函数
	SerArch_PurgeComm,
	
	//状态属性的功能 函数---for ClearCommError
	SerArch_GetComStat,
	//状态属性的功能 函数---for GetCommModemStatus
	SerArch_GetModemStatus,
	//状态属性的功能 函数---for GetCommProperties
	SerArch_GetCommProperties,	
	//状态属性的功能 函数---for SetCommState
	SerArch_SetDCB,
	SerArch_GetRxBufferSize
};

OBJECT_ARCH	SerObj =
{
	SYSINTR_SERIAL,
	(LPVOID)&SerVTbl
};


