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
#include <s2410.h>
//#define xyg_ser_sub_mask

void EdbgOutputDebugString(const char *sz, ...);

//

static volatile INTreg * const s2410INT = (INTreg *)INT_BASE;
static volatile IOPreg * const s2410IOP = (IOPreg *)IOP_BASE;
static volatile MMCreg * const s2410SDIO = (MMCreg *)MMC_BACE;
static volatile PWMreg * const s2410PWM = (PWMreg *)PWM_BASE;
static volatile ADCreg * const s2410ADC = (ADCreg *)ADC_BASE;
//static volatile struct udcreg * const s2410USBD = (volatile struct udcreg *)(0xB1200140); // 0xB1200140
static volatile struct udcreg * const s2410USBD = (volatile struct udcreg *)(0xB1200140);
static volatile USBD_GLOBALS * const usbdShMem=&((DRIVER_GLOBALS *)DRIVER_GLOBALS_PHYSICAL_MEMORY_START)->usbd;
static volatile TOUCH_GLOBALS * const tch =&((DRIVER_GLOBALS *)DRIVER_GLOBALS_PHYSICAL_MEMORY_START)->tch;


static UINT ulTickCount=0;
static BOOL bTouchSample = FALSE;
// ********************************************************************
//声明：BOOL OEM_InterruptEnable( DWORD idInt, LPVOID pvData, DWORD cbData )
//参数：
//	IN idInt-逻辑中断号
//	IN pvData-通过 INTR_Init 传递的参数
//  IN cbData-pvData指向数据的大小
//返回值：
//	成功，返回TRUE;失败，返回FALSE 
//功能描述：开中断
//引用: 当一个设备驱动程序调用INTR_Init时，由 INTR_Init 调用该函数
// ********************************************************************

#define DEBUG_OEM_INTERRUPT_ENABLE 1
BOOL OEM_InterruptEnable( DWORD idInt, LPVOID pvData, DWORD cbData )
{
	BOOL bRet = TRUE;

	INTR_OFF();
	
	switch (idInt) 
	{
//	case SYSINTR_VMINI:		// Vmini.
//		break;
	case SYSINTR_BREAK:		// There is no halt button on P2.
        break;
    case SYSINTR_DMA0:
        s2410INT->rINTMSK &= ~BIT_DMA0; // SDIO DMA interrupt
		RETAILMSG(DEBUG_OEM_INTERRUPT_ENABLE,(TEXT("::: SYSINTR_DMA0    OEMInterruptDisable\r\n")));
    	break;
	case SYSINTR_SDMMC:
		s2410INT->rINTMSK &= ~BIT_MMC;
		RETAILMSG(DEBUG_OEM_INTERRUPT_ENABLE,(TEXT("::: SYSINTR_SDMMC    OEMInterruptDisable\r\n")));
		break;
	case SYSINTR_SDMMC_SDIO_INTERRUPT:
		s2410INT->rINTMSK &= ~BIT_MMC;
		RETAILMSG(DEBUG_OEM_INTERRUPT_ENABLE,(TEXT("::: SYSINTR_SDMMC_SDIO_INTERRUPT    OEMInterruptEnable\r\n")));		
		break;
	case SYSINTR_SDMMC_CARD_DETECT:
		s2410IOP->rEINTPEND  = (1 << 18);
		s2410IOP->rEINTMASK &= ~(1 << 18);
		s2410INT->rSRCPND	= BIT_EINT8_23;
		if (s2410INT->rINTPND & BIT_EINT8_23) 
			s2410INT->rINTPND = BIT_EINT8_23; 
		s2410INT->rINTMSK  &= ~BIT_EINT8_23;
		RETAILMSG(DEBUG_OEM_INTERRUPT_ENABLE,(TEXT("::: SYSINTR_SDMMC_CARD_DETECT    OEMInterruptEnable\r\n")));
		break;     

    case SYSINTR_TOUCH:
		//RETAILMSG(0,(TEXT("OEMInterruptEnable:TOUCH\n\r\n")));
        break;	
    case SYSINTR_TOUCH_CHANGED:
		//RETAILMSG(0,(TEXT("OEMInterruptEnable:TOUCH CHANGED\r\n\r\n")));
//		s2410INT->rINTMSK &= ~BIT_ADC;
//		s2410INT->rINTSUBMSK &= ~INTSUB_TC;
//        bTouchSample = TRUE;
        break;
	case SYSINTR_KEYBOARD:	// Keyboard on EINT1.
/*
		s2410INT->rSRCPND  = BIT_EINT1;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_EINT1) s2410INT->rINTPND = BIT_EINT1;
*/
		s2410INT->rINTMSK &= ~BIT_EINT1;
		break;
	case SYSINTR_SERIAL:	// Serial port.
		s2410INT->rSUBSRCPND  = (INTSUB_RXD0 | INTSUB_TXD0 | INTSUB_ERR0);
#ifdef xyg_ser_sub_mask
		s2410INT->rINTSUBMSK &= ~(INTSUB_RXD0 | INTSUB_TXD0 | INTSUB_ERR0);
#endif
		s2410INT->rSRCPND     = BIT_UART0;
		if (s2410INT->rINTPND & BIT_UART0){ 
			s2410INT->rINTPND = BIT_UART0;
		}
		s2410INT->rINTMSK    &= ~BIT_UART0;
		break;
	case SYSINTR_IR:		// IrDA.
		s2410INT->rSUBSRCPND  = (INTSUB_RXD1 | INTSUB_TXD1 | INTSUB_ERR1);
#ifdef xyg_ser_sub_mask
		s2410INT->rINTSUBMSK &= ~(INTSUB_RXD1 | INTSUB_TXD1 | INTSUB_ERR1);
#endif
		s2410INT->rSRCPND     = BIT_UART1;
		if (s2410INT->rINTPND & BIT_UART1){ 
			s2410INT->rINTPND = BIT_UART1;
		}
		s2410INT->rINTMSK    &= ~BIT_UART1;
		break;


/*
	case SYSINTR_SERIAL:	// Serial port.
		s2410INT->rSUBSRCPND  = (INTSUB_RXD0 | INTSUB_TXD0 | INTSUB_ERR0);
		s2410INT->rINTSUBMSK &= ~INTSUB_RXD0;
		s2410INT->rINTSUBMSK &= ~INTSUB_TXD0;
		s2410INT->rINTSUBMSK &= ~INTSUB_ERR0;
		s2410INT->rSRCPND     = BIT_UART0;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_UART0) 
			s2410INT->rINTPND = BIT_UART0;
		s2410INT->rINTMSK    &= ~BIT_UART0;
		break;

	case SYSINTR_IR:		// IrDA.
		s2410INT->rSUBSRCPND  = (INTSUB_RXD2 | INTSUB_TXD2 | INTSUB_ERR2);
		s2410INT->rINTSUBMSK &= ~INTSUB_RXD2;
		s2410INT->rINTSUBMSK &= ~INTSUB_TXD2;
		s2410INT->rINTSUBMSK &= ~INTSUB_ERR2;
		s2410INT->rSRCPND     = BIT_UART2;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_UART2) 
			s2410INT->rINTPND = BIT_UART2;
		s2410INT->rINTMSK    &= ~BIT_UART2;
		break;
*/
/*
	case SYSINTR_AUDIO:		// Audio controller (the controller uses both DMA1 and DMA2 interrupts).
		// DMA1 (input).
		//
		s2410INT->rSRCPND  = BIT_DMA1;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_DMA1) s2410INT->rINTPND = BIT_DMA1;
		s2410INT->rINTMSK &= ~BIT_DMA1;
		// DMA2 (output).
		//
		s2410INT->rSRCPND  = BIT_DMA2;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_DMA2) s2410INT->rINTPND = BIT_DMA2;
		s2410INT->rINTMSK &= ~BIT_DMA2;
		break;
*/

	case SYSINTR_AUDIO:		// Audio controller (the controller uses both DMA1 and DMA2 interrupts).
		// DMA1 (input).
		//
		//s2410INT->rSRCPND  = BIT_DMA1;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		//if (s2410INT->rINTPND & BIT_DMA1) s2410INT->rINTPND = BIT_DMA1;
		s2410INT->rINTMSK &= ~BIT_DMA1;
		// DMA2 (output).
		//
		//s2410INT->rSRCPND  = BIT_DMA2;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		//if (s2410INT->rINTPND & BIT_DMA2) s2410INT->rINTPND = BIT_DMA2;
		s2410INT->rINTMSK &= ~BIT_DMA2;
		break;
	case SYSINTR_ADC:
		//return(FALSE);
		break;
    case SYSINTR_PCMCIA_LEVEL:	// PCMCIA data on EINT8.
		s2410INT->rINTMSK  &= ~BIT_EINT8_23;
		//s2410INT->rSRCPND  = BIT_EINT8_23;
		//s2410INT->rINTPND  = BIT_EINT8_23;
		s2410IOP->rEINTMASK &= ~0x100;
		//s2410IOP->rEINTPEND  = 0x100;
		RETAILMSG(1,(TEXT("::: SYSINTR_PCMCIA_LEVEL    OEMInterruptEnable\r\n")));
		break;
	case SYSINTR_PCMCIA_EDGE:
		//return(FALSE);
		break;
	case SYSINTR_PCMCIA_STATE:	// PCMCIA insertion interrupt.
		s2410INT->rSRCPND  = BIT_EINT3;  // to clear the previous pending states
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_EINT3) s2410INT->rINTPND = BIT_EINT3;
		s2410INT->rINTMSK &= ~BIT_EINT3;
		RETAILMSG(1,(TEXT("::: SYSINTR_PCMCIA_STATE    OEMInterruptEnable\r\n")));
		break;
	case SYSINTR_TIMING:
		//return(FALSE);
		break;
    case SYSINTR_ETHER:			// Ethernet on EINT9.
		s2410IOP->rEINTPEND   = 0x200;
		s2410IOP->rEINTMASK  &= ~0x200;
		s2410INT->rSRCPND     = BIT_EINT8_23;	// by shim
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_EINT8_23) s2410INT->rINTPND = BIT_EINT8_23;
		s2410INT->rINTMSK    &= ~BIT_EINT8_23;
        break;
#if 0
	case SYSINTR_USB:
   		// USB host interrupt enable bit. by hjcho
   		s2410INT->rINTMSK &= ~BIT_USBH;
   		break;

	case SYSINTR_USBD:
   		s2410INT->rINTMSK &= ~BIT_USBD;
		//RETAILMSG(1,(TEXT("::: SYSINTR_USBD     OEMInterruptEnable\r\n")));
   		break;
#else
	case SYSINTR_USB:			// USB host.
		s2410INT->rSRCPND  = BIT_USBH;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_USBH) s2410INT->rINTPND = BIT_USBH;
		s2410INT->rINTMSK &= ~BIT_USBH;
		break;
	case SYSINTR_USBD:
		s2410INT->rSRCPND  = BIT_USBD;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_USBD) s2410INT->rINTPND = BIT_USBD;
		s2410INT->rINTMSK &= ~BIT_USBD;
		break;
#endif
	case SYSINTR_POWER:
		s2410INT->rSRCPND  = BIT_EINT0;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_EINT0) s2410INT->rINTPND = BIT_EINT0;
		s2410INT->rINTMSK &= ~BIT_EINT0;
		s2410INT->rSRCPND  = BIT_EINT2;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_EINT2) s2410INT->rINTPND = BIT_EINT2;
		s2410INT->rINTMSK &= ~BIT_EINT2;
		break;     

	default:
		bRet = FALSE;	/* unsupported interrupt value */
		//return(FALSE);
        break;
	}
    
	INTR_ON();
    	
    return bRet;    
}

// ********************************************************************
//声明：void OEM_InterruptDisable( DWORD idInt )
//参数：
//	IN idInt-逻辑中断号
//返回值：无
//功能描述：关中断
//引用: 当一个设备驱动程序调用INTR_Disable时，由 INTR_Disable 调用该函数
// ********************************************************************

void OEM_InterruptDisable( DWORD idInt )
{
    INTR_OFF();
	
	switch (idInt) 
	{
	case SYSINTR_BREAK:		// There is no halt button on P2.
		break;

    case SYSINTR_DMA0:
        s2410INT->rINTMSK |= BIT_DMA0; // SDIO DMA interrupt
		//RETAILMSG(1,(TEXT("::: SYSINTR_DMA0    OEMInterruptDisable\r\n")));
		break;

	case SYSINTR_SDMMC:
		s2410INT->rINTMSK |= BIT_MMC;
		//RETAILMSG(1,(TEXT("::: SYSINTR_SDMMC    OEMInterruptDisable\r\n")));
		break;

	case SYSINTR_SDMMC_SDIO_INTERRUPT:
		s2410INT->rINTMSK |= BIT_MMC;
		s2410SDIO->rSDIINTMSK &= ~(0x1<<12);		// interrupt from SDIO card
		//RETAILMSG(1,(TEXT("::: SYSINTR_SDMMC_SDIO_INTERRUPT    OEMInterruptDisable\r\n")));
		break;

	case SYSINTR_SDMMC_CARD_DETECT:
		s2410IOP->rEINTMASK |= (1 << 18);
		s2410INT->rINTMSK    |= BIT_EINT8_23;
		//RETAILMSG(1,(TEXT("::: SYSINTR_SDMMC_CARD_DETECT    OEMInterruptDisable\r\n")));
		break;        


	case SYSINTR_TOUCH:
	    break;
	
	case SYSINTR_TOUCH_CHANGED:
//		s2410INT->rINTMSK |= BIT_ADC;
//		s2410INT->rINTSUBMSK |= INTSUB_TC;
//        bTouchSample = FALSE;		
	    break;

	case SYSINTR_KEYBOARD:
		s2410INT->rINTMSK |= BIT_EINT1;
		break;
/*
	case SYSINTR_SERIAL:
		s2410INT->rINTMSK    |= BIT_UART0;
		s2410INT->rINTSUBMSK |= INTSUB_RXD0;
		s2410INT->rINTSUBMSK |= INTSUB_TXD0;
		s2410INT->rINTSUBMSK |= INTSUB_ERR0;
		break;

	case SYSINTR_IR:
		s2410INT->rINTMSK    |= BIT_UART2;
		s2410INT->rINTSUBMSK |= INTSUB_RXD2;
		s2410INT->rINTSUBMSK |= INTSUB_TXD2;
		s2410INT->rINTSUBMSK |= INTSUB_ERR2;
		break;
*/
	case SYSINTR_SERIAL:
		s2410INT->rINTMSK    |= BIT_UART0;
#ifdef xyg_ser_sub_mask
		s2410INT->rINTSUBMSK |= INTSUB_RXD0;
		s2410INT->rINTSUBMSK |= INTSUB_TXD0;
		s2410INT->rINTSUBMSK |= INTSUB_ERR0;
#endif
		//RETAILMSG(1,(TEXT("::: SYSINTR_SERIAL    OEMInterruptDisable\r\n")));
		break;
	case SYSINTR_IR:
		s2410INT->rINTMSK    |= BIT_UART1;
#ifdef xyg_ser_sub_mask
		s2410INT->rINTSUBMSK |= INTSUB_RXD1;
		s2410INT->rINTSUBMSK |= INTSUB_TXD1;
		s2410INT->rINTSUBMSK |= INTSUB_ERR1;
#endif
		//RETAILMSG(1,(TEXT("::: SYSINTR_IR    OEMInterruptDisable\r\n")));		
		break;

	case SYSINTR_AUDIO:
        s2410INT->rINTMSK |= BIT_DMA1;	// Audio input DMA.
		s2410INT->rINTMSK |= BIT_DMA2;	// Audio output DMA.
		break;

	case SYSINTR_ADC:
		break;

    case SYSINTR_PCMCIA_LEVEL:
		s2410IOP->rEINTMASK |= 0x100;
		s2410INT->rINTMSK   |= BIT_EINT8_23;
		//RETAILMSG(1,(TEXT("::: SYSINTR_PCMCIA_LEVEL    OEMInterruptDisable\r\n")));
        break;

	case SYSINTR_PCMCIA_EDGE:
		break;

	case SYSINTR_PCMCIA_STATE:
		s2410INT->rINTMSK |= BIT_EINT3;
		//RETAILMSG(1,(TEXT("::: SYSINTR_PCMCIA_STATE    OEMInterruptDisable\r\n")));
		break;

    case SYSINTR_ETHER:
		s2410INT->rINTMSK   |= BIT_EINT8_23;
		s2410IOP->rEINTMASK |= 0x200;
        break;        

	case SYSINTR_USB:
		s2410INT->rINTMSK |= BIT_USBH;
		break;
        		
	case SYSINTR_USBD:
		s2410INT->rINTMSK |= BIT_USBD;
		//RETAILMSG(1,(TEXT("::: SYSINTR_USBD    OEMInterruptDisable\r\n")));
		break;
        
	case SYSINTR_POWER:
		s2410INT->rINTMSK |= BIT_EINT0;
		s2410INT->rINTMSK |= BIT_EINT2;
		break;        

	default:
		break;
	}
	
    INTR_ON();	
}

// ********************************************************************
//声明：void OEM_InterruptDone( DWORD idInt )
//参数：
//	IN idInt-逻辑中断号
//返回值：无
//功能描述：当设备驱动程序做完中断处理后，用该函数去让设备继续工作
//引用: 当一个设备驱动程序调用INTR_Done时，由 INTR_Done 调用该函数
// ********************************************************************

void OEM_InterruptDone( DWORD idInt )
{
    INTR_OFF();

	switch (idInt) 
	{
    case SYSINTR_DMA0:
        s2410INT->rINTMSK &= ~BIT_DMA0; // SDIO DMA interrupt
		//RETAILMSG(1,(TEXT("::: SYSINTR_DMA0    OEMInterruptDone\r\n")));
		break;

	case SYSINTR_SDMMC:
		s2410INT->rINTMSK &= ~BIT_MMC;
		//RETAILMSG(1,(TEXT("::: SYSINTR_SDMMC    OEMInterruptDone\r\n")));
		break;

	case SYSINTR_SDMMC_SDIO_INTERRUPT:
		s2410INT->rINTMSK &= ~BIT_MMC;
		//RETAILMSG(1,(TEXT("::: SYSINTR_SDMMC_SDIO_INTERRUPT    OEMInterruptDone\r\n")));
		break;

	case SYSINTR_SDMMC_CARD_DETECT:
		s2410IOP->rEINTMASK &= ~(1 << 18);
		s2410INT->rINTMSK   &= ~BIT_EINT8_23;
		//RETAILMSG(1,(TEXT("::: SYSINTR_SDMMC_CARD_DETECT    OEMInterruptDone\r\n")));
		break;        

    case SYSINTR_TOUCH:
        /*
         * Nothing has to be done here as interrupts are masked and unmasked by the touch
         * handler in the HAL.
         */
		s2410INT->rINTMSK &= ~BIT_TIMER1;
        break;

    case SYSINTR_TOUCH_CHANGED:
		s2410INT->rINTMSK &= ~BIT_TIMER1;
		
        /*
         * Nothing has to be done here as interrupts are masked and unmasked by the touch
         * handler in the HAL.
         */
//		s2410INT->rINTMSK &= ~BIT_ADC;
//		s2410INT->rINTSUBMSK &= ~INTSUB_TC;
		//bTouchSample = TRUE;
		RETAILMSG(1,(TEXT("OEMInterruptDone:TOUCH CHANGED\n\r\n")));
        break;
	case SYSINTR_KEYBOARD:
		s2410INT->rINTMSK &= ~BIT_EINT1;
		break;
/*
	case SYSINTR_SERIAL:
		s2410INT->rINTMSK    &= ~BIT_UART0;
		s2410INT->rINTSUBMSK &= ~INTSUB_RXD0;
		break;

	case SYSINTR_IR:
		s2410INT->rINTMSK    &= ~BIT_UART2;
		s2410INT->rINTSUBMSK &= ~INTSUB_RXD2;
		break;
*/
	case SYSINTR_SERIAL:
		s2410INT->rINTMSK    &= ~BIT_UART0;
#ifdef xyg_ser_sub_mask
		s2410INT->rINTSUBMSK &= ~INTSUB_RXD0;
#endif
	    //RETAILMSG(1,(TEXT(":::SYSINTR_SERIAL OEMInterruptDone\n\r\n")));
		break;
	case SYSINTR_IR:
		s2410INT->rINTMSK    &= ~BIT_UART1;
#ifdef xyg_ser_sub_mask
		s2410INT->rINTSUBMSK &= ~INTSUB_RXD1;
#endif
	    //RETAILMSG(1,(TEXT(":::SYSINTR_IR OEMInterruptDone\n\r\n")));
		break;

	case SYSINTR_AUDIO:
		// DMA1 is for audio input.
		// DMA2 is for audio output.
		s2410INT->rSRCPND = (BIT_DMA1 | BIT_DMA2); 
		if (s2410INT->rINTPND & BIT_DMA1) 
			s2410INT->rINTPND = BIT_DMA1;
		if (s2410INT->rINTPND & BIT_DMA2) 
			s2410INT->rINTPND = BIT_DMA2;
        s2410INT->rINTMSK &= ~BIT_DMA1;
        s2410INT->rINTMSK &= ~BIT_DMA2;
		break;

	case SYSINTR_ADC:
		break;

	case SYSINTR_PCMCIA_LEVEL:
		s2410INT->rSRCPND	= BIT_EINT8_23;
		if (s2410INT->rINTPND & BIT_EINT8_23) 
			s2410INT->rINTPND = BIT_EINT8_23; 
		s2410INT->rINTMSK   &= ~BIT_EINT8_23;
		s2410IOP->rEINTMASK &= ~(1<<8);
		//RETAILMSG(1,(TEXT("::: SYSINTR_PCMCIA_LEVEL    OEMInterruptDone\r\n")));
		break;

	case SYSINTR_PCMCIA_EDGE:
		//RETAILMSG(1,(TEXT("::: SYSINTR_PCMCIA_EDGE    OEMInterruptDone\r\n")));
		break;

	case SYSINTR_PCMCIA_STATE:
		s2410INT->rINTMSK &= ~BIT_EINT3;
		//RETAILMSG(1,(TEXT("::: SYSINTR_PCMCIA_STATE    OEMInterruptDone\r\n")));
		break;

	case SYSINTR_ETHER:
		s2410INT->rINTMSK   &= ~BIT_EINT8_23;
		s2410IOP->rEINTMASK &= ~0x200;
		break;
			        
	case SYSINTR_USB:
		s2410INT->rINTMSK &= ~BIT_USBH;
		break;

	case SYSINTR_USBD:
		s2410INT->rINTMSK &= ~BIT_USBD;
		//RETAILMSG(1,(TEXT("::: SYSINTR_USBD    OEMInterruptDone\r\n")));
		break;
        
	case SYSINTR_POWER:
		s2410INT->rSRCPND = BIT_EINT0;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_EINT0) 
			s2410INT->rINTPND = BIT_EINT0;
		s2410INT->rINTMSK &= ~BIT_EINT0;
		s2410INT->rSRCPND = BIT_EINT2;
		// S3C2410X Developer Notice (page 4) warns against writing a 1 to a 0 bit in the INTPND register.
		if (s2410INT->rINTPND & BIT_EINT2) 
			s2410INT->rINTPND = BIT_EINT2;
		s2410INT->rINTMSK &= ~BIT_EINT2;
		break;

	}
	INTR_ON();
}

// ********************************************************************
//声明：int OEM_InterruptHandler( unsigned int ra )
//参数：
//	IN ra-保留
//返回值：中断 id
//功能描述：中断处理接口，当系统产生一个中断后，内核会调用该函数
//          去做最重要的处理（如关掉自己）。
//引用: 
// ********************************************************************
static DWORD dwLoadPageEntryCount = 0;
LPDWORD lpdwLoadPageEntryCount = &dwLoadPageEntryCount;
int OEM_InterruptHandler(unsigned int ra)
{
	unsigned int IntPendVal;
	unsigned int SubIntPendVal;	// for serial
	DWORD			submask;
	volatile BYTE usbd_eir = 0, usbd_uir = 0;
	static DWORD tickCount = 0;
	static UINT intSave, nIntrCount=0;
	static BOOL bPrint = FALSE;

//    TOUCH_GLOBALS *odo_tsb;  //Sample buffer stuff
    // for this, You MUST modify bsp/inc/drv_glob.h.. check drv_glob.h_jylee
    //odo_tsb = &((DRIVER_GLOBALS *)DRIVER_GLOBALS_PHYSICAL_MEMORY_START)->tch;

  
	IntPendVal = s2410INT->rINTOFFSET;	// Get pending IRQ mode interrupt in INTPND.
    
	// Fake CPUEnterIdle needs to know about interrupt firing.
//	fInterruptFlag = TRUE;
/*
    if (fIntrTime) {
        //
        // We're doing interrupt timing. Get Time to ISR. We subtract TIMER_COUNTDOWN
        // here because the compare register has not been updated.
        //
        dwIsrTime1 = PerfCountSinceTick() - dwReschedIncrement;
        dwSPC = ra;
        wNumInterrupts++;
    }
*/
	//
	// Check the timer interrupt.
	//
	if (IntPendVal == INTSRC_TIMER4) 
	{
		DWORD ttmp;
		
		ulTickCount++;

		
//        if( tickCount >= 2000 )
  //      {
    //  		RETAILMSG(1, (TEXT("intr timer.\r\n") ));	
      //		tickCount = 0;
        //}

		s2410PWM->rTCNTB4 = RESCHED_INCREMENT;//dwReschedIncrement;
                             
		ttmp = s2410PWM->rTCON & (~(0xf << 20));
    	// ???                         
		s2410PWM->rTCON = ttmp | (2 << 20);		/* update TCVNTB4, stop					*/
		s2410PWM->rTCON = ttmp | (1 << 20);		/* one-shot mode,  start				*/
                             
		//
		// Clear the interrupt
		//
		s2410INT->rSRCPND = BIT_TIMER4;        
		if (s2410INT->rINTPND & BIT_TIMER4) 
			s2410INT->rINTPND = BIT_TIMER4;
		{
			extern DWORD dwIdleTick;
			extern int nCountRunning;
			void WINAPI KL_ProfileKernel( UINT uiOption, VOID * lpv );
			if( ulTickCount - dwIdleTick > 10000 )
			{	//死锁
				dwIdleTick = ulTickCount;
				RETAILMSG(1, (TEXT("_dead,run=%d,ret pc=0x%x,intSave=%d,nIntrCount=%d,SubPend=0x%x,SubMask=0x%x.\r\n"),nCountRunning, ra, intSave, nIntrCount, s2410INT->rSUBSRCPND, s2410INT->rINTSUBMSK ));
				KL_ProfileKernel( 1, NULL );  
				bPrint = TRUE;
			}
		}
		return SYSINTR_RESCHED;
	} 
	intSave = IntPendVal;


//	RETAILMSG(1, (TEXT("intr no(%d).\r\n"),IntPendVal ));	
	if(IntPendVal == INTSRC_MMC)	// SD, MMC
	{
		s2410INT->rINTMSK |= BIT_MMC;
        s2410INT->rSRCPND = BIT_MMC;        
        if (s2410INT->rINTPND & BIT_MMC) 
        	s2410INT->rINTPND = BIT_MMC;
		//RETAILMSG(1, (TEXT("ARMINT.C-INT:INTSRC_MMC INT\r\n")));
		if( s2410SDIO->rSDIDATSTA & (0x1<<9) ){
			//RETAILMSG(1, (TEXT("INT:SYSINTR_SDMMC_SDIO_INTERRUPT INT\r\n")));
			return SYSINTR_SDMMC_SDIO_INTERRUPT;
		}
		else {
			//RETAILMSG(1, (TEXT("INT:SYSINTR_SDMMC INT\r\n")));
			return SYSINTR_SDMMC;
		}
	}	
	else if(IntPendVal == INTSRC_DMA0)	// SD DMA interrupt
	{
		s2410INT->rINTMSK |= BIT_DMA0;
		s2410INT->rSRCPND = BIT_DMA0;
		if (s2410INT->rINTPND & BIT_DMA0) 
			s2410INT->rINTPND = BIT_DMA0;
		return SYSINTR_DMA0;
	}
	
	else if (IntPendVal == INTSRC_EINT1)	// Keyboard interrupt is connected to EINT1.
	{ 
		s2410INT->rINTMSK |= BIT_EINT1;
		s2410INT->rSRCPND  = BIT_EINT1;        
		if (s2410INT->rINTPND & BIT_EINT1) 
			s2410INT->rINTPND  = BIT_EINT1;

		return(SYSINTR_KEYBOARD);

	} 
	
	else if (IntPendVal == INTSRC_EINT2)	// EINT2
	{ 
		s2410INT->rINTMSK |= BIT_EINT2;
		s2410INT->rSRCPND  = BIT_EINT2;	/* Interrupt Clear				*/
		if (s2410INT->rINTPND & BIT_EINT2) 
			s2410INT->rINTPND  = BIT_EINT2;
//		RETAILMSG(1, (TEXT(">>> CPUPowerReset \r\n")));
		RETAILMSG(1,(TEXT(">>> >>> Reset Button Pressed <<< <<< \r\n")));
		// ???? by lilin
		// CPUPowerReset();
		return(SYSINTR_POWER);
	}
	else if (IntPendVal == INTSRC_EINT3)	// PCMCIA interrupt is connected to EINT3. (nINT_P_DEV)
	{ 
   		s2410INT->rINTMSK |= BIT_EINT3;
   		s2410INT->rSRCPND  =  BIT_EINT3;        
   		if (s2410INT->rINTPND & BIT_EINT3) 
   			s2410INT->rINTPND = BIT_EINT3;
		//RETAILMSG(1, (TEXT("ARMINT.C - SYSINTR_PCMCIA_STATE\r\n")));			
                           
   		return(SYSINTR_PCMCIA_STATE);
	}  
	else if (IntPendVal == INTSRC_EINT8_23)		// EINT8 ~ 23
	{ 
		s2410INT->rINTMSK |= BIT_EINT8_23;	
		submask = s2410IOP->rEINTPEND;

		if ( submask & (1 << 18)) // EINT28 : SDMMC_CARD_DETECT
		{
			s2410IOP->rEINTMASK |= (1 << 18);
			s2410IOP->rEINTPEND  = (1 << 18);
			s2410INT->rSRCPND  = BIT_EINT8_23;
			if (s2410INT->rINTPND & BIT_EINT8_23) 
				s2410INT->rINTPND = BIT_EINT8_23;

			//RETAILMSG(1, (TEXT("ARMINT.C - SYSINTR_SDMMC_CARD_DETECT\r\n")));			
			return SYSINTR_SDMMC_CARD_DETECT;
		}
		else if ( submask & (1 << 9))	// 0x200 EINT9 : CS8900
		{
			s2410IOP->rEINTMASK |= 0x200;
			s2410IOP->rEINTPEND = 0x200;

			s2410INT->rSRCPND = BIT_EINT8_23;        
			if (s2410INT->rINTPND & BIT_EINT8_23) 
				s2410INT->rINTPND = BIT_EINT8_23;

			//RETAILMSG(1, (TEXT("INT:SYSINTR_ETHER INT\r\n")));
			return SYSINTR_ETHER;
		}
		else if (submask & (1 << 8))	// 0x100 EINT8 : PCMCIA_LEVEL
		{
			s2410IOP->rEINTMASK |= 0x100;
			s2410IOP->rEINTPEND = 0x100;

			s2410INT->rSRCPND = BIT_EINT8_23;        
			if (s2410INT->rINTPND & BIT_EINT8_23) 
				s2410INT->rINTPND = BIT_EINT8_23;

			//RETAILMSG(1, (TEXT("INT:SYSINTR_PCMCIA_LEVEL INT\r\n")));
			return SYSINTR_PCMCIA_LEVEL;
		}
		else
		{
			s2410INT->rSRCPND = BIT_EINT8_23; 
			if (s2410INT->rINTPND & BIT_EINT8_23) 
				s2410INT->rINTPND = BIT_EINT8_23;

			RETAILMSG(0, (TEXT("INT:???\r\n")));
			return SYSINTR_NOP;
		}					
	}
/*
	else if (IntPendVal == INTSRC_ADC) // INTSRC_ADC
	{
        // Touch Panel Int
    	SubIntPendVal = s2410INT->rSUBSRCPND;
       	
    	if(SubIntPendVal & INTSUB_TC) 
		{
       		s2410INT->rINTSUBMSK |= INTSUB_TC;
       		s2410INT->rSUBSRCPND  = INTSUB_TC;
       	
            s2410INT->rINTMSK |= BIT_ADC;
   	        s2410INT->rSRCPND  = BIT_ADC;        
       	    if (s2410INT->rINTPND & BIT_ADC) 
       	    	s2410INT->rINTPND = BIT_ADC;
           
			if( (s2410ADC->rADCDAT0 & 0x8000) || (s2410ADC->rADCDAT1 & 0x8000) ){
				s2410INT->rINTMSK |= BIT_TIMER1;     // Mask timer1 interrupt.
		   		s2410INT->rSRCPND = BIT_TIMER1;     // Clear pending bit
   				if (s2410INT->rINTPND & BIT_TIMER1) 
   					s2410INT->rINTPND = BIT_TIMER1;
				//RETAILMSG(0,(TEXT("INT Touch pen up \r\n")));

      	        odo_tsb->status = TOUCH_PEN_UP;
      		}
			else {
				//RETAILMSG(0,(TEXT("INT Touch pen down \r\n")));
   	            odo_tsb->status = TOUCH_PEN_DOWN;
			}
       
	        return SYSINTR_TOUCH_CHANGED;
		}
       	else if(SubIntPendVal & INTSUB_ADC)
		{
       		s2410INT->rINTSUBMSK |= INTSUB_ADC;
       		s2410INT->rSUBSRCPND = INTSUB_ADC;

	        s2410INT->rINTMSK |= BIT_ADC;
   		    s2410INT->rSRCPND = BIT_ADC;        
       		if (s2410INT->rINTPND & BIT_ADC) 
       			s2410INT->rINTPND = BIT_ADC;
            	
        	s2410INT->rINTMSK &= ~BIT_ADC;

           	return SYSINTR_NOP;
       	}
       	else
       		return SYSINTR_NOP;
	}
*/
	else if (IntPendVal == INTSRC_TIMER1) // INTSRC_TIMER1
	{ /*
   	 if( bTouchSample )
  	  {
   		tickCount++;
   		if( tch->status = 0 )
   		{   //idle statue
   			#define TOUCH_IDLE_TICK ( 500 / RESCHED_PERIOD )	// 500ms
   			if( tickCount >= TOUCH_IDLE_TICK )
   			{   
 	  	    	tickCount = 0;
		       	return SYSINTR_TOUCH_CHANGED;
   			}
   		}
   		else
   		{	//down pen statue
   			#define TOUCH_DOWN_TICK ( 20 / RESCHED_PERIOD )	// 20ms
   			if( tickCount >= 5 )
   			{   // 50ms
 	  	    	tickCount = 0;
		       	return SYSINTR_TOUCH_CHANGED;
   			}
   		}
    	}		
	*/
   
		s2410INT->rINTMSK |= BIT_TIMER1;
    	s2410INT->rSRCPND = BIT_TIMER1;
	    if (s2410INT->rINTPND & BIT_TIMER1) 
    	   s2410INT->rINTPND = BIT_TIMER1;
		return SYSINTR_TOUCH_CHANGED;
		
/*		
       // Timer 1 interrupt to get touch point
      	s2410INT->rINTMSK |= BIT_TIMER1;
       	s2410INT->rSRCPND = BIT_TIMER1;
  	    if (s2410INT->rINTPND & BIT_TIMER1) 
  	    	s2410INT->rINTPND = BIT_TIMER1;

		// charlie, 020620
		if( (s2410ADC->rADCDAT0 & 0x8000) || (s2410ADC->rADCDAT1 & 0x8000) ){
			//RETAILMSG(0,(TEXT("INT Touch SYSINTR_TOUCH_CHANGED 1\r\n")));
			odo_tsb->status = TOUCH_PEN_UP;
			return SYSINTR_TOUCH_CHANGED;
		}

        if(odo_tsb->status == TOUCH_PEN_UP) 
		{
			//RETAILMSG(0,(TEXT("INT Touch SYSINTR_TOUCH_CHANGED 2 \r\n")));
			odo_tsb->status = TOUCH_PEN_UP;
			return SYSINTR_TOUCH_CHANGED;
		}
        else 
		{
			unsigned int TmpTCON;

        	odo_tsb->status = TOUCH_PEN_SAMPLE;
			TmpTCON = s2410PWM->rTCON;	// get TCON value to temp TCON register
			TmpTCON &= ~0xf00;     		// clear fields of Timer 1 
			TmpTCON |= 0x200;     		// interval mode(auto reload), update TCVNTB4, stop 
			s2410PWM->rTCON = TmpTCON;	// put the value to TCON register

			TmpTCON = s2410PWM->rTCON;	// get TCON value to temp TCON register
			TmpTCON &= ~0xf00;     		// clear fields of Timer 1 
			TmpTCON |= 0x100;     		// interval mode, no operation, start for Timer 4 
			s2410PWM->rTCON = TmpTCON;	// put the value to TCON register

			//RETAILMSG(0,(TEXT("INT Touch SYSINTR_TOUCH\r\n")));
	
        	return SYSINTR_TOUCH;
		}
*/		
	}

	else if (IntPendVal == INTSRC_EINT0)  { // POWER BUTTON
		s2410INT->rINTMSK |= BIT_EINT0;
		s2410INT->rSRCPND  = BIT_EINT0;	/* Interrupt Clear				*/
		if (s2410INT->rINTPND & BIT_EINT0) 
			s2410INT->rINTPND  = BIT_EINT0;

		return(SYSINTR_POWER);	
	}

	else if(IntPendVal == INTSRC_DMA1) // AUDIO DMA input.
	{  
		s2410INT->rINTMSK |= BIT_DMA1;
		s2410INT->rSRCPND  = BIT_DMA1;
		if (s2410INT->rINTPND & BIT_DMA1) 
			s2410INT->rINTPND  = BIT_DMA1;

		return(SYSINTR_AUDIO);
	}
	else if(IntPendVal == INTSRC_DMA2) // AUDIO DMA output.
	{  
		s2410INT->rINTMSK |= BIT_DMA2;
		s2410INT->rSRCPND  = BIT_DMA2;
		if (s2410INT->rINTPND & BIT_DMA2) 
			s2410INT->rINTPND  = BIT_DMA2;

		return(SYSINTR_AUDIO);
	}

	else if(IntPendVal == INTSRC_USBH) 	// USB.
	{
		s2410INT->rINTMSK |= BIT_USBH;
		s2410INT->rSRCPND  = BIT_USBH;        
		if (s2410INT->rINTPND & BIT_USBH) 
			s2410INT->rINTPND  = BIT_USBH;
		//RETAILMSG(1, (TEXT("INT:SYSINTR_USB INT\r\n")));
		return(SYSINTR_USB);     
	} 

	else if (IntPendVal == INTSRC_DMA3)
	{
		s2410INT->rINTMSK |= BIT_USBD; // USBD interrupt should be masked
		//s2410INT->rINTMSK |= BIT_DMA3;
		s2410INT->rSRCPND = BIT_DMA3;
		if (s2410INT->rINTPND & BIT_DMA3) 
			s2410INT->rINTPND = BIT_DMA3;
		usbdShMem->usbdDma3Int=1;
		return SYSINTR_USBD;  
		//Correct. DMA3 interrupt will be connected to USBD interrupt.
		//break;
	}

	else if(IntPendVal == INTSRC_USBD) 
	{
		s2410INT->rINTMSK |= BIT_USBD;
//		UsbdClearEir();	 //:-)
		{
//	static volatile struct udcreg *s2410USBD = (volatile struct udcreg *)(0xB1200140);
	
			usbdShMem->usbdEir|=*(volatile BYTE *)&s2410USBD->EIR;
			usbdShMem->usbdUir|=*(volatile BYTE *)&s2410USBD->UIR;
			*(volatile BYTE *)&s2410USBD->EIR=usbdShMem->usbdEir;
			*(volatile BYTE *)&s2410USBD->UIR=usbdShMem->usbdUir;			
		}
		s2410INT->rSRCPND = BIT_USBD;
		if (s2410INT->rINTPND & BIT_USBD) 
			s2410INT->rINTPND = BIT_USBD;
		//RETAILMSG(1,(TEXT("INT_USBD\r\n")));
		return SYSINTR_USBD;
    }

	else if(IntPendVal == INTSRC_UART0){
#ifdef xyg_ser_sub_mask
		SubIntPendVal = s2410INT->rSUBSRCPND;

		// Note that we only mask the sub source interrupt - the serial driver will clear the
		// sub source pending register.
		//
		if( bPrint )
		{
			RETAILMSG(1, (TEXT("rINTMSK=0x%x, SubIntPendVal=0x%x,rINTSUBMSK=0x%x.\r\n"),s2410INT->rINTMSK ,SubIntPendVal,s2410INT->rINTSUBMSK));
			bPrint = FALSE;
		}
		if(SubIntPendVal & INTSUB_ERR0){
			s2410INT->rINTSUBMSK |= INTSUB_ERR0;
		}
		else if(SubIntPendVal & INTSUB_RXD0){
			s2410INT->rINTSUBMSK |= INTSUB_RXD0;
		}
		else if(SubIntPendVal & INTSUB_TXD0){
			s2410INT->rINTSUBMSK |= INTSUB_TXD0;
		}
		else{
			RETAILMSG(1, (TEXT("1111111.\r\n")));			
			return(SYSINTR_NOP);
		}
#endif
		// NOTE: Don't clear INTSRC:UART0 here - serial driver does that.
		//
//		if (s2410INT->rINTPND & BIT_UART0){ 
		//	s2410INT->rINTPND  = BIT_UART0;
//		}
		//s2410INT->rINTMSK |= BIT_UART0;
		nIntrCount++;
		s2410INT->rINTMSK |= BIT_UART0;
		if( s2410INT->rINTPND & BIT_UART0 )
		    s2410INT->rINTPND = BIT_UART0;

		return(SYSINTR_SERIAL);
	}
	else if(IntPendVal == INTSRC_UART1){
#ifdef xyg_ser_sub_mask
		SubIntPendVal = s2410INT->rSUBSRCPND;

		if(SubIntPendVal & INTSUB_ERR1){
			s2410INT->rINTSUBMSK |= INTSUB_ERR1;
		}       
		else if(SubIntPendVal & INTSUB_RXD1){
			s2410INT->rINTSUBMSK |= INTSUB_RXD1;
		}       
		else if(SubIntPendVal & INTSUB_TXD1){
			s2410INT->rINTSUBMSK |= INTSUB_TXD1;
		}       
		else{
			RETAILMSG(1, (TEXT("222222.\r\n")));						
			return(SYSINTR_NOP);
		}
#endif

		if (s2410INT->rINTPND & BIT_UART1){ 
			s2410INT->rINTPND  = BIT_UART1;
		}
		s2410INT->rINTMSK |= BIT_UART1;
		
		return(SYSINTR_IR);
	}

/*
	else if(IntPendVal == INTSRC_UART0)	// SERIAL (UART0) (physical COM1: P1 connector).
	{  
		SubIntPendVal = s2410INT->rSUBSRCPND;

		// Note that we only mask the sub source interrupt - the serial driver will clear the
		// sub source pending register.
		//
		if(SubIntPendVal & INTSUB_ERR0) 
		{
			s2410INT->rINTSUBMSK |= INTSUB_ERR0;
		}
		else if(SubIntPendVal & INTSUB_RXD0) 
		{
			s2410INT->rINTSUBMSK |= INTSUB_RXD0;
		}
		else if(SubIntPendVal & INTSUB_TXD0) 
		{
			s2410INT->rINTSUBMSK |= INTSUB_TXD0;
		}
		else
		{
			return(SYSINTR_NOP);
		}
	
		// NOTE: Don't clear INTSRC:UART0 here - serial driver does that.
		//
		s2410INT->rINTMSK |= BIT_UART0;
		if (s2410INT->rINTPND & BIT_UART0) 
			s2410INT->rINTPND  = BIT_UART0;

		return(SYSINTR_SERIAL);
	}
	else if(IntPendVal == INTSRC_UART2)	// IrDA (UART2)
	{
		SubIntPendVal = s2410INT->rSUBSRCPND;

		if(SubIntPendVal & INTSUB_ERR2) 
		{
			s2410INT->rINTSUBMSK |= INTSUB_ERR2;
		}       
		else if(SubIntPendVal & INTSUB_RXD2) 
		{
			s2410INT->rINTSUBMSK |= INTSUB_RXD2;
		}       
		else if(SubIntPendVal & INTSUB_TXD2) 
		{
			s2410INT->rINTSUBMSK |= INTSUB_TXD2;
		}       
		else
		{
			return(SYSINTR_NOP);
		}

		// NOTE: Don't clear INTSRC:UART2 here - serial driver does that.
		//
		s2410INT->rINTMSK |= BIT_UART2;
		if (s2410INT->rINTPND & BIT_UART2) 
			s2410INT->rINTPND  = BIT_UART2;
		
		return(SYSINTR_IR);
	}
*/
	else if (IntPendVal == INTSRC_RTC)
	{
		s2410INT->rSRCPND  = BIT_RTC; 	/* Interrupt Clear 				*/
		if (s2410INT->rINTPND & BIT_RTC) 
			s2410INT->rINTPND  = BIT_RTC;
		s2410INT->rINTMSK  |= BIT_RTC;	/* Alarm Interrupt Disable 		*/

		return (SYSINTR_RTC_ALARM);
	}
	RETAILMSG(1, (TEXT("33333.\r\n")));				
	return(SYSINTR_NOP);
}

// ********************************************************************
//声明：DWORD OEM_GetTickCount( void )
//参数：无    
//返回值：
//功能描述：得到系统tick
//引用: 被GetTickCount调用
// ********************************************************************

DWORD OEM_GetTickCount( void )
{
	return (RESCHED_PERIOD * ulTickCount);
}

DWORD OEM_GetSysTickCount( void )
{
	return 0;
}




