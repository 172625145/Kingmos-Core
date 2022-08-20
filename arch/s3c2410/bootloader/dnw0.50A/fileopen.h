#ifndef __FILEOPEN_H__
#define __FILEOPEN_H__

void PopFileInitialize (HWND hwnd);
BOOL PopFileOpenDlg (HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);
void WriteCommBlock(char *lpByte, DWORD dwBytesWrite);

extern TCHAR szFileName[256],szTitleName[256];

#endif //__FILEOPEN_H__