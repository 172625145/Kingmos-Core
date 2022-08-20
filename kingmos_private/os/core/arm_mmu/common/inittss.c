#include <eframe.h>
#include <ecore.h> 
#include <eucore.h>

#include <eassert.h>
#include <epcore.h>
#include <coresrv.h>

#include <epalloc.h>

// put it to arch
void InitThreadTSS(
     LPTHREAD lpThread,
     LPTHREAD_START_ROUTINE lpStartAdr,
     LPBYTE lpStack,
     LPVOID lpParameter )
{
	memset( &lpThread->tss, 0, sizeof(lpThread->tss) );

//	EdbgOutputDebugString( "InitThreadTSS lpThread=%x, lpStartAdr=%x, lpStack=%x, lpParameter=%x\r\n", lpThread, lpStartAdr, lpStack, lpParameter );
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

#define DEBUG_CallUserStartupCode 0
int CallUserStartupCode( LPVOID lpIP, LPVOID lpParam )
{
	extern int _CallInMode( LPVOID , LPVOID lpIP, LPVOID lpsp, UINT mode );
	extern int KC_CaptureException(void);
	UINT mode = GetCPUMode( lpCurThread->lpCurProcess->dwFlags & 0xF );
	CALLSTACK * lpcs;
	
	DEBUGMSG( DEBUG_CallUserStartupCode, ( "CallInMode:lpParam(0x%x),lpIP(0x%x),lpdwThreadUserStack(0x%x),mode(0x%x).\r\n", lpParam, lpIP,lpCurThread->lpdwThreadUserStack, mode ) );
	
	lpcs = (CALLSTACK *)KHeap_Alloc( sizeof( CALLSTACK ) );

	lpcs->dwCallInfo = MAKE_OPTIONINFO( API_KERNEL, THREAD_EXIT ) | CALL_ALLOC;
	lpcs->lpvData = lpCurThread->lpCurProcess;
	lpcs->dwStatus = GetCPUMode( M_SYSTEM );
	lpcs->pfnRetAdress = KL_ExitThread;
	lpcs->akyAccessKey = lpCurThread->akyAccessKey | InitKernelProcess.akyAccessKey;
	
	// 保存调用链
	lpcs->lpNext = lpCurThread->lpCallStack;	
	lpCurThread->lpCallStack = lpcs;
	//永不返回
//	_CallInMode( lpParam, lpIP, lpCurThread->lpdwThreadUserStack, mode );
		    
		
	if( KC_CaptureException() == 0 )
	{
	    _CallInMode( lpParam, lpIP, lpCurThread->lpdwThreadUserStack, mode );
	    while(1)
	    {
    		ERRORMSG( DEBUG_CallUserStartupCode, ( "CallInMode:why to here.\r\n" ) );	//应该不会到这里
	    }	    
	}
	else
	{	// 一个无法解决的异常产生，这里只能做退出处理
		ERRORMSG( DEBUG_CallUserStartupCode, ( "CallInMode: sys capture a exception!.\r\n" ) );	//应该不会到这里
		KL_ExitThread(-1);
	}
	
//	( ( LPTHREAD_START_ROUTINE )lpIP )( lpParam );
}

// ********************************************************************
//声明：UINT GetCPUMode( DWORD dwMode )
//参数：
//	IN dwMode - 模式号，为以下值：
//				M_SYSTEM - 系统模式
//				M_USER - 用户模式
//				M_KERNEL - 内核模式

//返回值：
//	假如成功，返回CPU对应的模式；失败，返回-1
//功能描述：
//	得到CPU依赖的模式
//引用：
//	
// ********************************************************************
#define DEBUG_GETCPUMODE 0
UINT GetCPUMode( DWORD dwMode )
{
	switch( dwMode )
	{
	case M_SYSTEM:
		return 0x1f;
	case M_USER:
		return 0x10;
	default:
		ERRORMSG( DEBUG_GETCPUMODE, ( "error in GetCPUMode:dwMode=0x%x.\r\n", dwMode ) );
		ASSERT(0); //error
	}
	return -1;
}
