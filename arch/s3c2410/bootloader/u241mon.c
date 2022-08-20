/****************************************************************
 NAME: u241mon.c
 DESC: u241mon entry point,menu,download
 HISTORY:
 Mar.25.2002:purnnamu: S3C2400X profile.c is ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 Apr.01.2002:purnnamu: isDownloadReady flag is added.
 Apr.10.2002:purnnamu: - Selecting menu is available in the waiting loop. 
                         So, isDownloadReady flag gets not needed
                       - UART ch.1 can be selected for the console.
 Aug.20.2002:purnnamu: revision number change 0.2 -> R1.1       
 Sep.03.2002:purnnamu: To remove the power noise in the USB signal, the unused CLKOUT0,1 is disabled.
 ****************************************************************/
#include <eframe.h>
//#include <stdlib.h>
//#include <string.h>

#include "def.h"
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "2410slib.h"
#include "mmu.h"
#include "profile.h"
#include "memtest.h"

#include "usbmain.h"
#include "usbout.h"
#include "usblib.h"
#include "2410usb.h"

void Isr_Init(void);
void HaltUndef(void);
void HaltSwi(void);
void HaltPabort(void);
void HaltDabort(void);
void Lcd_Off(void);
void WaitDownload(void);
void Menu(void);

//#define DOWNLOAD_ADDRESS _RAM_STARTADDRESS
volatile U32 downloadAddress;

void (*restart)(void)=(void (*)(void))0x0;
void (*run)(void);


volatile unsigned char *downPt;
volatile U32 downloadFileSize;
volatile U16 checkSum;
volatile unsigned int err=0;
volatile U32 totalDmaCount;

volatile int isUsbdSetConfiguration;

int download_run=0;
U32 tempDownloadAddress;
int menuUsed=0;

//extern char Image$$RW$$Limit[];
char MagicBuff[20];
U32 *pMagicNum=(U32 *)MagicBuff;

int consoleNum;

void Main(void)
{
    char *mode;
    int i;
    U8 key;

//    *pMagicNum = 0x12345678;
    #if ADS10   
    __rt_lib_init(); //for ADS 1.0
    #endif
//	INTR_ON();
     ChangeClockDivider(1,1);    // 1:2:4   
     // rCLKDIVN=0x4;    //  1:4:4
    //ChangeMPllValue(82,2,1);  //FCLK=135.0Mhz     
    ChangeMPllValue(82,1,1);    //FCLK=180.0Mhz     
    //ChangeMPllValue(161,3,1); //FCLK=202.8Mhz 
    //ChangeMPllValue(117,1,1);    //FCLK=250.0Mhz 
    //ChangeMPllValue(122,1,1);    //FCLK=260.0Mhz 
    //ChangeMPllValue(125,1,1);    //FCLK=266.0Mhz 
    //ChangeMPllValue(127,1,1);    //FCLK=270.0Mhz  
    
    Port_Init();
    
    rGPHCON = rGPHCON&~(0xf<<18)|(0x5<<18);   
    //To enhance the USB signal quality.
    //CLKOUT 0,1=OUTPUT to reduce the power consumption.
    
    Isr_Init();
    if(*pMagicNum!=0x12345678)
    	consoleNum=0;
    else
    	consoleNum=1;
    	
    Uart_Init(0,115200);
    Uart_Select(consoleNum);

    rMISCCR=rMISCCR&~(1<<3); // USBD is selected instead of USBH1 
    rMISCCR=rMISCCR&~(1<<13); // USB port 1 is enabled.

//
//  USBD should be initialized first of all.
//
    isUsbdSetConfiguration=0;

#if 0
    UsbdMain(); 
    MMU_Init(); //MMU should be reconfigured or turned off for the debugger, 
    //After downloading, MMU should be turned off for the MMU based program,such as WinCE.	
#else
      MMU_EnableICache();
      UsbdMain();
#endif
    Delay(0);  //calibrate Delay()
    
//    pISR_SWI=(_ISR_STARTADDRESS+0xf0);    //for pSOS

    Led_Display(0x6);

#if USBDMA
    mode="DMA";
#else
    mode="Int";
#endif

/*    DbgPrintf( "Hello, Kingmos.\r\n" );
    Uart_Printf("Hello, Kingmos with Uart.\r\n"); 
	{
		char a[10];
	    Uart_Printf("hello, test a=%x ret:%x .\r\n", a, TestRet(a));
	}*/
    Uart_Printf("\n\n");
    Uart_Printf("+---------------------------------------------+\n");
    Uart_Printf("| S3C2410X USB Downloader ver R1.12 date(%s),time(%s) |\n", __DATE__, __TIME__ );
    Uart_Printf("+---------------------------------------------+\n");
    Uart_Printf("FCLK=%dMHz,%s mode\n",FCLK/1000000,mode); 
    Uart_Printf("USB: IN_ENDPOINT:1 OUT_ENDPOINT:3\n"); 
    Uart_Printf("FORMAT: <ADDR(DATA):4>+<SIZE(n+10):4>+<DATA:n>+<CS:2>\n");
    Uart_Printf("NOTE: 1. Power off/on or press the reset button for 1 sec\n");
    Uart_Printf("         in order to get a valid USB device address.\n");
    Uart_Printf("      2. For additional menu, Press any key. \n");
    Uart_Printf("\n");

    download_run=1; //The default menu is the Download & Run mode.

    Uart_Printf("INTR_ON.\r\n");    
    INTR_ON();

    while(1)
    {
    	if(menuUsed==1)Menu();
    	WaitDownload();
    }

}



void Menu(void)
{
    int i;
    U8 key;
    menuUsed=1;
    while(1)
    {
        Uart_Printf("\n###### Select Menu ######\n");
        Uart_Printf(" [0] Download & Run\n");
        Uart_Printf(" [1] Download Only\n");
        Uart_Printf(" [2] Test SDRAM \n");
        Uart_Printf(" [3] Change The Console UART Ch.\n");
        key=Uart_Getch();
        
        switch(key)
        {
        case '0':
            Uart_Printf("\nDownload&Run is selected.\n\n");
            download_run=1;
            return;
        case '1':
            Uart_Printf("\nDownload Only is selected.\n");
            Uart_Printf("Enter a new temporary download address(0x3...):");
            tempDownloadAddress=Uart_GetIntNum();
            download_run=0;
            Uart_Printf("The temporary download address is 0x%x.\n\n",tempDownloadAddress);
            return;
        case '2':
            Uart_Printf("\nMemory Test is selected.\n");        
			MemoryTest();
			Menu();
			return;
            break;
        case '3':
            Uart_Printf("\nWhich UART channel do you want to use for the console?[0/1]\n");
            if(Uart_Getch()!='1')
            {
	    	*pMagicNum=0x0;
			Uart_Printf("UART ch.0 will be used for console at next boot.\n");        	    	
			}
			else
			{
				*pMagicNum=0x12345678;
				Uart_Printf("UART ch.1 will be used for console at next boot.\n");        
                Uart_Printf("UART ch.0 will be used after long power-off.\n");
			}
            Uart_Printf("System is waiting for a reset. Please, Reboot!!!\n");
            while(1);
            break;

        default:
            break;
	}	
    }	    

}

#define CLR_EP3_OUT_PKT_READY() rOUT_CSR1_REG= ( out_csr3 &(~ EPO_WR_BITS)\
					&(~EPO_OUT_PKT_READY) ) 


void WaitDownload(void)
{
    U32 i;
    U32 j;
    U16 cs;
    U32 temp;
    U16 dnCS;
    int first=1;
    DWORD time;
    U8 tempMem[16];
    U8 key;
	int idle = 0;    
    
    checkSum=0;
    downloadAddress=(U32)tempMem; //_RAM_STARTADDRESS; 
    downPt=(unsigned char *)downloadAddress;
	//This address is used for receiving first 8 byte.
    downloadFileSize=0;
    
#if 0
    MMU_DisableICache(); 
        //For multi-ICE. 
        //If ICache is not turned-off, debugging is started with ICache-on.
#endif

    /*******************************/
    /*    Test program download    */
    /*******************************/
    j=0;

    if(isUsbdSetConfiguration==0)
    {
	Uart_Printf("USB host is not connected yet.\n");
    }

    while(downloadFileSize==0)
    {
        if(first==1 && isUsbdSetConfiguration!=0)
        {
            Uart_Printf("USB host is connected. Waiting a download.\n");
            first=0;
        }

		if(j%0x50000==0)Led_Display(0x6);
		if(j%0x50000==0x28000)Led_Display(0x9);
		j++;

		key=Uart_GetKey();
		if(key!=0)
		{
			Menu();
            first=1; //To display the message,"USB host ...."
		}

    }

    Timer_InitEx();      
    Timer_StartEx();  

#if USBDMA    

    rINTMSK&=~(BIT_DMA2);  

    ClearEp3OutPktReady(); 
    	// indicate the first packit is processed.
    	// has been delayed for DMA2 cofiguration.

    if(downloadFileSize>EP3_PKT_SIZE)
    {
        if(downloadFileSize<=(0x80000))
        {
			ConfigEp3DmaMode(downloadAddress+EP3_PKT_SIZE-8,downloadFileSize-EP3_PKT_SIZE);	
			//wait until DMA reload occurs.
			while((rDSTAT2&0xfffff)==0);
			
			//will not be used.
       	    rDIDST2=(downloadAddress+downloadFileSize-EP3_PKT_SIZE);  
       	    rDIDSTC2=(0<<1)|(0<<0);  
			rDCON2=rDCON2&~(0xfffff)|(0);
		}
		else
		{
			ConfigEp3DmaMode(downloadAddress+EP3_PKT_SIZE-8,0x80000-EP3_PKT_SIZE);
			//wait until DMA reload occurs.
			while((rDSTAT2&0xfffff)==0);
			
			if(downloadFileSize>(0x80000*2))//for 1st autoreload
			{
				rDIDST2=(downloadAddress+0x80000-8);  //for 1st autoreload.
				rDIDSTC2=(0<<1)|(0<<0);  
				rDCON2=rDCON2&~(0xfffff)|(0x80000);
				
				while(rEP3_DMA_TTC<0xfffff)
				{
					rEP3_DMA_TTC_L=0xff;
					rEP3_DMA_TTC_M=0xff;
					rEP3_DMA_TTC_H=0xf;
				}
			}
			else
			{
				rDIDST2=(downloadAddress+0x80000-8);  //for 1st autoreload.
				rDIDSTC2=(0<<1)|(0<<0);
				rDCON2=rDCON2&~(0xfffff)|(downloadFileSize-0x80000);
				while(rEP3_DMA_TTC<0xfffff)
				{
					rEP3_DMA_TTC_L=0xff; 
					rEP3_DMA_TTC_M=0xff;
					rEP3_DMA_TTC_H=0xf;
				}
			}
		}
		totalDmaCount=0;
    }
    else
    {
		totalDmaCount=downloadFileSize;
    }
#endif

    Uart_Printf("\nNow, Downloading [ADDRESS:%xh,TOTAL:%d]\n",
    		downloadAddress,downloadFileSize);
    Uart_Printf("RECEIVED FILE SIZE:%8d",0);

#if USBDMA    
    j=0x10000;
	
    while(1)
    {
		if( (rDCDST2-(U32)downloadAddress+8)>=j)
		{
			Uart_Printf("\b\b\b\b\b\b\b\b%8d",j);
			j+=0x10000;
		}
		if(totalDmaCount>=downloadFileSize)break;
    }

#else
    j=0x10000;
	
	Uart_Printf("MMMMMMM.\r\n", j);    
    while(((U32)downPt-downloadAddress)<(downloadFileSize-8))
    {		

		if( ((U32)downPt-downloadAddress)>=j)
		{
			Uart_Printf("\b\b\b\b\b\b\b\b%8d",j);
//			Uart_Printf("[%d]\r\n", j);
			j+=0x10000;
		}
#if 1
		{
			U8 out_csr3;
			U8 rSave = rINDEX_REG;
			rINDEX_REG=3;
			out_csr3=rOUT_CSR1_REG;
			if(out_csr3 & EPO_OUT_PKT_READY)
			{
				int num = rOUT_FIFO_CNT1_REG;
		    	idle = 0;
				while(num)
				{
					*downPt = (U8)rEP3_FIFO;
					checkSum += *downPt;
					downPt ++;
					num --;
				}				
				CLR_EP3_OUT_PKT_READY();
			}
			else
			{
				idle++;
		    if(out_csr3 & EPO_SENT_STALL)
		    {   
//		DbgPrintf("[STALL]");
			#define CLR_EP3_SENT_STALL()	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					&(~EPO_SENT_STALL) )
		    	
				Uart_Printf("[STALL]\r\n");
				CLR_EP3_SENT_STALL();
		    }
				
				if( idle > 500000 )
				{
					idle = 0;
					CLR_EP3_OUT_PKT_READY();
                	Uart_Printf("idle(0x%x).\r\n", out_csr3 );
				}
			}
			
			rINDEX_REG = rSave;
		}
#endif
    }
#endif
    time=Timer_StopEx();
    Uart_Printf("\b\b\b\b\b\b\b\b%8d",downloadFileSize);	
//    Uart_Printf("(%5.1fKB/S,%3.1fS)\n",(float)(downloadFileSize/time/1000.),time);
    
#if USBDMA    
    /*******************************/
    /*     Verify check sum        */
    /*******************************/

    Uart_Printf("Now, Checksum calculation\n");

    cs=0;
    i=(downloadAddress);
    j=(downloadAddress+downloadFileSize-10)&0xfffffffc;
    while(i<j)
    {
    	temp=*((U32 *)i);
    	i+=4;
    	cs+=(U16)(temp&0xff);
    	cs+=(U16)((temp&0xff00)>>8);
    	cs+=(U16)((temp&0xff0000)>>16);
    	cs+=(U16)((temp&0xff000000)>>24);
    }

    i=(downloadAddress+downloadFileSize-10)&0xfffffffc;
    j=(downloadAddress+downloadFileSize-10);
    while(i<j)
    {
		cs+=*((U8 *)i++);
    }
    
    checkSum=cs;
#else
    //checkSum was calculated including dnCS. So, dnCS should be subtracted.
    checkSum=checkSum - *((unsigned char *)(downloadAddress+downloadFileSize-8-2))
	     - *( (unsigned char *)(downloadAddress+downloadFileSize-8-1) );	
#endif	  

    dnCS=*((unsigned char *)(downloadAddress+downloadFileSize-8-2))+
	(*( (unsigned char *)(downloadAddress+downloadFileSize-8-1) )<<8);

    if(checkSum!=dnCS)
    {
		Uart_Printf("Checksum Error!!! MEM:%x DN:%x\n",checkSum,dnCS);
		return;
    }

    Uart_Printf("Download O.K.\n\n");
    Uart_TxEmpty(consoleNum);


    if(download_run==1)
    {
        rINTMSK=BIT_ALLMSK;
    	run=(void (*)(void))downloadAddress;
		run();
    }
}




void Isr_Init(void)
{
//    pISR_UNDEF=(unsigned)HaltUndef;
//    pISR_SWI  =(unsigned)HaltSwi;
//    pISR_PABORT=(unsigned)HaltPabort;
//    pISR_DABORT=(unsigned)HaltDabort;
    rINTMOD=0x0;	  // All=IRQ mode
    rINTMSK=BIT_ALLMSK;	  // All interrupt is masked.

    //pISR_URXD0=(unsigned)Uart0_RxInt;	
    //rINTMSK=~(BIT_URXD0);   //enable UART0 RX Default value=0xffffffff
#if 0
#if 1
    pISR_USBD =(unsigned)IsrUsbd;
    pISR_DMA2 =(unsigned)IsrDma2;
#else
    pISR_IRQ =(unsigned)IsrUsbd;	
    	//Why doesn't it receive the big file if use this. (???)
    	//It always stops when 327680 bytes are received.
#endif
#endif
    ClearPending(BIT_DMA2);
    ClearPending(BIT_USBD);
    //rINTMSK&=~(BIT_USBD);  
   
    //pISR_FIQ,pISR_IRQ must be initialized
}


void HaltUndef(void)
{
    Uart_Printf("Undefined instruction exception!!!\n");
    while(1);
}

void HaltSwi(void)
{
    Uart_Printf("SWI exception!!!\n");
    while(1);
}

void HaltPabort(void)
{
    Uart_Printf("Pabort exception!!!\n");
    while(1);
}

void HaltDabort(void)
{
    Uart_Printf("Dabort exception!!!\n");
    while(1);
}



