#include <ewindows.h>
#include <estring.h>
#include "romhdr.h"

ROMHDR 	   *const pTOC = 0;     // Gets replaced by RomLoader with real address

//char *hex2char(int val);

/**************************************************
������static void KernelRelocate(ROMHDR *const pTOC)
������
	IN	ROMHDR *const pTOC--pTOC��ָ��
	OUT
	IN/OUT

����ֵ����
������������ʼ��RW, ZI��
����: 
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
������void _InitKernel( UINT uiPhyFirstLevelPage )
������
	IN	UINT uiPhyFirstLevelPage--����
	OUT
	IN/OUT

����ֵ����
����������BootLoader C�������
����: 
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

