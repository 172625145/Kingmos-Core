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


#include <edef.h>
#include <ecore.h>
#include <eassert.h>
#include <oemfunc.h>
#include <cpu.h>
#include <s2410.h>

//#define ORIGINYEAR  1980  //valid date: 1980.1---2116.1,RaoDaChun 2000-12-26
//#define JAN1WEEK    2   /* Monday */
#define TO_BCD(n)       ((((DWORD)(n) / 10) << 4) | ((DWORD)(n) % 10))
#define FROM_BCD(n)     ((((n) >> 4) * 10) + ((n) & 0xf))

//static const unsigned int monthtable[] =      {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//static const unsigned int monthtable_leap[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//static SYSTEMTIME defst = {1998,1,0,1,23,59,50,0};


//------------------------------------------------------------------------------
//
// When this is called we will set up GPIO<1> to be a falling edge interrupt  
// InitClock sets up the OS timer to int via match reg 0 on the IRQ level
// an int is requested in 1ms from now.
//
// Interrupts are disable when this is called. check with Thomas.
//
//------------------------------------------------------------------------------
void 
InitClock(void) 
{
    volatile PWMreg *s2410PWM =(PWMreg *)PWM_BASE;
    volatile INTreg *s2410INT = (INTreg *)INT_BASE;  
    DWORD ttmp;  

	// Timer4 as OS tick and disable it first.
    s2410INT->rINTMSK |= BIT_TIMER4;		// Mask timer4 interrupt.
    s2410INT->rSRCPND = BIT_TIMER4;			// Clear pending bit
    s2410INT->rINTPND = BIT_TIMER4;

    // Operating clock : PCLK=101500000 (101.5 Mhz)    
	// IMPORTANT : MUST CHECK S2410.H DEFINITIONS !!!!
    s2410PWM->rTCFG0 &= ~(0xff << 8);		/* Prescaler 1's Value			*/
	s2410PWM->rTCFG0 |= (PRESCALER << 8);	// prescaler value=15
	
	/* Timer4 Divider field clear */
	s2410PWM->rTCFG1 &= ~(0xf << 16);
	
#if( SYS_TIMER_DIVIDER == D2 )
	  	s2410PWM->rTCFG1  |=  (D1_2   << 16);		/* 1/2							*/
#elif ( SYS_TIMER_DIVIDER == D4 )
		s2410PWM->rTCFG1  |=  (D1_4   << 16);		/* 1/4							*/
#elif ( SYS_TIMER_DIVIDER == D8 )
	  	s2410PWM->rTCFG1  |=  (D1_8   << 16);		/* 1/8							*/
#elif ( SYS_TIMER_DIVIDER == D16 )
	  	s2410PWM->rTCFG1  |=  (D1_16   << 16);		/* 1/16							*/
#endif
	s2410PWM->rTCNTB4 = RESCHED_INCREMENT;	//((RESCHED_PERIOD * OEM_CLOCK_FREQ) / 1000)  	
	ttmp = s2410PWM->rTCON & (~(0xf << 20));

	s2410PWM->rTCON = ttmp | (2 << 20);		/* update TCVNTB4, stop					*/
	s2410PWM->rTCON = ttmp | (1 << 20);		/* one-shot mode,  start				*/

    // Number of timer counts for reschedule interval
//    dwReschedIncrement = RESCHED_INCREMENT;

	// Set OEM timer count for 1 ms
//    OEMCount1ms = OEM_COUNT_1MS;
    
    // Set OEM clock frequency
//    OEMClockFreq = OEM_CLOCK_FREQ;

    s2410INT->rINTMSK &= ~BIT_TIMER4;		

    return;
}

static int isleap(int year)
{
    int leap;

    leap = 0;
    if ((year % 4) == 0) {
        leap = 1;
        if ((year % 100) == 0) {
            leap = (year%400) ? 0 : 1;
        }
    }

    return leap;
}

BOOL OEM_GetRealTime(LPSYSTEMTIME lpst)
{
#if 0
	volatile RTCreg *s2410RTC = (RTCreg *)RTC_BASE;

	do
	{
		lpst->wYear			= FROM_BCD(s2410RTC->rBCDYEAR) + 2000 ;
		lpst->wMonth		= FROM_BCD(s2410RTC->rBCDMON   & 0x1f);
		lpst->wDay			= FROM_BCD(s2410RTC->rBCDDAY   & 0x3f);

		lpst->wDayOfWeek	= (s2410RTC->rBCDDATE - 1);
	
		lpst->wHour			= FROM_BCD(s2410RTC->rBCDHOUR  & 0x3f);
		lpst->wMinute		= FROM_BCD(s2410RTC->rBCDMIN   & 0x7f);
		lpst->wSecond		= FROM_BCD(s2410RTC->rBCDSEC   & 0x7f);
		lpst->wMilliseconds	= 0;
	}
	while(!(lpst->wSecond));

	return(TRUE);
#else
	volatile RTCreg *s2410RTC; 

	s2410RTC = (RTCreg *)RTC_BASE;
    
	//RETAILMSG(1, (_T("OEMGetRealTime\r\n")));
    
	// enable RTC control
//	s2410RTC->rRTCCON = 0x1;
	lpst->wMilliseconds = 0;
    
	lpst->wSecond    = FROM_BCD(s2410RTC->rBCDSEC & 0x7f);
	lpst->wMinute	 = FROM_BCD(s2410RTC->rBCDMIN & 0x7f);
	lpst->wHour	 = FROM_BCD(s2410RTC->rBCDHOUR& 0x3f);
	lpst->wDayOfWeek = (s2410RTC->rBCDDATE - 1);
	lpst->wDay	 = FROM_BCD(s2410RTC->rBCDDAY & 0x3f);
	lpst->wMonth	 = FROM_BCD(s2410RTC->rBCDMON & 0x1f);
	// lpst->wYear	 = (2000 + s2410RTC->rBCDYEAR) ; 
	lpst->wYear	 = FROM_BCD(s2410RTC->rBCDYEAR) + 2000 ; 

	if ( lpst->wSecond == 0 )
	{
		lpst->wSecond    = FROM_BCD(s2410RTC->rBCDSEC & 0x7f);
		lpst->wMinute	 = FROM_BCD(s2410RTC->rBCDMIN & 0x7f);
		lpst->wHour		 = FROM_BCD(s2410RTC->rBCDHOUR& 0x3f);
		lpst->wDayOfWeek = (s2410RTC->rBCDDATE - 1);
		lpst->wDay		 = FROM_BCD(s2410RTC->rBCDDAY & 0x3f);
		lpst->wMonth	 = FROM_BCD(s2410RTC->rBCDMON & 0x1f);
		lpst->wYear = (2000 + s2410RTC->rBCDYEAR) ; 
	}
	// disable RTC control
//	s2410RTC->rRTCCON = 0;

	return TRUE;
#endif
}

BOOL OEM_SetRealTime( const SYSTEMTIME FAR * lpst )
{
#if 0
	volatile RTCreg *s2410RTC = (RTCreg *)RTC_BASE;
 
	// Enable RTC control.
	//
	s2410RTC->rRTCCON |= 1;

	s2410RTC->rBCDSEC  = (unsigned char)TO_BCD(lpst->wSecond );
	s2410RTC->rBCDMIN  = (unsigned char)TO_BCD(lpst->wMinute );
	s2410RTC->rBCDHOUR = (unsigned char)TO_BCD(lpst->wHour   );

	s2410RTC->rBCDDATE = (unsigned char)(lpst->wDayOfWeek + 1);

	s2410RTC->rBCDDAY  = (unsigned char)TO_BCD(lpst->wDay    );
	s2410RTC->rBCDMON  = (unsigned char)TO_BCD(lpst->wMonth  );
	s2410RTC->rBCDYEAR = (unsigned char)TO_BCD((lpst->wYear % 100));

	RETAILMSG(1,(TEXT("OEMSetRealTime: Year: %x, Month: %x, Day: %x, Hour: %x, Minute: %x, second: %x rcnr=%Xh\n"), \
   		s2410RTC->rBCDYEAR, s2410RTC->rBCDMON,s2410RTC->rBCDDAY, s2410RTC->rBCDHOUR, s2410RTC->rBCDMIN,s2410RTC->rBCDSEC,s2410RTC->rRTCCON));

	// Disable RTC control.
	//
	s2410RTC->rRTCCON &= ~1;
	
	return(TRUE);
#else
	volatile RTCreg *s2410RTC = (RTCreg *)RTC_BASE;
// 	static int firsttime = 0;	//????? by lilin remove

	// enable RTC control
	s2410RTC->rRTCCON =  0x1;

	s2410RTC->rBCDSEC  = (unsigned char)TO_BCD(lpst->wSecond );
	s2410RTC->rBCDMIN  = (unsigned char)TO_BCD(lpst->wMinute );
	s2410RTC->rBCDHOUR = (unsigned char)TO_BCD(lpst->wHour   );

	s2410RTC->rBCDDATE = (unsigned char)(lpst->wDayOfWeek + 1);
/*	????? by lilin remove
	if ( firsttime == 0 )
	{
		lpst->wYear = 2003;	lpst->wMonth = 9;	lpst->wDay = 1;
		firsttime = 1;
	}
*/
	s2410RTC->rBCDDAY  = (unsigned char)TO_BCD(lpst->wDay    );
	s2410RTC->rBCDMON  = (unsigned char)TO_BCD(lpst->wMonth  );
	s2410RTC->rBCDYEAR = (unsigned char)TO_BCD((lpst->wYear % 100));

	RETAILMSG(1,(TEXT("OEMSetRealTime: Year: %u, Month: %u, Day: %u, Hour: %u, Minute: %u, second: %u rcnr=%Xh\n"), lpst->wYear, lpst->wMonth,lpst->wDay, lpst->wHour, lpst->wMinute,lpst->wSecond,s2410RTC->rRTCCON));
	RETAILMSG(1,(TEXT("OEMSetRealTime(register): Year: %x, Month: %x, Day: %x, Hour: %x, Minute: %x, second: %x rcnr=%Xh\n"), \
   		s2410RTC->rBCDYEAR, s2410RTC->rBCDMON,s2410RTC->rBCDDAY, s2410RTC->rBCDHOUR, s2410RTC->rBCDMIN,s2410RTC->rBCDSEC,s2410RTC->rRTCCON));

	// disable RTC control
	s2410RTC->rRTCCON = 0; //&= ~0x1;
	
	// Just certify heart bit
//	timer_cnt = 0;

	return TRUE;
#endif	
}

BOOL OEM_SetAlarmTime( const SYSTEMTIME FAR * lpst)
{
	volatile INTreg *s2410INT = (INTreg *)INT_BASE;
	volatile RTCreg *s2410RTC = (RTCreg *)RTC_BASE;
	
	RETAILMSG(1,(TEXT("OEMSetAlarmTime: Year: %u, Month: %u, Day: %u, Hour: %u, Minute: %u, second: %u rcnr=%Xh\n"),
		lpst->wYear, lpst->wMonth, lpst->wDay, lpst->wHour, lpst->wMinute, lpst->wSecond, s2410RTC->rRTCCON));
	
	if ( !lpst ||
		(lpst->wSecond > 59)	||	// 0 - 59
		(lpst->wMinute > 59)	||	// 0 - 59
		(lpst->wHour > 23)		||	// 0 - 23
		(lpst->wDayOfWeek > 6)	||	// 0 - 6, Sun:0, Mon:1, ...
		(lpst->wDay > 31)		||	// 0 - 31
		(lpst->wMonth > 12)		||	// 0 - 12, Jan:1, Feb:2, ...
		(lpst->wMonth == 0)		||
		(lpst->wYear < 2000)	||	// We have a 100 year calander (2 BCD digits) with
		(lpst->wSecond > 2099)		// a leap year generator hard-wired to year 2000.
	)
		return FALSE;
	
	s2410RTC->rRTCCON = (1 << 0);		/* RTC Control Enable 	*/
	
	s2410RTC->rALMSEC  = (unsigned char)TO_BCD(lpst->wSecond );
	s2410RTC->rALMMIN  = (unsigned char)TO_BCD(lpst->wMinute );
	s2410RTC->rALMHOUR = (unsigned char)TO_BCD(lpst->wHour   );

	s2410RTC->rALMDAY  = (unsigned char)TO_BCD(lpst->wDay    );
	s2410RTC->rALMMON  = (unsigned char)TO_BCD(lpst->wMonth  );
	s2410RTC->rALMYEAR = (unsigned char)TO_BCD((lpst->wYear % 100));

	s2410RTC->rRTCALM = 0x7f;
	
	s2410RTC->rRTCCON = (0 << 0);		/* RTC Control Disable 	*/

	s2410INT->rSRCPND  = BIT_RTC;		/* RTC Alarm Interrupt Clear */
	if (s2410INT->rINTPND & BIT_RTC) s2410INT->rINTPND = BIT_RTC;
	s2410INT->rINTMSK &= ~BIT_RTC;		/* RTC Alarm Enable 	*/
			
	return TRUE;
}


DWORD OEM_TimeToJiffies( DWORD dwMilliseconds, DWORD dwNanoseconds )
{
    return dwMilliseconds / RESCHED_PERIOD;
}





