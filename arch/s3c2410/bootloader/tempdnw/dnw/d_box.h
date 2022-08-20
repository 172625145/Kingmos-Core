#ifndef __DNSTATUS_H__
#define __DNSTATUS_H__

BOOL CALLBACK DownloadProgressProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void InitDownloadProgress(void);
void DisplayDownloadProgress(int percent);
void CloseDownloadProgress(void);

int ReadUserConfig(void);
int WriteUserConfig(void);

BOOL CALLBACK OptionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern int downloadCanceled;

extern volatile HWND _hDlgDownloadProgress;
#endif //__DNSTATUS_H__