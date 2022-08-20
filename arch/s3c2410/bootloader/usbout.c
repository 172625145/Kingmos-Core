/****************************************************************
 NAME: usbout.c
 DESC: USB bulk-OUT operation related functions
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <eframe.h>
//#include <string.h>
#include "option.h"
#include "2410addr.h"
#include "2410lib.h"
#include "def.h"

#include "2410usb.h"
#include "usbmain.h"
#include "usb.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbout.h"

#include "u241mon.h"


static void PrintEpoPkt(U8 *pt,int cnt);
//static void RdPktEp3_CheckSum(U8 *buf,int num);
static int RdPktEp3_CheckSum(U8 *buf,int num);
static BOOL GetHead(U8 *buf, int num);


// ===================================================================
// All following commands will operate in case 
// - out_csr3 is valid.
// ===================================================================

 

#define CLR_EP3_OUT_PKT_READY() rOUT_CSR1_REG= ( out_csr3 &(~ EPO_WR_BITS)\
					&(~EPO_OUT_PKT_READY) ) 
#define SET_EP3_SEND_STALL()	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					| EPO_SEND_STALL) )
#define CLR_EP3_SENT_STALL()	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					&(~EPO_SENT_STALL) )
#define CLR_EP3_OVERUN()		rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					&(~EPO_OVER_RUN) )
#define FLUSH_EP3_FIFO() 	rOUT_CSR1_REG= ( out_csr3 & (~EPO_WR_BITS)\
					|EPO_FIFO_FLUSH) )

// ***************************
// *** VERY IMPORTANT NOTE ***
// ***************************
// Prepare for the packit size constraint!!!

// EP3 = OUT end point. 

U8 ep3Buf[EP3_PKT_SIZE];
static U8 tempBuf[64+1];

void Ep3Handler(void)
{
    U8 out_csr3;
    int fifoCnt;
    rINDEX_REG=3;
	
    out_csr3=rOUT_CSR1_REG;
    
//    DbgPrintf("<3:%x]",out_csr3);
    if(out_csr3 & EPO_OUT_PKT_READY)
    {   
		fifoCnt=rOUT_FIFO_CNT1_REG; 
#if 0
		RdPktEp3(ep3Buf,fifoCnt);
		PrintEpoPkt(ep3Buf,fifoCnt);
#else
		
		if(downloadFileSize==0)
		{
			int nFirstGet = 0;
			downPt = ep3Buf;
			if(!GetHead((U8 *)downPt,8))
			{
				while(1);
			}
			
			if(download_run==0)
			{
				downloadAddress=tempDownloadAddress;
			}
			else
			{
				downloadAddress=
					*((U8 *)(downPt+0))+
					(*((U8 *)(downPt+1))<<8)+
					(*((U8 *)(downPt+2))<<16)+
					(*((U8 *)(downPt+3))<<24);
			}
			downloadFileSize=
				*((U8 *)(downPt+4))+
				(*((U8 *)(downPt+5))<<8)+
				(*((U8 *)(downPt+6))<<16)+
				(*((U8 *)(downPt+7))<<24);
			checkSum=0;
			downPt=(U8 *)downloadAddress;

//			RdPktEp3_CheckSum((U8 *)downPt,fifoCnt-8); //The first 8-bytes are deleted.	    
//			downPt+=fifoCnt-8;
			downPt += RdPktEp3_CheckSum((U8 *)downPt,0);
			rINTMSK|=BIT_USBD; //for debug
#if USBDMA
			//CLR_EP3_OUT_PKT_READY() is not executed. 
			//So, USBD may generate NAK until DMA2 is configured for USB_EP3;
			rINTMSK|=BIT_USBD; //for debug
			return;	
#endif	
		}
		else
		{
#if USBDMA    	
			Uart_Printf("<ERROR>");
#endif    
//			RdPktEp3_CheckSum((U8 *)downPt,fifoCnt);
//			downPt+=fifoCnt;  //fifoCnt=64
//			CLR_EP3_OUT_PKT_READY();
			downPt += RdPktEp3_CheckSum((U8 *)downPt,0);
		}
#endif
		CLR_EP3_OUT_PKT_READY();
		return;
    }
	
    //I think that EPO_SENT_STALL will not be set to 1.
    if(out_csr3 & EPO_SENT_STALL)
    {   
//		DbgPrintf("[STALL]");
		Uart_Printf("[STALL]\r\n");
		CLR_EP3_SENT_STALL();
		return;
    }
}



void PrintEpoPkt(U8 *pt,int cnt)
{
    int i;
    DbgPrintf("[BOUT:%d:",cnt);
    for(i=0;i<cnt;i++)
    	DbgPrintf("%x,",pt[i]);
    DbgPrintf("]");
}


/*void RdPktEp3_CheckSum(U8 *buf,int num)
{
    int i;
    	
    for(i=0;i<num;i++)
    {
        buf[i]=(U8)rEP3_FIFO;
        checkSum+=buf[i];
    }
}*/
BOOL GetHead(U8 *buf, int num)
{
	int i = 0;
    while(rOUT_FIFO_CNT1_REG && num)
	{
		buf[i]=(U8)rEP3_FIFO;
		checkSum+=buf[i];
		i ++;
		num --;
	}
	if (num)
	{
		Uart_Printf("Get head fail\r\n");
		return FALSE;
	}
	return TRUE;
}

int RdPktEp3_CheckSum(U8 *buf, int num)
{
    int i = 0;
here:
	num = rOUT_FIFO_CNT1_REG;
    while(num)
	{
		buf[i]=(U8)rEP3_FIFO;
		checkSum+=buf[i];
		i ++;
		num --;
	}
	if(rOUT_FIFO_CNT1_REG)
	{
		Uart_Printf("fifo not empty\r\n");
		goto here;
	}
    return i;
}


void __irq IsrDma2(void)
{
    U8 out_csr3;
    U32 nextTotalDmaCount;
    U8 saveIndexReg=rINDEX_REG;
	
    rINDEX_REG=3;
    out_csr3=rOUT_CSR1_REG;
    
    ClearPending(BIT_DMA2);
    
    totalDmaCount+=0x80000;
	
    if(totalDmaCount>=downloadFileSize)// is last?
    {
		totalDmaCount=downloadFileSize;
		
		ConfigEp3IntMode();
		
		if(out_csr3& EPO_OUT_PKT_READY)
		{
			CLR_EP3_OUT_PKT_READY();
		}
        rINTMSK|=BIT_DMA2;
        rINTMSK&=~(BIT_USBD);
    }
    else
    {
		if((totalDmaCount+0x80000)<downloadFileSize)
		{
			nextTotalDmaCount=totalDmaCount+0x80000;
			
			if((nextTotalDmaCount+0x80000)<downloadFileSize)
			{
				//for (2~n)th autoreload.	 
				while((rDSTAT2&0xfffff)==0); //wait until autoreload occurs.
				rDIDST2=((U32)downloadAddress+nextTotalDmaCount-8);
				rDIDSTC2=(0<<1)|(0<<0);
				rDCON2=rDCON2&~(0xfffff)|(0x80000);
				
				while(rEP3_DMA_TTC<0xfffff)
				{
					rEP3_DMA_TTC_L=0xff;
					rEP3_DMA_TTC_M=0xff;
					rEP3_DMA_TTC_H=0xf;
					//0xfffff;
				}
			}
			else
			{
				while((rDSTAT2&0xfffff)==0); //wait until autoreload occurs.
				rDIDST2=((U32)downloadAddress+nextTotalDmaCount-8);  
				rDIDSTC2=(0<<1)|(0<<0);  
				rDCON2=rDCON2&~(0xfffff)|(downloadFileSize-nextTotalDmaCount); 		
				while(rEP3_DMA_TTC<0xfffff)
				{
					rEP3_DMA_TTC_L=0xff;
					rEP3_DMA_TTC_M=0xff;
					rEP3_DMA_TTC_H=0xf;
					//0xfffff;
				}
			}
		}
		else
		{
			while((rDSTAT2&0xfffff)==0); //wait until autoreload occurs.
			rDIDST2=((U32)downloadAddress+downloadFileSize-8);  //for next autoreload.	    		
			rDIDSTC2=(0<<1)|(0<<0);
			rDCON2=rDCON2&~(0xfffff)|(0);
			//There is no 2nd autoreload. This will not be used.  	    
			//rDMA_TX+=0x0; //USBD register		
		}
    }
    rINDEX_REG=saveIndexReg;
}


void ClearEp3OutPktReady(void)
{
    U8 out_csr3;
    rINDEX_REG=3;
    out_csr3=rOUT_CSR1_REG;
    CLR_EP3_OUT_PKT_READY();
}
