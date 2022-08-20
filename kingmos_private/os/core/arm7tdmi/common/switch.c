#include <edef.h>
#include <epcore.h>

extern void EdbgOutputDebugString(const unsigned char *sz, ...);
extern void Switch( TSS * lpPrevTSS, TSS * lpNextTSS );

void SwitchTo( LPTHREAD lpPrev, LPTHREAD lpNext )
{
//	EdbgOutputDebugString( "Switch From ID=%d TO ID=%d\r\n", lpPrev->dwThreadID, lpNext->dwThreadID );
	//EdbgOutputDebugString( "Switch To R0=%x, LR=%x SP=%x\r\n", lpNext->tss.r0, lpNext->tss.lr, lpNext->tss.sp );
//	extern int iCheckDDD;

//	if( iCheckDDD )
//	    EdbgOutputDebugString( "SwitchTo = %x,%x,sp=%x\r\n", &lpPrev->tss, &lpNext->tss,lpNext->tss.r13 );

	//ClearPageTable();
	Switch( &lpPrev->tss, &lpNext->tss );
	//EdbgOutputDebugString( "Switch back.\r\n" );
}

