#ifndef __OALINTR_H
#define __OALINTR_H

#ifndef __SYSINTR_H
#include <sysintr.h>
#endif

#define SYSINTR_KEYBOARD    	(SYSINTR_FIRMWARE+0)
#define SYSINTR_TOUCH       	(SYSINTR_FIRMWARE+1)
#define SYSINTR_ADC         	(SYSINTR_FIRMWARE+2)
#define SYSINTR_SERIAL      	(SYSINTR_FIRMWARE+3)
#define SYSINTR_AUDIO       	(SYSINTR_FIRMWARE+4)
#define SYSINTR_PCMCIA_STATE    (SYSINTR_FIRMWARE+5)
#define SYSINTR_PCMCIA_EDGE     (SYSINTR_FIRMWARE+6)
#define SYSINTR_PCMCIA_LEVEL    (SYSINTR_FIRMWARE+7)
#define SYSINTR_TOUCH_CHANGED   (SYSINTR_FIRMWARE+8)
#define SYSINTR_IR          	(SYSINTR_FIRMWARE+9)
#define SYSINTR_ETHER       	(SYSINTR_FIRMWARE+10)

// Register USB host interrupt
#define SYSINTR_USB         	(SYSINTR_FIRMWARE+11)
// Register USB device interrupt
#define SYSINTR_USBD        	(SYSINTR_FIRMWARE+12)
#define SYSINTR_POWER        	(SYSINTR_FIRMWARE+13)

/////////////////////////////////////////////////////////////
// charlie, SDIO
#define	SYSINTR_SDMMC				 (SYSINTR_FIRMWARE+14)
#define SYSINTR_SDMMC_CARD_DETECT	 (SYSINTR_FIRMWARE+15)
#define SYSINTR_SDMMC_SDIO_INTERRUPT (SYSINTR_FIRMWARE+16)
/////////////////////////////////////////////////////////////
#define SYSINTR_BUTTON               (SYSINTR_FIRMWARE+17)
#define	SYSINTR_DMA0				 (SYSINTR_FIRMWARE+18)



#endif // __OALINTR_H



