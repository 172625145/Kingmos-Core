#include <eframe.h>
#include <eapisrv.h>
//#include <Shellapi.h>

#include <epcore.h>


enum{
    SHELL_SHCHANGENOTIFY = 1,
};



typedef VOID (WINAPI * PSHCHANGENOTIFY)(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2);
VOID WINAPI SH_ChangeNotify(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2)
{
#ifdef CALL_TRAP
	(PSHCHANGENOTIFY)CALL_API( API_SHELL, SHELL_SHCHANGENOTIFY, 4 ) )(wEventId, uFlags, dwItem1, dwItem2);
	return;
#else
	PSHCHANGENOTIFY pSHChangeNotify;

	CALLSTACK cs;

	if( API_Enter( API_SHELL, SHELL_SHCHANGENOTIFY, &pSHChangeNotify, &cs ) )
	{
		dwItem1 = MapProcessPtr( dwItem1, (LPPROCESS)cs.lpvData );
		dwItem2 = MapProcessPtr( dwItem2, (LPPROCESS)cs.lpvData );

		pSHChangeNotify(wEventId, uFlags, dwItem1, dwItem2);
		API_Leave(  );
	}
	return ;
#endif
}

