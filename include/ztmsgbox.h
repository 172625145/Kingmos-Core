/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

#ifndef __ZTMSGBOX_H
#define __ZTMSGBOX_H

#include <ewindows.h>

#define ZTMessageBox ZTDlg_MessageBox
int WINAPI ZTDlg_MessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType );

#endif //__ZTMSGBOX_H
