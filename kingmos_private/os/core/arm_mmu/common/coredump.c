#include <edef.h>
#include <ecore.h>

#ifdef WINCE

void OEMInterruptHandlerFIQ(void)
{
}

int OEMInterruptHandler(unsigned int ra)
{
	return 0;
};

void OEMInit( void )
{
}

void OEMIdle( void )
{
}

void OEMInterruptDone(
    DWORD idInt     // @parm Interrupt ID. See <t Interrupt ID's>
                    // for the list of possible values.
)
{
}

BOOL OEMInterruptEnable (
    DWORD idInt,       // @parm Interrupt ID to be enabled. See 
                       //  <l Interrupt ID's.Interrupt ID's>  for a list of possble values.
    LPVOID pvData,     // @parm ptr to data passed in in the <f InterruptInitialize> call
    DWORD cbData       // @parm Size of data pointed to be <p pvData>
)
{
	return 0;
}

void OEMInterruptDisable(
    DWORD idInt     // @parm Interrupt ID to be disabled. See <t Interrupt ID's>
                    // for the list of possible values.
)
{
}

BOOL OEMGetExtensionDRAM(LPDWORD lpMemStart, LPDWORD lpMemLen)
{
    return (FALSE); // no extension DRAM
}

VOID InterruptDone ( DWORD idInt )
{
	//ES_OEMInterruptDone(idInt);
}

VOID InterruptDisable (DWORD idInt )
{
	//ES_OEMInterruptDisable(idInt);
}


//void TLBClear(void)
//{
//}

void OEMWriteDebugString( LPCTSTR str )
{
}

void OEMWriteDebugByte(unsigned char c) 
{
}

int OEMReadDebugByte( void )
{
	return 0;
}

BOOL OEMIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize,
	LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned) 
{
	return FALSE;
}


BOOL NoPPFS = FALSE;					// parallel port disconnected flag

unsigned int MyEPC; 

int OEMParallelPortGetByte(void)
{
	return 0;
}

VOID OEMParallelPortSendByte(BYTE chData)
{
}

void OEMInitDebugSerial(void) 
{
}

void OEMClearDebugCommError(void) 
{
}

//DWORD OEMARMCacheMode(void)
//{
//	return 0;
//}

BOOL OEMSetRealTime(LPSYSTEMTIME lpst)
{
	return 0;
}

BOOL OEMGetRealTime(LPSYSTEMTIME lpst)
{
	return FALSE;
}

BOOL OEMSetAlarmTime(LPSYSTEMTIME lpst)
{
	return FALSE;
}

//void FlushDCache(void)
//{
//}

//void FlushICache(void)
//{
//}

void OEMPowerOff( void )
{
}

DWORD SC_GetTickCount( void )
{
	return 0;
}

void InitClock( void )
{
}

#else

#ifndef VIRTUAL_MEM

int LoadFailurePage( DWORD dwAddress ) 
{
	return 0;
}

#endif

#endif