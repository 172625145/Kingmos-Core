/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：cpu模拟器
版本号：2.0.0
开发时期：2000
作者：李林
修改记录：
******************************************************/
//typedef void * HKEY;
#include <windows.h>
#include <eversion.h>
//#include <ewindows.h>
#include <w32intr.h>
//#include <ekeydrv.h>
#include <cpu.h>
#include <crtdbg.h>
/*
typedef struct _KERNEL_DATA
{
	_INTRBITS intrBits;
	_INTRMASK intrMask;
	int IntrEnable;
	MSG IntrMsg[32];
	MSG msgCurrent;
	HANDLE hKingmosThread;
	HANDLE hIntrThread;
	HANDLE hHardwareEvent;
	CONTEXT  ctSave;
	UINT uiLockSegStart;
	UINT uiLockSegEnd;
	CRITICAL_SECTION csMsg;
	LONG nLockCount;
}KERNEL_DATA, FAR * LPKERNEL_DATA;
*/
volatile LPKERNEL_DATA lpKernelData;

static void CPU_GetIRQEvent( void );

extern UINT LockSeg_Start( void );
extern UINT LockSeg_End( void );
extern void INTR_Interrupt( void );
extern void KingmosStart( LPVOID );

void CPU_IRQTrap( void );
extern void JmpBack( TSS *, volatile int * lpIntrEnable );

//VOID ReleaseKernelData( void )
//{
//	LONG lOldProtect;
//	if( InterlockedDecrement( &lpKernelData->nLockCount ) == 0 )
//	    VirtualProtect( lpKernelData, 1024 * 4, PAGE_NOACCESS, &lOldProtect );	
//}

//VOID GetKernelData( void )
//{
//	LONG lOldProtect;
//	if( InterlockedIncrement( &lpKernelData->nLockCount ) > 0 )
//	    VirtualProtect( lpKernelData, 1024 * 4, PAGE_READWRITE, &lOldProtect );
//	else
//	{
//		_ASSERT( 0 );
//	}	
//}

void UnlockIRQRestore( UINT * lpuiSave )
{
//    GetKernelData();
	InterlockedExchange( (LPVOID)&lpKernelData->IntrEnable, *lpuiSave );
//	ReleaseKernelData();
}

void LockIRQSave( UINT * lpuiSave )
{
  //  GetKernelData();
	*lpuiSave = (UINT)InterlockedExchange( (LPVOID)&lpKernelData->IntrEnable, FALSE );
	//ReleaseKernelData();
}

#ifdef __DEBUG

static char * lpFileName;
static int iLine;
void _INTR_OFF( char * lpfn, int line )
{  
	lpFileName = lpfn;
	iLine = line;

	//GetKernelData();
	InterlockedExchange( (LPLONG)&lpKernelData->IntrEnable, FALSE );
	//ReleaseKernelData();

	lpFileName = lpfn;
	iLine = line;
}

void _INTR_ON( char * lpfn, int line )
{
	//GetKernelData();
	InterlockedExchange( (LPLONG)&lpKernelData->IntrEnable, TRUE );
	//ReleaseKernelData();
}

#else

void INTR_OFF( void )
{
	//GetKernelData();
	InterlockedExchange( (LPLONG)&IntrEnable, FALSE );
	//ReleaseKernelData();
}

void INTR_ON( void )
{
	//GetKernelData();
	InterlockedExchange( (LPLONG)&IntrEnable, TRUE );
	//ReleaseKernelData();
}

#endif



void CPU_SetIRQEvent( MSG * lpmsg )
{
	EnterCriticalSection( &lpKernelData->csMsg );
    //GetKernelData();
	lpKernelData->msgCurrent = *lpmsg;
	CPU_GetIRQEvent();
    SetEvent(lpKernelData->hHardwareEvent);
	//ReleaseKernelData();
	LeaveCriticalSection( &lpKernelData->csMsg );
}

static void CPU_GetIRQEvent( void ) //MSG * lpmsg )
{
	int * lpIntr;
	int * lpMask;
	MSG * lpmsg;
//	UINT uiSave;
//	LockIRQSave( &uiSave );

	EnterCriticalSection( &lpKernelData->csMsg );
    //GetKernelData();

    lpIntr = (int *)&lpKernelData->intrBits;
	lpMask = (int *)&lpKernelData->intrMask;
	lpmsg = &lpKernelData->msgCurrent;
    
    if( lpmsg->message >= WM_MOUSEFIRST &&
		lpmsg->message <= WM_MOUSELAST )
	{
		if( lpmsg->message == WM_MOUSEMOVE && 
			(lpmsg->wParam & MK_LBUTTON) == 0 )
		{
			; // nothing to do
		}
        else
		{
			lpKernelData->intrBits.mouse = 1;        
			lpKernelData->IntrMsg[ID_INTR_MOUSE] = *lpmsg;
		}
    }
    else if( lpmsg->message == WM_KEYDOWN || 
             lpmsg->message == WM_KEYUP )
    {
        lpKernelData->intrBits.keyboard = 1;
        lpKernelData->IntrMsg[ID_INTR_KEYBOARD] = *lpmsg;
    }
    else if( lpmsg->message == WM_CLOSE )
    {
        lpKernelData->intrBits.keyboard = 1;
		lpKernelData->IntrMsg[ID_INTR_KEYBOARD].message = WM_KEYDOWN;
		lpKernelData->IntrMsg[ID_INTR_KEYBOARD].wParam = 0X06;//=VK_POWEROFF;
		lpKernelData->IntrMsg[ID_INTR_KEYBOARD].lParam = 0;
	}

	//ReleaseKernelData();

	LeaveCriticalSection( &lpKernelData->csMsg );

	/*
    else if( lpmsg->message == WM_TIMER )
    {
		intrBits.timer0 = 1;
        IntrMsg[ID_INTR_TIMER1] = *lpmsg;
    }*/

    //if( *lpMask & *lpIntr )
	//{
		//UnlockIRQRestore( &uiSave );
	    //SetEvent(hHardwareEvent);
	//}
	//else
      //  UnlockIRQRestore( &uiSave );


	//Sleep(1);
}
/*
static void IRQ_Handler( void )
{	
	extern void INTR_Interrupt(void);
	int * lpIntr = (int *)&intrBits;
	int * lpMask = (int *)&intrMask;

	if( *lpMask & *lpIntr )
	{
        INTR_Interrupt();
	}
}
*/

// the  CPU_IRQTrap is called by cpu_eml_thread
void CPU_IRQTrap( void )
{
	TSS tss;
	
	//GetKernelData();
	tss.eax = ((CONTEXT*)lpKernelData->bCPUConext)->Eax;
	tss.ebx = ((CONTEXT*)lpKernelData->bCPUConext)->Ebx;
	tss.ecx = ((CONTEXT*)lpKernelData->bCPUConext)->Ecx;
	tss.edx = ((CONTEXT*)lpKernelData->bCPUConext)->Edx;
	tss.edi = ((CONTEXT*)lpKernelData->bCPUConext)->Edi;
	tss.esi = ((CONTEXT*)lpKernelData->bCPUConext)->Esi;
	
	tss.ebp = ((CONTEXT*)lpKernelData->bCPUConext)->Ebp;
	tss.eip = ((CONTEXT*)lpKernelData->bCPUConext)->Eip;
	tss.esp = ((CONTEXT*)lpKernelData->bCPUConext)->Esp;

	tss.ss = (WORD)((CONTEXT*)lpKernelData->bCPUConext)->SegSs;
	tss.cs = (WORD)((CONTEXT*)lpKernelData->bCPUConext)->SegCs;

    tss.gs = (WORD)((CONTEXT*)lpKernelData->bCPUConext)->SegGs;
    tss.fs = (WORD)((CONTEXT*)lpKernelData->bCPUConext)->SegFs;
    tss.es = (WORD)((CONTEXT*)lpKernelData->bCPUConext)->SegEs;
    tss.ds = (WORD)((CONTEXT*)lpKernelData->bCPUConext)->SegDs;


	tss.eflags = ((CONTEXT*)lpKernelData->bCPUConext)->EFlags;

	_ASSERT( tss.eip != (long)CPU_IRQTrap );

	{
		int * lpIntr = (int *)&lpKernelData->intrBits;
		int * lpMask = (int *)&lpKernelData->intrMask;
		
		_ASSERT( *lpIntr && *lpMask );

	}
	_ASSERT( lpKernelData->IntrEnable == 0 );
	//ReleaseKernelData();

    INTR_Interrupt();

	// here to restart the thread
	//
	lpKernelData->IntrEnable = 0;
	JmpBack( &tss, &lpKernelData->IntrEnable );

	_ASSERT(0);

	while(1);

}


#define MEM_SIZE (1024*1024*16)
extern LPBYTE  lpbSysMainMem;// = _mem;
extern ULONG    ulSysMainMemLength;// = MEM_SIZE;
int  __ebp;
extern int fEnter;
BOOL bExitEsoft = 0;
extern HWND hwndDeskTop;


BOOL IsESOFTExit( void )
{
	return bExitEsoft;
}

extern void WINAPI KL_DebugOutString( LPTSTR );
static DWORD WINAPI CPU_StartUpThread( LPVOID lParam )
{
	int win32Stack[MEM_SIZE/sizeof(int)];	
	
	lpbSysMainMem = (LPBYTE)win32Stack;
	ulSysMainMemLength = MEM_SIZE;

	_asm{
		mov  __ebp, ebp
	}

	lpKernelData->intrMask.timer0 = 1;
//	intrMask.timer1 = 1;

	KL_DebugOutString( "begin Kingmos.......\r\n" );

    KingmosStart( lParam );

	return 0;
}

void CPU_Reset( void )
{
    PostMessage( hwndDeskTop, WM_CLOSE, 0, 0 );    
    bExitEsoft = 1;
}

WINBASEAPI
BOOL
WINAPI
IsDebuggerPresent(
    VOID
    );

DWORD WINAPI CPU_InterruptControlerThread( LPVOID lParam )
{
	int * lpIntr = (int *)&lpKernelData->intrBits;
	int * lpMask = (int *)&lpKernelData->intrMask;

	//if( IsDebuggerPresent() )
	//{			
	//}

	while( bExitEsoft == 0 )
	{
		int retv;
		UINT uiSuspendCount;
		retv = WaitForSingleObject( lpKernelData->hHardwareEvent, RESCHED_PERIOD );
		if( bExitEsoft )
			break;
#ifdef KINGMOS_DEMO
	    if( IsDebuggerPresent() )
		    break;
#endif
		
		while( (uiSuspendCount = SuspendThread( lpKernelData->hKingmosThread )) == 0xFFFFFFFF )
			Sleep(1);
		//while( *lpMask & *lpIntr )
		_ASSERT( uiSuspendCount == 0 );
		if( retv == WAIT_TIMEOUT )
		{
		    lpKernelData->intrBits.timer0 = 1;
            lpKernelData->IntrMsg[ID_INTR_TIMER0].message = WM_TIMER;// = *lpmsg;
		}
		//else
			//CPU_GetIRQEvent();

		if( (*lpMask & *lpIntr) && lpKernelData->IntrEnable )
		{
			BOOL bRetv;

			// ok, the thread suspended
			((CONTEXT*)lpKernelData->bCPUConext)->ContextFlags = CONTEXT_FULL;
			bRetv = GetThreadContext( lpKernelData->hKingmosThread, (CONTEXT*)&lpKernelData->bCPUConext );
			_ASSERT( bRetv );
			_ASSERT( ((CONTEXT*)lpKernelData->bCPUConext)->Eip != (UINT)CPU_IRQTrap );
			if( !( ((CONTEXT*)lpKernelData->bCPUConext)->Eip >= (DWORD)lpKernelData->uiLockSegStart &&
				((CONTEXT*)lpKernelData->bCPUConext)->Eip < (DWORD)lpKernelData->uiLockSegEnd ) )
			{
				CONTEXT newct = *( (CONTEXT*)&lpKernelData->bCPUConext );//&lpKernelData->ctSave;
				
				lpKernelData->IntrEnable = FALSE;
				newct.Eip = (UINT)CPU_IRQTrap;
				bRetv = SetThreadContext( lpKernelData->hKingmosThread, &newct );
				_ASSERT( bRetv );
			}
		}
		ResumeThread( lpKernelData->hKingmosThread );
		//Sleep(1);
	}
	return 0;
}

void CPU_Init( void )
{
	DWORD dwId;
	DWORD dwStartAdr = 0x70000000;

	while( lpKernelData == NULL )
	{
	    lpKernelData = (LPKERNEL_DATA)VirtualAlloc( (LPVOID)dwStartAdr, 1024 * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE | PAGE_NOCACHE );
		dwStartAdr -= 0x10000000;
	}
	if( lpKernelData )
	    memset( lpKernelData, 0, 1024 * 4 );
	else
		return;

    InitializeCriticalSection( &lpKernelData->csMsg );

    lpKernelData->uiLockSegStart = LockSeg_Start();
    lpKernelData->uiLockSegEnd = LockSeg_End();

    lpKernelData->hHardwareEvent = CreateEvent(0,FALSE,FALSE,0);
	// create main thread
	lpKernelData->hKingmosThread = CreateThread( 0, MEM_SIZE + 1024 * 10, CPU_StartUpThread, 0, 0, &dwId );
	lpKernelData->hIntrThread = CreateThread( 0, 0, CPU_InterruptControlerThread, 0, 0, &dwId );
	
}

void CPU_Deinit( void )
{
	//CloseHandle( hResetEsoftEvent );
	CloseHandle( lpKernelData->hKingmosThread );
	CloseHandle( lpKernelData->hIntrThread );
	CloseHandle( lpKernelData->hHardwareEvent );
	VirtualFree( lpKernelData, 0, MEM_RELEASE );
	lpKernelData = NULL;
}
