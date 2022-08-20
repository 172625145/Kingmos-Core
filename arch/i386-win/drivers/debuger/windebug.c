//typedef void * HKEY;
#include <windows.h>

extern void SaveLog(void);
void WinDebug( void )
{ 
#ifndef KINGMOS_DEMO
    SaveLog();
    //DebugBreak();
    //_ASSERT( 0 );

	//Beep( 1000, 20 );
#endif
}
           
void WarnSound( void )
{
#ifndef KINGMOS_DEMO
    SaveLog();
	//Beep( 1000, 20 );
#endif
}
