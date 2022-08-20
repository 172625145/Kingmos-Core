/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：内核中断处理
版本号：1.0.0
开发时期：2003-04-04
作者：李林
修改记录：
******************************************************/

#include "ewindows.h"
#include "drv_glob.h"
#include "oalintr.h"
#include "cpu.h"
#include "oemfunc.h"
#include <s2410.h>

extern void HalTimerInit();
extern DWORD OEM_GetTickCount( void );

// ********************************************************************
//声明：void InitSDMMC( void )
//参数：无
//返回值：无
//功能描述：OEM硬件的Smart Card基本初始化
//引用: 
// ********************************************************************

static void InitSDMMC(void) 
{
	volatile IOPreg *s2410IOP = (IOPreg *)IOP_BASE;

	/* Initialize SDMMC and Configure SDMMC Card Detect	*/
	/* ::::::::::::::::::::::::::::::::::: GPIO Configure ::::::::::::::::::::::::::::::::::::: */
	RETAILMSG(1,(TEXT("SDMMC config current rGPGCON: %x\r\n"), s2410IOP->rGPGCON));  
	/* We must need this PULL-UP routines to inialize. */
	//s2410IOP->rGPGUP = 0xF800;   
	s2410IOP->rGPGUP &= ~(1<<10);	
	s2410IOP->rGPGCON &= ~((0x3 << 20));   
	s2410IOP->rGPGCON |=  ((0x2 << 20));		/* External Interrupt #18 Enable				*/
	RETAILMSG(1,(TEXT("SDMMC config set rGPGCON: %x\r\n"), s2410IOP->rGPGCON));   

	s2410IOP->rEXTINT2 &= ~(0x7 << 8);			/* Configure EINT18 as Both Edge Mode		*/
	s2410IOP->rEXTINT2 |=  (0x7 << 8);

	/* Configure SDMMC Write Protect */
	s2410IOP->rGPHUP = 0x0;   
	s2410IOP->rGPHCON &= ~((0x3 << 16));   
	s2410IOP->rGPHCON |=  ((0x0 << 16));		/* GPH8/UCLK Write Protect Pin					*/

	RETAILMSG(1,(TEXT("SDMMC config Init Done.\r\n")));   
}

static void PowerMC35i()
{
	volatile IOPreg *s2410IOP = (IOPreg *)IOP_BASE;

	/* Initialize SDMMC and Configure SDMMC Card Detect	*/
	/* ::::::::::::::::::::::::::::::::::: GPIO Configure ::::::::::::::::::::::::::::::::::::: */
	s2410IOP->rGPFUP |= (1<<6 | 0x1<<7);	
	s2410IOP->rGPFCON &= ~((0x3 << 12) | (0x3<<14));   
	s2410IOP->rGPGCON |=  ((0x1 << 12) | (0x1<<14));	// work as output

	s2410IOP->rGPGDAT &= ~((0x1<<6) | (0x1<<7));		// clear data

	msWait(10);
	s2410IOP->rGPGDAT |= (0x1<<6);						// enable /IGT
	msWait(200);
	s2410IOP->rGPGDAT &= ~(0x1<<6);						// disable /IGT
}

// ********************************************************************
//声明：void OEMInitInterrupts( void )
//参数：无
//返回值：无
//功能描述：OEM硬件的中断基本初始化
//引用: 
// ********************************************************************

static void OEMInitInterrupts(void)
{
	volatile INTreg *s2410INT = (INTreg *)INT_BASE;
	volatile IOPreg *s2410IOP = (IOPreg *)IOP_BASE;

    //RETAILMSG(1, (TEXT("Clear pending interrupts...\r\n")));

	// Configure EINT9 for CS8900 interrupt.
	//
	s2410IOP->rGPGCON  = (s2410IOP->rGPGCON  & ~(0x3 << 0x2)) | (0x2 << 0x2);		// GPG1 == EINT9.
	s2410IOP->rGPGUP   = (s2410IOP->rGPGUP   |  (0x1 << 0x1));						// Disable pull-up.
	s2410IOP->rEXTINT1 = (s2410IOP->rEXTINT1 & ~(0xf << 0x4)) | (0x1 << 0x4);		// Level-high triggered.

	// Configure EINT8 for PD6710 interrupt.
	//
	s2410IOP->rGPGCON  = (s2410IOP->rGPGCON  & ~(0x3 << 0x0)) | (0x2 << 0x0);		// GPG0 == EINT8.
	s2410IOP->rGPGUP   = (s2410IOP->rGPGUP   |  (0x1 << 0x0));						// Disable pull-up.
	s2410IOP->rEXTINT1 = (s2410IOP->rEXTINT1 & ~(0xf << 0x0)) | (0x1 << 0x0);		// Level-high triggered.

	// Mask and clear all peripheral interrupts (these come through a second-level "GPIO" interrupt register).
	//
	s2410IOP->rEINTMASK = BIT_ALLMSK;	// Mask all EINT interrupts.
	s2410IOP->rEINTPEND = BIT_ALLMSK;	// Clear pending EINT interrupts.

	// Mask and clear all interrupts.
	//
    s2410INT->rINTMSK = BIT_ALLMSK;			// Mask all interrupts (reset value).
	s2410INT->rINTMSK &= ~BIT_BAT_FLT;
    s2410INT->rSRCPND = BIT_ALLMSK;			// Clear pending interrupts.
    s2410INT->rINTPND = s2410INT->rINTPND;	// S3C2410X developer notice (page 4) warns against writing a 1 to any
											//  0 bit field in the INTPND register.  Instead we'll write the INTPND value itself.
}

// ********************************************************************
//声明：void OEM_Init( void )
//参数：无
//返回值：无
//功能描述：OEM硬件的基本初始化
//引用: 
// ********************************************************************

#define DEBUG_OEM_INIT 0
void OEM_Init( void )
{
    // Instead of calling OEMWriteDebugString directly, call through exported
    // function pointer.  This will allow these messages to be seen if debug
    // message output is redirected to Ethernet or the parallel port.  Otherwise,
    // lpWriteDebugStringFunc == OEMWriteDebugString.
	DEBUGMSG( DEBUG_OEM_INIT, ("\nOEM_Init:Kingmos Firmware Init...\r\n") );

    //
    // Set up translation constant for GetIdleTime() (1 ms units).
    // Note: Since curridlehigh, curridlelow is counting in ms, and GetIdleTime()
    // reports in ms, the conversion ratio is one.  If curridlehigh, curridlelow
    // were using other units (like ticks), then the conversion would be calculated
    // from the clock frequency.
    //
//    idleconv = 1;

	// Initialize interrupts.
	//
    DEBUGMSG( DEBUG_OEM_INIT, (TEXT("OEM_Init: Initializing system interrupts...\r\n") ) );
    
	OEMInitInterrupts();

	// Initialize the system clock(s).
	//
    DEBUGMSG( DEBUG_OEM_INIT, (TEXT("OEM_Init: Initializing system clock(s)...\r\n")));
    InitClock();

    // Initialize driver globals area.
	//
    DEBUGMSG( DEBUG_OEM_INIT, (TEXT("OEM_Init: Initializing driver globals area...\r\n")) );
    memset((PVOID)DRIVER_GLOBALS_ZEROINIT_START, 0, DRIVER_GLOBALS_ZEROINIT_SIZE);
    
    // Initialize S2410X01 LCD controller
    //InitDisplay();
	
	//InitSDMMC();
    
    PowerMC35i();

	{
	}
	
    // Initialize debug Ethernet (KITL) connection.
    //
	//InitDebugEther();

	DEBUGMSG( DEBUG_OEM_INIT,(TEXT("OEM_Init Done...\r\n")));

}

// ********************************************************************
//声明：int OEM_EnumExtensionDRAM( LPMEM_SEGMENT lpSegs, UINT uiMaxSegNum )
//参数：
//OUT lpSegs-内存段数组
//IN  uiMaxSegNum-内存段数
//返回值：无
//功能描述：由OEM填写系统的其他附加的内存段
//引用: 
// ********************************************************************

int OEM_EnumExtensionDRAM( LPMEM_SEGMENT lpSegs, UINT uiMaxSegNum )
{
	return 0;
}

DWORD dwIdleTick = 0;
#define DEBUG_OEM_EnterIdleMode 1

int OEM_EnterIdleMode( DWORD dwFlag )
{
	
	DWORD dwCurTick = OEM_GetTickCount();
	
	if( dwCurTick - dwIdleTick > 5000 )
	{
		dwIdleTick = dwCurTick;
	    DEBUGMSG( DEBUG_OEM_EnterIdleMode, (TEXT("OEM_EnterIdleMode(%x).\r\n"), dwIdleTick) );		
	}
	return 0;
}

int OEM_PowerOff( DWORD dwFlag )
{
	return 0;
}




