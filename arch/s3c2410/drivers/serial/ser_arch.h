/**************************************************************************
* Copyright (c)  微逻辑(WEILUOJI). All rights reserved.                   *
**************************************************************************/

#ifndef		_SERARCH_H_
#define		_SERARCH_H_

#ifdef __cplusplus
extern "C" {
#endif

	
// Line Status Register
#define  COM2410_LSR_OE     0x1
#define  COM2410_LSR_PE     0x2
#define  COM2410_LSR_FE     0x4
#define  COM2410_LSR_BI     0x8   //Break Detect
// dummy value
#define  COM2410_LSR_THRE   0xe
#define  COM2410_LSR_DR     0xf
// Fifo Status Register

// following two cts values are read from modem control register.
#define  COM2410_MSR_CTS    0x1
#define  COM2410_MSR_DCTS   0x4
// following two cts values are read from modem GIO port.
#define  COM2410_MSR_DSR    0x2
#define  COM2410_MSR_DDSR   0x8



//事件回调函数
typedef	VOID	(*EVENT_FUNC)(PVOID Arg1, ULONG Arg2);

//dwComFun: 串口功能定义
#define	COM_UART		0
#define	COM_SIR			1
#define	COM_FIR			2

//dwOpt
#define	OPT_MODEM		0x00000001	//有MODEM引脚

//
//串口设备---ARCH层的实例结构
//
typedef struct _SERARCH_INFO
{
	//硬件地址信息
	volatile	PS2410_UART_REG	pUART;	//UART
	volatile	IOPreg*	pGPIO;	//GPIO
	volatile	INTreg*	pIrqCtrlAddr;//IRQ-interrupt
	volatile unsigned int  *UART_INTMASK;
	volatile unsigned int  *UART_INTSUBMASK;
	volatile unsigned int  *UART_INTPND;
	volatile unsigned int  *UART_INTSRCPND;
	volatile unsigned int  *UART_INTSUBSRCPND;

    volatile unsigned char *pUFTXH;  // Tx holding register for byte access.
    volatile unsigned char *pUFRXH;  // Rx holding register for byte access.
	//
	ULONG		bINT;
	ULONG		bTxINT;
	ULONG		bRxINT;
	ULONG		bErrINT;
	BOOL		fSW_EnTxINT;		// S/W flag of Enable Tx interrupt
	//BOOL		RxDiscard;			// S/W flag of Enable Tx interrupt

    //CRITICAL_SECTION	RegCritSec;		// @field Protects UART
/*
#ifndef CANCEL_XYG_SER_RX
	CRITICAL_SECTION	csBufRW;
	SER_RWBUF*			lpRWBuf;
#endif
*/

	//串口硬件功能信息
	DWORD				dwIndex;	//
	DWORD				dwIoBase;	//串口寄存器的 物理地址
	DWORD				dwISR;

	//DWORD				dwOpt;
	//DWORD				dwComFun;	//串口功能
	DWORD				dwSizeTbl;
	PBAUDTBL			pBaudTbl;	//波特率Table; (in "SERBAUD.H")
	
	//Driver层的信息
	PVOID				pHeadDrv;
	EVENT_FUNC			lpfnEventNotify;//上层回调函数---通知事件
	
	//设备属性
	DCB*				lpDCB;
	COMMPROP			CommProp;

	//操作信息---ISR
	HANDLE				hThrdISR;
	DWORD				dwThrdISR;
	HANDLE				hEvtISR;
	BOOL				fExitISR;

	//操作信息---XMit
	HANDLE				hEvtXMit;
	CRITICAL_SECTION	csTransmit;	//

	//操作信息---记录
	ULONG				dwOpenCount;	//调用hwopen的次数
	WORD				ModemStatus;
	WORD				msrChange;
	ULONG				CommErrors;

	//操作信息---位控制的：流、PIN、POWER
	DWORD				FlowOffDSR:1;
	DWORD				FlowOffCTS:1;
	DWORD				RestartTxForFlow:1;
	DWORD				fPowerOff:1;
//	DWORD				fWaitTxim:1;
	//DWORD				DTRPin :1;
	//DWORD				RTSPin :1;
	
} SERARCH_INFO, *PSERARCH_INFO;

//----------------------------------------------------

//设备属性
#define SP_SERIALCOMM		(0x00000001)
#define	SP3_SIZE_TXFIFO		8
#define	SP3_SIZE_RXFIFO		12


//// S3C2400 UART Register

///////++ UART CONTROL REGISTER ++
// Line control register bitvalue mask
#define SER2410_PARITY_MASK     0x38
#define SER2410_STOPBIT_MASK    0x4
#define SER2410_DATABIT_MASK    0x3
#define SER2410_IRMODE_MASK     0x40

// Fifo Status
#define SER2410_FIFOSTAT_MASK   0xf0

//
#define SER2410_FIFOFULL_TX     0x200
#define SER2410_FIFOCNT_MASK_TX 0xf0
#define SER2410_FIFO_DEPTH_TX 16

//
#define SER2410_INT_INVALID     0x5a5affff

// Modem control register
#define SER2410_AFC             (0x10)
#define SER2410_RTS             0x1
//Receive Mode
#define RX_MODE_MASK          (0x11)
#define RX_DISABLE            (0x00)
#define RX_INTPOLL            (0x01)
#define RX_DMA0               (0x10)
#define RX_DMA1               (0x11)
//Transmit Mode
#define TX_MODE_MASK          (0x11 << 2)
#define TX_DISABLE            (0x00 << 2)
#define TX_INTPOLL            (0x01 << 2)
#define TX_DMA0               (0x10 << 2)
#define TX_DMA1               (0x11 << 2)
//Send Break Signal
#define BS_MASK               (0x01 << 4)
#define BS_NORM               (0x00 << 4)
#define BS_SEND               (0x01 << 4)
//Loop-back Mode
#define LB_MASK               (0x01 << 5)
#define LB_NORM               (0x00 << 5)
#define LB_MODE               (0x01 << 5)
//Rx Error Status Interrupt Enable
#define RX_EINT_MASK          (0x01 << 6)
#define RX_EINTGEN_OFF        (0x00 << 6)
#define RX_EINTGEN_ON         (0x01 << 6)
//Rx Time Out Enable
#define RX_TIMEOUT_MASK       (0x01 << 7)
#define RX_TIMEOUT_DIS        (0x00 << 7)
#define RX_TIMEOUT_EN         (0x01 << 7)
//Rx Interrupt Type
#define RX_INTTYPE_MASK       (0x01 << 8)
#define RX_INTTYPE_PUSE       (0x00 << 8)
#define RX_INTTYPE_LEVEL      (0x01 << 8)
//Tx Interrupt Type
#define TX_INTTYPE_MASK       (0x01 << 9)
#define TX_INTTYPE_PUSE       (0x00 << 9)
#define TX_INTTYPE_LEVEL      (0x01 << 9)
// Clock selection
#define CS_MASK	(0x01 << 10)
#define CS_PCLK (0x00 << 10)
#define CS_UCLK	(0x01 << 10)

/////////////////////////////////////////////////////////////////////////////////////////

#define INREG(pInfo, reg)				(pInfo->pUART->reg)
#define OUTREG(pInfo, reg, value)		(pInfo->pUART->reg	= value)
// set register by orring..
#define SETREG(pInfo, reg, value)		(pInfo->pUART->reg	|= value)
#define CLEARREG(pInfo, reg, value)		(pInfo->pUART->reg	&= ~value)

#define DisEnINT(pInfo, value)			(*(pInfo->UART_INTMASK)		|= (value))
#define DisEnSubINT(pInfo, value)		(*(pInfo->UART_INTSUBMASK)	|= (value))

#define EnINT(pInfo, value)				(*(pInfo->UART_INTMASK)		&= ~(value))
#define EnSubINT(pInfo, value)			(*(pInfo->UART_INTSUBMASK)	&= ~(value))

#define GetSubINTStatus(pInfo)			((*(pInfo->UART_INTSUBMASK)) & 0x1ff)

#define ClearINTPnd(pInfo, value)		(*(pInfo->UART_INTSRCPND)		= (value))
#define ClearSubINTPnd(pInfo, value)	(*(pInfo->UART_INTSUBSRCPND)	= (value))
//#define GetSubINTPndStatus(pInfo)		(*(pInfo->UART_INTSUBSRCPND) & 0x1ff)
#define GetSubINTPndStatus(pInfo) 1



#ifdef __cplusplus
}	
#endif

#endif	//	_SERARCH_H_
