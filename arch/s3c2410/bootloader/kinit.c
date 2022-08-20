#include <ewindows.h>
#include <estring.h>
#include "romhdr.h"

ROMHDR 	   *const pTOC = 0;     // Gets replaced by RomLoader with real address

//char *hex2char(int val);

/**************************************************
声明：static void KernelRelocate(ROMHDR *const pTOC)
参数：
	IN	ROMHDR *const pTOC--pTOC的指针
	OUT
	IN/OUT

返回值：无
功能描述：初始化RW, ZI区
引用: 
************************************************/
static void KernelRelocate(ROMHDR *const pTOC)
{
    ULONG loop;
    COPYentry *cptr;
   
	if (pTOC == (ROMHDR *const)NULL ) {
		while(1);
	}
	
	//EdbgOutputDebugString("RAM Start=%x, End Addr=%x, Free RAM Address=%x, Free RAM Len=%dK.....\r\n",
	//	pTOC->ulRAMStart, pTOC->ulRAMEnd, pTOC->ulRAMFree, (pTOC->ulRAMEnd - pTOC->ulRAMFree) / 1024  );
    
    // This is where the data sections become valid... don't read globals until after this
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++) {
        cptr = (COPYentry *)(pTOC->ulCopyOffset + loop*sizeof(COPYentry));

        if (cptr->ulCopyLen){			
			memset((LPVOID)cptr->ulDest, 0, cptr->ulCopyLen);		
            memcpy((LPVOID)cptr->ulDest,(LPVOID)cptr->ulSource,cptr->ulCopyLen);
		}

        if (cptr->ulCopyLen != cptr->ulDestLen){
            memset((LPVOID)(cptr->ulDest+cptr->ulCopyLen),0,cptr->ulDestLen-cptr->ulCopyLen);		
		}
    }	
}

/**************************************************
声明：void _InitKernel( UINT uiPhyFirstLevelPage )
参数：
	IN	UINT uiPhyFirstLevelPage--保留
	OUT
	IN/OUT

返回值：无
功能描述：BootLoader C函数入口
引用: 
************************************************/
void _InitKernel( UINT uiPhyFirstLevelPage )
{
	int	const paddr = &pTOC;

//	OEM_InitDebugSerial();
//	Port_Init();	
//	Uart_DefaultInit();
//	Uart_SendString("\nHello,Kingmos \n");

	KernelRelocate( (ROMHDR *) (*((int *)paddr)) );
	Main();
	while(1);
}

