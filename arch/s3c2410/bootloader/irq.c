#include <2410addr.h>
#include <def.h>
#define IRQ_USBD  25
#define IRQ_DMA2  19
#define IRQ_WDT   9
typedef struct _reg{int reg;}*PREG;


void HandleIRQ( void )
{
	int reg;
	((PREG)(&reg))->reg = rINTOFFSET;
 //   Uart_Printf( "irq_entry++.(int = %d)\r\n", reg);
    switch(  reg )
    {
    	case IRQ_USBD:
//		    Uart_Printf( "usbd int.\r\n" );
    		IsrUsbd();
    		break;
    	case IRQ_DMA2:
//		    Uart_Printf( "dma2 int.\r\n" );
    		IsrDma2();
    		break;
    	case IRQ_WDT:
//		    Uart_Printf( "wdt int.\r\n" );
    		IsrWatchdog();
    		break;
    	default:
//        	IsrUsbd();
						
			rSRCPND = rSRCPND;
			rINTPND = rINTPND;
    		Uart_Printf( "!Error pending int,reg=(%d).\r\n", reg );
    }
    //Uart_Printf( "irq_entry--. (rSRCPND:%d, rINTPND:%d)\r\n", rSRCPND, rINTPND);
}

void INTR_HandleErrorTrap( int int_no )
{
    Uart_Printf( "abort int_no(%d),cpsr(0x%x).\r\n", int_no, GetCPSR() );	
    while(1);
}
