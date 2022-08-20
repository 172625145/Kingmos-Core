//#include <ewindows.h>
#include <edef.h>
#include <ecore.h>
#include <eassert.h>
#include <eapisrv.h>
#include <miscellany.h>
//----------------------------------------------
//added by xyg: register for com device
extern BOOL RegisterDevice_COM( DWORD nIndex );
#define	ID_COM5				5
#define	ID_COM1				1
//----------------------------------------------
BOOL _LoadSRAMDISK( void );

BOOL OEM_InitDefaultDevice( void )
{
	RegisterDevice_COM( ID_COM1 );
//	xygRas_AddCOM( );
	
    return TRUE;
}

// **************************************************
// 声明：BOOL GetSystemPowerStatusEx(
//					   PSYSTEM_POWER_STATUS_EX pstatus,
//					   BOOL fUpdate)
// 参数：INOUT pstatus--指向PSYSTEM_POWER_STATUS_EX的指针
// 	  IN    fUpdate--未用 	
// 返回值：TRUE/FALSE (成功/失败)
// 功能描述：得到电源状态。
// 引用: 
// **************************************************
BOOL
OEM_GetSystemPowerStatusEx(
					   PSYSTEM_POWER_STATUS_EX pstatus,
					   BOOL fUpdate)
{
	return FALSE;
}
