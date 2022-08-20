#include <eframe.h>
#include <gwmeobj.h>
#include <edevice.h>
#include <eobjtype.h>

#include <s2410.h>

extern const _DISPLAYDRV _drvDisplay16BPP;
static COLORREF _PutPixel( _LPPIXELDATA lpPixelData );
static COLORREF _GetPixel( _LPPIXELDATA lpPixelData );
static BOOL _Line( _LPLINEDATA lpData );
static BOOL _BlkBitTransparentBlt( _LPBLKBITBLT lpData );
static BOOL _BlkBitMaskBlt( _LPBLKBITBLT lpData );
static BOOL _BlkBitBlt( _LPBLKBITBLT lpData );
static COLORREF _RealizeColor( COLORREF color, LPCDWORD lpdwPalEntry, UINT uPalNumber, UINT uPalFormat );
static COLORREF _UnrealizeColor( COLORREF color, LPCDWORD lpdwPalEntry, UINT uPalNumber, UINT uPalFormat );

static const _DISPLAYDRV oemDisplay16BPP = {
    _PutPixel,
    _GetPixel,
    _Line,
    _BlkBitTransparentBlt,
    _BlkBitMaskBlt,
    _BlkBitBlt,
    _RealizeColor,
    _UnrealizeColor,
};


static void Draw3OriginalColor(unsigned C1,unsigned C2,unsigned C3);
static void Draw3OriginalColorAlternate(unsigned win,unsigned C1,unsigned C2,unsigned C3);
static void DisplayerOff(void);

extern void msWait(unsigned);       //Added by Bruce
extern const int nSkipHeight;
extern const int nSkipWidth;
extern const unsigned short wSkipImage[];

//#define  ROTATE

static const _BITMAPDATA oemDisplayBitmap = 
{ 
	{ OBJ_BITMAP, NULL, NULL, NULL, NULL }, 
		0,
		1,
		16,
		240,
		320,
		240 * 2,
		(unsigned *)(FRAMEBUF_BASE) 
};

#define DEBUG_InitDisplay 0
static void InitDisplay( void )
{
    //int i, j;
    volatile IOPreg *s2410IOP;
    volatile LCDreg *s2410LCD;    


    s2410IOP = (IOPreg *)IOP_BASE;
    s2410LCD = (LCDreg *)LCD_BASE; 
    RETAILMSG( DEBUG_InitDisplay, ( "s2410IOP=0x%x, s2410LCD=0x%x.\r\n", s2410IOP, s2410LCD ) );


	// alex add here for backlight initialize pwm control
	s2410IOP->rGPBCON &= ~(0x3<<0);
	s2410IOP->rGPBCON |= (0x1<<0);  // work as output
	s2410IOP->rGPBDAT |= (0x1<<0);
	//alex add end
	

    // LCD port initialize.
    s2410IOP->rGPCUP  = 0xFFFFFFFF;
    s2410IOP->rGPCCON = 0xAAAAAAAA;

    s2410IOP->rGPCCON = 0xAAAAAAAA;

    s2410IOP->rGPDUP  = 0xFFFFFFFF;
    s2410IOP->rGPDCON = 0xAAAAAAAA;

    //s2410IOP->rGPGCON &= ~(3 << 8);                 /* Set LCD_PWREN as output                          */
    //s2410IOP->rGPGCON |=  (1 << 8);

    //s2410IOP->rGPGDAT |=  (1 << 4);                 /* Backlight ON                                     */

//		lcdcon1 : LCD1_BPP_16T | LCD1_PNR_TFT | LCD1_CLKVAL(7) ,
		//lcdcon2 : LCD2_VBPD(4) | LCD2_VFPD(1) | LCD2_VSPW(1),
//		lcdcon3 : LCD3_HBPD(6) | LCD3_HFPD(30),
//		lcdcon4 : LCD4_HSPW(3) | LCD4_MVAL(13),
//		lcdcon5 : LCD5_FRM565 | LCD5_HWSWP | LCD5_PWREN,


//    s2410LCD->rLCDCON1   =  (6           <<  8) |   /* VCLK = HCLK / ((CLKVAL + 1) * 2) -> About 7 Mhz  */
//                            (MVAL_USED   <<  7) |   /* 0 : Each Frame                                   */
//                            (3           <<  5) |   /* TFT LCD Pannel                                   */
//                            (12          <<  1) |   /* 16bpp Mode                                       */
//                            (0           <<  0) ;   /* Disable LCD Output                               */
    s2410LCD->rLCDCON1   =  (6           <<  8) |   /* 7 VCLK = HCLK / ((CLKVAL + 1) * 2) -> About 7 Mhz  */
                            (MVAL_USED   <<  7) |   /* 0 : Each Frame                                   */
                            (3           <<  5) |   /* 3 TFT LCD Pannel                                   */
                            (12          <<  1) |   /* 12 16bpp Mode                                       */
                            (0           <<  0) ;   /* 0 Disable LCD Output                               */

    RETAILMSG( DEBUG_InitDisplay, ( "s2410LCD->rLCDCON1=0x%x.\r\n", s2410LCD->rLCDCON1 ) );                            

//    s2410LCD->rLCDCON2   =  (VBPD        << 24) |   /* VBPD          :   1                              */
  //                          (LINEVAL_TFT << 14) |   /* Virtical Size : 320 - 1                          */
    //                        (VFPD        <<  6) |   /* VFPD          :   2                              */
      //                      (VSPW        <<  0) ;   /* VSPW          :   1                              */

    s2410LCD->rLCDCON2   =  (4        << 24) |   /*4 VBPD          :   4                              */
                            (LINEVAL_TFT << 14) |   /* Virtical Size : 320 - 1                          */
                            (1        <<  6) |   /* 1 VFPD          :   1                              */
                            (1        <<  0) ;   /* 1 VSPW          :   1                              */
        
    RETAILMSG( 1, ( "s2410LCD->rLCDCON2=0x%x.\r\n", s2410LCD->rLCDCON2 ) );                                                        

//    s2410LCD->rLCDCON3   =  (HBPD        << 19) |   /* HBPD          :   6                              */
//                            (HOZVAL_TFT  <<  8) |   /* HOZVAL_TFT    : 240 - 1                          */
//                            (HFPD        <<  0) ;   /* HFPD          :   2                              */

    s2410LCD->rLCDCON3   =  (6        << 19) |   /* 6 HBPD          :   6                              */
                            (HOZVAL_TFT  <<  8) |   /* HOZVAL_TFT    : 240 - 1                          */
                            (30        <<  0) ;   /* HFPD          :   30                              */
    
    RETAILMSG( DEBUG_InitDisplay, ( "s2410LCD->rLCDCON3=0x%x.\r\n", s2410LCD->rLCDCON3 ) );
//    s2410LCD->rLCDCON4   =  (MVAL        <<  8) |   /* MVAL          :  13                              */
//                            (HSPW        <<  0) ;   /* HSPW          :   4                              */
    s2410LCD->rLCDCON4   =  (13        <<  8) |   /* MVAL          :  13                              */
                            (3        <<  0) ;   /* HSPW          :   4                              */

    RETAILMSG( DEBUG_InitDisplay, ( "s2410LCD->rLCDCON4=0x%x.\r\n", s2410LCD->rLCDCON4 ) );
    
//    s2410LCD->rLCDCON5   =  (0           << 12) |   /* BPP24BL       : LSB valid                        */
  //                          (1           << 11) |   /* FRM565 MODE   : 5:6:5 Format                     */
//                            (0           << 10) |   /* INVVCLK       : VCLK Falling Edge                */
//                            (1           <<  9) |   /* INVVLINE      : Inverted Polarity                */
//                            (1           <<  8) |   /* INVVFRAME     : Inverted Polarity                */
  //                          (0           <<  7) |   /* INVVD         : Normal                           */
    //                        (0           <<  6) |   /* INVVDEN       : Normal                           */
      //                      (0           <<  5) |   /* INVPWREN      : Normal                           */
        //                    (0           <<  4) |   /* INVENDLINE    : Normal                           */
          //                  (0           <<  3) |   /* PWREN         : Disable PWREN                    */
            //                (0           <<  2) |   /* ENLEND        : Disable LEND signal              */
              //              (0           <<  1) |   /* BSWP          : Swap Disable                     */
                //            (1           <<  0) ;   /* HWSWP         : Swap Enable                      */

    s2410LCD->rLCDCON5   =  (0           << 12) |   /* BPP24BL       : LSB valid                        */
                            (1           << 11) |   /* FRM565 MODE   : 5:6:5 Format                     */
                            (0           << 10) |   /* INVVCLK       : VCLK Falling Edge                */
                            (0           <<  9) |   /* INVVLINE      : Inverted Polarity                */
                            (0           <<  8) |   /* INVVFRAME     : Inverted Polarity                */
                            (0           <<  7) |   /* INVVD         : Normal                           */
                            (0           <<  6) |   /* INVVDEN       : Normal                           */
                            (0           <<  5) |   /* INVPWREN      : Normal                           */
                            (0           <<  4) |   /* INVENDLINE    : Normal                           */
                            (1           <<  3) |   /* PWREN         : Disable PWREN                    */
                            (0           <<  2) |   /* ENLEND        : Disable LEND signal              */
                            (0           <<  1) |   /* BSWP          : Swap Disable                     */
                            (1           <<  0) ;   /* HWSWP         : Swap Enable                      */
        
    RETAILMSG( 1, ( "s2410LCD->rLCDCON5=0x%x.\r\n", s2410LCD->rLCDCON5 ) );
    
    s2410LCD->rLCDSADDR1 = ((FRAMEBUF_DMA_BASE >> 22)     << 21) |
                           ((M5D(FRAMEBUF_DMA_BASE >> 1)) <<  0);
    RETAILMSG( 1, ( "s2410LCD->rLCDSADDR1=0x%x.\r\n", s2410LCD->rLCDSADDR1 ) );
    
    s2410LCD->rLCDSADDR2 = M5D((FRAMEBUF_DMA_BASE + (LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2)) >> 1);

    RETAILMSG( 1, ( "s2410LCD->rLCDSADDR2=0x%x.\r\n", s2410LCD->rLCDSADDR2 ) );
    
    s2410LCD->rLCDSADDR3 = (((LCD_XSIZE_TFT - LCD_XSIZE_TFT) / 1) << 11) | (LCD_XSIZE_TFT / 1);
    RETAILMSG( 1, ( "s2410LCD->rLCDSADDR3=0x%x.\r\n", s2410LCD->rLCDSADDR3 ) );
    
/*    
#if (BOARD_TYPE == 1)
    s2410LCD->rLPCSEL   |= 0x3;
#else
    s2410LCD->rLPCSEL   |= 0x2;
#endif */
    s2410LCD->rLPCSEL = 2; //240*320 disable lpc3600

    s2410LCD->rTPAL     = 0x0;
    s2410LCD->rLCDCON1 |= 1;

//    RETAILMSG( 1, ( "copy logo(wSkipImage=0x%x, size=%d)++.\r\n", wSkipImage, ARRAY_SIZE_TFT_16BIT ) );
        
	memcpy((void *)FRAMEBUF_BASE, wSkipImage, 240*320*2 );
	
//    RETAILMSG( 1, ( "copy logo--.\r\n" ) );

}


static void DisplayerOff(void)
{
}

/*
static void ShowSkip( void )
{
	int i, j;
	LPWORD lpwLine = wSkipImage;
	int ySkipOffset = 0;//(600 - nSkipHeight) / 2;
	int xSkipOffset = 0;//(800 - nSkipWidth) / 2;
		
	LPWORD lpwDestLine = (LPWORD)FRAMEBUF_BASE + ySkipOffset * 240 + xSkipOffset;
	int nScanWords = nSkipWidth;

	for( i = 0; i < 7; i++ )
	{	
		LPWORD lpwSrc = lpwLine;
		LPWORD lpwDest = lpwDestLine;
		for( j = 0; j < nSkipWidth; j++ )
		{
			if( *lpwSrc != 0xffdf )
    			*lpwDest = *lpwSrc;
			lpwDest++;
			lpwSrc++;			
		}
		lpwLine += nScanWords;
		lpwDestLine += 240;
	}
	
	for( ; i < nSkipHeight - 27; i++ )
	{	
		LPWORD lpwSrc = lpwLine;
		LPWORD lpwDest = lpwDestLine;
		for( j = 0; j < nSkipWidth; j++ )
		{
			*lpwDest++ = *lpwSrc++;
		}
		lpwLine += nScanWords;
		lpwDestLine += 240;
	}

	for( ; i < nSkipHeight; i++ )
	{	
		LPWORD lpwSrc = lpwLine;
		LPWORD lpwDest = lpwDestLine;
		for( j = 0; j < nSkipWidth; j++ )
		{
			if( *lpwSrc != 0xffdf )
    			*lpwDest = *lpwSrc;
			lpwDest++;
			lpwSrc++;			
		}
		lpwLine += nScanWords;
		lpwDestLine += 240;
	}

}
*/

static int DoDestroy( void )
{
	return 1;
}

static int DoCreate( void )
{
    return 1;
}

static int DoOpen( void )
{
	InitDisplay();
	
//	ShowSkip();
	Sleep( 3000 );
	return TRUE;
}

DWORD CALLBACK GWDI_DisplayEnter( UINT msg, DWORD dwParam, LPVOID lParam )
{
    switch( msg )
    {
    case GWDI_CREATE:   //初始化硬件
        return DoCreate();
    case GWDI_OPEN:
    	return DoOpen();
    case GWDI_DESTROY:   //关掉显示设备
        return DoDestroy();
    case GWDI_GET_MODE_NUM:
        return 1;  //返回有多少种显示模式
    //case GWDI_POWEROFF:
        // 开关机处理
      //  return DoPowerOff();
    //case GWDI_POWERON:
    // 开关机处理
      //  return DoPowerOn();
    //case GWDI_GETBITMAP:
      //  *((_LPBITMAPDATA)lParam) = *((_LPBITMAPDATA)&oemDisplayBitmap);
        //return 1;
	case GWDI_GET_DISPLAY_PTR:
		*((_LPDISPLAYDRV*)lParam) = (_LPDISPLAYDRV)&oemDisplay16BPP;
		return 1;
    case GWDI_GET_BITMAP_PTR:
        *((_LPBITMAPDATA*)lParam) = (_LPBITMAPDATA)&oemDisplayBitmap;
        return 1;
    }
    return 0;
}

static COLORREF _PutPixel( _LPPIXELDATA lpPixelData )
{
	return _drvDisplay16BPP.lpPutPixel( lpPixelData );
}

static COLORREF _GetPixel( _LPPIXELDATA lpPixelData )
{
	return _drvDisplay16BPP.lpGetPixel( lpPixelData );
}

static BOOL _Line( _LPLINEDATA lpData )
{
	return _drvDisplay16BPP.lpLine( lpData );
}

static BOOL _BlkBitTransparentBlt( _LPBLKBITBLT lpData )
{
	return _drvDisplay16BPP.lpBlkBitTransparentBlt( lpData );
}

static BOOL _BlkBitMaskBlt( _LPBLKBITBLT lpData )
{
	return _drvDisplay16BPP.lpBlkBitMaskBlt( lpData );
}

static BOOL _BlkBitBlt( _LPBLKBITBLT lpData )
{
	return _drvDisplay16BPP.lpBlkBitBlt( lpData );
}

static COLORREF _RealizeColor( COLORREF color, LPCDWORD lpdwPalEntry, UINT uPalNumber, UINT uPalFormat )
{
/*	
	DWORD r;
    DWORD g;
    DWORD b;
*/
    //  8b 8g 8r format to 5b 6g 5r
    //r = ((color << 24) >> 27) & 0x001f;
    //g = ((color << 16) >> 21) & 0x07e0;
    //b = ((color << 8) >> 16) & 0xF800;

    //2003-09-06-DEL-Begin
	//  8b 8g 8r format to 5r 6g 5b
    //r = ((color << 24) >> 16) & 0xF800;
    //g = ((color << 16) >> 21) & 0x07e0;
    //b = ((color << 8) >> 27) & 0x001f;
    //return (b | g | r);
	//2003-09-06-DEL-end
/*
	r = ( (color & 0xff) * 0x1f / 0xff ) << 11;	
	g = ( ( (color >> 8 ) & 0xff) * 0x3f / 0xff ) << 5;
	b = ( ( (color >> 16 ) & 0xff) * 0x1f / 0xff );
	
	return r | g | b;
*/
    return _drvDisplay16BPP.lpRealizeColor( color, lpdwPalEntry, uPalNumber, uPalFormat);
        

}

static COLORREF _UnrealizeColor( COLORREF color, LPCDWORD lpdwPalEntry, UINT uPalNumber, UINT uPalFormat )
{
/*	
    DWORD r;
    DWORD g;
    DWORD b;
*/
	//  5b 6g 5r format to  8b 8g 8r
    //r = color & 0x001f;
	//g = color & 0x07e0;
	//b = color & 0xf800;

	//r = ((r << 27) >> 24);
    //g = ((g << 21) >> 16);
    //b = ((b << 16) >> 8);

	//2003-09-06-DEL-Begin
	//  5r 6g 5b format to  8b 8g 8r
    //r = color & 0xf800;
	//g = color & 0x07e0;
	//b = color & 0x001f;

	//r = ((r << 16) >> 24);
    //g = ((g << 21) >> 16);
    //b = ((b << 27) >> 8);
	//return (b | g | r);
	//2003-09-06-DEL-end

/*    r = ( (color & 0xf800) >> 11) * 0xff / 0x1f;
	g = ( ( (color & 0x07e0) >> 5) * 0xff / 0x3f ) << 8;
	b = ( ( (color & 0x001f)) * 0xff / 0x1f ) << 16;

	return r | g | b;
*/
    return _drvDisplay16BPP.lpUnrealizeColor( color, lpdwPalEntry, uPalNumber, uPalFormat );    

    
}
