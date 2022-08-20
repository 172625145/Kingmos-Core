/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：
版本号：  1.0.0
开发时期：2003-12-01
作者：    肖远钢
修改记录：
******************************************************/
//root include
#include <ewindows.h>
//"\inc_app"
#include <eapisrv.h>
#include <ras.h>
//"\inc_local"
//#include "netif_ras.h"

/***************  全局区 定义， 声明 *****************/

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
	//添加成为 拨号设备
	API_WaitReady( API_TCPIP, -1, "xygRas_Thrd" );
//	RasRegisterModem( COM1_ACTIVE );
	return 0;
}

BOOL	API_WaitReady( UINT uiAPIId, DWORD dwWaitSec, LPCSTR pszInfo )
{
	DWORD		dwTick;

	// 等待指定ID号的API是否已经完成
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
