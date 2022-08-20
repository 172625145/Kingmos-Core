/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

#ifndef __ENLS_H
#define __ENLS_H

#ifdef __cplusplus
extern "C" {
#endif      // __cplusplus

// code page
#define CP_GB2312   2312
#define CP_UTF8     65001       // UTF-8 translation

#define MB_PRECOMPOSED            0x00000001  // use precomposed chars
#define MB_COMPOSITE              0x00000002  // use composite chars

#define MultiByteToWideChar CP_MultiByteToWideChar
int WINAPI CP_MultiByteToWideChar(
						UINT uiCodePage, 
						DWORD dwFlags, 
						LPCSTR lpMultiByteStr, 
						int cbMultiByte, 
						LPWSTR lpWideCharStr, 
						int cchWideChar 
						);

#define WideCharToMultiByte CP_WideCharToMultiByte
int WINAPI CP_WideCharToMultiByte(
						UINT uiCodePage, 
						DWORD dwFlags, 
						LPCWSTR lpWideCharStr, 
						int cchWideChar, 
						LPSTR lpMultiByteStr, 
						int cbMultiByte, 
						LPCSTR lpDefaultChar, 
						BOOL * lpUsedDefaultChar 
						);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif   // __ENLS_H
