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
// ������BOOL GetSystemPowerStatusEx(
//					   PSYSTEM_POWER_STATUS_EX pstatus,
//					   BOOL fUpdate)
// ������INOUT pstatus--ָ��PSYSTEM_POWER_STATUS_EX��ָ��
// 	  IN    fUpdate--δ�� 	
// ����ֵ��TRUE/FALSE (�ɹ�/ʧ��)
// �����������õ���Դ״̬��
// ����: 
// **************************************************
BOOL
OEM_GetSystemPowerStatusEx(
					   PSYSTEM_POWER_STATUS_EX pstatus,
					   BOOL fUpdate)
{
	return FALSE;
}
