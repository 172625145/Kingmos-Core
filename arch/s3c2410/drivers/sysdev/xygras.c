/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/

/*****************************************************
�ļ�˵����
�汾�ţ�  1.0.0
����ʱ�ڣ�2003-12-01
���ߣ�    ФԶ��
�޸ļ�¼��
******************************************************/
//root include
#include <ewindows.h>
//"\inc_app"
#include <eapisrv.h>
#include <ras.h>
//"\inc_local"
//#include "netif_ras.h"

/***************  ȫ���� ���壬 ���� *****************/

static	DWORD	WINAPI	xygRas_Thrd( LPVOID lpParam );
static	BOOL	API_WaitReady( UINT uiAPIId, DWORD dwWaitMS, LPCSTR pszInfo );

/******************************************************/

void	xygRas_AddCOM( )
{
	HANDLE	hThrd;
	DWORD	dwThrdID;
		
	hThrd = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)xygRas_Thrd, (LPVOID)0, 0, &dwThrdID );
	if( hThrd )
	{
		CloseHandle( hThrd );
	}
}

#define	COM1_ACTIVE			"Drivers\\Active\\96169"
DWORD	WINAPI	xygRas_Thrd( LPVOID lpParam )
{
	//��ӳ�Ϊ �����豸
	API_WaitReady( API_TCPIP, -1, "xygRas_Thrd" );
//	RasRegisterModem( COM1_ACTIVE );
	return 0;
}

BOOL	API_WaitReady( UINT uiAPIId, DWORD dwWaitSec, LPCSTR pszInfo )
{
	DWORD		dwTick;

	// �ȴ�ָ��ID�ŵ�API�Ƿ��Ѿ����
	dwTick = GetTickCount();
	while( 1 )
	{
		//
		if( API_IsReady( uiAPIId ) )
		{
			return TRUE;
		}
		RETAILMSG( 1, ( "(%s): wait api [%d] ready!\r\n", pszInfo, uiAPIId ) );
		//
		if( dwWaitSec!=-1 )
		{
			if( ((GetTickCount()-dwTick)/1000)>=dwWaitSec )
			{
				return FALSE;
			}
		}
		Sleep(100);
	}
	return FALSE;
}
