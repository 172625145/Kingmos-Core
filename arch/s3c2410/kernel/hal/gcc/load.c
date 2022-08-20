/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：内核重定位处理
版本号：1.0.0
开发时期：2003-04-010
作者：周兵
修改记录：
******************************************************/

#include "ewindows.h"
#include "ecore.h"
#include "eassert.h"
#include "romhdr.h"

ROMHDR *const pTOC = 0;     // Gets replaced by RomLoader with real address
extern void PutHex_Virtual( UINT );

static void KernelRelocate(ROMHDR *const pTOC)
{
    ULONG loop;
    COPYentry *cptr;

	if (pTOC == (ROMHDR *const)NULL ) {
		//EdbgOutputDebugString( "error : pTOC = 0x0!, stop\r\n" );
        while(1);       // spin forever!
    }
    
    // This is where the data sections become valid... don't read globals until after this
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++) {
    	
        cptr = (COPYentry *)(pTOC->ulCopyOffset + loop*sizeof(COPYentry));
        EdbgOutputDebugString( "cptr = 0x%x.\r\n",cptr );

        if (cptr->ulCopyLen){			
			memset((LPVOID)cptr->ulDest, 0, cptr->ulCopyLen);		
            memcpy((LPVOID)cptr->ulDest,(LPVOID)cptr->ulSource,cptr->ulCopyLen);
		}

        if (cptr->ulCopyLen != cptr->ulDestLen){			
            memset((LPVOID)(cptr->ulDest+cptr->ulCopyLen),0,cptr->ulDestLen-cptr->ulCopyLen);		
		}
    }	
}

extern void _SystemStart(  DWORD uiPhyFirstLevelPage, LPBYTE lpbFreeMemStart, DWORD dwFreeMemSize  );
void _InitKernel( UINT uiPhyFirstLevelPage )//, UINT sp0, UINT sp1, UINT sp2 )
{
//	while(1);
	OEM_InitDebugSerial();	
    EdbgOutputDebugString( "pTOC = 0x%x,val0(0x%x),val0(0x%x),val1(0x%x),val2(0x%x),val3(0x%x),val4(0x%x),,.\r\n", pTOC );    
	EdbgOutputDebugString( "FirstLevelPagePtr=0x%x.\r\n", uiPhyFirstLevelPage );
	EdbgOutputDebugString( "RAM Start=%x, End Addr=%x, Free RAM Address=%x, Free RAM Len=%dK.....\r\n",
		pTOC->ulRAMStart, pTOC->ulRAMEnd, pTOC->ulRAMFree, (pTOC->ulRAMEnd - pTOC->ulRAMFree) / 1024  );
	
	KernelRelocate(pTOC);
	OEM_InitDebugSerial();

//	EdbgOutputDebugString( "FirstLevelPagePtr%x, sp0=0x%x, sp2=0x%x, sp3=0x%x.....\r\n",
//		 uiPhyFirstLevelPage, sp0, sp1, sp2 );
	EdbgOutputDebugString( "FirstLevelPagePtr=0x%x.\r\n", uiPhyFirstLevelPage );

	EdbgOutputDebugString( "RAM Start=%x, End Addr=%x, Free RAM Address=%x, Free RAM Len=%dK.....\r\n",
		pTOC->ulRAMStart, pTOC->ulRAMEnd, pTOC->ulRAMFree, (pTOC->ulRAMEnd - pTOC->ulRAMFree) / 1024  );

	_SystemStart( uiPhyFirstLevelPage, (LPBYTE)pTOC->ulRAMFree, pTOC->ulRAMEnd - pTOC->ulRAMFree );
}

