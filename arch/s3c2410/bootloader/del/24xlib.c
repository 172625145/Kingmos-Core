/************************************************
 * NAME    : 24XLIB.C               *
 ************************************************/
// Revision History 
// 2001.3.15:SJS     :draft ver 0.0
// 2001.3.19:purnnamu:ChangeClockDivider() uses AsynBusMode.
// 2001.3.21:ctype.h was included for isalpha and isupper function.
// 2001.6.30:UBRDIVn calculation formula is changed 
//       UBRDIVn=( (int)(pclk/16./baud) -1 ); 
// 2001.8.25:Uart_GetIntNum() returns -1 when there is no input.
// 2001.8.25:Uart_Printf,Uart_PrintString function declaration is changed for C++

#include "def.h"
#include "option.h"
#include "24x.h"
#include "24xlib.h"
#include "24xslib.h" 

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

extern char Image$$RW$$Limit[];

void *mallocPt=Image$$RW$$Limit;


/**************************** SYSTEM ****************************/
static int delayLoopCount=400;

void Delay(int time)
// time=0: adjust the Delay function by WatchDog timer.
// time>0: the number of loop time
// resolution of time is 100us.
{
	int i,adjust=0;
	if(time==0)
	{
	time=200;
	adjust=1;
	delayLoopCount=400;
	rWTCON=((PCLK/1000000-1)<<8)|(2<<3);    //PCLK/1M,Watch-dog disable,1/64,interrupt disable,reset disable
	rWTDAT=0xffff;//for first update
	rWTCNT=0xffff;//resolution=64us @any PCLK 
	rWTCON=((PCLK/1000000-1)<<8)|(2<<3)|(1<<5); //Watch-dog timer start
	}
	for(;time>0;time--)
	for(i=0;i<delayLoopCount;i++);
	if(adjust==1)
	{
	rWTCON=((PCLK/1000000-1)<<8)|(2<<3);//Watch-dog timer stop
	i=0xffff-rWTCNT; // 1count->64us, 200*400 cycle runtime = 64*i us
	delayLoopCount=8000000/(i*64);  //200*400:64*i=1*x:100 -> x=80000*100/(64*i)   
	}
}


/*************************** PORTS ******************************/
void Port_Init(void)
{
	//CAUTION:Follow the configuration order for setting the ports. 
	// 1) setting value(PnDAT) 
	// 2) setting control register (PnCON)
	// 3) configure pull-up resistor(PnUP)  

	//32bit data bus configuration  
	//PORT A GROUP
	//nGCS[5] nGCS[4] nGCS[3] nGCS[2] nGCS[1] nCAS[1] nCAS[0] SCKE ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0 
	//      1,      1,      1,      1,      1,      1,      1,   1,     1,     1,     1,     1,     1,     1,     1,     1,     1,    1          
	rPACON=0x3ffff; 

	//PORT B GROUP
#if (BUSWIDTH==32)
	//DATA31 DATA30 DATA29 DATA28 DATA27 DATA26 DATA25 DATA24 DATA23 DATA22 DATA21 DATA20 DATA19 DATA18 DATA17 DATA16
	//    10,    10,    10,    10,    10,    10,    10,    10,    10,    10,    10,    10,    10,    10,    10,    10
	rPBCON=0xaaaaaaaa;
	rPBUP=0xffff;
#else //BUSWIDTH=16
	//All input
	rPBCON=0x0; 
	rPBUP=0x0;
#endif

	//PORT C GROUP
	//LED_OUTPUT LED_OUTPUT LED_OUTPUT LED_OUTPUT INPUT INPUT INPUT INPUT INPUT INPUT INPUT INPUT INPUT INPUT INPUT INPUT 
	//        01,        01,        01,        01,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00
	rPCCON=0x55000000;  
	rPCUP=0x0;  

	//PORT D GROUP
	//All input except GPD9=output
	rPDDAT=0;
	rPDCON=0x40000; 
	rPDUP=0x200; //GPD9(nDIS_OFF) pull-up resister should be turned off. GPD9 is pulled down by external resister.

	//PORT E GROUP
	//All input
	rPECON=0x0; 
	rPEUP=0x0;

	//PORT F GROUP
	//INPUT INPUT INPUT INPUT TXD[0] INPUT RXD[0]
	//   00,   00,   00,   00,    10,   00,    10
	rPFCON=0x22;
	rPFUP=0x5;

	//PORT G GROUP
	//All input
	rPGCON=0x0;
	rPGUP=0x0;

	//OPEN DRAIN CONTROL DISABLE
	rOPENCR=0x0;

	//All EINT[7:0] will be falling edge triggered. 
	rEXTINT=0x22222222; 
}


/*************************** UART ******************************/
static int whichUart=0;

void Uart_Init(int pclk,int baud)
{
	int i;
	if(pclk==0)
	pclk=PCLK;
	rUFCON0=0x0;     //FIFO disable
	rUFCON1=0x0;
	rUMCON0=0x0;
	rUMCON1=0x0;
//UART0
	rULCON0=0x3;     //Normal,No parity,1 stop,8 bit
	rUCON0=0x245;    //tx=level,rx=edge,disable timeout int.,enable rx error int.,normal,interrupt or polling
	rUBRDIV0=( (int)(pclk/16./baud) -1 );
//UART1
	rULCON1=0x3;
	rUCON1=0x245;
	rUBRDIV1=( (int)(pclk/16./baud) -1 );

	for(i=0;i<100;i++);
}


void Uart_Select(int ch)
{
	whichUart=ch;
}


void Uart_TxEmpty(int ch)
{
	if(ch==0)
	while(!(rUTRSTAT0 & 0x4)); //wait until tx shifter is empty.
	else
		while(!(rUTRSTAT1 & 0x4)); //wait until tx shifter is empty.
}


char Uart_Getch(void)
{
	if(whichUart==0)
	{       
	while(!(rUTRSTAT0 & 0x1)); //Receive data ready
	return RdURXH0();
	}
	else
	{
	while(!(rUTRSTAT1 & 0x1)); //Receive data ready
	return rURXH1;
	}
}


char Uart_GetKey(void)
{
	if(whichUart==0)
	{       
	if(rUTRSTAT0 & 0x1)    //Receive data ready
			return RdURXH0();
	else
		return 0;
	}
	else
	{
	if(rUTRSTAT1 & 0x1)    //Receive data ready
		return rURXH1;
	else
		return 0;
	}
}


void Uart_GetString(char *string)
{
	char *string2=string;
	char c;
	while((c=Uart_Getch())!='\r')
	{
	if(c=='\b')
	{
		if( (int)string2 < (int)string )
		{
		Uart_Printf("\b \b");
		string--;
		}
	}
	else 
	{
		*string++=c;
		Uart_SendByte(c);
	}
	}
	*string='\0';
	Uart_SendByte('\n');
}


int Uart_GetIntNum(void)
{
	char str[30];
	char *string=str;
	int base=10;
	int minus=0;
	int lastIndex;
	int result=0;
	int i;
	
	Uart_GetString(string);
	
	if(string[0]=='-')
	{
		minus=1;
		string++;
	}
	
	if(string[0]=='0' && (string[1]=='x' || string[1]=='X'))
	{
	base=16;
	string+=2;
	}
	
	lastIndex=strlen(string)-1;
	if(lastIndex<0)return -1;
	
	if( string[lastIndex]=='h' || string[lastIndex]=='H' )
	{
	base=16;
	string[lastIndex]=0;
	lastIndex--;
	}

	if(base==10)
	{
	result=atoi(string);
	result=minus ? (-1*result):result;
	}
	else
	{
	for(i=0;i<=lastIndex;i++)
	{
		if(isalpha(string[i]))
		{
		if(isupper(string[i]))
			result=(result<<4)+string[i]-'A'+10;
		else
			result=(result<<4)+string[i]-'a'+10;
		}
		else
		{
		result=(result<<4)+string[i]-'0';
		}
	}
	result=minus ? (-1*result):result;
	}
	return result;
}


void Uart_SendByte(int data)
{
	if(whichUart==0)
	{
	if(data=='\n')
	{
		while(!(rUTRSTAT0 & 0x2));
		Delay(10);  //because the slow response of hyper_terminal 
		WrUTXH0('\r');
	}
	while(!(rUTRSTAT0 & 0x2)); //Wait until THR is empty.
	Delay(10);
	WrUTXH0(data);
	}
	else
	{
	if(data=='\n')
	{
			while(!(rUTRSTAT1 & 0x2));
		Delay(10);  //because the slow response of hyper_terminal 
		rUTXH1='\r';
	}
	while(!(rUTRSTAT1 & 0x2));  //Wait until THR is empty.
	Delay(10);
	rUTXH1=data;
	}   
}       


void Uart_SendString(const char *pt)
{
	while(*pt)
	Uart_SendByte(*pt++);
}


//If you don't use vsprintf(), the code size is reduced very much.
void Uart_Printf(const char *fmt,...)
{
	va_list ap;
	char string[256];

	va_start(ap,fmt);
	vsprintf(string,fmt,ap);
	Uart_SendString(string);
	va_end(ap);
}


/****************** S3C2400X01 EV. BOARD LED *******************/
void Led_Display(int data)
{
	rPCDAT=(rPCDAT & 0xfff) | ((data & 0xf)<<12);
}


/************************* Timer ********************************/
void Timer_Start(int divider)  //0:16us,1:32us 2:64us 3:128us
{
	rWTCON=((PCLK/1000000-1)<<8)|(divider<<3);
	rWTDAT=0xffff;
	rWTCNT=0xffff;   

	// 1/16/(65+1),nRESET & interrupt  disable
	rWTCON=((PCLK/1000000-1)<<8)|(divider<<3)|(1<<5);   
}



int Timer_Stop(void)
{
	rWTCON=((PCLK/1000000-1)<<8);
	return (0xffff-rWTCNT);
}



/************************** MPLL *******************************/
void ChangeMPllValue(int mdiv,int pdiv,int sdiv)
{
	rMPLLCON=(mdiv<<12)|(pdiv<<4)|sdiv;
}


/************************ HCLK, PCLK ***************************/

void ChangeClockDivider(int hdivn,int pdivn)
// hdivn:pdivn FCLK:HCLK:PCLK
// 0:0         1:1:1 
// 0:1         1:1:2 
// 1:0         1:2:2
// 1:1         1:2:4
{
	rCLKDIVN=(hdivn<<1)|pdivn;    
	if(hdivn)MMU_SetAsyncBusMode();
	else MMU_SetFastBusMode();
}


/************************** UPLL *******************************/
void ChangeUPllValue(int mdiv,int pdiv,int sdiv)
{
	rUPLLCON=(mdiv<<12)|(pdiv<<4)|sdiv;
}


/************************* General Library **********************/
void * malloc(unsigned nbyte) 
/*Very simple; Use malloc() & free() like Stack*/
//void *mallocPt=Image$$RW$$Limit;
{
	void *returnPt=mallocPt;

	mallocPt= (int *)mallocPt+nbyte/4+((nbyte%4)>0); //to align 4byte

	if( (int)mallocPt > HEAPEND )
	{
	mallocPt=returnPt;
	return NULL;
	}
	return returnPt;
}


void free(void *pt)
{
	mallocPt=pt;
}
