#include <edef.h>
#include <ecore.h>
#include <sysintr.h>
//#include <ethread.h>
#include <oemfunc.h>
#include <eapisrv.h>

extern void InstallMouseDevice( void );
extern void InstallKeyboardDevice( void );
extern void xygRas_AddCOM( void );

//BOOL CALLBACK _MyInitDefaultDevice( void )
//{
//	int i=0;
//}
// call by system.c

BOOL OEM_InitDefaultDevice( void )
{
//	extern BOOL LoadFatFileSystem( void );
    // to register your device here
//	LoadFatFileSystem();

    return TRUE;
}
