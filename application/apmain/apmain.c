#include <ewindows.h>
#include <Eassert.h>
#include <eapisrv.h>
#include <eucore.h>



extern LRESULT CALLBACK WinMain_test( HINSTANCE hInstance,HINSTANCE hPrevInstance,
            LPSTR     lpCmdLine,int       nCmdShow );

void InstallApplication( void )
{
	
    RegisterApplication( "test", WinMain_test ,0 );
	
}
