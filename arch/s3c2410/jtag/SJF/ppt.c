//#include <stdio.h>
//#include "ppt.h"

#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "ppt.h"


int validPpt; 
#ifdef __linux__
int io_access_on( unsigned long port )
{
    if (ioperm (port, 3, 1)) {
        perror ("ioperm()");
        return 0;
    }
    if (ioperm (0x80, 1, 1)) {
        perror ("ioperm()");
        return 0;
    }
    return 1;
}

void io_access_off( unsigned long port )
{
    ioperm (port, 3, 0);
    ioperm (0x80, 1, 0);
}
#else
#define io_access_on(x) (1)
#define io_access_off(x)
#endif
int GetValidPpt(void)
{
	// search for valid parallel port
	if( io_access_on(LPT1) ){
		_outp(LPT1, 0x55);
		if((int)_inp(LPT1) == 0x55) return LPT1;
		io_access_off(LPT1);
	}

	if( io_access_on(LPT2) ){	
		_outp(LPT2, 0x55);
		if((int)_inp(LPT2) == 0x55) return LPT2;
		io_access_off(LPT2);
    }

    if( io_access_on(LPT3) ){
		_outp(LPT3, 0x55);
		if((int)_inp(LPT3) == 0x55) return LPT3;
		io_access_off(LPT3);
    }
	
	return 0;	
}

#define ECP_ECR		    (0x402)
#define ECR_STANDARD	    (0x0)
#define ECR_DISnERRORINT    (0x10)
#define ECR_DISDMA	    (0x0)
#define ECR_DISSVCINT	    (0x4)

void SetPptCompMode(void)
{
    //configure the parallel port at the compatibility mode.
    //_outp(validPpt+ECP_ECR,ECR_STANDARD | ECR_DISnERRORINT | ECR_DISDMA | ECR_DISSVCINT);
}
#ifndef __linux__
int InstallGiveIo(void)
{
    HANDLE h;
    OSVERSIONINFO osvi;
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
	
    if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
	//OS=NT/2000
	h = CreateFile("\\\\.\\giveio", GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(h);
	if(h == INVALID_HANDLE_VALUE)
    	    return 0;
	else
	    return 0x2000;
    }
    else
    {	//OS=WIN98
	return 0x0098;
    }
}
#endif

