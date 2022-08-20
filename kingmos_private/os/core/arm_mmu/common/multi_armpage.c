/******************************************************
Copyright(c) 版权所有，1998-2004微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：页调度功能，负责进程的页调入、调出。这是一个新的版本，
		之前的是用固定的系统页数，所有的线程共享这些页（内存利用高）。
		目前的是为每一个线程分配一个页
版本号：2.5.0
开发时期：2000
作者：李林
修改记录：
	
******************************************************/

#include <eframe.h>

#include <cpu.h>

#include <pagemgr.h>
#include <coresrv.h>
#include <virtual.h>
#include <epalloc.h>
#include <epcore.h>
#include "sysmem.h"


//#define MAX_SLOTS 1
//#define PAGE_TABLES 8
void ClearThreadPageTable ( LPTHREAD_PAGE_TABLE lpPageTable );

//#define INVALID_FIRST_LEVEL_ENTRY (-1)
//#define INVALID_SECOND_LEVEL_ENTRY (-1)
/*
typedef struct _PAGE_TABLE_SLOTS
{
	struct _PAGE_TABLE_SLOTS * lpNext;
	UINT uiFirstLevelEntry;  //该页表对应（代表）的第一级, -1 代表无效。
	UINT uiPageSlotsIndex;	 //该页表当前的设置的进入点对应（代表）的有效 4K位置，-1 代表无效。
	volatile DWORD * lpdwVirtualPageTable; //页表 1k ( 256 * 4k)
	volatile DWORD * lpdwPhyPageTable; //物理页表 1k ( 256 * 4k)
}PAGE_TABLE_SLOTS;
*/

#define GET_SLOT_ADR( dwAdr, dwVABase ) ( (dwAdr) >= 0x2000000 ? (dwAdr) : ( (dwAdr) | (dwVABase) ) )
//#define GET_FIRST_LEVEL_ENTRY( dwAdr, dwVABase ) ( GET_SLOT_ADR( (dwAdr), (dwVABase) ) >> 20 )
#define GET_FIRST_LEVEL_ENTRY( dwAdr, dwVABase ) ( (dwAdr) >> 20 )
#define GET_SECOND_LEVEL_ENTRY( dwAdr ) ( ( (dwAdr) >> 12 ) & 0xff ) 
//static PAGE_TABLE_INFO pti;
#define FirstPageTableEntryAdr( idxEntry )  ( (DWORD*)( lpdwFirstPTE + (idxEntry) ) )
//UINT * FirstPageTableEntryAdr( int idxEntry )
//{
//	return lpdwFirstPTE + idxEntry;
//}
//#define SecondPageTableEntryAdr( idxPage, idxEntry ) ( (UINT*)( lpdwSecondPTE + ( (idxPage) << 8) + (idxEntry) ) )
//UINT * SecondPageTableEntryAdr( int idxPage, int idxEntry )
//{
//	return lpdwSecondPTE + idxPage * 256 + idxEntry;
//}
//static PAGE_TABLE_SLOTS * lpFreeSysPTS = NULL;
//static PAGE_TABLE_SLOTS * lpUsedSysPTS;

#define MAX_CACHE_PAGES 32
//#define PROCESS_CACHE_PAGES (MAX_CACHE_PAGES-1)
//#define SYS_CACHE_PAGE_INDEX (MAX_CACHE_PAGES-1)
static THREAD_PAGE_TABLE pageTable[MAX_CACHE_PAGES];

//系统需要的函数，得到页描述符的有效地址占位
DWORD GetPhyPageAdrMask( void )
{
	return PG_PHYS_ADDR_MASK;
}

//系统需要的函数，得到页描述符的有效标志占位
DWORD GetPhyPageFlagMask( void )
{
	return PGF_MASK;
}

VOID FreeCPUPTS( VOID * lpPTS )
{
	ASSERT( (DWORD)lpPTS == 0xffff0000 );
}

VOID * AllocCPUPTS( VOID )
{
	return (VOID *)0xffff0000;
}

#define DEBUG_InitThreadPageTable 1
VOID InitThreadPageTable( LPTHREAD lpThread )
{	//因为该线程结构是4k,最后1k作为 page table
	if( pageTable[0].lpdwVirtualPageTable == NULL )
	{
		int i, j;
		DEBUGMSG( DEBUG_InitThreadPageTable, ( "InitThreadPageTable: lpdwFirstPTE=0x%x,entry.\r\n", lpdwFirstPTE ) );
		for( i = 0; i < MAX_CACHE_PAGES; )
		{
			DWORD dwPageAdr = (DWORD)KHeap_Alloc( PAGE_SIZE );
			for( j = 0; j < 4 && i < MAX_CACHE_PAGES; j++, i++, dwPageAdr += 1024 )
			{
				pageTable[i].lpdwVirtualPageTable = (LPDWORD)dwPageAdr;
				pageTable[i].lpdwPhyPageTable = (LPDWORD)( (DWORD)_GetPhysicalAdr( dwPageAdr ) | 1 ); // 1 mean second page
				pageTable[i].lpdwVirtualPageTable = (LPDWORD)CACHE_TO_UNCACHE(dwPageAdr);
				pageTable[i].lpdwFirstLevelEntry = FirstPageTableEntryAdr(i);				
				pageTable[i].lpdwSecondLevelEntry = pageTable[i].lpdwVirtualPageTable;//-1;
				*pageTable[i].lpdwFirstLevelEntry = (DWORD)pageTable[i].lpdwPhyPageTable;
				RETAILMSG( 1, ( "pageTable[%d].lpdwFirstLevelEntry=0x%x.\r\n", i, pageTable[i].lpdwFirstLevelEntry ) );
			}
		}
		DEBUGMSG( DEBUG_InitThreadPageTable, ( "InitThreadPageTable: leave.\r\n" ) );
	}
} 

VOID DeinitThreadPageTable( LPTHREAD lpThread )
{
	//	ClearThreadPageTable( &lpThread->pageTable );	
}

//系统需要的函数，得到页描述符的有效标志 sys logic flag - > cpu flag
DWORD GetAttribFromProtect( DWORD dwProtect )
{
	DWORD dwAttrib = 0;

	switch( dwProtect & ~(PAGE_GUARD | PAGE_NOCACHE) )
	{
	case PAGE_READONLY:
		dwAttrib = PGF_VALID | PGF_READ;
		break;
	case PAGE_READWRITE:
		dwAttrib = PGF_VALID | PGF_READWRITE;
		break;
	case PAGE_EXECUTE:
	case PAGE_EXECUTE_READ:
		dwAttrib = PGF_VALID | PGF_READ | PGF_EXECUTE;
		break;
    case PAGE_EXECUTE_READWRITE:
        dwAttrib = PGF_VALID | PGF_EXECUTE | PGF_READWRITE;
        break;
	default:
		RETAILMSG( 1, ( " error in GetAttribFromProtect: no any protect attrib!.\r\n" ) );
        KL_SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
	}
    dwAttrib |= (dwProtect & PAGE_NOCACHE) ? PGF_NOCACHE : PGF_CACHE;
	return dwAttrib;//0x55e;
}

//系统需要的函数，得到页描述符的有效标志 cpu flag - > sys logic flag
DWORD GetProtectFromAttrib( DWORD dwPageAttrib )
{
	DWORD dwProtect = 0;

	dwPageAttrib &= PGF_MASK;
	if( (dwPageAttrib & PGF_CACHE) == 0 )
	{
		dwProtect |= PAGE_NOCACHE;
	}
	if( (dwPageAttrib & PGF_READWRITE ) == 0 )
	{
		dwProtect |= PAGE_READONLY;
	}
	else
		dwProtect |= PAGE_READWRITE;

	return dwProtect;
}

//无效当前系统的所有的页

//void ClearThreadPageTable( LPTHREAD_PAGE_TABLE lpCurPTS )
//{
	/*
	UINT * lpui;
	UINT uiFirstLevelEntry;

	if( lpCurPTS )
	{
		*lpCurPTS->lpdwFirstLevelEntry = 0;
		lpCurPTS->lpdwFirstLevelEntry = FirstPageTableEntryAdr(0);
	}
	*/
//}


//无效当前系统的所有的页
static void ClearPageTable ( LPTHREAD lpThread )// PAGE_TABLE_SLOTS * lpCurPTS )
{
	if( pageTable[0].lpdwFirstLevelEntry )
	{
		int i;
		LPTHREAD_PAGE_TABLE lpCurPTS = &pageTable[0];
		
		for( i = 0; i < MAX_CACHE_PAGES; i++ ,lpCurPTS++ )
		{
			//*lpCurPTS->lpdwSecondLevelEntry = 0;
			*lpCurPTS->lpdwFirstLevelEntry = 0;
		}		
		
//		for( ; i < MAX_CACHE_PAGES; i++ ,lpCurPTS++ )
//		{
//			*lpCurPTS->lpdwFirstLevelEntry = 0;
//		}		
	}
}  

#define DEBUG_FlushCacheAndClearTLB 0
void FlushCacheAndClearTLB( void )
{
	UINT uiSave;

	DEBUGMSG( DEBUG_FlushCacheAndClearTLB, ( "FlushCacheAndClearTLB++.\r\n" ) );
	LockIRQSave( &uiSave );


    FlushDCache();
    FlushICache();
	ClearPageTable( lpCurThread );
    TLBClear();     // clear h/w TLBs

	UnlockIRQRestore( &uiSave );
	DEBUGMSG( DEBUG_FlushCacheAndClearTLB, ( "FlushCacheAndClearTLB--.\r\n" ) );    	
}

//void GetMMUContext( LPTHREAD lpThread, UINT flag )
void GetMMUContext( LPTHREAD lpThread, UINT uiDebugFlag, LPPROCESS lpNewProcess )
{
//#ifdef VIRTUAL_MEM
	LPPROCESS lpProcess;

	lpProcess = lpThread->lpCurProcess = lpNewProcess;

	if( lpProcess != &InitKernelProcess )  // LN, 2003-06-04, ADD
	    lppProcessSegmentSlots[0] = lpProcess->lpSegIndex;//->lpSeg;
	else              // LN, 2003-06-04, ADD
		lppProcessSegmentSlots[0] = 0;         // LN, 2003-06-04, ADD

	SetCPUId( lpProcess->dwVirtualAddressBase );
    
	
	ClearPageTable( lpThread );  //好象只需要清除当前线程的?
    TLBClear();

#ifdef __DEBUG
	if( lpThread->lpCurProcess->dwVirtualAddressBase != 0 && 
		lpThread->lpCurProcess->dwVirtualAddressBase !=  GetCPUId() )
	{
		if( uiDebugFlag != 0 )
		{
		    RETAILMSG( 1, ("ERROR:flag=%d,lpProcess(%x)->dwVirtualAddressBase=%x,lpThread->lpCurProcess(%x)->dwVirtualAddressBase=%x.\r\n", uiDebugFlag, lpProcess, lpProcess->dwVirtualAddressBase,lpThread->lpCurProcess, lpThread->lpCurProcess->dwVirtualAddressBase ) );
		}
	}
#endif	

//#endif
}

// call by data abort or prefetch abort
#define COUNT_HIT
//#define CHECK_LoadFailurePage
#define DEBUG_LoadFailurePage 0


int LoadFailurePage( DWORD dwAddress, UINT uFailurePC ) 
{
	extern LPSEGMENT Seg_FindSegment( DWORD dwAddress );

	UINT uiFirstLevelEntry, uiSecondLevelEntry;
	LPSEGMENT lpSeg;
	LPMEMBLOCK lpBlk;
	UINT uiPhyAdr;
	DWORD dwSegIndex;
	ACCESS_KEY aky;
	UINT oneMegIndex;
	
	dwSegIndex = GET_SEGMENT_INDEX( dwAddress );	
	uiFirstLevelEntry = GET_FIRST_LEVEL_ENTRY( dwAddress, lpCurThread->lpCurProcess->dwVirtualAddressBase );
	uiSecondLevelEntry = GET_SECOND_LEVEL_ENTRY( dwAddress );
	oneMegIndex = uiFirstLevelEntry & 0x1f;
	


//	RETAILMSG( 1, ( "p.\r\n" ) );

//	ASSERT( lpCurThread != lpInitKernelThread );
#ifdef COUNT_HIT
	//////////FOR TEST///////////

	static UINT uiTotalCount = 0;
	static UINT uiFirstOkCount = 0;
	static UINT uiFirstOkCacheCount = 0;
	static UINT uiTickCount = 0;	
//	static UINT uiProcessCount = 0;
	static UINT uiProcessIndexCount[32];
/////////////////////

	//////////FOR TEST///////////

	uiTotalCount++;

//	if( dwSegIndex < 32 )
//	{
//		uiProcessCount++;
		uiProcessIndexCount[oneMegIndex]++;
//	}
		
	if( uiTotalCount == 500000 )
	{
		int idx = 0;
		DWORD dwCur = KL_GetTickCount();
		RETAILMSG( 1, ( "perform:uiTotal/tick=%d,tickDiff=%d.\r\n", uiTotalCount / ( ( dwCur - uiTickCount ) / 1000 ), dwCur - uiTickCount  ) );
		
		while( idx < 32 )
		{
			if( uiProcessIndexCount[idx] )
			{
				RETAILMSG( 1, ( "idx=%d,count=%d.\r\n", idx,  uiProcessIndexCount[idx] ) );
				uiProcessIndexCount[idx] = 0;
			}
			idx++;
		}	 

		uiTotalCount = 0;
		uiFirstOkCount = 0;
		uiFirstOkCacheCount = 0;
//		uiProcessCount = 0;
		uiTickCount = KL_GetTickCount();
	}
	////////////////////////////
#endif

#ifdef CHECK_LoadFailurePage
	{
		DWORD dwVirtualAddressBase = lpCurThread->lpCurProcess->dwVirtualAddressBase;
		if( dwVirtualAddressBase != 0 && dwVirtualAddressBase !=  GetCPUId() )
		{
			ASSERT( 0 );
			RETAILMSG( 1, ( "lpCurProcess->lpName=%s,lpCurProcess->dwVirtualAddressBase=%x,GetCPUId()=%x,Address=%x.\r\n", lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->dwVirtualAddressBase, GetCPUId(), dwAddress ) );
		}
	}
	ASSERT( lpCurPTS );
#endif


		
	lpSeg = Seg_FindSegment( dwAddress );

//	if( lpSeg &&
//		dwSegIndex < MAX_PROCESSES &&
//		( 
//		  dwSegIndex == 0 ||
//		  ( ( aky = (1 << dwSegIndex) ) & lpCurThread->akyAccessKey  )
//		)
//	  )
	if( lpSeg &&
		(
		  ( ( aky = (1 << dwSegIndex) ) & lpCurThread->akyAccessKey ) ||
		  dwSegIndex == SHARE_SEGMENT_INDEX ||
		  dwSegIndex == KERNEL_SEGMENT_INDEX		 
		)
	  )
	{
_REPEAT:
		if( ( lpBlk = lpSeg->lpBlks[BLOCK_INDEX(dwAddress)] ) > RESERVED_BLOCK &&
			( uiPhyAdr = lpBlk->uiPages[PAGE_INDEX(dwAddress)] ) )
		{   // the page has been commit
			register LPTHREAD_PAGE_TABLE lpCurPTS;// = &pageTable;//&lpCurThread->pageTable;
			

			
//			if( dwSegIndex < PROCESS_CACHE_PAGES )
//				lpCurPTS = &pageTable[dwSegIndex];
//			else
//				lpCurPTS = &pageTable[SYS_CACHE_PAGE_INDEX];
			lpCurPTS = &pageTable[oneMegIndex];
				
			
			*lpCurPTS->lpdwSecondLevelEntry = 0;
			lpCurPTS->lpdwSecondLevelEntry = lpCurPTS->lpdwVirtualPageTable + uiSecondLevelEntry;
			*lpCurPTS->lpdwSecondLevelEntry = uiPhyAdr;

			*lpCurPTS->lpdwFirstLevelEntry = 0;
			lpCurPTS->lpdwFirstLevelEntry = FirstPageTableEntryAdr( uiFirstLevelEntry );
			*lpCurPTS->lpdwFirstLevelEntry = (DWORD)lpCurPTS->lpdwPhyPageTable;		

		    return TRUE;
		}
		else
		{
			//RETAILMSG( 1, ( "autocommit ?.\r\n" ) );
			if( lpBlk > RESERVED_BLOCK && 
				(lpBlk->uiFlag & MF_AUTO_COMMIT) && 
				uiPhyAdr == 0 )
			{
				extern BOOL AutoCommitPage( LPMEMBLOCK lpBlk, int idxStartPage );
				RETAILMSG( 1, ( "autocommit page:lpBlk=%x,dwAddress=%x.\r\n", lpBlk, dwAddress ) );
				if( AutoCommitPage( lpBlk, PAGE_INDEX(dwAddress) ) == TRUE )
				{
					goto _REPEAT;
				}
			}
			RETAILMSG( 1, ( "page error:lpBlk=%x,uiPhyAdr=%x.\r\n", lpBlk, uiPhyAdr ) );
		}
		//RETAILMSG( 1, ( "page4.\r\n" ) );
_error_return:
		{		
			UINT index = dwAddress >> 20;
	        RETAILMSG( 1, ( "page_error: dwAddress=0x%x,pc=0x%x,dwSegIndex=%d,lpSeg=%x,lpBlk=%x,uiPhyAdr=%x,ProcessName=%s,lpProcessSegIndex=%x,lpPTE[%d]=%x,cur sp(0x%x),OwnerProces=(%s),ThreadId(0x%x),ThreadHandle(0x%x),VM=0x%x.\r\n", 
							dwAddress, uFailurePC, dwSegIndex, lpSeg, lpBlk, uiPhyAdr, lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->lpSegIndex, index, lpdwFirstPTE[index], &index, 
							lpCurThread->lpOwnerProcess->lpszApplicationName, lpCurThread->dwThreadId, lpCurThread->hThread, lpCurThread->lpCurProcess->dwVirtualAddressBase ) );	
		}
	}
	else{
		UINT index = dwAddress >> 20;
        RETAILMSG( 1, ( "page_error: invalid lpSeg=0x%x,dwAddress=0x%x,pc=0x%x,dwSegIndex=%d,ProcessName=%s,lpProcessSegIndex=%x,lpPTE[%d]=%x,cur sp(0x%x),OwnerProces=(%s),ThreadId(0x%x),ThreadHandle(0x%x),aky=0x%x,VM=0x%x.\r\n", 
			lpSeg, dwAddress, uFailurePC, dwSegIndex, lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->lpSegIndex, index, lpdwFirstPTE[index], &index, 
			lpCurThread->lpOwnerProcess->lpszApplicationName, lpCurThread->dwThreadId, lpCurThread->hThread,lpCurThread->akyAccessKey, lpCurThread->lpCurProcess->dwVirtualAddressBase ) );
        RETAILMSG( 1, ( "page_error: CurAky=0x%x,CurProcessAky=0x%x,OwnerProcessAky=0x%x.\r\n",
			                         lpCurThread->akyAccessKey,
			                         lpCurThread->lpCurProcess->akyAccessKey,
			                         lpCurThread->lpOwnerProcess->akyAccessKey ) );
			
	}
    return FALSE;
}

//int LoadPrefetchFailurePage( DWORD dwAddress ) 
//{
//	int retv;
//	retv = LoadFailurePage( dwAddress );
//	RETAILMSG( 1, ( "LoadPrefetchFailurePage1:dwAddress=%x,retv=%d.\r\n",dwAddress,retv ) );
//	return retv;
//}

//LPVOID AllocThreadKernelStackPage( VOID )
//{
//	if( lpCurThread->lpdwThreadKernelStack == NULL )
//	{
//		lpCurThread->lpdwThreadKernelStack = KHeap_Alloc( PAGE_SIZE );
//	}
//	return lpCurThread->lpdwThreadKernelStack;
//}
