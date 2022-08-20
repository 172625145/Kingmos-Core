//#include <estring.h>
//#include <stdlib.h>
//#include <eassert.h>
//#include <eframe.h>
#include <ewindows.h>

BOOL _InitDebug(void)
{
	return 1;
}

//void __ShowDebugStr( int x, int y, char *lpstr )
//{
//}

//void __ShowDebugValue( int x, int y, int v )
//{
//}

//static char tMsg[256];
//static char tCond[256];
//static char tFile[256];
//
//void FormatStr( char * ptchar, const char *pchar )
//{
//	while( *pchar )
//		*ptchar++ = *pchar++;
//	*ptchar = 0;
//}

//void __AssertFail( char *__msg, char *__cond, char *__file, int __line )
void __AssertFail( char *__msg, char *__cond, char *__file, int __line, char * __notify )
{	
	//FormatStr( tMsg, __msg );
	//FormatStr( tCond, __cond );
	//FormatStr( tFile, __file );
	//RETAILMSG( 1, (tMsg, tCond, tFile, __line) );
	//EdbgOutputDebugString(tMsg);
	//EdbgOutputDebugString(tCond);
	//EdbgOutputDebugString(tFile);
	//EdbgOutputDebugString(tMsg);
	if( __notify )
	{
		EdbgOutputDebugString( "%s", __notify );
	}

	EdbgOutputDebugString(__msg, __cond, __file, __line);
}
/*
void KL_DebugOutString( char * lpszStr )
{
	//FormatStr( tMsg, lpszStr );
	//RETAILMSG( 1, (tMsg) );
	EdbgOutputDebugString(lpszStr);
}
*/





