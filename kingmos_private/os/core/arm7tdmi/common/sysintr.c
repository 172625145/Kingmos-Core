/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/


/*****************************************************
�ļ�˵����ϵͳ�жϴ���
�汾�ţ�2.0.0
����ʱ�ڣ�2002-06-23
���ߣ�����
�޸ļ�¼��
******************************************************/

#include <edef.h>
#include <ewindows.h>
#include <isr.h>
#include <oemfunc.h>
#include <epcore.h>
#include <coresrv.h>
#include <sysintr.h>

/*********************************************************************************************************
** ��������: INTR_Software()
** ��������: swi �ж��쳣��������û�������Ҫ�Լ��ı����
**           
** �䡡��: lr, uSWI_number, pData
**
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
**���ߣ�����
**�޸ļ�¼��AlexZeng
**����ʱ�ڣ�2002-06-23
********************************************************************************************************/
void INTR_Software( int ra, UINT apiInfo )
{	
	EdbgOutputDebugString("Software int ra=0x%x, apiInfo=0x%x.\r\n", ra, apiInfo );
	while(1);
}

/*********************************************************************************************************
** ��������: INTR_PrefetchAbort()
** ��������: PrefetchAbort �ж��쳣��������û�������Ҫ�Լ��ı����
**           
** �䡡��: ��
**
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
**���ߣ�����
**�޸ļ�¼��AlexZeng
**����ʱ�ڣ�2002-06-23
********************************************************************************************************/
void INTR_PrefetchAbort( int ra )
{
	EdbgOutputDebugString("====================================================\r\n" );
	EdbgOutputDebugString("Prefetch Abort, pc return address=0x%x.\r\n", ra );
	EdbgOutputDebugString("Current Process:%s, hThread:%x,APIId(%d),functionId(%d),RetAdr(0x%x).\r\n", lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->hThread, GET_APIID( lpCurThread->lpCallStack->dwCallInfo ), GET_OPTIONID( lpCurThread->lpCallStack->dwCallInfo ), lpCurThread->lpCallStack->pfnRetAdress );
    EdbgOutputDebugString("====================================================\r\n" );

	if( GetInstructionBreakpoint() == ra )
	{// a instruction breakpoint happen
		EdbgOutputDebugString("InstructionBreakpoint.\r\n" );
		return;
	}    
	KL_ExitThread(-1);
	while(1);
	
}

/*********************************************************************************************************
** ��������: INTR_DataAbort()
** ��������: DataAbort �ж��쳣��������û�������Ҫ�Լ��ı����
**           
** �䡡��: ��
**
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
**���ߣ�����
**�޸ļ�¼��AlexZeng
**����ʱ�ڣ�2002-06-23
********************************************************************************************************/

void INTR_DataAbort( UINT ra, UINT iFault_address, UINT iFault_status )
{
	EdbgOutputDebugString("=============================================================================\r\n" );
	EdbgOutputDebugString("Data Abort( abort pc adderss=0x%x, rw_data_address=0x%x, fault_status=0x%x).\r\n", ra, iFault_address, iFault_status );
	EdbgOutputDebugString("Current Process:%s,hThread:%x,ThreadID=%x,Owner Process=%s,lpCurThread->lpCallStack(0x%x).\r\n", lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->hThread, lpCurThread->dwThreadId, lpCurThread->lpOwnerProcess->lpszApplicationName, lpCurThread->lpCallStack );
	if( lpCurThread->lpCallStack )
	    EdbgOutputDebugString("CallStack APIId(%d),functionId(%d),RetAdr(0x%x).\r\n", GET_APIID( lpCurThread->lpCallStack->dwCallInfo ), GET_OPTIONID( lpCurThread->lpCallStack->dwCallInfo ), lpCurThread->lpCallStack->pfnRetAdress );
    EdbgOutputDebugString("==============================================================================\r\n" );
	if( iFault_status & 0x200 )
	{  // data breakpoint
	    EdbgOutputDebugString("data breakpoint\r\n" );
		return;
	}
	if( lpCurThread->lpdwTLS[TLS_TRY] )
	{
		EdbgOutputDebugString("Try GetData in DataAbort\r\n" );
		lpCurThread->lpdwTLS[TLS_TRY] = 0;
		//while(1);
	}
	else
	{
		EdbgOutputDebugString("===========Now Exit Current Thread============\r\n" );
	    KL_ExitThread(-1);
	    while(1);
	}

}

/*********************************************************************************************************
** ��������: INTR_HandleErrorTrap
** ��������: INTR_HandleErrorTrap ��һ�÷������ж��쳣��������û�������Ҫ�Լ��ı����
**           
** �䡡��: ��
** �䡡��: ��
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**���ߣ�����
**�޸ļ�¼��AlexZeng
**����ʱ�ڣ�2002-06-23
********************************************************************************************************/
void INTR_HandleErrorTrap( int msg )
{
	if( msg == 1 )
	    EdbgOutputDebugString( "error: a undef interrupt generate!.\r\n" );
	else if( msg == 2 )
        EdbgOutputDebugString( "error: a fast interrupt generate!.\r\n" );
	while(1);
}

//int bDebugTimer = 0;
//int iReturnAddress;
int INTR_Interrupt(int ra )//, int spsr)
{
	//static int count = 0;
	extern BOOL bNeedResched;
	int rv;

//	iReturnAddress = ra;

	switch ( ( rv = OEM_InterruptHandler(0) ) ) 
	{
		case SYSINTR_RESCHED:
			DoTimer( 0 );
#ifdef TIMER_RESCHE
			bNeedResched = 1;
#endif
			break;
		case SYSINTR_NOP:
		    goto _error;
		default:
			ISR_Active( rv );
    }

	if( iISRActiveCount )
	{   // if enable, to do isr
       ISR_Handler( rv );
	}

_error:
	;

	return rv;
}

//DWORD dwRetAddress;

void DefaultHandler( UINT ra )
{
#ifdef TIMER_RESCHE

	extern BOOL bNeedResched;
	extern int iISREntry;
    extern void CALLBACK Schedule(void);
	extern int KL_GetCPSR(void);

	INTR_OFF();
	if( iISRActiveCount )
    {   
        ISR_Handler( ISR_ALL_INTRS );
    }    


	if( bNeedResched && 
//		iISREntry == 0 &&
		lpCurThread->nLockScheCount == 0 )
	{
//		dwRetAddress = ra;
		INTR_ON();
		Schedule();
	}
	else
	    INTR_ON();
#endif
}

