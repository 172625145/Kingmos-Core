#include <edef.h>
#include <ecore.h>
#include <eassert.h>

#define  MSCount  (3686) 

typedef struct COPYentry {
    ULONG   ulSource;               // copy source address
    ULONG   ulDest;                 // copy destination address
    ULONG   ulCopyLen;              // copy length
    ULONG   ulDestLen;              // copy destination length 
                                    // (zero fill to end if > ulCopyLen)
} COPYentry;

typedef struct ROMHDR {
    ULONG   dllfirst;               // first DLL address
    ULONG   dlllast;                // last DLL address
    ULONG   physfirst;              // first physical address
    ULONG   physlast;               // highest physical address
    ULONG   nummods;                // number of TOCentry's
    ULONG   ulRAMStart;             // start of RAM
    ULONG   ulRAMFree;              // start of RAM free space
    ULONG   ulRAMEnd;               // end of RAM
    ULONG   ulCopyEntries;          // number of copy section entries
    ULONG   ulCopyOffset;           // offset to copy section
    ULONG   ulProfileLen;           // length of PROFentries RAM 
    ULONG   ulProfileOffset;        // offset to PROFentries
    ULONG   numfiles;               // number of FILES
    ULONG   ulKernelFlags;          // optional kernel flags from ROMFLAGS .bib config option
    ULONG   ulFSRamPercent;         // Percentage of RAM used for filesystem 
                                        // from FSRAMPERCENT .bib config option
                                        // byte 0 = #4K chunks/Mbyte of RAM for filesystem 0-2Mbytes 0-255
                                        // byte 1 = #4K chunks/Mbyte of RAM for filesystem 2-4Mbytes 0-255
                                        // byte 2 = #4K chunks/Mbyte of RAM for filesystem 4-6Mbytes 0-255
                                        // byte 3 = #4K chunks/Mbyte of RAM for filesystem > 6Mbytes 0-255

    ULONG   ulDrivglobStart;        // device driver global starting address
    ULONG   ulDrivglobLen;          // device driver global length
    USHORT  usCPUType;       		// CPU (machine) Type
    USHORT  usMiscFlags;         	// Miscellaneous flags
	PVOID	pExtensions;			// pointer to ROM Header extensions
    ULONG   ulTrackingStart;        // tracking memory starting address
    ULONG   ulTrackingLen;          // tracking memory ending address
} ROMHDR;


extern int  OEM_InterruptHandler(unsigned int ra);
extern void OEM_InterruptDone(DWORD idInt);
//void EdbgOutputDebugString(const unsigned char *sz, ...);
//void Timer3_Process(void);

static UINT global_count=0;

extern ROMHDR *const pTOC;// = (ROMHDR *)-1;     // Gets replaced by RomLoader with real address
//ROMHDR *const pTOC = (ROMHDR *)-1;

extern void OEM_Init(void);
//extern void Start(int); 
extern DWORD StartEsoft( LPVOID lParam );
//extern void TouchPanel_ISR(void);

//int heapTest=987654321;

//void __While( void )
//{
//	while(1);
//}
 
//int a;
static void KernelRelocate(ROMHDR *const pTOC)
{
    ULONG loop;
    COPYentry *cptr;

    
	if (pTOC == (ROMHDR *const) 0xffffffff) {
		//cptr = *(int*)(0);
		//return;
		EdbgOutputDebugString( "error : pTOC = 0xffffffff!, stop\r\n" );
        while(1);
        // spin forever!

    }
	EdbgOutputDebugString( "pTOC = 0x%x\r\n", pTOC );

    // This is where the data sections become valid... don't read globals until after this
    for (loop = 0; loop < pTOC->ulCopyEntries; loop++) {
        cptr = (COPYentry *)(pTOC->ulCopyOffset + loop*sizeof(COPYentry));
        if (cptr->ulCopyLen)
            memcpy((LPVOID)cptr->ulDest,(LPVOID)cptr->ulSource,cptr->ulCopyLen);
        if (cptr->ulCopyLen != cptr->ulDestLen)
            memset((LPVOID)(cptr->ulDest+cptr->ulCopyLen),0,cptr->ulDestLen-cptr->ulCopyLen);
    }
    EdbgOutputDebugString( "init pTOC OK................\r\n" );
}

BOOL OEM_InterruptEnable (
    DWORD idInt,       // @parm Interrupt ID to be enabled. See 
                       //  <l Interrupt ID's.Interrupt ID's>  for a list of possble values.
    LPVOID pvData,     // @parm ptr to data passed in in the <f InterruptInitialize> call
    DWORD cbData       // @parm Size of data pointed to be <p pvData>
);

void ES_Entry( UINT uiPhyFirstLevelPage )
{
	int i, j;
	//extern void ES_IRQHandler( void ); 
	int * ps, *pe;
	int iPC;

	OEM_InitDebugSerial();
	//OEM_InitDebugSerial();
	EdbgOutputDebugString( "init debug serial ok..................\r\n" );

	KernelRelocate(pTOC);

	//EdbgOutputDebugString( "ok pTOC OK :......, \r\n" );
	OEM_InitDebugSerial();
	EdbgOutputDebugString( "init debug serial ok..................\r\n" );
	//EdbgOutputDebugString( "Pts=%x\r\n", pts );

	InitPTE( uiPhyFirstLevelPage ); 
	SetProtect( 0x80000000 );//, 16 );
	TestVirtual();

	
	EdbgOutputDebugString( "Kingmos entry address=0x%x\r\n", ES_Entry );
	EdbgOutputDebugString( "RAM Start=%x, Free RAM Address=%x, Free RAM Len=%dK.....\r\n", pTOC->ulRAMStart, pTOC->ulRAMFree, (pTOC->ulRAMEnd - pTOC->ulRAMFree) / 1024  );
	//EdbgOutputDebugString( "Now Zero Free RAM, len = %dk.....\r\n", (pTOC->ulRAMEnd - pTOC->ulRAMFree) / 1024 );

	//ES_IRQHandler();
	//ES_IRQHandler();
	{
        DWORD dwStart, dwEnd;
		extern LPBYTE  lpbSysMainMem;// = _mem;
        extern ULONG   ulSysMainMemLength;// = MEM_SIZE;

		//RemapVirtualAdrTo( pTOC->ulRAMFree
//#ifdef __DEBUG
		memset( pTOC->ulRAMFree, 0xCCCCCCCC, pTOC->ulRAMEnd - pTOC->ulRAMFree );
//#endif
		dwStart = ( pTOC->ulRAMFree + 1024 * 1024 - 1 ) & 0xfff00000; // up to 1 M
		dwEnd = ( pTOC->ulRAMEnd ) & 0xfff00000; // down to 1m

		//RemapVirtualAdrTo( 0x00200000, dwStart, 5 ); // remap 5m

		lpbSysMainMem = pTOC->ulRAMFree;
		ulSysMainMemLength = pTOC->ulRAMEnd - pTOC->ulRAMFree;//5 * 1024 * 1024;
	}

	//ps = (int*)pTOC->ulRAMFree;
	//pe = (int*)pTOC->ulRAMEnd;
	//while( ps < pe )
	//{
	//	*ps++ = 0;
	//}
  	
	OEM_Init();

	//INTR_Initialize( ISR_TIMER3, Timer3_Process, SYSINTR_TIMER3, 0, 0 );
	//OEM_InterruptEnable(SYSINTR_TIMER3, 0, 0);
	//OEM_InterruptEnable(SYSINTR_RESCHED, 0, 0);
	
	EdbgOutputDebugString( "begin kingmos..................\r\n" );
	StartEsoft(0);
	
	while(1);
}

//extern DWORD __sysTickCount;
/*
void Timer3_Process(void)
{
	extern void CALLBACK _PutTimerEvent(void);

	_PutTimerEvent();
	OEM_InterruptDone(SYSINTR_TIMER3);
}
*/

