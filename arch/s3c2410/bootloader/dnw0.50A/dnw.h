#ifndef __DNW_H__
#define __DNW_H__

void MenuConnect(HWND hwnd);
void MenuTransmit(HWND hwnd);
void MenuOptions(HWND hwnd);
void MenuAbout(HWND hwnd);
void Quit(HWND hwnd);

void UpdateWindowTitle(void);

BOOL OpenComPort(int port);
void CloseComPort(void);

void DoRxTx(void *args);
int ReadCommBlock(char *buf,int maxLen);
void TxFile(void *args);
void WriteCommBlock(char c);



#define APPNAME TEXT("DNW v0.50A")
#define EDIT_SCREEN_X (8*80)  //640
#define EDIT_SCREEN_Y (16*24) //384

#define SCREEN_X    (EDIT_SCREEN_X+2+16+2) //border(1+1)+scroll_bar(16)+space(1+1)
#define SCREEN_Y    (EDIT_SCREEN_Y+2+16+2) //border(1+1)+scroll_bar(16)+space(1+1)
/*
#define WINDOW_XSIZE (SCREEN_X+6)
#define WINDOW_YSIZE (SCREEN_Y+6+38) 
*/
#define WINDOW_XSIZE (SCREEN_X+8)
#define WINDOW_YSIZE (SCREEN_Y+8+38) 

#endif //__DNW_H__