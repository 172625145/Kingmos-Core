/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/


/*****************************************************
�ļ�˵����ϵͳ�жϴ���
�汾�ţ�2.0.0
����ʱ�ڣ�2000
���ߣ�����
�޸ļ�¼��
**�޸ļ�¼��AlexZeng
******************************************************/

#include <edef.h>
#include <ewindows.h>
//#include <isr.h>
#include <oemfunc.h>
#include <epcore.h>
#include <coresrv.h>
#include <sysintr.h>

/*********************************************************************************************************
** ��������: LPBYTE INTR_GetAbortStack( VOID )
** ��������: �õ��쳣����� �̵߳�sp
**           
** �䡡��: ��
**
** �䡡��: sp
**         
** ȫ�ֱ���: ��
** ����ģ��: ��
**
********************************************************************************************************/
//LPBYTE INTR_GetAbortStack( VOID )
//{
//	return (LPBYTE)lpCurThread + (THREAD_STRUCT_SIZE-1024) - sizeof(int);
//}


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
********************************************************************************************************/
void INTR_Software( int ra, UINT apiInfo )
{	
	INTR_OFF();
	EdbgOutputDebugString("====================================================\r\n" );
	EdbgOutputDebugString("Software int ra=0x%x, apiInfo=0x%x.\r\n", ra, apiInfo );
	EdbgOutputDebugString("Current Process:%s, hThread:%x,APIId(%d),functionId(%d),RetAdr(0x%x).\r\n",
	                       lpCurThread->lpCurProcess->lpszApplicationName,
	                       lpCurThread->hThread, 
                       	lpCurThread->lpCallStack ? GET_APIID( lpCurThread->lpCallStack->dwCallInfo ) : 0,
                           lpCurThread->lpCallStack ? GET_OPTIONID( lpCurThread->lpCallStack->dwCallInfo ) : 0,
                       	lpCurThread->lpCallStack ? lpCurThread->lpCallStack->pfnRetAdress : 0 );
    EdbgOutputDebugString("====================================================\r\n" );
	INTR_ON();

    LeaveException( TRUE );  //�Ƿ���try ?
	KL_ExitThread(-1);
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
********************************************************************************************************/
void INTR_PrefetchAbort( int ra )
{
	int k;
	INTR_OFF();
	EdbgOutputDebugString("====================================================\r\n" );
	EdbgOutputDebugString("Prefetch Abort, pc return address=0x%x, &k=0x%x,lpCurThread=0x%x.\r\n", ra, &k,lpCurThread  );
	EdbgOutputDebugString("Current Process:%s, hThread:%x,APIId(%d),functionId(%d),RetAdr(0x%x).\r\n",
	                       lpCurThread->lpCurProcess->lpszApplicationName,
	                       lpCurThread->hThread, 
                       	lpCurThread->lpCallStack ? GET_APIID( lpCurThread->lpCallStack->dwCallInfo ) : 0,
                           lpCurThread->lpCallStack ? GET_OPTIONID( lpCurThread->lpCallStack->dwCallInfo ) : 0,
                       	lpCurThread->lpCallStack ? lpCurThread->lpCallStack->pfnRetAdress : 0 );
    EdbgOutputDebugString("====================================================\r\n" );
	INTR_ON();

//	if( GetInstructionBreakpoint() == ra )
//	{// a instruction breakpoint happen
		//EdbgOutputDebugString("InstructionBreakpoint.\r\n" );
//		return;
//	}    
    LeaveException( TRUE );  //�Ƿ���try ?
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
********************************************************************************************************/

void INTR_DataAbort( UINT ra, UINT iFault_address, UINT iFault_status )
{
	INTR_OFF();
	EdbgOutputDebugString("=============================================================================\r\n" );
	EdbgOutputDebugString("Data Abort( abort pc adderss=0x%x, rw_data_address=0x%x, fault_status=0x%x).\r\n", ra, iFault_address, iFault_status );
	EdbgOutputDebugString("Current Process:%s,hThread:%x,ThreadID=%x,Owner Process=%s,lpCurThread->lpCallStack(0x%x).\r\n", lpCurThread->lpCurProcess->lpszApplicationName, lpCurThread->hThread, lpCurThread->dwThreadId, lpCurThread->lpOwnerProcess->lpszApplicationName, lpCurThread->lpCallStack );
	if( lpCurThread->lpCallStack )
	{
		CALLSTACK * lpcs = lpCurThread->lpCallStack;
		do{
	        EdbgOutputDebugString("lpcs(0x%x),CallStack APIId(%d),functionId(%d),RetAdr(0x%x).\r\n", 
	                               lpcs,
	                               GET_APIID( lpcs->dwCallInfo ), 
	                               GET_OPTIONID( lpcs->dwCallInfo ), 
	                               lpcs->pfnRetAdress );
	        lpcs = lpcs->lpNext;
		}while( lpcs );
	}
    EdbgOutputDebugString("==============================================================================\r\n" );
    
	INTR_ON();
	
//	if( iFault_status & 0x200 )
//	{  // data breakpoint
//	    EdbgOutputDebugString("data breakpoint\r\n" );
//		return;
//	}
//	if( lpCurThread->lpdwTLS[TLS_TRY] )
//	{
//		EdbgOutputDebugString("Try GetData in DataAbort\r\n" );
//		lpCurThread->lpdwTLS[TLS_TRY] = 0;
//	}
//	else
	{
        LeaveException( TRUE );  //�Ƿ���try ?		
        
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

	switch ( ( rv = OEM_InterruptHandler(ra) ) ) 
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

// 2003-10-08, ADD
	INTR_OFF();
	
	
	if( iISRActiveCount )
    {		
        ISR_Handler( ISR_ALL_INTRS );
    }    
// 2003-10-08
	if( bNeedResched && 
//		iISREntry == 0 &&   // 2004-02-17
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

