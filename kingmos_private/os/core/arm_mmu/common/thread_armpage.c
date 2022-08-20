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

/*
//分配PTS
#define DEBUG_AllocCPUPTS 1
LPVOID AllocCPUPTS( void )
{	
	UINT uiSave;	
	
_REPEAT_ALLOC:
	LockIRQSave( &uiSave );
	if( lpFreeSysPTS )
	{
		PAGE_TABLE_SLOTS * lpret;
		lpret = lpFreeSysPTS;
		lpFreeSysPTS = lpret->lpNext;
		lpret->lpNext = NULL;
		UnlockIRQRestore( &uiSave );
		return lpret;
	}
	else
	{
		PAGE_TABLE_SLOTS * lpNew;
		int n;
		DWORD dwVirtualPageAdr, dwPhyPageAdr;

		UnlockIRQRestore( &uiSave );

		n = PAGE_SIZE / PAGE_TABLE_SIZE;
		dwVirtualPageAdr = (DWORD)KHeap_Alloc( PAGE_SIZE );
		DEBUGMSG( DEBUG_AllocCPUPTS, ( " AllocCPUPTS: dwVirtualPageAdr=0x%x,n=%d.\r\n", dwVirtualPageAdr, n ) );
		
		if( dwVirtualPageAdr )
		{
			memset( (LPVOID)dwVirtualPageAdr, 0, PAGE_SIZE );
			dwPhyPageAdr = _GetPhysicalPageAdr( dwVirtualPageAdr );
			DEBUGMSG( DEBUG_AllocCPUPTS, ( " AllocCPUPTS: dwPhyPageAdr=0x%x.\r\n", dwPhyPageAdr ) );
			lpNew = (PAGE_TABLE_SLOTS *)KHeap_Alloc( sizeof( PAGE_TABLE_SLOTS ) * n );
			if( lpNew )
			{
				int i;
				
				DEBUGMSG( DEBUG_AllocCPUPTS, ( " AllocCPUPTS: lpNew=0x%x.\r\n", lpNew ) );
				memset( lpNew, 0, sizeof( PAGE_TABLE_SLOTS ) * n );
				for( i = 0; i < n; i++ )
				{
					lpNew[i].uiFirstLevelEntry = INVALID_FIRST_LEVEL_ENTRY;
					lpNew[i].uiPageSlotsIndex = INVALID_SECOND_LEVEL_ENTRY;
					lpNew[i].lpdwVirtualPageTable = (LPDWORD)(dwVirtualPageAdr + i * PAGE_TABLE_SIZE);
					lpNew[i].lpdwPhyPageTable = (LPDWORD)(dwPhyPageAdr + i * PAGE_TABLE_SIZE);
					lpNew[i].lpNext = &lpNew[i+1];
				}
				{
					UINT uiSaveIRQ;

					LockIRQSave( &uiSaveIRQ );

					lpNew[n-1].lpNext = lpFreeSysPTS;
					lpFreeSysPTS = &lpNew[0];

					UnlockIRQRestore( &uiSaveIRQ );
				}
				goto _REPEAT_ALLOC;
			}
			else
			{
				KHeap_Free( (LPVOID)dwVirtualPageAdr, PAGE_SIZE );
			}
		}
		return NULL;
	}
}
//释放PTS
VOID FreeCPUPTS( void * lpPTS )
{
	UINT uiSave;
	PAGE_TABLE_SLOTS * lpFree  = ( PAGE_TABLE_SLOTS * )lpPTS;
	ASSERT( lpFree->lpNext == NULL );
	
	LockIRQSave( &uiSave );

	lpFree->lpNext = lpFreeSysPTS;
	lpFreeSysPTS = lpFree;

	UnlockIRQRestore( &uiSave );
}
*/
#define DEBUG_InitThreadPageTable 0
VOID InitThreadPageTable( LPTHREAD lpThread )
{	//因为该线程结构是4k,最后1k作为 page table
	DWORD dwPageAdr = (DWORD)lpThread;
	dwPageAdr = ( dwPageAdr + 4 * 1024 - 1 ) & ~(1024-1); //用第3k
	
	lpThread->pageTable.lpdwVirtualPageTable = (LPDWORD)dwPageAdr;
	lpThread->pageTable.lpdwPhyPageTable = (LPDWORD)( (DWORD)_GetPhysicalAdr( dwPageAdr ) | 1 ); // 1 mean second page
	//memset( (LPVOID)lpThread->pageTable.lpdwVirtualPageTable, 0, 1024 );
	lpThread->pageTable.lpdwVirtualPageTable = (LPDWORD)CACHE_TO_UNCACHE(lpThread->pageTable.lpdwVirtualPageTable);
	lpThread->pageTable.lpdwFirstLevelEntry = FirstPageTableEntryAdr(0);
	lpThread->pageTable.lpdwSecondLevelEntry = lpThread->pageTable.lpdwVirtualPageTable;//-1;

	DEBUGMSG( DEBUG_InitThreadPageTable, ( "InitThreadPageTable:lpThread=0x%x,vir=0x%x,phy=0x%x.\r\n", lpThread, lpThread->pageTable.lpdwVirtualPageTable, lpThread->pageTable.lpdwPhyPageTable ) );
} 

VOID DeinitThreadPageTable( LPTHREAD lpThread )
{
	ClearThreadPageTable( &lpThread->pageTable );
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
void ClearThreadPageTable( LPTHREAD_PAGE_TABLE lpCurPTS )
{
	UINT * lpui;
	UINT uiFirstLevelEntry;

	//while( lpCurPTS )
	//ASSERT( lpCurPTS );
	if( lpCurPTS )
	{
		*lpCurPTS->lpdwFirstLevelEntry = 0;
		lpCurPTS->lpdwFirstLevelEntry = FirstPageTableEntryAdr(0);

		//ASSERT( lpCurPTS->uiFirstLevelEntry < 4096 );
		//*FirstPageTableEntryAdr( lpCurPTS->uiFirstLevelEntry ) = 0;  //CPU属性
		//lpCurPTS->uiFirstLevelEntry = 0;
/*
		if( (uiFirstLevelEntry = lpCurPTS->uiFirstLevelEntry) != INVALID_FIRST_LEVEL_ENTRY )
		{
			*FirstPageTableEntryAdr( uiFirstLevelEntry ) = 0;  //CPU属性
			lpCurPTS->uiFirstLevelEntry = INVALID_FIRST_LEVEL_ENTRY;			
		}
		//lpCurPTS = lpCurPTS->lpNext;
*/
	}
}

//无效当前系统的所有的页
static void ClearPageTable ( LPTHREAD lpThread )// PAGE_TABLE_SLOTS * lpCurPTS )
{
	if( lpThread->pageTable.lpdwFirstLevelEntry )
		ClearThreadPageTable( &lpThread->pageTable );
	//ClearProcessPageTable( lpCurThread->lpOwnerProcess->lpCpuPTS );
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
//#define COUNT_HIT
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


//	RETAILMSG( 1, ( "p.\r\n" ) );

//	ASSERT( lpCurThread != lpInitKernelThread );
#ifdef COUNT_HIT
	//////////FOR TEST///////////

	static UINT uiTotalCount = 0;
	static UINT uiFirstOkCount = 0;
	static UINT uiFirstOkCacheCount = 0;
	static UINT uiTickCount = 0;	
/////////////////////

	//////////FOR TEST///////////

	uiTotalCount++;
	if( uiTotalCount == 500000 )
	{
		RETAILMSG( 1, ( "perform:uiTotal/tick=%d.\r\n", uiTotalCount / ( ( KL_GetTickCount() - uiTickCount ) / 1000 ) ) );
		uiTotalCount = 0;
		uiFirstOkCount = 0;
		uiFirstOkCacheCount = 0;
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

	dwSegIndex = GET_SEGMENT_INDEX( dwAddress );
		
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
			register LPTHREAD_PAGE_TABLE lpCurPTS = &lpCurThread->pageTable;
			uiFirstLevelEntry = GET_FIRST_LEVEL_ENTRY( dwAddress, lpCurThread->lpCurProcess->dwVirtualAddressBase );
			uiSecondLevelEntry = GET_SECOND_LEVEL_ENTRY( dwAddress );
				
			//if( lpCurPTS->uiPageSlotsIndex != INVALID_SECOND_LEVEL_ENTRY )
			//{
				//lpCurPTS->lpdwVirtualPageTable[ lpCurPTS->uiPageSlotsIndex ] = 0;  //clear
			*lpCurPTS->lpdwSecondLevelEntry = 0;//FirstPageTableEntryAdr(0)
			lpCurPTS->lpdwSecondLevelEntry = lpCurPTS->lpdwVirtualPageTable + uiSecondLevelEntry;
			*lpCurPTS->lpdwSecondLevelEntry = uiPhyAdr;
			//}
			//lpCurPTS->lpdwVirtualPageTable[lpCurPTS->uiPageSlotsIndex = uiSecondLevelEntry] = uiPhyAdr;
			
			//if( lpCurPTS->uiFirstLevelEntry != INVALID_FIRST_LEVEL_ENTRY )
			//{
				//*FirstPageTableEntryAdr(lpCurPTS->uiFirstLevelEntry) = 0; //clear
			//}
			//*FirstPageTableEntryAdr( lpCurPTS->uiFirstLevelEntry = uiFirstLevelEntry ) = (DWORD)lpCurPTS->lpdwPhyPageTable;  // 1 mean second level 
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

