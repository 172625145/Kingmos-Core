#include "ewindows.h"
#include "ecore.h"

extern void KingmosStart( LPVOID );

void _SystemStart(  DWORD uiPhyFirstLevelPage, LPBYTE lpbFreeMemStart, DWORD dwFreeMemSize  )
{
	extern LPBYTE  lpbSysMainMem;
    extern ULONG   ulSysMainMemLength;

#ifdef VIRTUAL_MEM
	InitPTE( uiPhyFirstLevelPage ); 
//	SetProtect( 0x80000000 );
#endif

	//TestVirtual();
	lpbSysMainMem = lpbFreeMemStart;
	ulSysMainMemLength = dwFreeMemSize;
#ifdef DEBUG
	memset( lpbSysMainMem, 0xcccccccc, dwFreeMemSize );
#endif
  	
	OEM_Init();

	INTR_ON();

	EdbgOutputDebugString( "begin Kingmos..................\r\n" );

	KingmosStart(0);
	
	while(1);

}
