/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/


/*****************************************************
文件说明：系统中断处理
版本号：2.0.0
开发时期：2000
作者：李林
修改记录：
**修改记录：AlexZeng
******************************************************/

#include <edef.h>
#include <ewindows.h>
//#include <isr.h>
#include <oemfunc.h>
#include <epcore.h>
#include <coresrv.h>
#include <sysintr.h>

/*********************************************************************************************************
** 函数名称: LPBYTE INTR_GetAbortStack( VOID )
** 功能描述: 得到异常处理的 线程的sp
**           
** 输　入: 无
**
** 输　出: sp
**         
** 全局变量: 无
** 调用模块: 无
**
********************************************************************************************************/
//LPBYTE INTR_GetAbortStack( VOID )
//{
//	return (LPBYTE)lpCurThread + (THREAD_STRUCT_SIZE-1024) - sizeof(int);
//}


/*********************************************************************************************************
** 函数名称: INTR_Software()
** 功能描述: swi 中断异常处理程序，用户根据需要自己改变程序
**           
** 输　入: lr, uSWI_number, pData
**
** 输　出: 无
**         
** 全局变量: 无
** 调用模块: 无
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

    LeaveException( TRUE );  //是否有try ?
	KL_ExitThread(-1);
	while(1);
}

/*********************************************************************************************************
** 函数名称: INTR_PrefetchAbort()
** 功能描述: PrefetchAbort 中断异常处理程序，用户根据需要自己改变程序
**           
** 输　入: 无
**
** 输　出: 无
**         
** 全局变量: 无
** 调用模块: 无
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
    LeaveException( TRUE );  //是否有try ?
	KL_ExitThread(-1);
	while(1);
	
}

/*********************************************************************************************************
** 函数名称: INTR_DataAbort()
** 功能描述: DataAbort 中断异常处理程序，用户根据需要自己改变程序
**           
** 输　入: 无
**
** 输　出: 无
**         
** 全局变量: 无
** 调用模块: 无
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
        LeaveException( TRUE );  //是否有try ?		
        
		EdbgOutputDebugString("===========Now Exit Current Thread============\r\n" );		        
	    KL_ExitThread(-1);
	    while(1);
	}

}

/*********************************************************************************************************
** 函数名称: INTR_HandleErrorTrap
** 功能描述: INTR_HandleErrorTrap 不一该发生的中断异常处理程序，用户根据需要自己改变程序
**           
** 输　入: 无
** 输　出: 无
**         
** 全局变量: 无
** 调用模块: 无
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

