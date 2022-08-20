#include <windows.h>
#include <eversion.h>
#include <edevice.h>
#include "..\include\w32cfg.h"
#include <eobjtype.h>
//#include <edef.h>


typedef struct _DIRTYRECT
{
    LONG left;//short left;
	LONG top;//short top;
	LONG right;//short right;
	LONG bottom;//short bottom;
}_DIRTYRECT, *LPDIRTYRECT;

typedef struct _BITMAPDATA
{
    DWORD        objType;      // == OBJ_BITMAP
	DWORD       dwDumy1;
	DWORD       dwDumy2;
	DWORD       dwDumy3;
	DWORD       dwDumy4;

    //BYTE      bmType;
	//BYTE      bmDumy;
	WORD      bmFlags;//
	BYTE      bmPlanes;
	BYTE      bmBitsPixel;

    int       bmWidth;
    int       bmHeight;
    int       bmWidthBytes;
    //WORD        bmPlanes;
    //WORD        bmBitsPixel;
    LPBYTE    bmBits;
}_BITMAPDATA, * _LPBITMAPDATA;


_BITMAPDATA oemDisplayBitmap;// = { OBJ_BITMAP, 0, 160, 160, 160, 1, 8, 0 };

LPBYTE __lpwDisplayFrameBuf = 0;

HDC hDeskTopDC = 0;
HDC hMemDC = 0;
RECT rcDirty = { 0, 0, 0, 0 };
extern CRITICAL_SECTION csDisplay;


void DrawDirtyRect( _LPBITMAPDATA lpDestImage, void * lp )
{
	//extern HANDLE hFlushEvent;
    LPDIRTYRECT lprect = (LPDIRTYRECT)lp;
//    COLORREF color;
    RECT rc;
//    HBITMAP h;
    //static int f = 0;
   

    rc.left = lprect->left;
    rc.top = lprect->top;
    rc.right = lprect->right;
    rc.bottom = lprect->bottom;
    EnterCriticalSection(&csDisplay);
    UnionRect(&rcDirty,&rcDirty,&rc);
    LeaveCriticalSection(&csDisplay);
    //SetEvent( hFlushEvent );
    //return ;
/*

    h = CreateBitmap( iDisplayWidth, iDisplayHeight, iPlane, iBitsPerPel, __lpwDisplayFrameBuf );
    //f = 1 - f;
	h = (HBITMAP)SelectObject( hMemDC, h );
    color = SetBkColor( hDeskTopDC, CL_BKLIGHT );
    //SetBkMode( hDeskTopDC, OPAQUE );
	BitBlt( hDeskTopDC, lprect->left+iDisplayOffsetX, lprect->top+iDisplayOffsetY,
		    (lprect->right - lprect->left),
			(lprect->bottom - lprect->top ),
			hMemDC,
			lprect->left, lprect->top,
			SRCCOPY );
    //SetBkColor( hDeskTopDC, color );
    h = (HBITMAP)SelectObject( hMemDC, h );
	DeleteObject( h );
*/
}

BOOL DoDisplayerDestroy( void )
{
    DeleteDC( hMemDC );
	ReleaseDC( hwndDeskTop, hDeskTopDC );
	//if( __lpwDisplayFrameBuf )
	//	free( __lpwDisplayFrameBuf );
	return TRUE;
}


int DoDisplayerCreate( void )
{
// = { OBJ_BITMAP, 0, 160, 160, 20, 1, 1, (BYTE*)__wDisplayFrameBuf };
	memset( &oemDisplayBitmap, 0, sizeof( oemDisplayBitmap ) );
    oemDisplayBitmap.objType = OBJ_BITMAP;
    oemDisplayBitmap.bmFlags = 0;
    oemDisplayBitmap.bmWidth = iDisplayWidth;
    oemDisplayBitmap.bmHeight = iDisplayHeight;
    oemDisplayBitmap.bmPlanes = iPlane;
    oemDisplayBitmap.bmBitsPixel = iBitsPerPel;
    oemDisplayBitmap.bmWidthBytes = iDisplayWidth * iBitsPerPel / 8;
    oemDisplayBitmap.bmBits = malloc( oemDisplayBitmap.bmWidthBytes * oemDisplayBitmap.bmHeight );
	__lpwDisplayFrameBuf = oemDisplayBitmap.bmBits;

	if( hwndDeskTop && hDeskTopDC == 0 )
	{
		hDeskTopDC = GetDC( hwndDeskTop );
		if( hDeskTopDC )
		{
            hMemDC = CreateCompatibleDC( hDeskTopDC );
		}
	}
	return 1;
}

