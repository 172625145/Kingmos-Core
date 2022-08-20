/****************************************************************
 NAME: usbmain.c
 DESC: endpoint interrupt handler
       USB init jobs
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <eframe.h>
//#include <string.h>
//#include <stdarg.h>
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "def.h"

#include "2410usb.h"
#include "usbmain.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbout.h"
#include "usbin.h"

/**************************
    Some PrepareEp1Fifo() should be deleted
 **************************/   

void UsbdMain(void)
{
    int i;
    U8 tmp1;
    U8 oldTmp1=0xff;
    
    //ChangeUPllValue(0x48,0x3,0x2);  //UCLK=48Mhz     
    //ChangeUPllValue(40,1,2);  //UCLK=48Mhz  setting in init.s
    InitDescriptorTable();
    //ResetUsbd();
    
    ConfigUsbd();

    //DetectVbus(); //not used in S3C2400X
	PrepareEp1Fifo();
#if 0    
    while(1)
    {
    	if(DbgPrintfLoop())continue;
    	
    	Delay(5000);
    	if((i++%2)==0)Led_Display(0x8);
    	else Led_Display(0x0);
    }
#endif    
}

void __irq IsrUsbd()
{
    unsigned int usbdIntpnd,epIntpnd;
    unsigned int saveIndexReg=(unsigned int)rINDEX_REG;

    usbdIntpnd=(unsigned int)rUSB_INT_REG;
    epIntpnd=(unsigned int)rEP_INT_REG;
    
//    DbgPrintf( "[INT:EP_I=%x,USBI=%x]",epIntpnd,usbdIntpnd );
//	Uart_Printf("U:[INT:EP_I=%x,USBI=%x].\r\n",epIntpnd,usbdIntpnd);

    if(usbdIntpnd&SUSPEND_INT)
    {
    	rUSB_INT_REG=SUSPEND_INT;
    	DbgPrintf( "<SUS]");
		Uart_Printf( "SUSPEND_INT.\r\n" );
    }
    if(usbdIntpnd&RESUME_INT)
    {
    	rUSB_INT_REG=RESUME_INT;
    	DbgPrintf("<RSM]");
//   	 Uart_Printf( "RESUME_INT.\r\n" );
    }
    if(usbdIntpnd&RESET_INT)
    {
    	DbgPrintf( "<RST]");
    	
    	//ResetUsbd();
    	ReconfigUsbd(); // remove by lilin

    	rUSB_INT_REG=RESET_INT;  //RESET_INT should be cleared after ResetUsbd(). 
		PrepareEp1Fifo();
//        PrepareEp1Fifo(); 	// remove by lilin
		Uart_Printf( "RESET_INT.\r\n" );
    }

    if(epIntpnd&EP0_INT)
    {
	    rEP_INT_REG=EP0_INT;
    	Ep0Handler();
//		Uart_Printf( "EP0_INT.\r\n" );
    }
    if(epIntpnd&EP1_INT)
    {
    	rEP_INT_REG=EP1_INT;  
    	Ep1Handler();
		Uart_Printf( "EP1_INT.\r\n" );
    }

    if(epIntpnd&EP2_INT)
    {
    	rEP_INT_REG=EP2_INT;  
    	DbgPrintf("<2:TBD]");   //not implemented yet	
    	//Ep2Handler();
		Uart_Printf( "EP2_INT.\r\n" );
    }

    if(epIntpnd&EP3_INT)
    {    	
		rEP_INT_REG=EP3_INT;
    	Ep3Handler();
//		Uart_Printf( "EP3_INT.\r\n" );    	    	
    }

    if(epIntpnd&EP4_INT)
    {
    	rEP_INT_REG=EP4_INT;
    	DbgPrintf("<4:TBD]");   //not implemented yet	
    	//Ep4Handler();
		Uart_Printf( "EP4_INT.\r\n" );
    }

	ClearPending(BIT_USBD);
		
//    rINDEX_REG = 0;
//    Uart_Printf( "rEP0_CSR=0x%x.\r\n",  rIN_CSR1_REG );   
        
    rINDEX_REG=saveIndexReg;
    
//    Uart_Printf( "rEP_INT_EN_REG=0x%x.\r\n", rEP_INT_EN_REG );
//    Uart_Printf( "rUSB_INT_EN_REG=0x%x.\r\n", rUSB_INT_EN_REG );    
//    Uart_Printf( "rMISCCR=0x%x.\r\n", rMISCCR ); 
//    Uart_Printf( "rINTMSK=0x%x.\r\n", rINTMSK );     
//    Uart_Printf( "rINTMOD=0x%x.\r\n", rINTMOD );         
}


/******************* Consol printf for debug *********************/

#define DBGSTR_LENGTH (0x1000)
U8 dbgStrFifo[DBGSTR_LENGTH];
volatile U32 dbgStrRdPt=0;
volatile U32 dbgStrWrPt=0;



void _WrDbgStrFifo(U8 c)
{
    dbgStrFifo[dbgStrWrPt++]=c;
    if(dbgStrWrPt==DBGSTR_LENGTH)dbgStrWrPt=0;

}


int DbgPrintfLoop(void)
{
    if(dbgStrRdPt==dbgStrWrPt)return 0;
    Uart_SendByte(dbgStrFifo[dbgStrRdPt++]);

    if(dbgStrRdPt==DBGSTR_LENGTH)dbgStrRdPt=0;
    return 1;
}


#if 0
void DbgPrintf(char *fmt,...)
{
    int i,slen;
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    
    slen=strlen(string);
    
    for(i=0;i<slen;i++)
    	_WrDbgStrFifo(string[i]);
    
    va_end(ap);
}
#else
void DbgPrintf(char *fmt,...)
{
}
#endif


