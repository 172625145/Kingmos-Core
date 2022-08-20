#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <string.h>

#include "resource.h"

#include "def.h"
#include "dnw.h"
#include "engine.h"

static LOGFONT logFont;
static HFONT hFont;

void SetFont(HWND hwndEdit)
{
    //TEXTMETRIC tm;
    static struct
     {
          int     idStockFont ;
          TCHAR * szStockFont ;
     }
     stockfont [] = { OEM_FIXED_FONT,      TEXT ("OEM_FIXED_FONT"),
                      ANSI_FIXED_FONT,     TEXT ("ANSI_FIXED_FONT"),    
                      ANSI_VAR_FONT,       TEXT ("ANSI_VAR_FONT"),
                      SYSTEM_FONT,         TEXT ("SYSTEM_FONT"),
                      DEVICE_DEFAULT_FONT, TEXT ("DEVICE_DEFAULT_FONT"),
                      SYSTEM_FIXED_FONT,   TEXT ("SYSTEM_FIXED_FONT"),
                      DEFAULT_GUI_FONT,    TEXT ("DEFAULT_GUI_FONT") } ;

    GetObject(GetStockObject(stockfont[5].idStockFont),sizeof(LOGFONT),
	      (PTSTR)&logFont);
    hFont=CreateFontIndirect(&logFont);
    SendMessage(hwndEdit,WM_SETFONT,(WPARAM)hFont,0);
}


void ReleaseFont(void)
{
    DeleteObject(hFont);
}