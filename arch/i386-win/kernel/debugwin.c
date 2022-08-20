#include <windows.h>
#include "windebug.h"
#include "resource.h"

#define DEMO


/*****************************************************************/
// The DebugOutWindow Class
/*****************************************************************/
static WORD RegisterDebugOutWindow(HINSTANCE hInst);
static BOOL CALLBACK DlgDebugProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

static LRESULT WINAPI DebugWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
static LRESULT DoPAINT( HWND hWnd ,WPARAM wParam,LPARAM lParam);
static LRESULT DoVSCROLL( HWND hWnd ,WPARAM wParam,LPARAM lParam);
static LRESULT DoHSCROLL( HWND hWnd ,WPARAM wParam,LPARAM lParam);
static LRESULT DoKEYDOWN( HWND hWnd ,WPARAM wParam,LPARAM lParam);
static LRESULT DoCREATE( HWND hWnd ,WPARAM wParam,LPARAM lParam);
static LRESULT DoDestroy( HWND hWnd ,WPARAM wParam,LPARAM lParam);
static LRESULT DoAddString( HWND hWnd ,WPARAM wParam,LPARAM lParam);
static LRESULT DoLoadString( HWND hWnd ,WPARAM wParam,LPARAM lParam);

static LRESULT DoDebugWindowSIZE( HWND hWnd, WPARAM wParam, LPARAM lParam );

static void DrawItem(HWND hWnd,HDC hdc,WORD iItem);
static void VScrollWindow(HWND hWnd,short nScrollLine);
static void SetVScrollBar(HWND hWnd);
static void SetScrollPage(HWND hWnd,short iWindowRow);
static short GetWindowRows(HWND hWnd);

static BOOL bRecord=TRUE;

#define DDM_ADDSTRING  0x6000
#define DDM_LOADSTRING 0x6001

#define MAXDATANUM 1024
#define MAX_LINE_DATALEN	1024
#define TOOLWINDOW_HEIGHT 40
#define TOOLWINDOW_STARTX 8
#define TEXT_LINE_HEIGHT  20

typedef struct _DEBUGDATA{
	char lpData[MAXDATANUM][MAX_LINE_DATALEN];
	int iCurItem;
	int iVisibleRow;
	int iStartItem;
	int iTotalItem;
	int xWindowOrg;
}DEBUGDATA, * LPDEBUGDATA;

static HWND  hwndDebugWnd = NULL;
void InitWin32DebugWindow( HINSTANCE hInstance )
{
   WNDCLASS wc;
// register MyButton
   wc.hInstance=hInstance;
   wc.lpszClassName= "DebugOutMainWindow";
// the proc is class function
   wc.lpfnWndProc = DlgDebugProc;
   wc.style=0;
   wc.hIcon= 0;
// at pen window, no cursor
   wc.hCursor= LoadCursor(NULL,IDC_ARROW);
// to auto erase background, must set a valid brush
// if 0, you must erase background yourself
   wc.hbrBackground = GetStockObject( WHITE_BRUSH );

   wc.lpszMenuName=NULL;
   wc.cbClsExtra=0;
// !!! it's important to save state of button, align to long
   wc.cbWndExtra=0;

   RegisterClass(&wc);
#ifndef KINGMOS_DEMO
   hwndDebugWnd = CreateWindow( "DebugOutMainWindow", 
	                            "DebugWindow", WS_POPUPWINDOW|WS_VISIBLE|WS_DLGFRAME|WS_THICKFRAME, 800 - 320, 0, 320, 500, NULL, NULL, hInstance, 0 );
#endif

}

void OEM_WriteDebugString( LPTSTR lpszStr )
{
#ifndef KINGMOS_DEMO

    SendMessage( hwndDebugWnd, WM_OUTDEBUGTEXTLINE, 0, (LPARAM)lpszStr );
#endif
}


WORD RegisterDebugOutWindow(HINSTANCE hInst)
{
   WNDCLASS wc;
// register MyButton
   wc.hInstance=hInst;
   wc.lpszClassName= "DebugOutWindow";
// the proc is class function
   wc.lpfnWndProc = DebugWndProc;
   wc.style=0;
   wc.hIcon= 0;
// at pen window, no cursor
   wc.hCursor= LoadCursor(NULL,IDC_ARROW);
// to auto erase background, must set a valid brush
// if 0, you must erase background yourself
   wc.hbrBackground = GetStockObject( WHITE_BRUSH );

   wc.lpszMenuName=NULL;
   wc.cbClsExtra=0;
// !!! it's important to save state of button, align to long
   wc.cbWndExtra=4;

   return RegisterClass(&wc);
}


static LRESULT WINAPI DebugWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch( message )
	{
	case WM_PAINT:
		return DoPAINT( hWnd ,wParam,lParam);
    case DDM_ADDSTRING:
        if( bRecord )
            return DoAddString( hWnd, wParam ,lParam );
        break;
	case DDM_LOADSTRING:
        return DoLoadString( hWnd, wParam ,lParam );
    case WM_VSCROLL:
        return DoVSCROLL( hWnd, wParam ,lParam );
    case WM_HSCROLL:
        return DoHSCROLL( hWnd, wParam ,lParam );
    case WM_KEYDOWN:
        return DoKEYDOWN( hWnd, wParam, lParam );
	case WM_SIZE:
		return DoDebugWindowSIZE( hWnd, wParam, lParam );
    case WM_CREATE:
		return DoCREATE( hWnd, wParam,lParam );
    case WM_DESTROY:
        return DoDestroy( hWnd ,wParam,lParam);
   default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}


static LRESULT DoPAINT( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
	LPDEBUGDATA lpDebugData;
	HDC hdc;
	int i,iVisibleItem;
	short iWindowRow;
    PAINTSTRUCT ps;
	RECT rect;

    hdc= BeginPaint( hWnd, &ps );
	lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
	if (lpDebugData==0)
	{
	    EndPaint( hWnd, &ps );
		return  0;
	}
	GetClientRect( hWnd, &rect );

    //SetWindowOrgEx( hdc, lpDebugData->xWindowOrg, 0, NULL );
	//SetWindowExtEx( hdc, lpDebugData->xWindowOrg + rect.right, rect.bottom, NULL );

//	hdc=GetDC(hWnd);
	iVisibleItem=lpDebugData->iStartItem+lpDebugData->iVisibleRow;
//	if (iVisibleItem>MAXDATANUM)
//		iVisibleItem-=MAXDATANUM;
//	for (i=iVisibleItem;i<lpDebugData->iCurItem;i++)
	iWindowRow=GetWindowRows(hWnd);
	for (i=0;i<iWindowRow;i++)
	{
		if (iVisibleItem>=MAXDATANUM)
			iVisibleItem-=MAXDATANUM;
		DrawItem(hWnd,hdc,(WORD)iVisibleItem++);
//		TextOut(hdc,0,(i-lpDebugData->iVisibleRow)*TEXT_LINE_HEIGHT,lpDebugData->lpData[i],strlen(lpDebugData->lpData[i]));
	}

    EndPaint( hWnd, &ps );
//	ReleaseDC(hWnd,hdc);
    return 0;
}
static LRESULT DoVSCROLL( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
    short nScrollLine,iWindowRow;
    short yPos;
//	RECT rect;
        
//	  GetClientRect(hWnd,&rect);
//	  iWindowRow=(rect.bottom-rect.top)/TEXT_LINE_HEIGHT;
	  iWindowRow=GetWindowRows(hWnd);
//	  nPageLines=GetPageLine(hWnd);
      yPos=GetScrollPos(hWnd,SB_VERT);
      switch(LOWORD(wParam))
        {
        case SB_PAGEUP:  // page up
          nScrollLine=0-iWindowRow;
          break;
        case SB_PAGEDOWN:  //page down
          nScrollLine=iWindowRow;
          break;
        case SB_LINEUP:  // line up
          nScrollLine=-1;
          break;
        case SB_LINEDOWN:  // line down
          nScrollLine=1;
          break;
        case SB_THUMBTRACK: // drag thumb track
          nScrollLine=HIWORD(wParam)-yPos;
          break;
        default:
          nScrollLine=0;
          return 0;
        }
				// vertical scroll window
        VScrollWindow(hWnd,nScrollLine);
		return 0;
}
static LRESULT DoHSCROLL( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
	LPDEBUGDATA lpDebugData;
	int x;
	RECT rc;
	
	lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
	GetClientRect( hWnd, &rc );
	x = lpDebugData->xWindowOrg;
	switch( LOWORD( wParam ) )
	{
	case SB_LINELEFT:
		x -= 8;
		break;
	case SB_LINERIGHT:
		x += 8;
		break;
	case SB_PAGELEFT:
		x -= rc.right;
		break;
	case SB_PAGERIGHT:
		x += rc.right;
		break;
	case SB_THUMBTRACK:
		x = HIWORD( wParam );
		break;
	}
	if( x < 0 )
		x = 0;
	if( x > 2048 )
		x = 2048;

	SetScrollPos( hWnd, SB_HORZ, x, TRUE );

	lpDebugData->xWindowOrg = GetScrollPos( hWnd, SB_HORZ );
	InvalidateRect( hWnd, NULL, TRUE );
    return 0;
}
static LRESULT DoKEYDOWN( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
    return 0;
}
static LRESULT DoCREATE( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
	LPDEBUGDATA lpDebugData;
	int i;
	SCROLLINFO si;
	RECT rc;

	lpDebugData=malloc(sizeof(DEBUGDATA));

	if (lpDebugData==0)
		return  -1;
	memset( lpDebugData, 0, sizeof(DEBUGDATA) );
	for (i=0;i<MAXDATANUM;i++)
		lpDebugData->lpData[i][0]=0;
	lpDebugData->iCurItem=0;
	lpDebugData->iVisibleRow=0;
	lpDebugData->iStartItem=0;
	lpDebugData->iTotalItem=0;

	SetWindowLong(hWnd,0,(DWORD)lpDebugData);

	GetClientRect( hWnd, &rc );
	si.cbSize = sizeof( si );
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nMax = 2048;
	si.nMin = 0;
	si.nPage = rc.right;
	si.nPos = 0;
	
	SetScrollInfo( hWnd, SB_HORZ, &si, FALSE );
	return 0;

}

static LRESULT DoDebugWindowSIZE( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	LPDEBUGDATA lpDebugData;
	int iWindowRow;
	SCROLLINFO si;
	RECT rc;

	iWindowRow=GetWindowRows(hWnd);

	SetVScrollBar( hWnd );

	//水平滚动条
	GetClientRect( hWnd, &rc );
	si.cbSize = sizeof( si );
	si.fMask = SIF_PAGE;
	si.nMax = 0;
	si.nMin = 0;
	si.nPage = rc.right;
	si.nPos = 0;	
	SetScrollInfo( hWnd, SB_HORZ, &si, FALSE );


	lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);

	if (lpDebugData->iTotalItem > iWindowRow )
	{
		if (lpDebugData->iVisibleRow<MAXDATANUM-iWindowRow)
		{
			lpDebugData->iVisibleRow = lpDebugData->iTotalItem - iWindowRow;//++;
			SetScrollPos(hWnd,SB_VERT,lpDebugData->iVisibleRow,TRUE);
		}
	}

	
	InvalidateRect( hWnd, NULL, FALSE );
	UpdateWindow( hWnd );
	return 0;
}

static LRESULT DoDestroy( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
	LPDEBUGDATA lpDebugData;

	lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
	if (lpDebugData==0)
		return  0;

//	for (i=0;i<MAXDATANUM;i++)
//	{
//		if (lpDebugData->lpData[i]!=0)
//			free(lpDebugData->lpData[i]);
//	}
	free(lpDebugData);
	SetWindowLong(hWnd,0,0);
    return 0;
}

static LRESULT DoAddString( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
	LPDEBUGDATA lpDebugData;
	LPSTR lpString;
	LPSTR lpString2222;
	int cbSize;
	RECT rect;
	int iWindowRow;
	HDC hdc;

	lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
	if (lpDebugData==0)
		return  0;

	lpString=(LPSTR)lParam;

	cbSize=strlen(lpString);
	if(cbSize >= MAX_LINE_DATALEN)
	{
		return 0;
	}
	if (lpDebugData->iTotalItem>=MAXDATANUM)
	{
//		free(lpDebugData->lpData[lpDebugData->iCurItem]);
		lpDebugData->iStartItem=lpDebugData->iCurItem+1;
	}
//	lpDebugData->lpData[lpDebugData->iCurItem]=malloc(cbSize+1);
//	if (lpDebugData->lpData[lpDebugData->iCurItem]==NULL)
//		return 0;
	strcpy(lpDebugData->lpData[lpDebugData->iCurItem],lpString);
	//
	lpString2222 = lpDebugData->lpData[lpDebugData->iCurItem];
	while( *lpString2222 )
	{
		if( (*lpString2222=='\r') || (*lpString2222=='\n') )
		{
			*lpString2222 = ' ';
		}
		lpString2222 ++;
	}
	if (lpDebugData->iTotalItem<MAXDATANUM)
	{
		lpDebugData->iTotalItem++;
		SetVScrollBar(hWnd);
	}
	lpDebugData->iCurItem++;
	if (lpDebugData->iCurItem>=MAXDATANUM)
	{
		lpDebugData->iCurItem=0;
	}
	GetClientRect(hWnd,&rect);
//	iWindowRow=(rect.bottom-rect.top)/TEXT_LINE_HEIGHT;
	iWindowRow=GetWindowRows(hWnd);
//	if ((lpDebugData->iCurItem-lpDebugData->iVisibleRow)>iWindowRow)
	if (lpDebugData->iTotalItem>iWindowRow)
	{
		//rect.top+=TEXT_LINE_HEIGHT;
		//ScrollWindow(hWnd,0,-TEXT_LINE_HEIGHT,&rect,NULL);
		if (lpDebugData->iVisibleRow<=MAXDATANUM-iWindowRow)
		{
			lpDebugData->iVisibleRow = lpDebugData->iTotalItem - iWindowRow;//++;
			SetScrollPos(hWnd,SB_VERT,lpDebugData->iVisibleRow,TRUE);
			InvalidateRect( hWnd, NULL, FALSE );
			UpdateWindow( hWnd );
		}
	}
//	if (lpDebugData->iCurItem>iWindowRow)
//	{
//		SetVScrollBar(hWnd);
//	}
	hdc=GetDC(hWnd);
	//SetWindowOrgEx( hdc, lpDebugData->xWindowOrg, 0, NULL );
	//SetWindowExtEx( hdc, lpDebugData->xWindowOrg + rect.right, rect.bottom, NULL );
	if (lpDebugData->iCurItem==0)
		DrawItem(hWnd,hdc,(WORD)(MAXDATANUM+lpDebugData->iCurItem-1));
	else
		DrawItem(hWnd,hdc,(WORD)(lpDebugData->iCurItem-1));
	ReleaseDC(hWnd,hdc);
//	InvalidateRect(hWnd,NULL,TRUE);
    return 0;
}
static LRESULT DoLoadString( HWND hWnd ,WPARAM wParam,LPARAM lParam)
{
	int iItem,iRealItem;
	LPSTR lpData;
	LPDEBUGDATA lpDebugData;

		lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
		if (lpDebugData==0)
			return  FALSE;

		iItem=(int)wParam;
		lpData=(LPSTR)lParam;

		iRealItem=lpDebugData->iStartItem+iItem;
		if (iRealItem>=MAXDATANUM)
		{
			iRealItem-=MAXDATANUM;
		}
		if (lpDebugData->iTotalItem>iRealItem)
		{
			strcpy(lpData,lpDebugData->lpData[iRealItem]);
			return TRUE;
		}
		return FALSE;
}


static void SetVScrollBar(HWND hWnd)
{
	LPDEBUGDATA lpDebugData;
		DWORD dwStyle;
    short nMinPos,nMaxPos;
    short iWindowRow;
//	RECT rect;
        
//		  GetClientRect(hWnd,&rect);
//		  iWindowRow=(rect.bottom-rect.top)/TEXT_LINE_HEIGHT;
			iWindowRow=GetWindowRows(hWnd);

			lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
			if (lpDebugData==NULL)
					return ;
			if (lpDebugData->iTotalItem<iWindowRow)
				return;
			// get window style 
			dwStyle=GetWindowLong(hWnd,GWL_STYLE);
			if ((dwStyle&WS_VSCROLL)==0)
			{
				ShowScrollBar(hWnd,SB_VERT,TRUE);
			}	
			nMinPos=0;
			nMaxPos=lpDebugData->iTotalItem-1;
			SetScrollRange(hWnd,SB_VERT,nMinPos,nMaxPos,TRUE);
			SetScrollPage(hWnd,iWindowRow);
//		SetVScrollPage(hWnd);

}
static void SetScrollPage(HWND hWnd,short iWindowRow)
{
//   short nPageLine;
   SCROLLINFO ScrollInfo;
   DWORD dwStyle;
//    short iWindowRow;
//	RECT rect;
        
//		  GetClientRect(hWnd,&rect);
//		  iWindowRow=(rect.bottom-rect.top)/TEXT_LINE_HEIGHT;
      
      dwStyle=GetWindowLong(hWnd,GWL_STYLE);

//      nPageLine=GetPageLine(hWnd);

      ScrollInfo.cbSize=sizeof(SCROLLINFO);
      ScrollInfo.fMask=SIF_PAGE;
      // Set Vertical Scroll Page
      if (dwStyle&WS_VSCROLL)
      {
        ScrollInfo.nPage =iWindowRow;
        SetScrollInfo(hWnd,SB_VERT,&ScrollInfo,TRUE);
      }

}

static void DrawItem(HWND hWnd,HDC hdc,WORD iItem)
{
//		TextOut(hdc,0,iItem*TEXT_LINE_HEIGHT,lpString,strlen(lpString));
	LPDEBUGDATA lpDebugData;
	RECT rect;
//	HDC hdc;
	int iRow;
	LPSTR lpString;
	//SIZE size;
		lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
		if (lpDebugData==NULL)
				return ;

		iRow=iItem-lpDebugData->iStartItem-lpDebugData->iVisibleRow;
		if (iRow<0)
			iRow+=MAXDATANUM;
//		if (iItem>=lpDebugData->iVisibleRow)
//			iRow=iItem-lpDebugData->iVisibleRow;		
//		else
//			iRow=MAXDATANUM+iItem-lpDebugData->iVisibleRow;		
		lpString=lpDebugData->lpData[iItem];
		GetClientRect(hWnd,&rect);	
		//GetWindowOrgEx( hdc, &pt );
		rect.top=iRow*TEXT_LINE_HEIGHT;	
		rect.bottom=rect.top+TEXT_LINE_HEIGHT;	
		//rect.right = size.cx;
		rect.left -= lpDebugData->xWindowOrg;
		
//		hdc=GetDC(hWnd);
		if (lpString)
		{
			//ExtTextOut(hdc,0,iRow*TEXT_LINE_HEIGHT,ETO_OPAQUE,&rect,lpString,strlen(lpString),NULL);
			FillRect( hdc, &rect, GetStockObject( WHITE_BRUSH ) );
			DrawText( hdc, lpString, -1, &rect, 0  );
		}
//		ReleaseDC(hWnd,hdc);
}
static void VScrollWindow(HWND hWnd,short nScrollLine)
{
	LPDEBUGDATA lpDebugData;
    short iWindowRow;
//	RECT rect;
        
//		GetClientRect(hWnd,&rect);
//		iWindowRow=(rect.bottom-rect.top)/TEXT_LINE_HEIGHT;
		iWindowRow=GetWindowRows(hWnd);

		lpDebugData=(LPDEBUGDATA)GetWindowLong(hWnd,0);
		if (lpDebugData==NULL)
				return ;
		lpDebugData->iVisibleRow+=nScrollLine;
		if (lpDebugData->iVisibleRow<0)
			lpDebugData->iVisibleRow=0;
		if ((lpDebugData->iTotalItem-lpDebugData->iVisibleRow)<iWindowRow)
		{
			lpDebugData->iVisibleRow=lpDebugData->iTotalItem-iWindowRow;
		}
		SetScrollPos(hWnd,SB_VERT,lpDebugData->iVisibleRow,TRUE);
		InvalidateRect(hWnd,NULL,TRUE);
}
static short GetWindowRows(HWND hWnd)
{
    short iWindowRow;
	RECT rect;
        
	  GetClientRect(hWnd,&rect);
	  iWindowRow=(rect.bottom-rect.top)/TEXT_LINE_HEIGHT;
	  if( iWindowRow < 0 )
		  iWindowRow = 0;
	  return iWindowRow;
}
/*******************************************************************/
/*******************************************************************/
static HWND hDlgWindow = 0;
void SaveLog();
static void DoInitialDialog(HWND hWnd);

BOOL CALLBACK DlgDebugProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HWND hWndDebug;
    switch( uMsg )
    {
    //case WM_INITDIALOG:
    case WM_OUTDEBUGTEXTLINE:		
		hWndDebug=GetDlgItem( hWnd, IDC_OUTWINDOW );
		SendMessage(hWndDebug,DDM_ADDSTRING,0,lParam);
		//SaveLog();
		return 1;         
//	case WM_CLOSE:
    case WM_COMMAND:
        if( LOWORD( wParam )==IDC_STOP && HIWORD( wParam )==BN_CLICKED ) 
        {
            HWND hWndItem=GetDlgItem( hWnd, IDC_START );

            ShowWindow( (HWND)lParam, SW_HIDE );
            ShowWindow( hWndItem, SW_SHOW );
            bRecord=FALSE;
        }
        else if( LOWORD( wParam )==IDC_START && HIWORD( wParam )==BN_CLICKED )
        {
            HWND hWndItem=GetDlgItem( hWnd, IDC_STOP );

            ShowWindow( (HWND)lParam, SW_HIDE );
            ShowWindow( hWndItem, SW_SHOW );
            bRecord=TRUE;
        }
        break;
	case WM_SIZE:
		{
			RECT rect;
			HWND hWndCtrl;
			hWndCtrl=GetDlgItem( hWnd, IDC_OUTWINDOW );
			GetClientRect( hWnd, &rect );
			rect.bottom -= TOOLWINDOW_HEIGHT;
			SetWindowPos( hWndCtrl, NULL, 0, 0, rect.right, rect.bottom, SWP_NOMOVE | SWP_NOZORDER );
			hWndCtrl=GetDlgItem( hWnd, IDC_START );
			rect.bottom += 5;
			SetWindowPos( hWndCtrl, NULL, TOOLWINDOW_STARTX, rect.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
			hWndCtrl=GetDlgItem( hWnd, IDC_STOP );
			SetWindowPos( hWndCtrl, NULL, TOOLWINDOW_STARTX, rect.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER );			
		}
		return 0;

	case WM_CREATE:
        hDlgWindow = hWnd;
		DoInitialDialog(hWnd);
        return 1;
	case WM_CLOSE:
		return 0;
	case WM_DESTROY:
		SaveLog();
		break;
    default:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }
    return 0;
}

void SaveLog(void)
{
	HANDLE hFile;
	HWND hWndDebug; 
	DWORD dwSize;
	char *lpData;
	int i;
    HWND hWnd = hDlgWindow;

		hWndDebug=GetDlgItem( hWnd, IDC_OUTWINDOW );
//		hWndEdit = GetDlgItem( hWnd, IDC_OUTWINDOW );
//		dwSize=SendMessage(hWndEdit,WM_GETTEXTLENGTH,0,0)+1;
		lpData=(char *)malloc(MAX_LINE_DATALEN);
		if (lpData==NULL)
			return;

		hFile=CreateFile(".\\___debug.log",
			GENERIC_WRITE|GENERIC_READ,
			0,//FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile==INVALID_HANDLE_VALUE)
		{
			free(lpData);
			return;
		}
//		SendMessage(hWndEdit,WM_GETTEXT,dwSize,lpData);
		for (i=0;i<MAXDATANUM;i++)
		{
			if (SendMessage(hWndDebug,DDM_LOADSTRING,i,(LPARAM)lpData)==0)
			{
				free(lpData);
				CloseHandle(hFile);
				return;
			}

			if (WriteFile(hFile,lpData,strlen(lpData),&dwSize,NULL)==0)
			{
				free(lpData);
				CloseHandle(hFile);
				return;
			}
			if (WriteFile(hFile,"\r\n",2,&dwSize,NULL)==0)
			{
				free(lpData);
				CloseHandle(hFile);
				return;
			}
		}
		free(lpData);
		CloseHandle(hFile);
}

static void DoInitialDialog(HWND hWnd)
{
  RECT rect;
  HINSTANCE hInst;
  HWND hItem;
	
     
  SetWindowText( hWnd, "巨果·Kingmos 应用开发-调试输出窗口" );
	hInst=(HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE);



	RegisterDebugOutWindow(hInst);

	GetClientRect(hWnd,&rect);

	hItem=CreateWindow("DebugOutWindow",
		"",
		WS_VISIBLE|WS_CHILD|WS_BORDER|WS_HSCROLL,
		rect.left,
		rect.top,
		rect.right,
		rect.bottom - TOOLWINDOW_HEIGHT,
		hWnd,
		(HMENU)IDC_OUTWINDOW,
		hInst,
		0);
	CreateWindow("Button",
		"开始",
		BS_PUSHBUTTON |WS_CHILD,
		rect.left+8,
		rect.bottom - TOOLWINDOW_HEIGHT + 5,
		80,
		25,
		hWnd,
		(HMENU)IDC_START,
		hInst,
		0);

	CreateWindow("Button",
		"停止",
		WS_VISIBLE|BS_PUSHBUTTON |WS_CHILD,
		rect.left+8,
		rect.bottom-TOOLWINDOW_HEIGHT + 5,
		80,
		25,
		hWnd,
		(HMENU)IDC_STOP,
		hInst,
		0);
}





