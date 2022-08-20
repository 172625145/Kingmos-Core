/******************************************************
Copyright(c) 版权所有，1998-2004微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：页调度功能，否则进程的页调入、调出。
版本号：2.0.0
开发时期：2000
作者：李林
修改记录：
//2004-09-03,修改 LoadFailurePage	
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
#define PAGE_TABLES 8

typedef struct _PAGE_TABLE_SLOTS
{
	//UINT uiCurPos;	//2004-09-03 delete
	UINT uiFirstLevelEntry;  //该页表对应（代表）的第一级
	UINT uiPageSlotsIndex;//[MAX_SLOTS];  //该页表对应（代表）的有效 4K位置
}PAGE_TABLE_SLOTS;

typedef struct _PAGE_TABLE_INFO
{
	UINT uiCurPos;
	UINT uiActive;

	PAGE_TABLE_SLOTS pts[PAGE_TABLES];
}PAGE_TABLE_INFO;

#define GET_SLOT_ADR( dwAdr, dwVABase ) ( (dwAdr) >= 0x2000000 ? (dwAdr) : ( (dwAdr) | (dwVABase) ) )
#define GET_FIRST_LEVEL_ENTRY( dwAdr, dwVABase ) ( GET_SLOT_ADR( (dwAdr), (dwVABase) ) >> 20 )
#define GET_SECOND_LEVEL_ENTRY( dwAdr ) ( ( (dwAdr) >> 12 ) & 0xff ) 

static PAGE_TABLE_INFO pti;

#define FirstPageTableEntryAdr( idxEntry )  ( (UINT*)( lpdwFirstPTE + (idxEntry) ) )
//UINT * FirstPageTableEntryAdr( int idxEntry )
//{
//	return lpdwFirstPTE + idxEntry;
//}
#define SecondPageTableEntryAdr( idxPage, idxEntry ) ( (UINT*)( lpdwSecondPTE + ( (idxPage) << 8) + (idxEntry) ) )
//UINT * SecondPageTableEntryAdr( int idxPage, int idxEntry )
//{
//	return lpdwSecondPTE + idxPage * 256 + idxEntry;
//}

DWORD GetPhyPageAdrMask( void )
{
	return PG_PHYS_ADDR_MASK;
}

DWORD GetPhyPageFlagMask( void )
{
	return PGF_MASK;
}

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

VOID FreeCPUPTS( VOID * lpPTS )
{
	ASSERT( (DWORD)lpPTS == 0xffff0000 );
}

VOID * AllocCPUPTS( VOID )
{
	return (VOID *)0xffff0000;
}

static void ClearPageTable( void )
{
	int i, k;
	PAGE_TABLE_SLOTS * lpPTS = pti.pts;
	//PAGE_TABLE_SLOTS * lpPTS_End = lpPTS + PAGE_TABLES;//pti.pts;
	UINT * lpui;

	for( i = 0; i < PAGE_TABLES; i++, lpPTS++ )
	{
		if( lpPTS->uiFirstLevelEntry )
		{	//有效
			//clear first level page table entry
			*FirstPageTableEntryAdr( lpPTS->uiFirstLevelEntry - 1 ) = 0;
			//clear second level page table entry
			lpui = &lpPTS->uiPageSlotsIndex;
			if( *lpui )
			{
				*SecondPageTableEntryAdr( i, *lpui - 1 ) = 0;
				*lpui = 0;
			}
			/* 2004-09-03-delete
			lpui = lpPTS->uiPageSlotsIndex;
			for( k = 0; k < MAX_SLOTS; k++, lpui++ )
			{
				if( *lpui )
				{
					*SecondPageTableEntryAdr( i, *lpui - 1 ) = 0;
					*lpui = 0;
				}				
			}
			*/
			lpPTS->uiFirstLevelEntry = 0;
		}
	}
	pti.uiCurPos = 0;
	pti.uiActive = 0;
}

#define DEBUG_FlushCacheAndClearTLB 0
void FlushCacheAndClearTLB( void )
{
	UINT uiSave;

	DEBUGMSG( DEBUG_FlushCacheAndClearTLB, ( "FlushCacheAndClearTLB++.\r\n" ) );
	LockIRQSave( &uiSave );


    FlushDCache();
    FlushICache();
	ClearPageTable();
    TLBClear();     // clear h/w TLBs

	UnlockIRQRestore( &uiSave );
	DEBUGMSG( DEBUG_FlushCacheAndClearTLB, ( "FlushCacheAndClearTLB--.\r\n" ) );    	
}

void GetMMUContext( LPPROCESS lpProcess, UINT flag )
{
//#ifdef VIRTUAL_MEM

	if( lpProcess != &InitKernelProcess )  // LN, 2003-06-04, ADD
	    lppProcessSegmentSlots[0] = lpProcess->lpSegIndex;//->lpSeg;
	else              // LN, 2003-06-04, ADD
		lppProcessSegmentSlots[0] = 0;         // LN, 2003-06-04, ADD

	SetCPUId( lpProcess->dwVirtualAddressBase );
    
	
	ClearPageTable();
    TLBClear();

#ifdef __DEBUG
	if( lpCurThread->lpCurProcess->dwVirtualAddressBase != 0 && 
		lpCurThread->lpCurProcess->dwVirtualAddressBase !=  GetCPUId() )
	{
		//ASSERT( 0 );
		//RETAILMSG( 1, ( "lpCurProcess->lpName=%s,lpCurProcess->dwVirtualAddressBase=%x,GetCPUId()=%x,Address=%x.\r\n", lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->dwVirtualAddressBase, GetCPUId(), dwAddress ) );
		if( flag != 0 )
		    RETAILMSG( 1, ("ERROR:flag=%d,lpProcess(%x)->dwVirtualAddressBase=%x,lpCurThread->lpCurProcess(%x)->dwVirtualAddressBase=%x.\r\n", flag, lpProcess, lpProcess->dwVirtualAddressBase,lpCurThread->lpCurProcess, lpCurThread->lpCurProcess->dwVirtualAddressBase ) );
	}
#endif	

//#endif
}

//#define TEST_ENTERY_COUNT

#ifdef TEST_ENTERY_COUNT
extern LPDWORD lpdwLoadPageEntryCount;
#endif
// call by data abort or prefetch abort
//#define COUNT_HIT
//#define CHECK_LoadFailurePage
int LoadFailurePage( DWORD dwAddress, UINT uFailurePC ) 
{
	extern LPSEGMENT Seg_FindSegment( DWORD dwAddress );

	UINT uiFirstLevelEntry, uiSecondLevelEntry;
	LPSEGMENT lpSeg;
	LPMEMBLOCK lpBlk;
	UINT uiPhyAdr;
	//DWORD dwSegIndex;

//	RETAILMSG( 1, ( "p.\r\n" ) );
#ifdef TEST_ENTERY_COUNT
	if( lpdwLoadPageEntryCount )
		*lpdwLoadPageEntryCount += 1;
#endif	

#ifdef COUNT_HIT
	//////////FOR TEST///////////

	static UINT uiTotalCount = 0;
	static UINT uiFirstOkCount = 0;
	static UINT uiFirstOkCacheCount = 0;
	static UINT tickCount = 0;
/////////////////////

	//////////FOR TEST///////////

	uiTotalCount++;
	if( uiTotalCount == 500000 )
	{
		RETAILMSG( 1, ( "perform:uiTotal/tick=%d,uiFirstOk=%d,uiFirstOkCache=%d,dwAddress=%x.\r\n", uiTotalCount / ((KL_GetTickCount()-tickCount)/1000), uiFirstOkCount, uiFirstOkCacheCount, dwAddress ) );
		uiTotalCount = 0;
		uiFirstOkCount = 0;
		uiFirstOkCacheCount = 0;
		tickCount = KL_GetTickCount();
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
#endif

	//dwSegIndex = GET_SEGMENT_INDEX( dwAddress );
	lpSeg = Seg_FindSegment( dwAddress );

	//RETAILMSG( 1, ( "LoadFailurePage:lpSeg=%x,adr=%x,process=%s,ProId=%d,dwSegIndex=%d.\r\n", lpSeg, dwAddress, lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->dwProcessId, dwSegIndex ) );
	//RETAILMSG( 1, ( "lpSeg=%x,ProId=%d.\r\n", lpSeg, lpCurThread->lpCurProcess->dwProcessId ) );

	if( lpSeg )//lpSegments[dwSegIndex]) )
	{
_REPEAT:
		if( ( lpBlk = lpSeg->lpBlks[BLOCK_INDEX(dwAddress)] ) > RESERVED_BLOCK &&
			( uiPhyAdr = lpBlk->uiPages[PAGE_INDEX(dwAddress)] ) )
		{   // the page has been commit
			UINT * lpFirstEntry;
			uiFirstLevelEntry = GET_FIRST_LEVEL_ENTRY( dwAddress, lpCurThread->lpCurProcess->dwVirtualAddressBase );//lpCurThread->lpCurProcess->dwProcessId );
			uiSecondLevelEntry = GET_SECOND_LEVEL_ENTRY( dwAddress );
			lpFirstEntry = FirstPageTableEntryAdr( uiFirstLevelEntry );//FirstTable( dwAddress );
			//RETAILMSG( 1, ( "page1:uiFirstLevelEntry=%d,uiSecondLevelEntry=%d.\r\n",uiFirstLevelEntry , uiSecondLevelEntry ) );

			if( *lpFirstEntry )
			{   // second level page table is valid
				PAGE_TABLE_SLOTS * lpPTS;// = &pti.pts[0];//uiPageTableUse;
//				UINT * lpuiSlots;
				UINT i, * lpui;
#ifdef COUNT_HIT
	//////////FOR TEST///////////
	            uiFirstOkCount++;
	////////////////////////////
#endif


			//	RETAILMSG( 1, ( "page2.\r\n" ) );
				uiFirstLevelEntry++;

				i = pti.uiActive;
				ASSERT( i < PAGE_TABLES );
				lpPTS = &pti.pts[i];
				if( lpPTS->uiFirstLevelEntry != uiFirstLevelEntry )
				{				
					lpPTS = &pti.pts[0];
					for( i = 0; i < PAGE_TABLES; lpPTS++, i++ )
					{
						if( lpPTS->uiFirstLevelEntry == uiFirstLevelEntry )
						{
							pti.uiActive = i;
							break;
						}
					}
				}
				else
				{
#ifdef COUNT_HIT
	//////////FOR TEST///////////
	                uiFirstOkCacheCount++;
					if( uiFirstOkCacheCount == 100000 )
					{
						RETAILMSG( 1, ( "LLLLLL:i=%d,lpPTS->uiFirstLevelEntry=%d,uiSecondLevelEntry=%d,uiPhyAdr=%x,VAB=%x,ProcID=%x.\r\n", i, lpPTS->uiFirstLevelEntry, uiSecondLevelEntry, uiPhyAdr, lpCurThread->lpCurProcess->dwVirtualAddressBase, GetCPUId() ) );
					}
	////////////////////////////
#endif
				}

				uiFirstLevelEntry--;
				//ASSERT( i != PAGE_TABLES );
				if( i == PAGE_TABLES )
					goto _error_return;
				//2004-09-03, begin
				//lpui = &lpPTS->uiPageSlotsIndex[lpPTS->uiCurPos]; 
				//if( *lpui )
				  //  *SecondPageTableEntryAdr( i, *lpui - 1 ) = 0; // clear entry 				
				//*SecondPageTableEntryAdr( i, uiSecondLevelEntry ) = uiPhyAdr;				
                //*lpui = uiSecondLevelEntry + 1;  // 1 of base
				//lpPTS->uiCurPos = ( lpPTS->uiCurPos + 1 ) & ( MAX_SLOTS - 1 );

				//lpui = &lpPTS->uiPageSlotsIndex[lpPTS->uiCurPos]; 
				lpui = &lpPTS->uiPageSlotsIndex;
				if( *lpui )
				    *SecondPageTableEntryAdr( i, *lpui - 1 ) = 0; // clear entry 				
				*SecondPageTableEntryAdr( i, uiSecondLevelEntry ) = uiPhyAdr;
                *lpui = uiSecondLevelEntry + 1;  // 1 of base
				//lpPTS->uiCurPos = ( lpPTS->uiCurPos + 1 ) & ( MAX_SLOTS - 1 );

				//2004-09-03,end

				//TLBClear();
                return TRUE;
			}
			else
			{   // is invalid, to get one
				PAGE_TABLE_SLOTS * lpPTS;// = &pti.pts[0];//uiPageTableUse;
				UINT uiFirst;
				int i, *lpui;

			//	RETAILMSG( 1, ( "page3.\r\n" ) );

				lpPTS = &pti.pts[pti.uiCurPos];
				uiFirst = lpPTS->uiFirstLevelEntry;
				if( uiFirst )
				{  // current pagetable has been used, free it
                    // clear old entry(first)
//					RETAILMSG( 1, ( "page3-1.\r\n" ) );
					*FirstPageTableEntryAdr( uiFirst - 1 ) = 0;
					// clear old entry(second)
					lpui = &lpPTS->uiPageSlotsIndex;
					if( *lpui )
					{
					    *SecondPageTableEntryAdr( pti.uiCurPos, *lpui - 1 ) = 0;
						*lpui = 0;
					}
					/*2004-09-03, delete
					for( i = 0; i < MAX_SLOTS; i++ )
					{
						if( *lpui )
						{
						    *SecondPageTableEntryAdr( pti.uiCurPos, *lpui - 1 ) = 0;
							*lpui = 0;
						}
					}
					*/
				}
				i = 0;
				// init lpPTS
				
				lpPTS->uiFirstLevelEntry = uiFirstLevelEntry + 1;
				/*2004-09-03, delete
				lpPTS->uiPageSlotsIndex[i] = uiSecondLevelEntry + 1;
                lpPTS->uiCurPos = ( i + 1 ) & ( MAX_SLOTS - 1 );
				*/
				lpPTS->uiPageSlotsIndex = uiSecondLevelEntry + 1;
                //lpPTS->uiCurPos = ( i + 1 ) & ( MAX_SLOTS - 1 );

				// reset second page table
				*SecondPageTableEntryAdr( pti.uiCurPos, uiSecondLevelEntry  ) = uiPhyAdr;
				// reset first page table
				*FirstPageTableEntryAdr( uiFirstLevelEntry ) = ( (DWORD)lpdwSecondPhyPTE + pti.uiCurPos * PAGE_TABLE_SIZE ) | 1;  // 1 mean second level 
				pti.uiActive = pti.uiCurPos;
			//	RETAILMSG( 1, ( "page3-2:fe=%d,se=%d,adr=%x,fpte=%x,pti.uiCurPos=%d.\r\n", uiFirstLevelEntry, uiSecondLevelEntry, uiPhyAdr, *FirstPageTableEntryAdr( uiFirstLevelEntry ), pti.uiCurPos ) );
				pti.uiCurPos = (pti.uiCurPos + 1) & ( PAGE_TABLES - 1 );
			    return TRUE;
			}
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
	        RETAILMSG( 1, ( "page_error: dwAddress=0x%x,pc=0x%x,dwSegIndex=%d,lpSeg=%x,lpBlk=%x,uiPhyAdr=%x,ProcessName=%s,lpProcessSegIndex=%x,lpPTE[%d]=%x,cur sp(0x%x),OwnerProces=(%s),ThreadId(0x%x),ThreadHandle(0x%x).\r\n", 
							dwAddress, uFailurePC, GET_SEGMENT_INDEX( dwAddress ), lpSeg, lpBlk, uiPhyAdr, lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->lpSegIndex, index, lpdwFirstPTE[index], &index, 
							lpCurThread->lpOwnerProcess->lpszApplicationName, lpCurThread->dwThreadId, lpCurThread->hThread ) );	
		}
	}
	else{
		UINT index = dwAddress >> 20;
        RETAILMSG( 1, ( "page_error: invalid null lpSeg, dwAddress=0x%x,pc=0x%x,dwSegIndex=%d,ProcessName=%s,lpProcessSegIndex=%x,lpPTE[%d]=%x,cur sp(0x%x),OwnerProces=(%s),ThreadId(0x%x),ThreadHandle(0x%x).\r\n", 
			dwAddress, uFailurePC, GET_SEGMENT_INDEX( dwAddress ), lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->lpSegIndex, index, lpdwFirstPTE[index], &index, 
			lpCurThread->lpOwnerProcess->lpszApplicationName, lpCurThread->dwThreadId, lpCurThread->hThread ) );
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

