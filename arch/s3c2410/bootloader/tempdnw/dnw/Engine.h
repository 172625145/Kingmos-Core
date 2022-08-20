// ================================================
// Name: engine.h
// Programmer: Inwook,Kong
// Description: Header file for engine.cpp
// ================================================

#ifndef __ENGINE_H__
#define __ENGINE_H__

// Macros
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

// The Engine Class
BOOL Engine_OnCreate(HWND hwnd,
                      CREATESTRUCT FAR* lpCreateStruct);
void Engine_OnCommand(HWND hwnd, int id,
                       HWND hwndCtl, UINT codeNotify);
void Engine_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void Engine_OnPaint(HWND hwnd);
void Engine_OnChar(HWND hwnd,WPARAM wParam);

// Variables
// .....
extern HINSTANCE _hInstance;
extern HWND _hwnd;

// Some Procs
BOOL Register(HINSTANCE hInstance);
BOOL Create(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWindow,
          UINT Message, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT Message,
//                           WPARAM wParam, LPARAM lParam);

#define CONSOLE_XSIZE	(80)
#define CONSOLE_YSIZE	(24)
#define CHAR_HEIGHT	(16)

extern "C" void EB_Printf(TCHAR *fmt,...);
void EB_Update(void);
//void _ScrBufScroll(void);
//void _ScrCrLf(void);
//void ScrClear(HWND hwnd);

#endif //__ENGINE_H__