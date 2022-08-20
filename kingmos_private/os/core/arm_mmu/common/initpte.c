#include <edef.h>
#include <erect.h>
#include <eassert.h>
//#include <cpu.h>
//#include <halether.h>

typedef struct _OEM_MEM_ENTRY
{
	DWORD dwVirtual;
	DWORD dwPhysical;
	DWORD dwSize;  // 1m base
}OEM_MEM_ENTRY, FAR * LPOEM_MEM_ENTRY;

//extern OEM_MEM_ENTRY OEMAddressTable[];
//DWORD  * lpdwPTE = (DWORD*)DRAM_BANK1A_PHYSICAL;
#define SECTION_SIZE (1024*1024)
#define SECTION_SHIFT 20
#define SECTION_MASK 0xfff00000

#define PTE_SIZE  (4*1024*4)
//extern DWORD FirstPT[];

extern OEM_MEM_ENTRY OEMAddressTable[];
DWORD * lpdwFirstPTE = (DWORD *)(0xFFFD0000);//( 0xAC000000 );
// 不需要全局第二级页表，放到每一个线程 -2004-12-28 修改
//DWORD * lpdwSecondPTE = (DWORD *)(0xFFFD4000);//( 0xAC000000 + 1024 * 4 * 4 );
//DWORD * lpdwSecondPhyPTE;// = (DWORD *)( 0xC0000000 + 1024 * 4 * 4  + 1 );
//

DWORD _GetPhysicalPageAdr( DWORD dwVirtualAdr );
// test code
//#define FREE_ADR_START 0x8C600000
LPBYTE lpFreeAdr = (LPBYTE)0x8C600000;
LPBYTE lpFreeAdr_Un = (LPBYTE)0xAC600000;

//extern DWORD PaFromVa( DWORD va, OEM_MEM_ENTRY * );
//DWORD * VirtualToPhycial( DWORD va )
//{
//	return PaFromVa( va, OEMAddressTable );
//}
/*
void TestVirtual( void )
{
	DWORD dwFreePhy;
	//lpdwFirstPTE[0] = (DWORD)lpdwSecondPhyPTE;
	RETAILMSG( 1, ( "OEMAddressTable=%x., &lpdwFirstPTE=%x\r\n", OEMAddressTable, &lpdwFirstPTE ) );
	return ;

	lpdwFirstPTE[0] = (DWORD)lpdwSecondPhyPTE;
	//RETAILMSG( 1, ( "TestVirtual0:%s,lpdwFirstPTE[0]=%x.\r\n", lpFreeAdr, lpdwFirstPTE[0] ) );
	dwFreePhy = _GetPhysicalPageAdr( (DWORD)lpFreeAdr );

	lpdwSecondPTE[0] = dwFreePhy | 0xffe;
	//lpdwSecondPTE[32] = dwFreePhy | 0xffe;
	//SetCPUId( 1 << 25 ); // first slot
	//FlushDCache();
	//TLBClear();
	//RETAILMSG( 1, ( "TestVirtual1:*lpdwSecondPTE =%x.\r\n", lpdwSecondPTE[0] ) );
	strcpy( 0, "    abcdef." );
	//strcpy( lpFreeAdr, "    hello-world" );
	RETAILMSG( 1, ( "TestVirtual3:4=%s,lpFreeAdr=%s,lpFreeAdr_Un=%s.\r\n", 4, lpFreeAdr, lpFreeAdr_Un ) );

	//RETAILMSG( 1, ( "FlushDCache0.\r\n" ) );
    FlushDCache();
	//RETAILMSG( 1, ( "TLBClear0.\r\n" ) );
	TLBClear();

    memset( (DWORD)lpFreeAdr + 1024 * 4, 0, 1024 );
	RETAILMSG( 1, ( "change page.\r\n" ) );
	//SetCPUId( 2 << 25 ); // first slot
	lpdwFirstPTE[0] = (DWORD)lpdwSecondPhyPTE + 1024;
	lpdwSecondPTE[256] = (dwFreePhy + 1024 * 4 ) | 0xffe;



	//RETAILMSG( 1, ( "TLBClear.\r\n" ) );
	//TLBClear();

//
//	RETAILMSG( 1, ( "FlushDCache.\r\n" ) );
  //  FlushDCache();
	//RETAILMSG( 1, ( "TestVirtual5:4=%s,lpFreeAdr=%s,lpFreeAdr_Un=%s,lpFreeAdr+4*1024=%s.\r\n", 4, lpFreeAdr, lpFreeAdr_Un, lpFreeAdr+4*1024 ) );


	strcpy( 0, "    123456." );

	//RETAILMSG( 1, ( "TestVirtual5:4=%s,lpFreeAdr=%s,lpFreeAdr_Un=%s,lpFreeAdr+4*1024=%s.\r\n", 4, lpFreeAdr, lpFreeAdr_Un, lpFreeAdr+4*1024 ) );

	//RETAILMSG( 1, ( "FlushDCache.\r\n" ) );
    //FlushDCache();

	
	//RETAILMSG( 1, ( "TestVirtual5_1:4=%s,lpFreeAdr=%s,lpFreeAdr_Un=%s,lpFreeAdr+4*1024=%s.\r\n", 4, lpFreeAdr, lpFreeAdr_Un, lpFreeAdr+4*1024 ) );

	//strcpy( lpFreeAdr + 1024 * 4, "    hello-world002" );
    //FlushICache();
	//RETAILMSG( 1, ( "FlushDCache.\r\n" ) );
    //FlushDCache();
	//RETAILMSG( 1, ( "TestVirtual5_2:4=%s,lpFreeAdr=%s,lpFreeAdr_Un=%s,lpFreeAdr+4*1024=%s.\r\n", 4, lpFreeAdr, lpFreeAdr_Un, lpFreeAdr+4*1024 ) );



	//RETAILMSG( 1, ( "TLBClear1.\r\n" ) );
	//TLBClear();


	RETAILMSG( 1, ( "TestVirtual6:4=%s,lpFreeAdr=%s,lpFreeAdr_Un=%s,lpFreeAdr+4*1024=%s.\r\n", 4, lpFreeAdr, lpFreeAdr_Un, lpFreeAdr+4*1024 ) );

	while(1);
}
*/

DWORD _GetPhysicalPageAdr( DWORD dwVirtualAdr )
{
	return ( lpdwFirstPTE[dwVirtualAdr >> 20] & 0xFFF00000 ) | 
	       ( dwVirtualAdr & 0x000FF000 );
}

DWORD _GetPhysicalAdr( DWORD dwVirtualAdr )
{
	return ( lpdwFirstPTE[dwVirtualAdr >> 20] & 0xFFF00000 ) | 
	       ( dwVirtualAdr & 0x000FFFFF );
}

DWORD _GetVirtualPageAdr( DWORD dwPhysicalAdr )
{
    LPOEM_MEM_ENTRY lpOEMEntry =  OEMAddressTable;

	//RETAILMSG( 1, ( "dwPhysicalAdr=%x.\r\n", dwPhysicalAdr ) );

	while( lpOEMEntry->dwVirtual )
	{
		//RETAILMSG( 1, ( "lpOEMEntry->dwPhysical=%x, dwSize=%d.\r\n", lpOEMEntry->dwPhysical, lpOEMEntry->dwSize ) );
		if( dwPhysicalAdr >= lpOEMEntry->dwPhysical &&
			dwPhysicalAdr < ( lpOEMEntry->dwPhysical + ( lpOEMEntry->dwSize << 20 ) ) )
		{
			//RETAILMSG( 1, ( "GetVirtualPageAdr, found virtual adr=%x.\r\n", ( lpOEMEntry->dwVirtual + ( dwPhysicalAdr - lpOEMEntry->dwPhysical ) ) & 0xfffff000 ) );
			return ( lpOEMEntry->dwVirtual + ( dwPhysicalAdr - lpOEMEntry->dwPhysical ) ) & 0xfffff000; // align 4096
		}
		lpOEMEntry++;
	}
	//RETAILMSG( 1, ( "GetVirtualPageAdr, not found virtual adr.\r\n" ) );
	return 0;
}
/*
void RemapVirtualAdrTo( DWORD dwVirtualDest, DWORD dwVirtualSrc, DWORD dwSize )
{
	//DWORD dwSize;
	//LPOEM_MEM_ENTRY lpOEMEntry =  OEMAddressTable;
	DWORD * lpdwPTE = lpdwFirstPTE;//(DWORD *)0x8C000000;//FirstPT;
	//DWORD dwCacheOrNot;
	//DWORD dwAttrib;
	DWORD i;

	dwVirtualDest = (dwVirtualDest & SECTION_MASK) >> SECTION_SHIFT;  // align 1m
	dwVirtualSrc = (dwVirtualSrc & SECTION_MASK) >> SECTION_SHIFT;  // align 1m

	for( i = 0; i < dwSize; i++ )
	{
		lpdwFirstPTE[dwVirtualDest] = lpdwFirstPTE[dwVirtualSrc];
		dwVirtualDest++;
		dwVirtualSrc++;
	}
	TLBClear();
}
*/

void InitPTE( DWORD uiFirstLevelPagePhyAdr )
{
//	lpdwSecondPhyPTE = (LPDWORD)( uiFirstLevelPagePhyAdr + 0x4000 );
//	RETAILMSG( 1, ( "uiFirstLevelPagePhyAdr=%x,lpdwSecondPhyPTE=%x.\r\n", uiFirstLevelPagePhyAdr,lpdwSecondPhyPTE ) );
	RETAILMSG( 1, ( "uiFirstLevelPagePhyAdr=%x.\r\n", uiFirstLevelPagePhyAdr ) );
}

void SetProtect( DWORD dwVirtual )//, DWORD dwSize ) // LPOEM_MEM_ENTRY lpOEMEntry,  DWORD * lpdwPTE )
{
	DWORD dwSize;
	LPOEM_MEM_ENTRY lpOEMEntry =  OEMAddressTable;
	DWORD * lpdwPTE = lpdwFirstPTE;//(DWORD *)0x8C000000;//FirstPT;
	//DWORD dwCacheOrNot;
	DWORD dwAttrib;
	
	
	int i;
	
	// TEST
	//RETAILMSG( 1, ( "TEST BEGIN, fistPTE=%x,secondPTE=%x.\r\n", lpdwFirstPTE, lpdwSecondPTE ) );
	//lpdwSecondPTE[16] = 0xc062effe;
	//*lpdwFirstPTE = (DWORD)lpdwSecondPhyPTE | 1;
	//TLBClear();
	//*( (DWORD*)0x10004 ) = 0;
	//RETAILMSG( 1, ( "TEST END.\r\n" ) );
	//
	
    //memset( lpdwPTE, 0, PTE_SIZE );

	//dwCacheOrNot = 0;  // cache
	//set kernel r/w permission					
	//{can be R/W in SUPERVISOR mode , but no access in USER mode}
	//dwAttrib = 0x400; 

	//dwAttrib = 0;  // read only
	dwAttrib = 0x0E; //1MB cachable bufferable {using SECTION mode , not PAGE mode}

	RETAILMSG( 1, ( "SetProtect Entry,lpdwFirstPTE=%x.\r\n", lpdwFirstPTE ) );


	while( lpOEMEntry->dwVirtual )
	{
		if( lpOEMEntry->dwVirtual == dwVirtual )
		{
			//DWORD dwVirtual = lpOEMEntry->dwVirtual;
			for( i = 0; i < 2; i++ )
			{
				DWORD dwFirstV, dwFirstP;
				DWORD * lpdwPTEStart;
				
				dwFirstV = dwVirtual >> SECTION_SHIFT;
				lpdwPTEStart = lpdwPTE + dwFirstV;
				dwFirstP = (lpOEMEntry->dwPhysical & SECTION_MASK);
				RETAILMSG( 1, ( "dwFirstV=%d, lpdwPTEStart=%x, dwFirstP=%x, dwSize=%d.\r\n", dwFirstV, lpdwPTEStart, dwFirstP, lpOEMEntry->dwSize ) );
				for( dwSize = lpOEMEntry->dwSize; dwSize; dwSize-- )
				{
					*lpdwPTEStart++ = dwFirstP | dwAttrib; 
					dwFirstP += SECTION_SIZE;
				}
				dwVirtual |= 0x20000000; // uncache address
				dwAttrib = 0x02;  // uncache unbuffer, attrib
			}
		}
		lpOEMEntry++; 
	}
	FlushCacheAndClearTLB();

	//RETAILMSG( 1, ( "Now Test...\r\n" ) );
	//*((DWORD*)0x80000000) = 12345678;
}


