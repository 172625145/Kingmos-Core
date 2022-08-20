/**************************************************************************
* Copyright (c)  ΢�߼�(WEILUOJI). All rights reserved.                   *
**************************************************************************/

#ifndef __GDC_H
#define __GDC_H

#ifdef __cplusplus
extern "C" {
#endif      /* __cplusplus */

#ifndef __GWMEOBJ_H
#include <gwmeobj.h>
#endif

#define DCF_BEGIN_PAINT_ALLOC 0x80000000
#define DCF_GETDC_ALLOC       0x40000000

typedef struct __GDCDATA{
    OBJLIST obj;	
    //DWORD objType;
	//ULONG ulOwner;

    BYTE stretchMode;
    BYTE mapMode;
    BYTE rop;
    BYTE backMode;
    COLORREF backColor;
    COLORREF textColor;
    WORD textAlign;
	WORD arcMode;
    HPEN hPen;
	_PENATTRIB penAttrib;
    HBRUSH hBrush;
	_BRUSHATTRIB brushAttrib;
    HFONT hFont;
    POINT position;

    POINT viewportOrg;
    POINT windowOrg;
    POINT deviceOrg;
    SIZE  deviceExt;

    HWND hwnd;
	UINT uiState;
	UINT uiFlags;	
    HRGN hrgn;
//	HBITMAP hMemBitmap;// Create by CreateComptialDC, will remove future
    HBITMAP hBitmap;   // 
	_LPCDISPLAYDRV lpDispDrv;	//��������������λͼ��ʽ��
	LPDWORD lpdwPal;	//��ɫ������
	WORD    wPalNumber;	//��ɫ�����
	WORD    wPalFormat;	//��ɫ���ʽ
}_GDCDATA, * _PGDCDATA, FAR* _LPGDCDATA;
_LPGDCDATA _GetHDCPtr( HDC hdc );
_LPGDCDATA _GetSafeDrawPtr( HDC hdc );

HDC GetPaintDC( HWND hwnd, LPCRECT lprect, HRGN hrgnClip, UINT uiFlags );

void FreeInstanceIcon( HANDLE hOwnerProcess );
void FreeInstanceCursor( HANDLE hOwnerProcess );
BOOL DeleleGdiObject( HANDLE hOwnerProcess );
_LPCDISPLAYDRV GetDisplayDeviceDriver( _LPBITMAPDATA lpbmp );

_LPICONDATA _GetHICONPtr( HICON hIcon );
extern const _FONTDRV FAR * _lpSYSFontDrv;
extern const UINT _uSysFontID;
extern const UINT _uSysSymbolFontID;


//����λͼ����
HBITMAP _WinGdi_CreateBitmap( 
						int nWidth, 
						int nHeight, 
						UINT cPlanes,
						UINT cBitsPerPel,
						int  iDir,
						const VOID *lpvBits,									
						_LPBITMAPDATA * lppBitmap,
						UINT uiObjFlag,
						CONST BITMAPINFO *lpbmi,
						BOOL bIsShareResource
					);

HBITMAP _GetShareBitmapObj( HINSTANCE hInst, DWORD dwName );

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  // __GDC_H


