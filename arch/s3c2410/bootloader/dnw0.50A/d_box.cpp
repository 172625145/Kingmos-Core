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
#include "d_box.h"
#include "dnw.h"
#include "engine.h"
#include "fileopen.h"

extern TCHAR szFileName[256];
extern HINSTANCE hInstance;

// 'volatile' is important because _hDlgDownloadProgress is accessed in two threads.
volatile HWND _hDlgDownloadProgress=NULL; 
HWND _hProgressControl;
volatile HWND _hDlgSerialSettings;

int downloadCanceled;

UINT hex2int(TCHAR *str);

BOOL CALLBACK DownloadProgressProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szTitle[256];
    switch(message)
    {
    case WM_INITDIALOG:
	lstrcpy(szTitle,TEXT("Downloading "));
	lstrcat(szTitle,szFileName);
	SetWindowText(hDlg,szTitle);
	_hDlgDownloadProgress=hDlg;
	return TRUE;
	
    case WM_CLOSE: 
	//If the [X] button is clicked, WM_CLOSE message is received.
	//Originally, DefWindowProc() called DestroyWindow();
	//But, there is no DefWindowProc(), We have to process WM_CLOSE message.
	DestroyWindow(hDlg);
	_hDlgDownloadProgress=NULL;
	downloadCanceled=1;
	//break;
	return TRUE; //why not?
    }

    return FALSE;
}

void InitDownloadProgress(void)
{
    while(_hDlgDownloadProgress==NULL); //wait until the progress dialog box is ready.

    _hProgressControl=GetDlgItem(_hDlgDownloadProgress,IDC_PROGRESS1);
    SendMessage(_hProgressControl,PBM_SETRANGE,0,MAKELPARAM(0, 100));
    downloadCanceled=0;
}

void DisplayDownloadProgress(int percent)
{
    SendMessage(_hProgressControl,PBM_SETPOS,(WPARAM)percent,0); 
}
    
void CloseDownloadProgress(void)
{
    if(_hDlgDownloadProgress!=NULL)
    {
    	//DestroyWindow(_hDlgDownloadProgress); 
	//Doesn't work because CloseDownloadProgress() is called another thread,
	//which is different the thread calling CreatDialog().
    
        SendMessage(_hDlgDownloadProgress,WM_CLOSE,0,0); 
    	_hDlgDownloadProgress=NULL;
    }
}


extern int idBaudRate,userBaudRate;
extern int userComPort;
extern DWORD downloadAddress;
extern TCHAR szDownloadAddress[100];

static int tempIdBaudRate,tempComPort;

BOOL CALLBACK OptionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    int baudTable[]={CBR_115200,CBR_57600,CBR_38400,CBR_19200,CBR_14400,CBR_9600};
    int tmp;
    switch(message)
    {
    case WM_INITDIALOG:
	_hDlgSerialSettings=hDlg;

	tempIdBaudRate=idBaudRate;
	tempComPort=userComPort;
	CheckRadioButton(hDlg,IDC_RADIO115200,IDC_RADIO9600,IDC_RADIO115200+tempIdBaudRate);
	CheckRadioButton(hDlg,IDC_RADIOCOM1,IDC_RADIOCOM4,IDC_RADIOCOM1+(tempComPort-1));

	SendMessage(GetDlgItem(hDlg,IDC_EDIT1),EM_REPLACESEL,0,(LPARAM)(LPCSTR)szDownloadAddress);  
    	return TRUE;

    case WM_COMMAND:
	switch(LOWORD(wParam))
	{
	case IDOK:
	    //get downloadAddress
	    *((WORD *)szDownloadAddress)=15;	
		//The first WORD should be max character number for EM_GETLINE!
	    tmp=SendMessage(GetDlgItem(hDlg,IDC_EDIT1),
		        EM_GETLINE,(WPARAM)0,(LPARAM)(LPCSTR)szDownloadAddress);
	    szDownloadAddress[tmp]='\0'; //EM_GETLINE string doesn't have '\0'.
	    downloadAddress=hex2int(szDownloadAddress);	     

	    EndDialog(hDlg,1);
	    idBaudRate=tempIdBaudRate;
	    userBaudRate=baudTable[idBaudRate];
	    userComPort=tempComPort;
	    WriteUserConfig();
	    return TRUE;
	case IDCANCEL:
	    EndDialog(hDlg,0);
	    return TRUE;

	case IDC_RADIOCOM1:
        case IDC_RADIOCOM2:
	case IDC_RADIOCOM3:
	case IDC_RADIOCOM4:
	    tempComPort=LOWORD(wParam)-IDC_RADIOCOM1+1;
	    return TRUE;

        case IDC_RADIO115200:
	case IDC_RADIO57600:
	case IDC_RADIO38400:
	case IDC_RADIO19200:
	case IDC_RADIO14400:
	case IDC_RADIO9600:
	    tempIdBaudRate=LOWORD(wParam)-IDC_RADIO115200;
	    return TRUE;
	}
	return FALSE;
    default:
	return FALSE;
    }
    return FALSE;
}


//ReadUserConfig() is called once at first
int ReadUserConfig(void)
{
    HANDLE hFile;
    DWORD fileSize;
    DWORD temp;
    TCHAR *buf;
    int baudTable[]={CBR_115200,CBR_57600,CBR_38400,CBR_19200,CBR_14400,CBR_9600};
    UINT i;

    hFile=CreateFile(TEXT("dnw.ini"),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    
    if(hFile==INVALID_HANDLE_VALUE)
    {
	//EB_Printf(TEXT("[ERROR:Can't open dnw.ini. So, the default configuration is used.]\n") );
	userComPort=1;
	idBaudRate=0;
	userBaudRate=baudTable[idBaudRate];
	downloadAddress=0xc000000;
	lstrcpy(szDownloadAddress,TEXT("0xc000000"));
	return TRUE;
    }
    
    fileSize=GetFileSize(hFile,NULL);
    buf=(char *)malloc(fileSize);
    
    ReadFile(hFile,buf,fileSize,&temp,NULL);
    
    userComPort=*((TCHAR *)buf+0)-'0';	    //UNICODE problem!!!
    idBaudRate=*((TCHAR *)buf+1)-'0';	    //UNICODE problem!!!
    userBaudRate=baudTable[idBaudRate];
 
    if(buf[2]!='\n')
    {
	for(i=0;i<fileSize;i++) //search first '\n'
	{
	    if(buf[i]=='\n')
	    {
		buf[i]='\0'; 
		break;
	    }
	}
	lstrcpy(szDownloadAddress,(TCHAR *)buf+2);
	downloadAddress=hex2int(szDownloadAddress);	     
    }
    else
    {
	downloadAddress=0xc000000;
	lstrcpy(szDownloadAddress,TEXT("0xc000000"));
    }
    free(buf);

    CloseHandle(hFile);
    
    return TRUE;
}



int WriteUserConfig(void)
{
    HANDLE hFile;
    DWORD temp;
    TCHAR buf[256];

    hFile=CreateFile(TEXT("dnw.ini"),GENERIC_READ|GENERIC_WRITE,
		     FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
    
    if(hFile==INVALID_HANDLE_VALUE)
    {
	EB_Printf(TEXT("[ERROR:Can't create dnw.ini]\n") );
	return FALSE;
    }
    
    buf[0]=userComPort+'0';
    buf[1]=idBaudRate+'0';
    lstrcpy(buf+2,szDownloadAddress);
    lstrcat(buf,TEXT("\n"));
    lstrcat(&buf[0],TEXT("||+download Address\n"));
    lstrcat(&buf[0],TEXT("|+-0:115200 1:57600 2:38400 3:19200 4:14400 5:9600\n"));
    lstrcat(&buf[0],TEXT("+- 1:COM1 2:COM2 3:COM3 4:COM4\n"));

    if(WriteFile(hFile,buf,lstrlen(buf),&temp,NULL)==0)
    {
	EB_Printf(TEXT("[ERROR:Can't write dnw.ini]\n"));
    }
        
    CloseHandle(hFile);
    return TRUE;
}


UINT hex2int(TCHAR *str)
{
    int i;
    UINT number=0; 
    int order=1;
    TCHAR ch;
    for(i=lstrlen(str)-1;i>=0;i--)
    {
	ch=str[i];
	if(ch=='x' || ch=='X')break;
	
	if(ch>='0' && ch<='9')
	{
	    number+=order*(ch-'0');
	    order*=16;
	}
	if(ch>='A' && ch<='F')
	{
	    number+=order*(ch-'A'+10);
	    order*=16;
	}
	if(ch>='a' && ch<='f')
	{
	    number+=order*(ch-'a'+10);
	    order*=16;
	}
    }
    return number;
}