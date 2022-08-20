/***************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
***************************************************/
/*****************************************************
文件说明：触摸屏驱动程序
版本号：1.0.0
开发时期：2003-12-01
作者：AlexZeng
修改记录：
******************************************************/
#include "ewindows.h" 
#include <oalintr.h>
#include <s2410.h>
#include <touch.h>
#include <drv_glob.h>
#include <reg.h>
#include <tchpdd.h>

const UINT uMinCalCount = 25;
const UINT idTouchIrq = SYSINTR_TOUCH;
const UINT idTouchChangedIrq = SYSINTR_TOUCH_CHANGED;

static volatile IOPreg * const v_pIOPregs = (volatile IOPreg *)IOP_BASE;
static volatile ADCreg * const v_pADCregs = (volatile ADCreg *)ADC_BASE;
static volatile PWMreg * const v_pPWMregs = (volatile PWMreg *)PWM_BASE;
static volatile INTreg * const v_pINTregs = (volatile INTreg *)INT_BASE;
static volatile PDRIVER_GLOBALS   const v_pDriverGlobals = (PDRIVER_GLOBALS)DRIVER_GLOBALS_PHYSICAL_MEMORY_SIZE;


static void TouchPanelPowerOn( void );
static void TouchPanelPowerOff( void );

// Global flag to indicate whether we are in power handler routine, so
// we know to avoid system calls.
static BOOL bInPowerHandler = FALSE;
static volatile unsigned short xbuf[10], ybuf[10];


#define TOUCH_TIMER_PRESCALER	24
#define TOUCH_TIMER_DIVIDER 4

#define JYLEE_TEST 1

#define TFT240_320	1
#define TFT640_480	4
#define LCD_TYPE	TFT240_320	//TFT640_480

#define  MAX_ADVAL 1020

#if ( LCD_TYPE == TFT640_480 )
	#define ADC_DELAY_TIME	5000		// charlie, 020620
#else
//lilin
//	#define ADC_DELAY_TIME	1000	// Charlie
	#define ADC_DELAY_TIME	0xff	// Charlie
#endif

//#define ADCPRS  49
//lilin
#define ADCPRS  49

#define DELAY   1000
#define SDELAY  5


#define FILTER_LIMIT 25
#define INT int



//
// PDD Internal Support Routines
//

/*++
    @doc IN_TOUCH_DDSI INTERNAL DRIVERS PDD TOUCH_PANEL

    @func VOID | PddpTouchPanelGetSamples |
    Copies from the pen dma area the most recent point sample into the location
    pointed to by pPointSamples.  During the copy the sample information is
    adjusted to be consistent with the 12 bit pen data format.
    Has the side effect of reinitializing ioPenPointer if we are near the
    end of the pen sample area.
--*/
static
void
PddpTouchPanelGetSamples(
    PTOUCHPANEL_POINT_SAMPLE pPointSamples //@PARM Pointer to where the samples will be stored.
    )
{
//    ULONG   devDrvPointer;
    ULONG   irg;

    //
    // Copy the samples to our buffer munging the data for the 12 bit
    //  pen data format.
    //

    for ( irg = 0; irg < NUMBER_SAMPLES_PER_POINT; irg++ )
    {
        pPointSamples[ irg ].XSample = xbuf[irg];
        pPointSamples[ irg ].YSample = ybuf[irg];
    }
}


/*++

Routine Description:

    Gathers the most recent sample and evaluates the sample returing
    the determined tip state and the `best guess' for the X and Y coordinates.

    Note: Determined empirically that the variance of the X coordinate of the
          first sample from all other samples is large enough that in order
          to keep the nominal variance small, we discard the first sample.

          Cases of a light touch that locks the ADC into
          seeing X and Y coordinate samples of 0x277 regardless of how the pen
          moves or presses have been seen. XXXXX


Arguments:

    pTipState   Pointer to where the tip state information will be returned.

    pUnCalX     Pointer to where the x coordinate will be returned.

    pUnCalY     Pointer to where the y coordinate will be returned.

Return Value:

    None.

Autodoc Information:

    @doc IN_TOUCH_DDI INTERNAL DRIVERS PDD TOUCH_PANEL

    @func VOID | PddpTouchPanelEvaluateSamples |
    Gathers the most recent sample and evaluates the sample returing
    the determined tip state and the `best guess' for the X and Y coordinates.

--*/

#define ZONE_SAMPLES 0
static void
PddpTouchPanelEvaluateSamples(
    DWORD	*pSampleFlags, //@PARM Pointer to where the tip state information will be returned.
    INT							*pUncalX,      //@PARM Pointer to where the x coordinate will be returned.
	INT							*pUncalY       //@PARM Pointer to where the y coordinate will be returned.

    )
{
    LONG    dlXDiff0;
    LONG    dlXDiff1;
    LONG    dlXDiff2;
    LONG    dlYDiff0;
    LONG    dlYDiff1;
    LONG    dlYDiff2;

    TOUCHPANEL_POINT_SAMPLES rgPointSamples;

    //
    // Get the sample.
    //

    PddpTouchPanelGetSamples( rgPointSamples );


    //
    // Calcuate the differences for the X samples and insure that
    // the resulting number is positive.
    //

    dlXDiff0 = rgPointSamples[ 0 ].XSample - rgPointSamples[ 1 ].XSample;
    dlXDiff1 = rgPointSamples[ 1 ].XSample - rgPointSamples[ 2 ].XSample;
    dlXDiff2 = rgPointSamples[ 2 ].XSample - rgPointSamples[ 0 ].XSample;
    dlXDiff0 = dlXDiff0 > 0  ? dlXDiff0 : -dlXDiff0;
    dlXDiff1 = dlXDiff1 > 0  ? dlXDiff1 : -dlXDiff1;
    dlXDiff2 = dlXDiff2 > 0  ? dlXDiff2 : -dlXDiff2;

    //
    // Calcuate the differences for the Y samples and insure that
    // the resulting number is positive.
    //

    dlYDiff0 = rgPointSamples[ 0 ].YSample - rgPointSamples[ 1 ].YSample;
    dlYDiff1 = rgPointSamples[ 1 ].YSample - rgPointSamples[ 2 ].YSample;
    dlYDiff2 = rgPointSamples[ 2 ].YSample - rgPointSamples[ 0 ].YSample;
    dlYDiff0 = dlYDiff0 > 0  ? dlYDiff0 : -dlYDiff0;
    dlYDiff1 = dlYDiff1 > 0  ? dlYDiff1 : -dlYDiff1;
    dlYDiff2 = dlYDiff2 > 0  ? dlYDiff2 : -dlYDiff2;

    //
    // The final X coordinate is the average of coordinates of
    // the two MIN of the differences.
    //

    if ( dlXDiff0 < dlXDiff1 )
    {
        if ( dlXDiff2 < dlXDiff0 )
        {

            *pUncalX = (ULONG)( ( ( rgPointSamples[ 0 ].XSample + rgPointSamples[ 2 ].XSample ) >> 1 ) );
        }
        else
        {

            *pUncalX = (ULONG)( ( ( rgPointSamples[ 0 ].XSample + rgPointSamples[ 1 ].XSample ) >> 1 ) );
        }
    }
    else if ( dlXDiff2 < dlXDiff1 )
    {

            *pUncalX = (ULONG)( ( ( rgPointSamples[ 0 ].XSample + rgPointSamples[ 2 ].XSample ) >> 1 ) );
    }
    else
    {

            *pUncalX = (ULONG)( ( ( rgPointSamples[ 1 ].XSample + rgPointSamples[ 2 ].XSample ) >> 1 ) );
    }

    //
    //
    // The final Y coordinate is the average of coordinates of
    // the two MIN of the differences.
    //

    if ( dlYDiff0 < dlYDiff1 )
    {
        if ( dlYDiff2 < dlYDiff0 )
        {

            *pUncalY = (ULONG)( ( ( rgPointSamples[ 0 ].YSample + rgPointSamples[ 2 ].YSample ) >> 1 ) );
        }
        else
        {

            *pUncalY = (ULONG)( ( ( rgPointSamples[ 0 ].YSample + rgPointSamples[ 1 ].YSample ) >> 1 ) );
        }
    }
    else if ( dlYDiff2 < dlYDiff1 )
    {

            *pUncalY = (ULONG)( ( ( rgPointSamples[ 0 ].YSample + rgPointSamples[ 2 ].YSample ) >> 1 ) );
    }
    else
    {

            *pUncalY = (ULONG)( ( ( rgPointSamples[ 1 ].YSample + rgPointSamples[ 2 ].YSample ) >> 1 ) );
    }

    //
    // Validate the coordinates and set the tip state accordingly.
    //

    if ( dlXDiff0 > DELTA_X_COORD_VARIANCE ||
         dlXDiff1 > DELTA_X_COORD_VARIANCE ||
         dlXDiff2 > DELTA_X_COORD_VARIANCE ||
         dlYDiff0 > DELTA_Y_COORD_VARIANCE ||
         dlYDiff1 > DELTA_Y_COORD_VARIANCE ||
         dlYDiff2 > DELTA_Y_COORD_VARIANCE )
    {

//#ifdef DBGPOINTS1
        DEBUGMSG( ZONE_SAMPLES, (TEXT("Sample 0: X 0x%x Y 0x%x\r\n"),
               rgPointSamples[ 0 ].XSample, rgPointSamples[ 0 ].YSample) );
        DEBUGMSG( ZONE_SAMPLES, (TEXT("Sample 1: X 0x%x Y 0x%x\r\n"),
               rgPointSamples[ 1 ].XSample, rgPointSamples[ 1 ].YSample) );
        DEBUGMSG( ZONE_SAMPLES, (TEXT("Sample 2: X 0x%x Y 0x%x\r\n"),
               rgPointSamples[ 2 ].XSample, rgPointSamples[ 2 ].YSample) );


        if ( dlXDiff0 > DELTA_X_COORD_VARIANCE )
            DEBUGMSG( ZONE_SAMPLES, (TEXT("XDiff0 too large 0x%x\r\n"), dlXDiff0) );
        if ( dlXDiff1 > DELTA_X_COORD_VARIANCE )
            DEBUGMSG( ZONE_SAMPLES, (TEXT("XDiff1 too large 0x%x\r\n"), dlXDiff1) );
        if ( dlXDiff2 > DELTA_X_COORD_VARIANCE )
            DEBUGMSG( ZONE_SAMPLES, (TEXT("XDiff2 too large 0x%x\r\n"), dlXDiff2) );

        if ( dlYDiff0 > DELTA_Y_COORD_VARIANCE )
            DEBUGMSG( ZONE_SAMPLES, (TEXT("YDiff0 too large 0x%x\r\n"), dlYDiff0) );
        if ( dlYDiff1 > DELTA_Y_COORD_VARIANCE )
            DEBUGMSG( ZONE_SAMPLES, (TEXT("YDiff1 too large 0x%x\r\n"), dlYDiff1) );
        if ( dlYDiff2 > DELTA_Y_COORD_VARIANCE )
            DEBUGMSG( ZONE_SAMPLES, (TEXT("YDiff2 too large 0x%x\r\n"), dlYDiff2) );

//#endif // DBGPOINTS1

    }
    else
    {
        //
        // Sample is valid. Set tip state accordingly.
        //
		*pSampleFlags = SAMPLE_VALID | SAMPLE_DOWN;//TouchSampleValidFlag | TouchSampleDownFlag;
    }

    DEBUGMSG( ZONE_SAMPLES, (TEXT("Filtered - SampleFlags: 0x%x X: 0x%x Y: 0x%x\r\n"),
           *pSampleFlags, *pUncalX, *pUncalY) );

}

// **************************************************
// 申明：static BOOL Touch_Pen_filtering(INT *px, INT *py)
// 参数：
//	INT px - 
//	INT py
// 返回值：
//	TRUE/FALSE 创建(成功/失败)
// 功能描述：
//	检查x,y的有效性
// 引用: 
// **************************************************

static BOOL Touch_Pen_filtering(int *px, int *py)
{
	BOOL RetVal = TRUE;
	// TRUE  : Valid pen sample
	// FALSE : Invalid pen sample
	static int count = 0;
	static int x[2], y[2];
	int TmpX, TmpY;
	int dx, dy;
	

	count++;

	if (count > 2) 
	{ 
		// apply filtering rule
		count = 2;
		
		// average between x,y[0] and *px,y
		TmpX = (x[0] + *px) / 2;
		TmpY = (y[0] + *py) / 2;
		
		// difference between x,y[1] and TmpX,Y
		dx = (x[1] > TmpX) ? (x[1] - TmpX) : (TmpX - x[1]);
		dy = (y[1] > TmpY) ? (y[1] - TmpY) : (TmpY - y[1]);
		
		if ((dx > FILTER_LIMIT) || (dy > FILTER_LIMIT)) {

			// Invalid pen sample

			*px = x[1];
			*py = y[1]; // previous valid sample
			RetVal = FALSE;
			count = 0;
			
		} else {
			// Valid pen sample
			x[0] = x[1]; y[0] = y[1];		
			x[1] = *px; y[1] = *py; // reserve pen samples
			
			RetVal = TRUE;
		}
		
	} else { // till 2 samples, no filtering rule
	
		x[0] = x[1]; y[0] = y[1];		
		x[1] = *px; y[1] = *py; // reserve pen samples
		
		RetVal = FALSE;	// <- TRUE jylee 2003.03.04 
	}
	
	return RetVal;
}

//
// PddpSetupPenDownIntr()
//
// Set up the UCB to give pen down interrupts.  If EnableIntr flag is set, enable
// the interrupt, otherwise, leave it disabled.  Note: - Caller must hold semaphore
// when this function is called to protect access to the shared UCB registers.
//
static BOOL PddpSetupPenDownIntr(BOOL EnableIntr)
{
//	USHORT intrMask;

    // I don't know why we enable the interrupt here.
    
    
    //
    // Setup ADC register
    //

	// kang code
    // Enable Prescaler,Prescaler,AIN5/7 fix,Normal,Disable read start,No operation
    // Down,YM:GND,YP:AIN5,XM:Hi-z,XP:AIN7,XP pullup En,Normal,Waiting for interrupt mode
	// kang code end
    // Down Int, YMON:0, nYPON:1, XMON:0;nXPON:1, Pullup:1, Auto Conv.,Waiting.
   //   xp_ain=(1 << 4)
//	  YP_AIN = (1 << 6)
//	  YM_GND= (1 << 7)
					
//    v_pADCregs->rADCTSC =(0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
//    v_pADCregs->rADCTSC =(0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
    v_pADCregs->rADCTSC =(0<<8)|(1<<7)|(1<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(3);	
//    v_pADCregs->rADCTSC=(1 << 5) | (1 << 6) | (1 << 3);
		
    v_pADCregs->rADCDLY = ADC_DELAY_TIME;//default value for delay.    
//    v_pADCregs->rADCCON	= (1<<14)|(ADCPRS<<6)|(7<<3);	//setup channel, ADCPRS, Touch Input
    v_pADCregs->rADCCON	= (1<<14)|(ADCPRS<<6)|(2<<3);	//setup channel, ADCPRS, Touch Input    
//    v_pADCregs->rADCCON	= (1<<14)|(ADCPRS<<6)|(0<<3);	//setup channel, ADCPRS, Touch Input        


	return TRUE;

}

// **************************************************
// 申明：void TouchPanelPowerOn( void )
// 参数：
//	无
// 返回值：
//	TRUE/FALSE (成功/失败)
// 功能描述：
//	加电操作/使Touch可以工作
// 引用: 
// ************************************************

static void TouchPanelPowerOn( void )
{
    DWORD tmp = 0;

    RETAILMSG(1,(TEXT("TouchPanelPowerOn entry.\r\n")));

    //
    // Setup GPIOs for touch
    // 
    
    // Clear GPG15, 14, 13, 12
    v_pIOPregs->rGPGCON &= ~((0x03 << 30)|(0x03 << 28)|(0x03 << 26)|(0x03 << 24));
    // Set GPG15 to use as nYPON, GPG14 to use as YMON, GPG13 to use as nXPON, GPG12 to use as XMON
    v_pIOPregs->rGPGCON |= ((0x03 << 30)|(0x03 << 28)|(0x03 << 26)|(0x03 << 24));
    // Disable full up function
    v_pIOPregs->rGPGUP |= ((0x01 << 15)|(0x01 << 14)|(0x01 << 13)|(0x01 << 12));
    
    //
    // Setup ADC register
    //

    // Down Int, YMON:0, nYPON:1, XMON:0;nXPON:1, Pullup:1, Auto Conv.,Waiting.
//    v_pADCregs->rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
//    v_pADCregs->rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(3);    
//	v_pADCregs->rADCDLY = ADC_DELAY_TIME;//default value for delay.    
//    v_pADCregs->rADCCON	= (1<<14)|(ADCPRS<<6)|(7<<3);	//setup channel, ADCPRS, Touch Input
//    v_pADCregs->rADCCON	= (1<<14)|(ADCPRS<<6)|(2<<3);	//setup channel, ADCPRS, Touch Input	
//    v_pADCregs->rADCCON	= (1<<14)|(ADCPRS<<6)|(0<<3);	//setup channel, ADCPRS, Touch Input		
    
	return ;
}

// **************************************************
// 申明：void TouchPanelPowerOff( void )
// 参数：
//	无
// 返回值：
//	TRUE/FALSE (成功/失败)
// 功能描述：
//	关电操作/使Touch无法工作
// 引用: 
// ************************************************

static void TouchPanelPowerOff( void )
{
}


// **************************************************
// 申明：BOOL Touch_Timer0_Setup( void )
// 参数：
//	无
// 返回值：
//	TRUE/FALSE (成功/失败)
// 功能描述：
//	建立timer0 去继续得到后续的sample
// 引用: 
// ************************************************

static BOOL Touch_Timer0_Setup(void) {
	unsigned int TmpTCON;

    //
    // We use Timer1 of PWM as OS Clock.

    // Disable Timer1 Init..
    v_pINTregs->rINTMSK |= BIT_TIMER1;     // Mask timer1 interrupt.
    v_pINTregs->rSRCPND = BIT_TIMER1;     // Clear pending bit
    v_pINTregs->rINTPND = BIT_TIMER1;

    //we operate our board with PCLK=203M/4 = 50750000Hz (50.75 Mhz)    
    v_pPWMregs->rTCFG0 |= (TOUCH_TIMER_PRESCALER); // prescaler value = 24 + 1
  	v_pPWMregs->rTCFG1 &= ~(0xf0);
#if( TOUCH_TIMER_DIVIDER == 2 )
  	v_pPWMregs->rTCFG1  |=  (0   << 4);		/* 1/2							*/
#elif ( TOUCH_TIMER_DIVIDER == 4 )
  	v_pPWMregs->rTCFG1  |=  (1   << 4);		/* 1/4							*/
#elif ( TOUCH_TIMER_DIVIDER == 8 )
  	v_pPWMregs->rTCFG1  |=  (2   << 4);		/* 1/8							*/
#elif ( TOUCH_TIMER_DIVIDER == 16 )
  	v_pPWMregs->rTCFG1  |=  (3   << 4);		/* 1/16							*/
#endif
//    v_pPWMregs->rTCNTB1 = 1000; // about 10 ms(203M/4/2/(255+1))=10ms  	
    v_pPWMregs->rTCNTB1 = (10 * (S2410PCLK / (TOUCH_TIMER_PRESCALER+1) / TOUCH_TIMER_DIVIDER)) / 1000;		// 10msec, Charlie
							
    v_pPWMregs->rTCMPB1 = 0;   	

	TmpTCON = v_pPWMregs->rTCON;	// get TCON value to temp TCON register
	TmpTCON &= ~0xf00;     			// clear fields of Timer 1 
	TmpTCON |= 0x200;     			// interval mode(auto reload), update TCVNTB4, stop 
	v_pPWMregs->rTCON = TmpTCON;	// put the value to TCON register

	TmpTCON = v_pPWMregs->rTCON;	// get TCON value to temp TCON register
	TmpTCON &= ~0xf00;     			// clear fields of Timer 1 
	TmpTCON |= 0x100;     			// interval mode, no operation, start for Timer 4 
	v_pPWMregs->rTCON = TmpTCON;	// put the value to TCON register

    v_pINTregs->rINTMSK &= ~BIT_TIMER1;    
 
 	return TRUE;   
}    


//
// @doc EX_TOUCH_DDSI EXTERNAL DRIVERS DDSI TOUCH_PANEL
//
// @func void | DdsiTouchPanelGetPoint |
// Returns the most recently acquired point and its associated tip state
// information.
//
// @parm PDDSI_TOUCHPANEL_TIPSTATE | pTipState |
// Pointer to where the tip state information will be returned.
// @parm PLONG | pUnCalX |
// Pointer to where the x coordinate will be returned.
// @parm PLONG | pUnCalY |
// Pointer to where the y coordinate will be returned.
//
// @comm
// Implmented in the PDD.
//
#if ( LCD_TYPE == TFT640_480 )
	#define TOUCH_MAX_X 1000
	#define TOUCH_MIN_X 30
	#define TOUCH_MAX_Y 980
	#define TOUCH_MIN_Y 30

	#define TOUCH_X	640
	#define TOUCH_Y	480
#else 

	#define TOUCH_MAX_X 955 //950
	#define TOUCH_MIN_X 100 //90
	#define TOUCH_MAX_Y 925 //960
	#define TOUCH_MIN_Y 70 //50
/*
	#define TOUCH_MAX_X 970 // 950
	#define TOUCH_MIN_X 100 // 90
	#define TOUCH_MAX_Y 920 // 960 // 910
	#define TOUCH_MIN_Y 55 // 70 //50
*/
	#define TOUCH_X	240
	#define TOUCH_Y	320
#endif
#define TOUCH_ERR	15
static int second = 0;

static VOID Touch_CoordinateConversion(INT *px, INT *py)
{
	INT TmpX, TmpY;
	INT TmpX0, TmpY0;

	TmpX0 = *px; TmpY0 = *py;

	TmpX = (*px >= TOUCH_MAX_X) ? (TOUCH_MAX_X) : *px;
	TmpY = (*py >= TOUCH_MAX_Y) ? (TOUCH_MAX_Y) : *py;
	
	TmpX -= TOUCH_MIN_X;
    TmpY -= TOUCH_MIN_Y;
    
    TmpX = (TmpX) ? TmpX : 0;
    TmpY = (TmpY) ? TmpY : 0;
    
	*px = ((TmpX * TOUCH_X) / (TOUCH_MAX_X-TOUCH_MIN_X))*4;
	*py = (TOUCH_Y - (TmpY * TOUCH_Y) / (TOUCH_MAX_Y-TOUCH_MIN_Y))*4;
	
	*px = (*px >= TOUCH_X*4)? (TOUCH_X*4 - 1) : *px;
	*py = (*py >= TOUCH_Y*4)? (TOUCH_Y*4 - 1) : *py;

#if 0	
	RETAILMSG(1, (TEXT("first *px,y = (%d, %d)\r\n"), TmpX0, TmpY0));
	RETAILMSG(1, (TEXT("TOUCH_MAX_X : %d\r\n"), TOUCH_MAX_X));
	RETAILMSG(1, (TEXT("TOUCH_MAX_Y : %d\r\n"), TOUCH_MAX_Y));
	RETAILMSG(1, (TEXT("TOUCH_MIN_X : %d\r\n"), TOUCH_MIN_X));
	RETAILMSG(1, (TEXT("TOUCH_MIN_Y : %d\r\n"), TOUCH_MIN_Y));
	RETAILMSG(1, (TEXT("TOUCH_X     : %d\r\n"), TOUCH_X));
	RETAILMSG(1, (TEXT("TOUCH_Y     : %d\r\n"), TOUCH_Y));	
	RETAILMSG(1, (TEXT("last  *px,y = (%d, %d)\r\n"), *px, *py));	
#endif
	
	return;
}

/**************************************************
申明：BOOL TouchPddCreate( void )
参数：无

返回值：TRUE/FALSE 创建(成功/失败)
功能描述：初始化触摸屏设备层需要的资源
引用: 
************************************************/
BOOL TouchPddCreate( void )
{
	return TRUE;
}

/**************************************************
申明：void TouchPddDestroy( void )
参数：无

返回值：无
功能描述：释放触摸屏设备层分配的资源
引用: 
************************************************/
void TouchPddDestroy( void )
{
	INTR_Disable(SYSINTR_TOUCH);
}

/**************************************************
申明：BOOL TouchPddEnable( void )
参数：无

返回值：TRUE/FALSE (成功/失败)
功能描述：允许中断
引用: 
************************************************/
#define DEBUG_TouchPddEnable 1
BOOL TouchPddEnable( void )
{
    RETAILMSG(DEBUG_TouchPddEnable, (TEXT("TouchPddEnable entry.\r\n") ));
    // Power on touch panel
    TouchPanelPowerOn();
    
    // Setup pen down interrupts, but leave ints disabled until InterruptEnable().
    PddpSetupPenDownIntr(TRUE);

    // int enable
    INTR_Done( idTouchChangedIrq );
//	s2410INT->rINTMSK &= ~BIT_ADC;
//	s2410INT->rINTSUBMSK &= ~INTSUB_TC;
//    // 
    RETAILMSG(1,(TEXT("TouchPanelPowerOn leave.\r\n")));
        
    RETAILMSG(DEBUG_TouchPddEnable, (TEXT("TouchPddEnable leave.\r\n") ));    

    return( TRUE );     // we always succeed!!!!!!
}

/**************************************************
申明：void TouchPddDisable( void )
参数：无

返回值：无
功能描述：禁止中断
引用: 
************************************************/
void TouchPddDisable( void )
{
	TouchPanelPowerOff();  // Power down the device
}

/**************************************************
申明：void TouchPddClose( void )
参数：无

返回值：无
功能描述：
引用: 
************************************************/
void TouchPddClose( void )
{
	TouchPddDisable();
}


/**************************************************
申明：void TouchPddPowerHandle( BOOL bPowerOff )
参数：bPowerOff--(TRUE/FALSE)/(开/关)机

返回值：TRUE/FALSE 创建(成功/失败)
功能描述：处理触摸屏(开/关)机
引用: 
************************************************/
void TouchPddPowerHandle( BOOL bPowerOff )
{
    // Set flag so we know to avoid system calls
    bInPowerHandler = TRUE;

    if (bPowerOff) {
        TouchPanelPowerOff();
    }
    else {
        TouchPanelPowerOn();
        PddpSetupPenDownIntr(TRUE);
    }

    bInPowerHandler = FALSE;
}


/**************************************************
申明：BOOL TouchPddGetPoint(int *pUnCalx, int *pUnCaly, DWORD *pFlag)
参数：*pUnCalx--返回未刻度的x坐标
	  *pUnCaly--返回未刻度的y坐标
	  *pFlag--当前笔的状态
	
返回值：TRUE/FALSE (成功/失败)
功能描述：获得未刻度的坐标和笔的状态
引用: 
************************************************/
VOID TouchPddGetPoint(int *pUncalX, int *pUncalY, DWORD *pTipStateFlags)
{
	/*
	ULONG status;
    static int SampleCount = 0;
    static DWORD PrevStateFlags = SAMPLE_IGNORE;//TouchSampleIgnore;
    static INT PrevX = 0;
    static INT PrevY = 0;
    DWORD TmpStateFlags;
    INT TmpX = 0;
    INT TmpY = 0;
    int i;

    RETAILMSG( JYLEE_TEST, (TEXT("TouchPddGetPoint entry.\r\n")));        
    // Read the status passed back by the HAL
    status = READ_REGISTER_ULONG( &(v_pDriverGlobals->tch.status) );

    if(status == TOUCH_PEN_UP) {
		v_pADCregs->rADCTSC = 0xD3;	// Set stylus down interrupt
		*pTipStateFlags = SAMPLE_VALID;
		*pUncalX = PrevX;
		*pUncalY = PrevY;
	    INTR_Done( idTouchChangedIrq );
    }
    else if(status == TOUCH_PEN_DOWN){
		*pTipStateFlags |= SAMPLE_IGNORE;
		*pUncalX = PrevX;
		*pUncalY = PrevY;
		Touch_Timer0_Setup();
		INTR_Done( idTouchChangedIrq );
	}
	else {		
		if( (v_pADCregs->rADCDAT0 & 0x8000) || (v_pADCregs->rADCDAT1 & 0x8000) ){
			v_pADCregs->rADCTSC = 0xD3;	// Set stylus down interrupt
			*pTipStateFlags = SAMPLE_VALID;
			*pUncalX = PrevX;
			*pUncalY = PrevY;
			INTR_Done( idTouchChangedIrq );
		}
		else{		// charlie
			// <Auto X-Position and Y-Position Read>

			for (i =0; i < 3; i++) {
				v_pADCregs->rADCTSC=(0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);
				// Stylus Down,Don't care,Don't care,Don't care,Don't care,XP pullup Dis,Auto,No operation
				v_pADCregs->rADCCON|=0x1;	// Start Auto conversion

				while(v_pADCregs->rADCCON & 0x1);		//check if Enable_start is low
				while(!(0x8000&v_pADCregs->rADCCON));	// Check ECFLG
		
				ybuf[i] = 0x3ff & v_pADCregs->rADCDAT0;
				xbuf[i] = 0x3ff & v_pADCregs->rADCDAT1;
			}	

	 	   PddpTouchPanelEvaluateSamples( &TmpStateFlags, &TmpX, &TmpY);
	
	
			v_pADCregs->rADCTSC=(1<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
	
			Touch_CoordinateConversion(&TmpX, &TmpY);    
			
			
			if (Touch_Pen_filtering(&TmpX, &TmpY)) // Valid touch pen
		    {
		    	//RETAILMSG(JYLEE_TEST, (TEXT("valid touch pen\r\n")));
				*pTipStateFlags = SAMPLE_VALID | SAMPLE_DOWN;			
				*pTipStateFlags &= ~SAMPLE_IGNORE;
			}
			else	// Invalid touch pen 
			{
		    	//RETAILMSG(JYLEE_TEST, (TEXT("invalid touch pen\r\n")));
				*pTipStateFlags = SAMPLE_VALID;
				*pTipStateFlags |= SAMPLE_IGNORE;				
			}		
			*pUncalX = PrevX = TmpX;
			*pUncalY = PrevY = TmpY;
	
	   	 INTR_Done( idTouchIrq );
	   		
    		RETAILMSG( JYLEE_TEST, (TEXT("0 - (%d, %d) 0x%X\r\n"), *pUncalX, *pUncalY, *pTipStateFlags));
		}
	}

    return;
	*/
	
	{
		int x, y;
		
		RETAILMSG(1, (TEXT("toucher entry0.\r\n")));		

		/*
		v_pADCregs->rADCTSC =(0<<8)|(1<<7)|(0<<6)|(1<<5)|(1<<4)|(1<<3)|(0<<2)|(0);   //power to Y channel
		v_pADCregs->rADCCON &= ~(0x7<<3);
		v_pADCregs->rADCCON |= (2<<3) | (1<<0);
		while(!(0x8000&v_pADCregs->rADCCON));
		if( ( x = (0x3ff & v_pADCregs->rADCDAT0)) >= MAX_ADVAL)
		{
			RETAILMSG(1, (TEXT("x=%d.\r\n"), x ));
			*pTipStateFlags |= SAMPLE_IGNORE;
		}*/
		v_pADCregs->rADCTSC = (0<<8)|(0<<7)|(0<<6)|(0<<5)|(1<<4)|(1<<3)|(0<<2)|(0);   //power to Y channel
		v_pADCregs->rADCCON &= ~(0x7<<3);
		v_pADCregs->rADCCON |= (0<<3) | (1<<0);
		while(!(0x8000&v_pADCregs->rADCCON));

		if( ( x = (0x3ff & v_pADCregs->rADCDAT0)) == MAX_ADVAL)
		{
			RETAILMSG(1, (TEXT("x=%d.\r\n"), x ));
			*pTipStateFlags |= SAMPLE_IGNORE;
		}else{
			*pTipStateFlags |= SAMPLE_IGNORE;
			return;

		}

		{
			v_pADCregs->rADCTSC =(0<<8)|(1<<7)|(0<<6)|(0<<5)|(1<<4)|(1<<3)|(0<<2)|(0);   //power to Y channel
			v_pADCregs->rADCCON &= ~(0x7<<3);	// Start Auto conversion
			v_pADCregs->rADCCON |= (0<<3) | (1<<0);
			while(v_pADCregs->rADCCON & 0x1);		//check if Enable_start is low
			while(!(0x8000&v_pADCregs->rADCCON));	// Check ECFLG
			x = 0x3ff & v_pADCregs->rADCDAT0;
	
			RETAILMSG(1, (TEXT("toucher entry1.\r\n")));		
					
			v_pADCregs->rADCCON &= ~(0x7<<3);	// Start Auto conversion
			v_pADCregs->rADCCON |= (0<<3) | (1<<0);
			while(v_pADCregs->rADCCON & 0x1);		//check if Enable_start is low
			while(!(0x8000&v_pADCregs->rADCCON));	// Check ECFLG
			y = 0x3ff & v_pADCregs->rADCDAT0;
	
			v_pADCregs->rADCTSC =(0<<8)|(0<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(0<<2)|(0);   //power to idle

			*pUncalX = x;
			*pUncalY = y;			

			*pTipStateFlags |= SAMPLE_VALID | SAMPLE_DOWN;
	
			RETAILMSG(1, (TEXT("x=%d, y=%d\r\n"), x, y));			
		}
	}
	
}
