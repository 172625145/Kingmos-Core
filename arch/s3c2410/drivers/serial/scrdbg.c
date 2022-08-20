
#include <ewindows.h>

#define	DEBUG_SWITCH
//
#ifdef	DEBUG_SWITCH

/***************  全局区 定义， 声明 *****************/

#define		XPOS_FIRST		40
#define		XPOS_MAX		580
#define		YLINE_MAX		26
#define		YPOS_FIRST		320
#define		YPOS_MAX		754


static	DWORD	g_nXPos = XPOS_FIRST;
static	DWORD	g_nYLineCount = 0;
static	DWORD	g_nYPos = YPOS_FIRST;

static	BOOL	g_fAutoNewLine = FALSE;

static	HDC		g_hdcDebug = NULL;
static	BOOL	g_fChinese = TRUE;

/******************************************************/


void	OEM_WriteDebugByte_ToWnd(TCHAR c) 
{
	DWORD	Len;
	static	BOOL	fHave = FALSE;
	static	TCHAR	chChinese[4]={ 0, 0, 0 };
	//TCHAR ch;

	if( g_hdcDebug==NULL )
	{
		g_hdcDebug = GetDC( NULL );
		if( g_hdcDebug==NULL )
			return ;
		SetTextColor( g_hdcDebug, RGB( 0xff, 0, 0 ) );
	}
	if( g_nYPos > YPOS_MAX )
		g_nYPos = YPOS_FIRST;

	if( c=='\n' )
	{
		//char	pText[100];
		TCHAR	pText[100];

		//g_fAutoNewLine = FALSE;

		g_nYLineCount ++;
		g_nYPos += 16;
		//show lines, and ready line
		swprintf( pText, TEXT("%d "), g_nYLineCount );
		Len = wcslen( pText );
		memset( pText+Len, ' ', 99-Len );
		pText[99]= 0;
		ExtTextOut(g_hdcDebug, 8, g_nYPos, ETO_OPAQUE, NULL, pText, ((XPOS_MAX-8)/8)<99 ? ((XPOS_MAX-8)/8):99, NULL );
	}
	else if( c=='\r' )
	{
		g_fAutoNewLine = FALSE;

		g_nXPos = XPOS_FIRST;
	}
	else
	{
		if( g_fAutoNewLine )
		{
			//char	pText[100];
			TCHAR	pText[100];
			//int		iii;

			memset( pText, ' ', 99 );
			pText[99]= 0;
			//iii = (XPOS_MAX-8)/8;
			//if( iii>99 )
			//	iii = 99;
			ExtTextOut(g_hdcDebug, 8, g_nYPos, ETO_OPAQUE, NULL, pText, 99, NULL );
			//
			g_fAutoNewLine = FALSE;
		}

		//
		if( g_fChinese && (c & 0x80) )
		{
			if( fHave )
			{
				chChinese[1] = c;

				ExtTextOut(g_hdcDebug, g_nXPos, g_nYPos, ETO_OPAQUE, NULL, chChinese, 2, NULL );
				g_nXPos += 16;
				fHave = FALSE;
			}
			else
			{
				chChinese[0] = c;
				fHave = TRUE;
			}
		}
		else
		{
			//ch = c;
			ExtTextOut(g_hdcDebug, g_nXPos, g_nYPos, ETO_OPAQUE, NULL, (LPCTSTR)&c, 1, NULL );
			g_nXPos += 8;
		}
	}

	//new line auto
	if( g_nXPos > XPOS_MAX )
	{
		g_nXPos = XPOS_FIRST;
		g_nYPos += 16;
		g_nYLineCount ++;

		g_fAutoNewLine = TRUE;
	}
}

void	ScrDbg_Clean( )
{
	int		nYPos;

	if( g_hdcDebug==NULL )
	{
		g_hdcDebug = GetDC( NULL );
		if( g_hdcDebug==NULL )
			return ;
	}
	//
	for( nYPos=YPOS_FIRST; nYPos<=YPOS_MAX;  )
	{
		TCHAR	pText[100];

		memset( pText, ' ', 99 );
		pText[99]= 0;
		ExtTextOut(g_hdcDebug, 8, nYPos, ETO_OPAQUE, NULL, pText, 99, NULL );
		nYPos += 16;
	}

	//
	g_nXPos = XPOS_FIRST;
	g_nYLineCount = 0;
	g_nYPos = YPOS_FIRST;

	g_fAutoNewLine = FALSE;
	g_fChinese = TRUE;
}



//ScrDbg_Out( (TEXT("serial test: baud=[%d]"), dwBaudrate) );
void	ScrDbg_Out( LPCTSTR lpszFormat, ... )
{
	TCHAR			szBuffer[4096];
	int				iOut;
	int				i;
	va_list			args; 

	//if( g_hdcDebug==NULL )
	//{
	//	g_hdcDebug = GetDC( NULL );
	//	if( g_hdcDebug==NULL )
	//		return ;
	//}
	va_start(args, lpszFormat); 
    iOut = wvsprintf (szBuffer, lpszFormat, args);
	va_end(args); 

	for( i=0; i<iOut; i++ )
	{
		OEM_WriteDebugByte_ToWnd( szBuffer[i] );
	}
}
#endif
