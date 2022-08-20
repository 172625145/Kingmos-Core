#ifndef __OPTION_H__
#define __OPTION_H__

//#define FCLK 100000000
#define FCLK 133500000
//#define FCLK 190000000
//#define FCLK 200000000
//#define FCLK 210000000
//#define FCLK 220000000

#define HCLK (FCLK/2)
#define PCLK  (HCLK/2)


//BUSWIDTH; 16,32
#define BUSWIDTH    (32)


#define _RAM_STARTADDRESS 	0xc000000
#define _ISR_STARTADDRESS 	0xdffff00     //GCS6:128M SDRAM x 2
#define _MMUTT_STARTADDRESS	0xdff8000     //KIW ADD
#define HEAPEND		  	0xdff0000

//If you use ADS1.x, please define ADS10
//#define ADS10 TRUE

#define USBDMA	TRUE
#define USBDMA_DEMAND FALSE	//the downloadFileSize should be (64*n)

// note: makefile,option.a should be changed

#endif /*__OPTION_H__*/

