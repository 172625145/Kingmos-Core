#include <eframe.h>
#include <ecore.h> 

#include <eassert.h>
#include <epcore.h>
// put it to arch
void InitThreadTSS(
     LPTHREAD lpThread,
     LPTHREAD_START_ROUTINE lpStartAdr,
     LPBYTE lpStack,
     LPVOID lpParameter )
{
	memset( &lpThread->tss, 0, sizeof(lpThread->tss) );

	EdbgOutputDebugString( "InitThreadTSS lpThread=%x, lpStartAdr=%x, lpStack=%x, lpParameter=%x\r\n", lpThread, lpStartAdr, lpStack, lpParameter );
	lpThread->tss.r14 = (UINT)lpStartAdr;
	lpThread->tss.pc = (UINT)lpStartAdr;
	lpThread->tss.r13 = (UINT)lpStack;// + dwSize - 4;
	lpThread->tss.r0 = (UINT)lpParameter;		
	lpThread->tss.cpsr = 0x0000001f;//SYS_MODE//13;//0x0000001f;//0x00000013;//0x0000001f;//0x00000013;//0x1f;


/*
#ifdef EML_DOS
    WORD * p;
     p = (WORD*)(lpStack + dwSize);
     *((DWORD*)(p-4)) = (DWORD)ExitThread;
     *(p-1) = 0;
     lpThread->tss.ip = (WORD)lpStartAdr;
     lpThread->tss.cs = (WORD)((DWORD)lpStartAdr>>16);
     lpThread->tss.sp = (WORD)((DWORD)(p-4));
     lpThread->tss.ss = (WORD)(((DWORD)(p-4))>>16);
#endif

#ifdef EML_WIN32
     int * p;
     p = (int*)(lpStack + dwSize);
     *(p-1) = 0;// exit code for ExitThread
     *((DWORD*)(p-2)) = (DWORD)lpParameter;
     *((DWORD*)(p-3)) = (DWORD)ExitThread;
     lpThread->tss.eip = (DWORD)lpStartAdr;
     lpThread->tss.esp = (DWORD)(p-3);
     {
     
         TSS * p = &lpThread->tss;
         _asm
         {
             mov eax, p
             mov WORD PTR [eax+REG_SS], ss
             mov WORD PTR [eax+REG_CS], cs
         }
     }
#endif
*/
}

DWORD SetThreadIP( LPTHREAD lpThread, DWORD dwIP )
{
    DWORD dwOld = lpThread->tss.r14;
	lpThread->tss.r14 = dwIP;
	return dwOld;
}