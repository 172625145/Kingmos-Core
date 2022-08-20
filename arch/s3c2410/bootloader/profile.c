/**************************************************************
 NAME: profile.c
 DESC: To measuure the USB download speed, the WDT is used.
       To measure up to large time, The WDT interrupt is used.
 HISTORY:
 MAR.25.2002:purnnamu:  
     S3C2400X profile.c is ported for S3C2410X.
 **************************************************************/
#include <eframe.h>
#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h" 
/* ln
#include <stdarg.h> ln
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
*/
#define __irq
/************************* Timer ********************************/
static int intCount;

void __irq IsrWatchdog(void);

void Timer_InitEx(void)
{
	intCount=0;	
	pISR_WDT=(U32)IsrWatchdog;
	ClearPending(BIT_WDT);
	rINTMSK&=~(BIT_WDT);
}


void Timer_StartEx(void)
{
	int divider=0;
	rWTCON=((PCLK/1000000-1)<<8)|(0<<3)|(1<<2);	// 16us
	rWTDAT=0xffff;
	rWTCNT=0xffff;   

	// 1/16/(65+1),interrupt enable,reset disable,watchdog enable
	rWTCON=((PCLK/1000000-1)<<8)|(0<<3)|(1<<2)|(0<<0)|(1<<5);   
}



DWORD Timer_StopEx(void)
{
	int count;
	rWTCON=((PCLK/1000000-1)<<8);
	rINTMSK|=BIT_WDT;
	count=(0xffff-rWTCNT)+(intCount*0xffff);
//	return ((float)count*(16e-6));
	return count/62500;
}


void __irq IsrWatchdog(void)
{
	ClearPending(BIT_WDT);
	intCount++;   	
}



