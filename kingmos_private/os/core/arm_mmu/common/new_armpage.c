/******************************************************
Copyright(c) ��Ȩ���У�1998-2004΢�߼�����������Ȩ����
******************************************************/

/*****************************************************
�ļ�˵����ҳ���ȹ��ܣ�������̵�ҳ���롢����������һ���µİ汾��
		֮ǰ�����ù̶���ϵͳҳ�������е��̹߳�����Щҳ���ڴ����øߣ���
		Ŀǰ����Ϊÿһ���̷߳���һ��ҳ
�汾�ţ�2.5.0
����ʱ�ڣ�2000
���ߣ�����
�޸ļ�¼��
	
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

#define INVALID_FIRST_LEVEL_ENTRY (-1)
#define INVALID_SECOND_LEVEL_ENTRY (-1)

typedef struct _PAGE_TABLE_SLOTS
{
	struct _PAGE_TABLE_SLOTS * lpNext;
	UINT uiFirstLevelEntry;  //��ҳ����Ӧ���������ĵ�һ��, -1 ������Ч��
	UINT uiPageSlotsIndex;	 //��ҳ����ǰ�����õĽ�����Ӧ������������Ч 4Kλ�ã�-1 ������Ч��
	volatile DWORD * lpdwVirtualPageTable; //ҳ�� 1k ( 256 * 4k)
	volatile DWORD * lpdwPhyPageTable; //����ҳ�� 1k ( 256 * 4k)
}PAGE_TABLE_SLOTS;

#define GET_SLOT_ADR( dwAdr, dwVABase ) ( (dwAdr) >= 0x2000000 ? (dwAdr) : ( (dwAdr) | (dwVABase) ) )
#define GET_FIRST_LEVEL_ENTRY( dwAdr, dwVABase ) ( GET_SLOT_ADR( (dwAdr), (dwVABase) ) >> 20 )
#define GET_SECOND_LEVEL_ENTRY( dwAdr ) ( ( (dwAdr) >> 12 ) & 0xff ) 
//static PAGE_TABLE_INFO pti;
#define FirstPageTableEntryAdr( idxEntry )  ( (UINT*)( lpdwFirstPTE + (idxEntry) ) )
//UINT * FirstPageTableEntryAdr( int idxEntry )
//{
//	return lpdwFirstPTE + idxEntry;
//}
//#define SecondPageTableEntryAdr( idxPage, idxEntry ) ( (UINT*)( lpdwSecondPTE + ( (idxPage) << 8) + (idxEntry) ) )
//UINT * SecondPageTableEntryAdr( int idxPage, int idxEntry )
//{
//	return lpdwSecondPTE + idxPage * 256 + idxEntry;
//}
static PAGE_TABLE_SLOTS * lpFreeSysPTS = NULL;
//static PAGE_TABLE_SLOTS * lpUsedSysPTS;

//ϵͳ��Ҫ�ĺ������õ�ҳ����������Ч��ַռλ
DWORD GetPhyPageAdrMask( void )
{
	return PG_PHYS_ADDR_MASK;
}

//ϵͳ��Ҫ�ĺ������õ�ҳ����������Ч��־ռλ
DWORD GetPhyPageFlagMask( void )
{
	return PGF_MASK;
}
//����PTS
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
//�ͷ�PTS
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

//ϵͳ��Ҫ�ĺ������õ�ҳ����������Ч��־ sys logic flag - > cpu flag
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

//ϵͳ��Ҫ�ĺ������õ�ҳ����������Ч��־ cpu flag - > sys logic flag
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

//��Ч��ǰϵͳ�����е�ҳ
static void ClearProcessPageTable ( PAGE_TABLE_SLOTS * lpCurPTS )
{
	UINT * lpui;
	UINT uiFirstLevelEntry;

	//while( lpCurPTS )
	//ASSERT( lpCurPTS );
	if( lpCurPTS )
	{
		if( (uiFirstLevelEntry = lpCurPTS->uiFirstLevelEntry) != INVALID_FIRST_LEVEL_ENTRY )
		{
			*FirstPageTableEntryAdr( uiFirstLevelEntry ) = 0;  //CPU����
			lpui = &lpCurPTS->uiPageSlotsIndex;
			if( *lpui != INVALID_SECOND_LEVEL_ENTRY )
			{
				lpCurPTS->lpdwVirtualPageTable[*lpui] = 0;;//lpdwidxEntry
				//*SecondPageTableEntryAdr( i, *lpui ) = 0;	//CPU����
				*lpui = INVALID_SECOND_LEVEL_ENTRY;
			}
			lpCurPTS->uiFirstLevelEntry = INVALID_FIRST_LEVEL_ENTRY;
		}
		//lpCurPTS = lpCurPTS->lpNext;
	}
}

//��Ч��ǰϵͳ�����е�ҳ
static void ClearPageTable ( VOID )// PAGE_TABLE_SLOTS * lpCurPTS )
{
	ClearProcessPageTable( lpCurThread->lpCurProcess->lpCpuPTS );
	ClearProcessPageTable( lpCurThread->lpOwnerProcess->lpCpuPTS );
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
    
	
	ClearPageTable();  //����ֻ��Ҫ�����ǰ�̵߳�?
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

// call by data abort or prefetch abort
#define COUNT_HIT
#define DEBUG_LoadFailurePage 1
int LoadFailurePage( DWORD dwAddress, UINT uFailurePC ) 
{
	extern LPSEGMENT Seg_FindSegment( DWORD dwAddress );

	UINT uiFirstLevelEntry, uiSecondLevelEntry;
	LPSEGMENT lpSeg;
	LPMEMBLOCK lpBlk;
	UINT uiPhyAdr;
	//DWORD dwSegIndex;
	PAGE_TABLE_SLOTS * lpCurPTS = lpCurThread->lpCurProcess->lpCpuPTS;

//	RETAILMSG( 1, ( "p.\r\n" ) );

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

#ifdef __DEBUG
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

	//dwSegIndex = GET_SEGMENT_INDEX( dwAddress );
	lpSeg = Seg_FindSegment( dwAddress );

//	DEBUGMSG( DEBUG_LoadFailurePage, ( "LoadFailurePage:lpSeg=%x,adr=%x,process=%s,ProId=%d,dwSegIndex=%d.\r\n", lpSeg, dwAddress, lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->lpCurProcess->dwProcessId, dwSegIndex ) );
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
//			DEBUGMSG( DEBUG_LoadFailurePage, ( "page2:uiFirstLevelEntry=%d,uiSecondLevelEntry=%d.\r\n",uiFirstLevelEntry , uiSecondLevelEntry ) );
/*
			if( *lpFirstEntry )
			{   // second level page table is valid
				//PAGE_TABLE_SLOTS * lpPTS;// = &pti.pts[0];//uiPageTableUse;
//				UINT * lpuiSlots;
				//UINT i;
#ifdef COUNT_HIT
	//////////FOR TEST///////////
	            uiFirstOkCount++;
	////////////////////////////
#endif

			   DEBUGMSG( DEBUG_LoadFailurePage, ( "page1:lpCurPTS->lpdwPhyPageTable=0x%x,lpCurPTS->uiFirstLevelEntry=0x%.\r\n",lpCurPTS->lpdwPhyPageTable ,  lpCurPTS->uiFirstLevelEntry ) );
				//ASSERT( lpCurPTS->uiFirstLevelEntry != INVALID_FIRST_LEVEL_ENTRY );
				ASSERT( *lpFirstEntry == (DWORD)lpCurPTS->lpdwPhyPageTable );
				ASSERT( uiFirstLevelEntry == lpCurPTS->uiFirstLevelEntry );
				//{
				//}
				if( lpCurPTS->uiPageSlotsIndex != INVALID_SECOND_LEVEL_ENTRY )
					lpCurPTS->lpdwVirtualPageTable[lpCurPTS->uiPageSlotsIndex] = 0;	//clear
				// new
				lpCurPTS->lpdwVirtualPageTable[uiSecondLevelEntry] = uiPhyAdr; 
				lpCurPTS->uiPageSlotsIndex = uiSecondLevelEntry;
                return TRUE;
			}
//			else
			*/
			{   			
				
				if( lpCurPTS->uiFirstLevelEntry != INVALID_FIRST_LEVEL_ENTRY )
				{
					*FirstPageTableEntryAdr(lpCurPTS->uiFirstLevelEntry) = 0; //clear
				}
				if( lpCurPTS->uiPageSlotsIndex != INVALID_SECOND_LEVEL_ENTRY )
					lpCurPTS->lpdwVirtualPageTable[lpCurPTS->uiPageSlotsIndex ] = 0;  //clear
				//new
				lpCurPTS->lpdwVirtualPageTable[uiSecondLevelEntry] = uiPhyAdr; 
				lpCurPTS->uiPageSlotsIndex = uiSecondLevelEntry;

				lpCurPTS->uiFirstLevelEntry = uiFirstLevelEntry;
				*FirstPageTableEntryAdr( uiFirstLevelEntry ) = (DWORD)lpCurPTS->lpdwPhyPageTable | 1;  // 1 mean second level 
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
        RETAILMSG( 1, ( "page_error: invalid null lpSeg,dwAddress=0x%x,pc=0x%x,dwSegIndex=%d,ProcessName=%s,lpProcessSegIndex=%x,lpPTE[%d]=%x,cur sp(0x%x),OwnerProces=(%s),ThreadId(0x%x),ThreadHandle(0x%x).\r\n", 
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
