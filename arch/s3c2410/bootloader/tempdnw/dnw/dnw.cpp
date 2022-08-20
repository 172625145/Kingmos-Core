#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <string.h>
#include <process.h>
#include <stdlib.h>

#include "resource.h"

#include "def.h"
#include "dnw.h"
#include "engine.h"
#include "fileopen.h"
#include "d_box.h"
#include "usbtxrx.h"

//NOTE: _beginthread() 
// To use _beginenthread(),Activate "Project/Settings/C/C++/Categry/Code generation/
// Use Run-Time library/Multithreaded or Debug Multithreaded"

// If DDK2000 is used,
// 1. Project_Settings/CC++/Preprocessor/Additional_include_directories = C:\NTDDK\inc
// 2. Project_Settings/Link/Input/Additional_library_path=c:\NTDDK\libfre\i386

// If DDKXP is used,
// 1. Project_Settings/CC++/Preprocessor/Additional include directories = C:\WINDDK\2600\inc\w2k
// 2-1. Project_Settings/Link/Input/Additional_library_path=C:\WINDDK\2600\lib\w2k\i386
//      This causes some warning about '.rdata' section attribute.
// 2-2. Project_Settings/Link/Input/Additional_library_path==C:\WINDDK\2600\lib\i386\free
//      There's no warning.

/*
===================== REVISON HISTORY =====================
1. 2000. 3.30: V0.01 
  First release of DNW
2. 2000.10.11: V0.2
  The edit control is used for scroll, copy&paste, smooth screen update
3. 2001.1.26: V0.3
  a) The CPU usage will be less. Sleep() is inserted during TX.
  b) The filesize and checksum are transmitted together with bin file.
  c) WriteCommBlock() bug is fixed. The txEmpty flag should be changed in only DoRxTx().
4. 2001.2.24: V0.31
  a) The size of edit buffer is changed by EM_LIMITTEXT message.
     EDIT_BUF_SIZE(30000) -> MAX_EDIT_BUF_SIZE(65000)
  b) If the edit box is greater than 50000,
     the size of edit box is reduced by 10000.
  c) The horizontal scroll bar is removed for better look. 
  d) In WaitCommEvent() loop, 
     the following condition is inserted to clear the overrun condition.
    	if((dwEvtMask & EV_ERR){...}
  e) EB_Printf() have some error to process large string data.
5. 2001.3.8: V0.32
  a) EDIT_BUF_SIZE is reduced 25000 because the EM_REPLACESEL message is done very slowly
     if the size is over about 30000.
6. 2001.4.11: V0.32A
  a) Experimentally, MAX_EDIT_BUF_SIZE is set to the default value(32767). 
     //SendMessage(_hwndEdit, EM_SETLIMITTEXT, MAX_EDIT_BUF_SIZE, 0L);
     RESULT: MAX_EDIT_BUF_SIZE doesn't affect the display delay problem.
             I think that the new method for deleting the contents should be applied
	     Let's do tonight
7. 2001.5.14: V0.34
   a) I have known that the edit control isn't adequate for console program.
      So, I would give up the development of DNW using the edit control
      The last decision is as follows;
      MAX_EDIT_BUF_SIZE (65000) //up to 65535
      EDIT_BUF_SIZE (30000)   
      EDIT_BUF_DEC_SIZE (10000)
   b) If the selected text is deleted, the edit control displays the first of the text.
      In this case, to show the end of the text, dummy REPLACE_SEL is added.

8. 2001.11.23: V0.4
   a) USB download function is added.
   b) GetOverlappedResult() is used in TxFile() in order to save the cpu time more efficiently 
   c) Serial Configuration dialog box
   d) In secbulk.sys is changed to support IRP_MN_QUERY_CAPABILITIES.
      So, the surpriseRemoval is allowed. When the USB is yanked impolitely, 
      the warning dialog box won't appear in WIN2000.

9. 2001.11.24: v0.41alpha
   a) WriteFile() supports overlapped I/O to check broken pipe.
   b) progress bar is added for transmit operation.
   c) USB,serial status is printed on the window title bar
   
10. 2001.12.5: v0.42a
   a) In secbulk.sys, the maximum number of bulk packit in 1ms frame duration is changed to 16 from 4.
      So, transfer rate is increased from 220KB/S to 405KB/S 
   b) Although the fileopen was failed(or canceled), the transmit wasn't canceled. This is fixed.      
   c) When the options menu is selected, a serial port will be reconnected.
   d) The receive test menu is added. 

11. 2001.12.6: v0.43
   a) Fou USB tx operation, TX_SIZE is increased from 2KB to 16KB.
      So, transfer rate is increased from 405KB/S to 490KB/S.
      (2KB:405KB/S 4KB:450KB/S  8KB:470KB/S 16KB:490KB/S)

12. 2001.12.7: v0.44b
   a) Although a serial port is not connected, the serial port is connected 
      after Configuration/Options menu -> fixed.
   b) The name dnw.cfg for v 0.4x is changed to dnw.ini 
      in order not to confuse old dnw.cfg for ver 0.3x
      
12. 2002.01.2: v0.44c
   a) The edit box size is changed to display 80x25 characters 
      
13. 2002.02.22: v0.45
   a) In windows95, DNW doesn't search USB stack.
   b) When download, there should be a cancel button
   c) Sometimes, the progress bar is not filled although download is doing.
      I think it's because InitDownloadProgress() is executed 
      before than DownloadProgressProc().
      So, I inserted the code to wait until DownloadProgressProc() is executed as follows;
	  while(_hDlgDownloadProgress==0); //wait until the progress dialog box is ready.
   d) secbulk is optimized 
14. 2002.04.01: v0.46
   a) If DNW is start to transmit although the b/d is not ready, DNW will be hung.
      It's only solution to turn off and on the b/d.
      To solve this problem smoothly, the overlapped I/O will be used for USB transfer. 
      But, secbulk.sys may not support overlapped I/O. 
      Although I implemented the overlapped I/O, WriteFile() function didn't return before its completion.
   b) Because the overlapped I/O doens't work as my wish,
      and in order to quit the dnw.exe hung, modaless dialogue box is used for the progress bar.
   c) *.nb0

  15. 2002.04.10: v0.47
   a) I reduce the edit box size as follows;  
	#define MAX_EDIT_BUF_SIZE (0x7FFE) 
	#define EDIT_BUF_SIZE (0x6000)   
	#define EDIT_BUF_DEC_SIZE (0x1000)
      There is no good cause about why I change. I want to just reduce the edit box size.
   b) Sometimes, when transmit, there is no transmit although the transmit progrss dialog box is shown.
      I think that it's because the _enthread() fails.
      To debug this problem, the _enthread result is checked.

  16. 2002.04.19: v0.48
   a) The bug of WIN2K WINAPI is found. ->It's not fixed perfectly.
      Please let me know of the solution to avoid the memory problem of SetWindowText() API.

      The SetWindowText()(also,WM_SETTEXT message) consumes 4KB system memory every times.
      I think there is some bug in SetWindowText API.
      In my case, because SetWindowText() is called every 1 seconds, 
      the system memory used by DNW.exe is increased by 4KB every 1 seconds.
      For emergency fix, SetWindowText() will be called only when its content is changed.
      
      NOTE. This memory problem is not memory leakage problem 
            because the memory is flushed when the window is shrinked.

  17. 2002.05.03:v0.49
   a) Sometimes, when transmit, there is no transmit although the transmit progrss dialog box is shown.
      I have found the cause.
      It's because _hDlgDownloadProgress.
      If the TxFile thread is executed first than WM_INITDIALOG message,
      while(_hDlgDownloadProgress==NULL); will not exited because _hDlgDownloadProgress 
      in CPU register will be checked. So, the volatile should have been used 
      because _hDlgDownloadProgress value is changed in another thread.
      The solution is as follows;
      volatile HWND _hDlgDownloadProgress=NULL;
   b) Sometimes, the CPU usage will be always 100% if dnw.exe is being executed.
      This is because of the just above problem.
      If the problem 17-a) is occurred, the TxFile will be an obsolete thread.
      while(_hDlgDownloadProgress==NULL); will use the CPU at 100%.
      I think that this problem may be cleared because the problem 17-a) is cleared.
   c) The small icon,DNW is displayed in the window shell task bar and the window title.

  18. 2003.04.25:v0.50
   a) In case that a USB2Serial cable is used when COMPAQ Presario 1700, 
      a few last character display is postponed until the next group is received.
      -> MSDN recommends the while() for read operation, which has solved the problem.
   b) In DoRxTx(), the EV_ERR is added to SetCommMask(); -> which is more correct, I think.
   c) For overlapped WaitCommEvent,ReadFile,WriteFile, the return value should be checked.
      For ReadFile,WriteFile cases, the operation has no change.
      In previous version(V0.49), WaitCommEvent(,,NULL) should have not been used because OVERLAPPED I/O was being used.
      The combination of WaitCommEvent(,,&os) and GetOverlappedResult() will fix this problem.
   d) During developing the DNW v0.50, I had experienced some blue screen without any USB connection when I reset the SMDK2410.
      This problem was occurred when WaitCommEvent(,,&os) without GetOverlappedResult().
      This problem was not revived when is WaitCommEvent(,,NULL) although it's not correct. 
      
      From this phenomenon, I assume that the blue-screen, which is caused sometimes when resetting the SMDK2410, 
      may be caused by the mis-use of WaitCommEvent(,,NULL).

      But, it's still truth that the abnormal USBD response may cause the windows blue-screen
      ,which I experienced many times during my secbulk.sys development.
   
  19. 2003.04.29:v0.50A
   a) If the break condition is detected, "[DBG:EV_ERR]" message is displayed.
      So, the display of "[DBG:EV_ERR]" is removed.
*/

/*   
Items to be enhanced.
   - avoid SetWindowText() API problem.
   - remove debug stuff in following functions.
         void InitDownloadProgress(void);   
	 void DisplayDownloadProgress(int percent);
   - make status bar.
   - malloc() uses too much memory for large file transfer.
   - Enlarge the scroll buffer -> the edit box is not adequate.
   - file logging function
   - Ctrl+C should be work as copy function.
*/ 

/*
Edit Box Note:
- Check if the edit box is scrolled to show the text being deleted
  when the selected text is deleted in Win2000. In windows98, It's scrolled to show the deleted portion unfortunately.
*/

#define IS_OVERLAPPED_IO    (TRUE)

int userBaudRate,idBaudRate;
int userComPort;

#define MAX_BLOCK_SIZE (4096)

HANDLE idComDev;
OVERLAPPED osWrite,osRead;
volatile int isConnected=FALSE;
TCHAR rxBuf[MAX_BLOCK_SIZE+1];
volatile char *txBuf;
volatile DWORD iTxBuf;
DWORD txBufSize;

volatile int txEmpty=TRUE;



void MenuAbout(HWND hwnd)
{
    MessageBox(hwnd,TEXT("Serial Console with Serial/USB Download\n\n")
		    TEXT("1. e-mail: purnnamu@sec.samsung.com\n")
	  	    TEXT("2. USB Tx format: addr(4)+size(4)+data(n)+cs(2)   \n")
		    TEXT("3. Serial Tx format: size(4)+data(n)+cs(2)\n"),
	   	    TEXT("About ")APPNAME,MB_OK | MB_ICONINFORMATION );
}





void Quit(HWND hwnd)
{
    CloseComPort();
}



void MenuConnect(HWND hwnd)
{
    if(isConnected==TRUE)
    {
	CloseComPort();
    }

    OpenComPort(userComPort);
}



void UpdateWindowTitle(void)
{
    TCHAR title[256];
    TCHAR tch[16];
    
    static int prevComPort=0xff;
    static int prevBaudRate=0xff;
    static int prevUsbAlive=0xff;
    static int prevIsConnected=0xff;
    int usbAlive;

    lstrcpy(title,APPNAME);
    lstrcat(title,TEXT("   [COM"));

    if(isConnected==TRUE) //serial O.K.
    {
	tch[0]='0'+userComPort;
	tch[1]='\0';
	lstrcat(title,tch);
	lstrcat(title,TEXT(","));
	lstrcat(title,_itot(userBaudRate,tch,10) );   //_itot :TCHAR version of itoa 
	lstrcat(title,TEXT("bps]"));
    }
    else
    {
	lstrcat(title,TEXT(":x]"));
    }

    
    if(IsUsbConnected())
    {
	lstrcat(title,TEXT("[USB:OK]"));
	usbAlive=1;
    }
    else
    {
	lstrcat(title,TEXT("[USB:x]"));
	usbAlive=0;
    }

    //The bug of WIN2K WINAPI is found.
    //The SetWindowText()(also,WM_SETTEXT message) consumes 4KB system memory every times.
    //I think there is some bug in SetWindowText API.
    //In my case, because SetWindowText() is called every 1 seconds, 
    //the system memory used by DNW.exe is increased by 4KB every 1 seconds.
    //For emergency fix, SetWindowText will called only when its content is changed.
    //NOTE. This memory leakage is flushed when window is shrinked.

    if(userComPort!=prevComPort || userBaudRate!=prevBaudRate || usbAlive!=prevUsbAlive ||
       isConnected!=prevIsConnected )
    {
	SetWindowText(_hwnd,title);
    }
    prevComPort=userComPort;
    prevBaudRate=userBaudRate;
    prevUsbAlive=usbAlive;
    prevIsConnected=isConnected;
}



void MenuTransmit(HWND hwnd)
{
    HANDLE hFile;
    DWORD fileSize;
    unsigned short cs=0;
    DWORD i;
    BOOL result;
    unsigned long threadResult;

    if(!isConnected)
    {
	EB_Printf(TEXT("[ERROR:Not Connected]\n"));
	return;
    }
    result=PopFileOpenDlg(hwnd,szFileName,szTitleName);
    
    if(result==0) //file open fail
    {
	return;
    }

    hFile=CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

    if(hFile==INVALID_HANDLE_VALUE)
    {
	EB_Printf(TEXT("[ERROR:File Open]\n") );
	return;
    }
    
    fileSize=GetFileSize(hFile,NULL);

    txBuf=(char *)malloc(fileSize+6);
    if(txBuf==0)
    {
	EB_Printf(TEXT("[ERROR:Memory Allocation Fail:(%d)]\n"),fileSize+6 );
	return;
    }

    ReadFile(hFile,(void *)(txBuf+4),fileSize,&txBufSize,NULL);
    if(txBufSize!=fileSize)    
    {
    	EB_Printf(TEXT("[ERROR:File Size Error]\n") );
	return;
    }

    *((DWORD *)txBuf)=fileSize+6;   //attach filesize(n+6) 

    for(i=4;i<(fileSize+4);i++)
	cs+=(BYTE)(txBuf[i]);
    *((WORD *)(txBuf+4+fileSize))=cs;   //attach checksum 
    
    txBufSize+=6;
    iTxBuf=0;

    CloseHandle(hFile);

    //EB_Printf(TEXT("cs=%x\n"),cs);
    threadResult=_beginthread( (void (*)(void *))TxFile,0x2000,(void *)0);
    if(threadResult!=-1)
    {
	//Create the download progress dialogbox.
	CreateDialog(_hInstance,MAKEINTRESOURCE(IDD_DIALOG1),hwnd,DownloadProgressProc); //modaless
	//ShowWindow(_hDlgDownloadProgress,SW_SHOW); 
    	//isn't needed because the dialog box already has WS_VISIBLE attribute.
    }
    else
    {
	EB_Printf(TEXT("[ERROR:Can't creat a thread. Memory is not sufficient]\n"));
    }
    //The dialog box will be closed at the end of TxFile().
    //free(txBuf) & CloseHandle(hWrite) will be done in TxFile()
}



void MenuOptions(HWND hwnd)
{
    int result;
    //Create the download progress dialogbox.
    result=DialogBox(_hInstance,MAKEINTRESOURCE(IDD_DIALOG2),_hwnd,OptionsProc); //modal
    if(result==1 && isConnected==1)
    {
	MenuConnect(hwnd); //reconfig the serial port.
    }
}





/*****************************
 *                           *
 * serial communication part * 
 *                           *
 *****************************/

BOOL OpenComPort(int port)
//port=1,2,3,4
{
    TCHAR *textCom[]={TEXT("COM1"),TEXT("COM2"),TEXT("COM3"),TEXT("COM4")};
    DCB dcb;
    COMMTIMEOUTS commTimeOuts;
    
//====================================================
    osRead.Offset=0;
    osRead.OffsetHigh=0;
    osWrite.Offset=0;
    osWrite.OffsetHigh=0;

    osRead.hEvent = CreateEvent(NULL,TRUE/*bManualReset*/,FALSE,NULL);
     //manual reset event object should be used. 
     //So, system can make the event objecte nonsignalled.
     //osRead.hEvent & osWrite.hEvent may be used to check the completion of 
     // WriteFile() & ReadFile(). But, the DNW doesn't use this feature.
    if(osRead.hEvent==NULL)
    {
	EB_Printf(TEXT("[ERROR:CreateEvent for osRead.]\n"));
	return FALSE;
    }

    osWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(osWrite.hEvent==NULL)
    {
	EB_Printf(TEXT("[ERROR:CreateEvent for osWrite.]\n"));
	return FALSE;
    }

    
//====================================================
    idComDev=CreateFile(textCom[port-1],
			GENERIC_READ|GENERIC_WRITE,
			0, //exclusive access
			/*FILE_SHARE_READ|FILE_SHARE_WRITE,*/ 
			NULL,	
			OPEN_EXISTING,
		    #if IS_OVERLAPPED_IO
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		    #else
			/*FILE_ATTRIBUTE_NORMAL,*/ 0,
		    #endif	
			NULL);

    if(idComDev==INVALID_HANDLE_VALUE)
    {
	//EB_Printf(TEXT("[ERROR:CreateFile for opening COM port.]\n") );
	return FALSE;
    }

    SetCommMask(idComDev,EV_RXCHAR);
    SetupComm(idComDev,4096,4096);
    PurgeComm(idComDev,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
    
    commTimeOuts.ReadIntervalTimeout=0xffffffff;
    commTimeOuts.ReadTotalTimeoutMultiplier=0;
    commTimeOuts.ReadTotalTimeoutConstant=1000;
    commTimeOuts.WriteTotalTimeoutMultiplier=0;
    commTimeOuts.WriteTotalTimeoutConstant=1000;
    SetCommTimeouts(idComDev,&commTimeOuts);

//====================================================
    dcb.DCBlength=sizeof(DCB);
    GetCommState(idComDev,&dcb);

    dcb.fBinary=TRUE;
    dcb.fParity=FALSE;
    dcb.BaudRate=userBaudRate; //CBR_115200;
    dcb.ByteSize=8;
    dcb.Parity=0;
    dcb.StopBits=0;
    dcb.fDtrControl=DTR_CONTROL_DISABLE;
    dcb.fRtsControl=RTS_CONTROL_DISABLE;
    dcb.fOutxCtsFlow=0;
    dcb.fOutxDsrFlow=0;
    
    if(SetCommState(idComDev,&dcb)==TRUE)
    {	
	isConnected=TRUE;
	_beginthread( (void (*)(void *))DoRxTx,0x2000,(void *)0);
	return TRUE;
    }
    else
    {
	isConnected=FALSE;
	CloseHandle(idComDev);
	return FALSE;
    }
}




void CloseComPort(void)
{
    if(isConnected)
    {
	isConnected=FALSE;
	SetCommMask(idComDev,0);
	    //disable event notification and wait for thread to halt
	EscapeCommFunction(idComDev,CLRDTR);
	PurgeComm(idComDev,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	CloseHandle(idComDev);

        CloseHandle(osRead.hEvent);
	CloseHandle(osWrite.hEvent);
    }
    
    Sleep(100); 
    //wait until CloseComPort() effective.
    //If not, OpenComPort()::CreateFile(...) will fail.

}
	



void DoRxTx(void *args)
{
    OVERLAPPED os;
    DWORD dwEvtMask;
    int nLength;
    BOOL fStat;
    DWORD temp;

    memset(&os,0,sizeof(OVERLAPPED));
    os.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(os.hEvent==NULL)
    {
	EB_Printf(TEXT("[ERROR:DoRxTx os.hEvent]\n"));
	_endthread();
    }

    if(!SetCommMask(idComDev,EV_RXCHAR|EV_TXEMPTY|EV_ERR))
    {
	EB_Printf(TEXT("[ERROR:SetCommMask()]\n"));
	CloseHandle(os.hEvent);
	_endthread();
    }

    while(isConnected)
    {
	dwEvtMask=0;
    #if IS_OVERLAPPED_IO
	fStat=WaitCommEvent(idComDev,&dwEvtMask,&os);   
	//Apr.28.2003: fStat should be checked for the cpu time saving. 
	if(!fStat)   //Apr.28.2003:GetOverlappedResult() is needed.  
	{
	    //By experiment. Only when there was no signalled event, the following was executed.
	    //EB_Printf(TEXT("\n[WaitCommEvent=false]\n") ); 
	    if(GetLastError()==ERROR_IO_PENDING)
	    {
		GetOverlappedResult(idComDev,&os,&temp,TRUE); 
	    }
	    else
	    {
		EB_Printf(TEXT("[RXTX_THREAD_ERR]") );
	    }
	}
    #else
	WaitCommEvent(idComDev,&dwEvtMask,NULL);  //wait until any event is occurred.
    #endif

	if( (dwEvtMask & EV_TXEMPTY) == EV_TXEMPTY )
	    txEmpty=TRUE;

	if((dwEvtMask & EV_RXCHAR) == EV_RXCHAR)
	{
	    do   //Apr.28.2003:The caveat on MSDN,"Serial Communications in Win32" recommends while();
	    {
		if( nLength=ReadCommBlock(rxBuf,MAX_BLOCK_SIZE) )
		{
		    if(nLength>=MAX_BLOCK_SIZE /*for debug*/)
		    {
			EB_Printf(TEXT("\n\n\n\n\n\n\n[ERROR:nLength>4096]\n"));
		    }
		    else
		    {
		    rxBuf[nLength]='\0';
		    //OutputDebugString(rxBuf);
		    EB_Printf(rxBuf);
		    }
		}
	    }while(nLength);  
	}

	// Clear OVERRUN condition.
	// If OVERRUN error is occurred,the tx/rx will be locked.
	if(dwEvtMask & EV_ERR)
	{
	    COMSTAT comStat;
	    DWORD dwErrorFlags;
	    ClearCommError(idComDev,&dwErrorFlags,&comStat);
	    //EB_Printf(TEXT("[DBG:EV_ERR=%x]\n"),dwErrorFlags);
	}
    }	
    CloseHandle(os.hEvent);
    _endthread();
}




int ReadCommBlock(char *buf,int maxLen)
{
    BOOL fReadStat;
    COMSTAT comStat;
    DWORD dwErrorFlags;
    DWORD dwLength;

    ClearCommError(idComDev,&dwErrorFlags,&comStat);
    dwLength=min((DWORD)maxLen,comStat.cbInQue);
    if(dwLength>0)
    {
    #if IS_OVERLAPPED_IO
	fReadStat=ReadFile(idComDev,buf,dwLength,&dwLength,&osRead);
	if(!fReadStat)   //Apr.28.2003:GetOverlappedResult() may be needed.
	{
	    //By experiment, fReadStat was always TRUE,of course, and the following was never executed.
	    //EB_Printf(TEXT("\n[RX_RD_WAIT]\n") ); 
	    if(GetLastError()==ERROR_IO_PENDING)
	    {
		GetOverlappedResult(idComDev,&osRead,&dwLength,TRUE); 
	    }
	    else
	    {
		EB_Printf(TEXT("[RXERR]") );
	    }
	}
    #else
	fReadStat=ReadFile(idComDev,buf,dwLength,&dwLength,NULL);
	if(!fReadStat)
	{
	    EB_Printf(TEXT("[RXERR]") );
	}

    #endif
    }
    return dwLength;
}





#define TX_SIZE 1024
//TX_SIZE must be less than 4096
void TxFile(void *args)
{
    void *txBlk;
    DWORD txBlkSize;
    DWORD fWriteStat;
    DWORD temp;
    InitDownloadProgress();
    while(txEmpty==FALSE);
    while(1)
    {
        if((txBufSize-iTxBuf) > TX_SIZE)
			txBlkSize=TX_SIZE;
		else
			txBlkSize=txBufSize-iTxBuf;
	
		txBlk=(void *)(txBuf+iTxBuf);

		txEmpty=FALSE;
#if IS_OVERLAPPED_IO

		fWriteStat=WriteFile(idComDev,txBlk,txBlkSize,&temp,&osWrite);
		if(!fWriteStat)   //Apr.28.2003:GetOverlappedResult() may be needed.
		{
	    
			if(GetLastError()==ERROR_IO_PENDING)
			{
				GetOverlappedResult(idComDev,&osWrite,&temp,TRUE); 
				//more efficient in order to save the cpu time
			}
			else
			{
				EB_Printf(TEXT("[TXERR]") );
			}
		}
		else
		{
			//By experiment, never executed.
			//EB_Printf(TEXT("\n[TX_NO_WR_WAIT]\n") ); 
		}
#else
		WriteFile(idComDev,txBlk,txBlkSize,&temp,NULL);
		Sleep(TX_SIZE*1000/11520-10); //to save cpu time.
		while(txEmpty==FALSE);
#endif
		iTxBuf+=TX_SIZE;
		
		DisplayDownloadProgress(iTxBuf*100/txBufSize);
		
		if(downloadCanceled==1)break;	//download is canceled by user.
		
		if(iTxBuf>=txBufSize)break;
    }
    free((void *)txBuf);
	
    CloseDownloadProgress();
	
    _endthread();
}




void WriteCommBlock(char c)
{
    void *txBlk;
    DWORD txBlkSize;
    static char _c;
    DWORD temp;

    _c=c;

    while(txEmpty==FALSE);
    txBlk=&_c;
    txBlkSize=1;

    //txEmpty=FALSE; why needed??? this line should be removed.
#if IS_OVERLAPPED_IO
    WriteFile(idComDev,txBlk,txBlkSize,&temp,&osWrite);
#else
    WriteFile(idComDev,txBlk,txBlkSize,&temp,NULL);
#endif
    while(txEmpty==FALSE);
}
    

