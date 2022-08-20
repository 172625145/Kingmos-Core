/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/


/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2001. Samsung Electronics, co. ltd  All rights reserved.
--*/

#include <ewindows.h>
#include <s2410.h>

#define     UART0_BAUDRATE	115200
#define     UART1_BAUDRATE	115200 //38400
#define     UART2_BAUDRATE	115200 //38400

#define USE_UART2

#ifdef USE_UART0
	#define     DEBUG_UART_BASE   UART0_BASE
	#define     DEBUG_UART_BAUDRATE UART0_BAUDRATE
#endif

#ifdef USE_UART1
	#define     DEBUG_UART_BASE   UART1_BASE
	#define     DEBUG_UART_BAUDRATE UART1_BAUDRATE
#endif

#ifdef USE_UART2
	#define     DEBUG_UART_BASE   UART2_BASE
	#define     DEBUG_UART_BAUDRATE UART2_BAUDRATE
#endif


void  OEM_WriteDebugString( char * str) ;

/*
    @func   void | OEMInitDebugSerial | Initialize serial debug monitor port.
    @rdesc  N/A.
    @comm    
    @xref   
*/
void OEM_InitDebugSerial(void) 
{
    volatile S2410_UART_REG   *s2410UART	= (S2410_UART_REG *)DEBUG_UART_BASE;
    volatile IOPreg     *s2410IOP	= (IOPreg *)IOP_BASE;

	// UART1 (TXD1 & RXD1) used for debug serial.
	//

	// Configure port H for UART.
	//
    #ifdef USE_UART0
		s2410IOP->rGPHCON &= ~((3 << 4) | (3 << 6));	// Configure GPH2 and GHP3 for UART0 Tx and Rx, respectively.
		s2410IOP->rGPHCON |=  ((2 << 4) | (2 << 6));	//
		//s2410IOP->rGPHUP  |=   (1 << 4) | (1 << 5);		// Disable pull-up on TXD1 and RXD1.
		s2410IOP->rGPHUP = 0x7ff;
    #endif

    #ifdef USE_UART1
		s2410IOP->rGPHCON &= ~((3 << 8) | (3 << 10));	// Configure GPH2 and GHP3 for UART1 Tx and Rx, respectively.
		s2410IOP->rGPHCON |=  ((2 << 8) | (2 << 10));	//
		s2410IOP->rGPHUP  |=   (1 << 4) | (1 << 5);		// Disable pull-up on TXD1 and RXD1.
    #endif

    #ifdef USE_UART2
		s2410IOP->rGPHCON &= ~((3 << 14) | (3 << 12));	// Configure GPH2 and GHP3 for UART2 Tx and Rx, respectively.
		s2410IOP->rGPHCON |=  ((2 << 14) | (2 << 12));	//
		s2410IOP->rGPHUP  |=   (1 << 7) | (1 << 6);		// Disable pull-up on TXD1 and RXD1.
    #endif
    

	// Configure UART.
	//
	s2410UART->rUFCON  = 0x0;		// Disable the FIFO (TODO: do we need to enable the FIFO?)
	s2410UART->rUMCON  = 0x0;		// Disable AFC.
	s2410UART->rULCON  = 0x3;		// Normal mode, N81.
	s2410UART->rUCON   = 0x245;	// Rx pulse interrupt, Tx level interrupt, Rx error status interrupt enabled.
//	s2410UART->rUBRDIV = ( (int)(S2410PCLK/16.0/DEBUG_UART_BAUDRATE + 0.5) -1 );		// Set up baudrate (38400).
	s2410UART->rUBRDIV = ( S2410PCLK+8*DEBUG_UART_BAUDRATE ) / (16*DEBUG_UART_BAUDRATE) -1;

	OEM_WriteDebugString("*************************************************\r\n");
	OEM_WriteDebugString("********* debug output port is serial   *********\r\n");	
	OEM_WriteDebugString("*************************************************\r\n");
	

}


/*****************************************************************************
*
*
*   @func   void    |   OEMWriteDebug | Display value to specified LED port.
*
*   The wIndex parameter can be used to specify a write to the discrete LEDS (if
*   0xffff), otherwise is used as an offset (DWORD aligned) for the Alpha LEDS
*   for triggering.
*/
void OEM_WriteDebugLED(WORD wIndex, DWORD dwPattern)
{
	volatile	IOPreg *s2410IOP = (IOPreg * )IOP_BASE;

    // The S24x0X01 Eval platform supports 4 LEDs..
	//
    s2410IOP->rGPFDAT=(s2410IOP->rGPFDAT & 0xf) | ((dwPattern & 0xf)<<4);
}



/*****************************************************************************
*
*
*   @func   void    |   OEMWriteDebugByte | Output byte to the monitor port.
*
*   @parm   unsigned char *| str |
*           Points to the output buffer.
*/
void OEM_WriteDebugByte(UCHAR ch) 
{
    volatile S2410_UART_REG *s2410UART	= (S2410_UART_REG *)DEBUG_UART_BASE;

	// Wait for transmit buffer to be empty.
	//
	while(!(s2410UART->rUTRSTAT & 0x2))
	{
	}

    s2410UART->rUTXH = ch;
}

/*****************************************************************************
*
*
*   @func   void    |   OEMWriteDebugString | Display string to the monitor port.
*
*   @parm   unsigned short * | str |
*           Points to the receiving buffer.
*/
void  OEM_WriteDebugString( char *str ) 
{
	// Loop through text string, sending characters.
	//
    while (str && *str)
	{
        OEM_WriteDebugByte((UCHAR)*str++);
	}
}

int OEM_ReadDebugByte() 
{
    unsigned char ch;
    volatile S2410_UART_REG *s2410UART = (S2410_UART_REG *)DEBUG_UART_BASE;
    
	// Any receive data for us?
	//
    if (!(s2410UART->rUTRSTAT & 0x1))
	{
        ch = 0;//OEM_DEBUG_READ_NODATA;		// No data.
	}
    else
	{
        ch = s2410UART->rURXH;			// Read character.
	}

    return (ch);
}


/*****************************************************************************
*
*
*   @func   void    |   OEMClearDebugComError | Clear a debug communications error
*
*/
void OEM_ClearDebugCommError(void) 
{
	unsigned int ReadReg;
    volatile S2410_UART_REG *s2410UART	= (S2410_UART_REG *)DEBUG_UART_BASE;
    
	// Reading UART error status clears the status.
	//
	ReadReg=s2410UART->rUERSTAT;
}


