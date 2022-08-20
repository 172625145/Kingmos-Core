#include <eframe.h>
#include <gwmeobj.h>
#include <edevice.h>

extern _BITMAPDATA oemDisplayBitmap;

extern void DrawDirtyRect( _LPBITMAPDATA lpDestImage, void * lp );

static COLORREF _WinPutPixel( _LPPIXELDATA lpPixelData );
static COLORREF _WinGetPixel( _LPPIXELDATA lpPixelData );
static BOOL _WinLine( _LPLINEDATA lpLineData );
//static BOOL _WinVertialLine( _LPLINEDATA lpLineData );
//static BOOL _WinScanLine( _LPLINEDATA lpData );
//static BOOL _WinTextBitBlt( _LPBLKBITBLT lpData );
static BOOL _WinBlkBitTransparentBlt( _LPBLKBITBLT lpData );
static BOOL _WinBlkBitMaskBlt( _LPBLKBITBLT lpData );
static BOOL _WinBlkBitBlt( _LPBLKBITBLT lpData );
static COLORREF _WinRealizeColor( COLORREF color, LPCDWORD lpcPal, UINT uPalNumber, UINT uPalFormat );
static COLORREF _WinUnrealizeColor( COLORREF color, LPCDWORD lpcPal, UINT uPalNumber, UINT uPalFormat );

#ifdef COLOR_1BPP
extern const _DISPLAYDRV _drvDisplay1BPP;
#define DEFAULT_DRV _drvDisplay1BPP
#endif

#ifdef COLOR_4BPP
extern const _DISPLAYDRV _drvDisplay4BPP;
#define DEFAULT_DRV _drvDisplay4BPP
#endif


#ifdef COLOR_8BPP
extern const _DISPLAYDRV _drvDisplay8BPP;
#define DEFAULT_DRV _drvDisplay8BPP
#endif

#ifdef COLOR_16BPP
extern const _DISPLAYDRV _drvDisplay16BPP;
#define DEFAULT_DRV _drvDisplay16BPP
#endif

const _DISPLAYDRV _oemDisplayDefault = {
    _WinPutPixel,
    _WinGetPixel,
  //  _WinVertialLine,
//    _WinScanLine,
    _WinLine,
    //_WinTextBitBlt,
    _WinBlkBitTransparentBlt,
    _WinBlkBitMaskBlt,
    _WinBlkBitBlt,
    _WinRealizeColor,
    _WinUnrealizeColor
};

extern _BITMAPDATA oemDisplayBitmap;
//const _DISPLAYDRV * _lpDrvDisplayDefault = &_drvDisplayDefault;

COLORREF _WinPutPixel( _LPPIXELDATA lpPixelData )
{
	_PIXELDATA pix;
	COLORREF col;

	if( lpPixelData->lpDestImage == &oemDisplayBitmap )
	{
		pix = *lpPixelData;
	    lpPixelData = &pix;
		lpPixelData->x += GetSystemMetrics( SM_XVIEW );
		lpPixelData->y += GetSystemMetrics( SM_YVIEW );
	}
	col = DEFAULT_DRV.lpPutPixel( lpPixelData );
    if( lpPixelData->lpDestImage == &oemDisplayBitmap )
	{
		    RECT rect;
			rect.left = lpPixelData->x;
			rect.right = lpPixelData->x + 1;
			rect.top = lpPixelData->y;
			rect.bottom = lpPixelData->y + 1;
			DrawDirtyRect( lpPixelData->lpDestImage, (void*)&rect );
	}
	return col;
}

COLORREF _WinGetPixel( _LPPIXELDATA lpPixelData )
{
	_PIXELDATA pix;
	
	if( lpPixelData->lpDestImage == &oemDisplayBitmap )
	{
 	    pix = *lpPixelData;
		lpPixelData = &pix;
		lpPixelData->x += GetSystemMetrics( SM_XVIEW );
		lpPixelData->y += GetSystemMetrics( SM_YVIEW );
	}

    return DEFAULT_DRV.lpGetPixel( lpPixelData );
}

BOOL _WinLine( _LPLINEDATA lpLineData )
{
	int ret;
    _LINEDATA ld;
	RECT rc;
	if( lpLineData->lpDestImage == &oemDisplayBitmap )
	{
 	    ld = *lpLineData;
		lpLineData = &ld;
		rc = *lpLineData->lprcClip;
		lpLineData->lprcClip = &rc;
		OffsetRect( &rc, GetSystemMetrics( SM_XVIEW ), GetSystemMetrics( SM_YVIEW ) );		
		lpLineData->xStart += GetSystemMetrics( SM_XVIEW );
		lpLineData->yStart += GetSystemMetrics( SM_YVIEW );
	}


	ret = DEFAULT_DRV.lpLine( lpLineData );
	if( ret )
	{
		if( lpLineData->lpDestImage == &oemDisplayBitmap )
		{
		    RECT bounds;

		int N_plus_1;			// Minor length of bounding rect + 1
		
		if( lpLineData->dn)	// The line has a diagonal component (we'll refresh the bounding rect)
			N_plus_1 = 2 + ((lpLineData->cPels * lpLineData->dm) / lpLineData->dm);
		else
			N_plus_1 = 1;
		
		switch(lpLineData->iDir) 
		{
		case 0:
			bounds.left = lpLineData->xStart;
			bounds.top = lpLineData->yStart;
			bounds.right = lpLineData->xStart + lpLineData->cPels + 1;
			bounds.bottom = bounds.top + N_plus_1;
			break;
		case 1:
			bounds.left = lpLineData->xStart;
			bounds.top = lpLineData->yStart;
			bounds.bottom = lpLineData->yStart + lpLineData->cPels + 1;
			bounds.right = bounds.left + N_plus_1;
			break;
		case 2:
			bounds.right = lpLineData->xStart + 1;
			bounds.top = lpLineData->yStart;
			bounds.bottom = lpLineData->yStart + lpLineData->cPels + 1;
			bounds.left = bounds.right - N_plus_1;
			break;
		case 3:
			bounds.right = lpLineData->xStart + 1;
			bounds.top = lpLineData->yStart;
			bounds.left = lpLineData->xStart - lpLineData->cPels;
			bounds.bottom = bounds.top + N_plus_1;
			break;
		case 4:
			bounds.right = lpLineData->xStart + 1;
			bounds.bottom = lpLineData->yStart + 1;
			bounds.left = lpLineData->xStart - lpLineData->cPels;
			bounds.top = bounds.bottom - N_plus_1;
			break;
		case 5:
			bounds.right = lpLineData->xStart + 1;
			bounds.bottom = lpLineData->yStart + 1;
			bounds.top = lpLineData->yStart - lpLineData->cPels;
			bounds.left = bounds.right - N_plus_1;
			break;
		case 6:
			bounds.left = lpLineData->xStart;
			bounds.bottom = lpLineData->yStart + 1;
			bounds.top = lpLineData->yStart - lpLineData->cPels;
			bounds.right = bounds.left + N_plus_1;
			break;
		case 7:
			bounds.left = lpLineData->xStart;
			bounds.bottom = lpLineData->yStart + 1;
			bounds.right = lpLineData->xStart + lpLineData->cPels + 1;
			bounds.top = bounds.bottom - N_plus_1;
			break;
		}

			InflateRect(&bounds, 1 + lpLineData->width, 1 + lpLineData->width ); 
			if( lpLineData->width > 1 )			
			    IntersectRect(&bounds, &bounds, lpLineData->lprcClip );
			if( bounds.left < bounds.right &&
				bounds.top < bounds.bottom )
			    DrawDirtyRect( lpLineData->lpDestImage, &bounds );
		}
	}
	return ret;
}

BOOL _WinBlkBitTransparentBlt( _LPBLKBITBLT lpData )
{
	_BLKBITBLT blk;

	if( lpData->lpDestImage == &oemDisplayBitmap )
	{
		blk = *lpData;
		lpData = &blk;		
		OffsetRect( (LPRECT)lpData->lprcDest, GetSystemMetrics( SM_XVIEW ), GetSystemMetrics( SM_YVIEW ) );
	}

    {
		int ret = DEFAULT_DRV.lpBlkBitTransparentBlt( lpData );
		if( ret )
		{
			if( lpData->lpDestImage == &oemDisplayBitmap )
			{
				DrawDirtyRect( lpData->lpDestImage, (void*)lpData->lprcDest );                
			}
		}
		if( lpData->lpDestImage == &oemDisplayBitmap )
		    OffsetRect( (LPRECT)lpData->lprcDest, -GetSystemMetrics( SM_XVIEW ), -GetSystemMetrics( SM_YVIEW ) );
		return ret;
	}
}

BOOL _WinBlkBitMaskBlt( _LPBLKBITBLT lpData )
{
	_BLKBITBLT blk;

	if( lpData->lpDestImage == &oemDisplayBitmap )
	{
		blk = *lpData;
		lpData = &blk;		
		OffsetRect( (LPRECT)lpData->lprcDest, GetSystemMetrics( SM_XVIEW ), GetSystemMetrics( SM_YVIEW ) );
	}

	{
		
		int ret = DEFAULT_DRV.lpBlkBitMaskBlt( lpData );
		if( ret )
		{
			if( lpData->lpDestImage == &oemDisplayBitmap )
				DrawDirtyRect( lpData->lpDestImage, (void*)lpData->lprcDest );
		}
		if( lpData->lpDestImage == &oemDisplayBitmap )
			OffsetRect( (LPRECT)lpData->lprcDest, -GetSystemMetrics( SM_XVIEW ), -GetSystemMetrics( SM_YVIEW ) );
		
		return ret;
		
	}
}


BOOL _WinBlkBitBlt( _LPBLKBITBLT lpData )
{
	_BLKBITBLT blk;

	if( lpData->lpDestImage == &oemDisplayBitmap )
	{
		blk = *lpData;
		lpData = &blk;		
		OffsetRect( (LPRECT)lpData->lprcDest, GetSystemMetrics( SM_XVIEW ), GetSystemMetrics( SM_YVIEW ) );
	}

	{
		int ret = DEFAULT_DRV.lpBlkBitBlt( lpData );
		
		if( ret )
		{
			if( lpData->lpDestImage == &oemDisplayBitmap )
				DrawDirtyRect( lpData->lpDestImage, (void*)lpData->lprcDest );
		}
		if( lpData->lpDestImage == &oemDisplayBitmap )
			OffsetRect( (LPRECT)lpData->lprcDest, -GetSystemMetrics( SM_XVIEW ), -GetSystemMetrics( SM_YVIEW ) );
		
		return ret;
	}
}

static COLORREF _WinRealizeColor( COLORREF color, LPCDWORD lpcPal, UINT uPalNumber, UINT uPalFormat )
{/*
    int like = 0;
	const BYTE realColor[] = { 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0 };

    BYTE r = (BYTE)color; 
    BYTE g = (BYTE)(color >> 8);
    BYTE b = (BYTE)(color >> 16);

    return realColor[ (BYTE) ( (r * 30l + g * 59l + b * 11l) / 100l ) >> 4 ];/// 16;
*/
    return DEFAULT_DRV.lpRealizeColor( color, lpcPal, uPalNumber, uPalFormat );
}

static COLORREF _WinUnrealizeColor( DWORD color, LPCDWORD lpcPal, UINT uPalNumber, UINT uPalFormat )
{/*
    static const PALETTEENTRY _rgbIdentity[16] =
	{
    	{ 0xff, 0xff, 0xff, 0 },   // 
	    { 0xee, 0xee, 0xee, 0 },  //
	    { 0xdd, 0xdd, 0xdd, 0 },  //
	    { 0xcc, 0xcc, 0xcc, 0 },  //

	    { 0xbb, 0xbb, 0xbb, 0 },   //
	    { 0xaa, 0xaa, 0xaa, 0 },  //
	    { 0x99, 0x99, 0x99, 0 },  //
	    { 0x88, 0x88, 0x88, 0 },  //

	    { 0x77, 0x77, 0x77, 0 },   //
	    { 0x66, 0x66, 0x66, 0 },  //
	    { 0x55, 0x55, 0x55, 0 },  //
	    { 0x44, 0x44, 0x44, 0 },  //

	    { 0x33, 0x33, 0x33, 0 },   //
	    { 0x22, 0x22, 0x22, 0 },  //
	    { 0x11, 0x11, 0x11, 0 },  //
	    { 0x00, 0x00, 0x00, 0 }  //
	};

    return *((COLORREF*)&_rgbIdentity[color&0xf]);
*/
	return DEFAULT_DRV.lpUnrealizeColor( color, lpcPal, uPalNumber, uPalFormat );
}


BOOL DoDisplayerCreate( void );
BOOL DoDisplayerDestroy( void );

DWORD CALLBACK GWDI_DisplayEnter( UINT msg, DWORD dwParam, LPVOID lParam )
{
    switch( msg )
    {
	//case GWDI_OPEN:
	//	return 1;
	//case GWDI_CLOSE
    //	return 1;
    case GWDI_CREATE:   //初始化硬件
        return DoDisplayerCreate();
    case GWDI_DESTROY:   //关掉显示设备
        return DoDisplayerDestroy();
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
		*((_LPDISPLAYDRV*)lParam) = (_LPDISPLAYDRV)&_oemDisplayDefault;
		return 1;
    case GWDI_GET_BITMAP_PTR:
        *((_LPBITMAPDATA*)lParam) = (_LPBITMAPDATA)&oemDisplayBitmap;
        return 1;
    }
    return 0;
}


