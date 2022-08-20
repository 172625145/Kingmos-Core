#include <eframe.h>
#include <epcore.h>

//extern void EdbgOutputDebugString(const unsigned char *sz, ...);
extern void Switch( TSS * lpPrevTSS, TSS * lpNextTSS, LPTHREAD * lppCurThread, LPTHREAD lpNext );
void ClearThreadPageTable ( LPTHREAD_PAGE_TABLE lpPageTable );
void SwitchTo( LPTHREAD lpPrev, LPTHREAD lpNext )
{
//	EdbgOutputDebugString( "Switch From ID=%d TO ID=%d\r\n", lpPrev->dwThreadID, lpNext->dwThreadID );
	//EdbgOutputDebugString( "Switch To R0=%x, LR=%x SP=%x\r\n", lpNext->tss.r0, lpNext->tss.lr, lpNext->tss.sp );
//	extern int iCheckDDD;

//	if( iCheckDDD )
//	    EdbgOutputDebugString( "SwitchTo = %x,%x,sp=%x\r\n", &lpPrev->tss, &lpNext->tss,lpNext->tss.r13 );

	//ClearPageTable();
#ifdef __DEBUG	
	if( (lpNext->tss.r14 & 0xffff0000) == 0 )
	{
		EdbgOutputDebugString( "SwitchTo pc=0x%x,CurProcessName=%s,OwnerProcessName=%s.\r\n", lpNext->tss.r14, lpNext->lpCurProcess->lpszApplicationName, lpNext->lpOwnerProcess->lpszApplicationName );
		
	}
#endif

//#ifdef USE_THREAD_PAGE_TABLE
//	ClearThreadPageTable( &lpPrev->pageTable );
//#endif	
//	lpCurThread = lpNext;
	Switch( &lpPrev->tss, &lpNext->tss, &lpCurThread, lpNext );
	//EdbgOutputDebugString( "Switch back.\r\n" );
}

