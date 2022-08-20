#include <eversion.h>
#include <windows.h>
#include <cpu.h>

//#define LOCK_KINGMOS()   INTR_OFF()
//#define UNLOCK_KINGMOS() INTR_ON()
#define LOCK_KINGMOS()   
#define UNLOCK_KINGMOS() 

BOOL WINAPI Win32_SetEvent( HANDLE hEvent )
{   
	BOOL bRetv;
	LOCK_KINGMOS();
    bRetv = SetEvent( hEvent );
	UNLOCK_KINGMOS();
	return bRetv;
}

VOID WINAPI Win32_Sleep( UINT dwTicks )
{
	//LOCK_KINGMOS();
	Sleep( dwTicks );
	//UNLOCK_KINGMOS();
}

HANDLE WINAPI Win32_CreateEvent(
						 LPSECURITY_ATTRIBUTES lpEventAttributes, 
						 BOOL bManualReset,
						 BOOL bInitialState,
						 LPCTSTR lpName )
{
	HANDLE retv;
	LOCK_KINGMOS();
    retv = CreateEvent(
        lpEventAttributes, 
        bManualReset,
        bInitialState,
        lpName );
	UNLOCK_KINGMOS();
	return retv;
} 

 
DWORD WINAPI Win32_WaitForSingleObject( HANDLE hHandle, DWORD dwMilliseconds )
{
	DWORD retv;

	LOCK_KINGMOS();
    retv = WaitForSingleObject( hHandle, dwMilliseconds );
	UNLOCK_KINGMOS();
	return retv;
}

LPVOID WINAPI Win32_VirtualAlloc( LPVOID lpAddress, DWORD dwSize, DWORD dwAllocationType, DWORD dwProtect )
{
	LPVOID retv;

	LOCK_KINGMOS();
	retv = VirtualAlloc( lpAddress, dwSize, dwAllocationType, dwProtect );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_VirtualFree( LPVOID lpAddress, DWORD dwSize, DWORD dwFreeType )
{
	BOOL retv;
	LOCK_KINGMOS();
	retv = VirtualFree( lpAddress, dwSize, dwFreeType );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_VirtualProtect(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    )
{
	BOOL retv;
	LOCK_KINGMOS();
	retv = VirtualProtect( lpAddress, dwSize, flNewProtect, lpflOldProtect );
	UNLOCK_KINGMOS();
	return retv;
}


static BOOL   bDataMode = FALSE;
static HANDLE hComFile = INVALID_HANDLE_VALUE;
static HANDLE hComFileForData = INVALID_HANDLE_VALUE;
static int    nDataModeCount = 0;
static int    nComRefCount = 0;

HANDLE WINAPI Win32_CreateFile(
  LPCTSTR lpFileName,                         // file name
  DWORD dwDesiredAccess,                      // access mode
  DWORD dwShareMode,                          // share mode
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
  DWORD dwCreationDisposition,                // how to create
  DWORD dwFlagsAndAttributes,                 // file attributes
  HANDLE hTemplateFile                        // handle to template file
)
{
	HANDLE retv;
	//if( dwDesiredAccess & 0x0004 )
	LOCK_KINGMOS();
	if( strncmp( lpFileName, "COM", 3 ) == 0 )
	{
		if( hComFile == INVALID_HANDLE_VALUE )
		{
			hComFile = CreateFile( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
				dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
			nComRefCount = 1;
		}
		else
		{
			nComRefCount++;
		}

		(DWORD)hComFile |= 0x80000000;
		hComFileForData = (HANDLE)((DWORD)hComFile | 0x40000000);
		if( dwDesiredAccess & 0x4 )
		{
			nDataModeCount++;
			bDataMode = TRUE;
			retv = hComFileForData;
		}
		else
		{
			bDataMode = FALSE;
			retv = hComFile;
		}
	}
	else
		retv = CreateFile( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
				dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );

	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_CreateDirectory(
	LPCTSTR lpFileName,                         // file name
	LPSECURITY_ATTRIBUTES lpSecurityAttributes  // SD
)
{
	BOOL retv;
	LOCK_KINGMOS();
	retv = CreateDirectory( lpFileName, lpSecurityAttributes );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_SetEndOfFile(
							   HANDLE hFile
)
{
	BOOL retv;
	LOCK_KINGMOS();
	retv = SetEndOfFile( hFile );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_ReadFile(
  HANDLE hFile,                // handle to file
  LPVOID lpBuffer,             // data buffer
  DWORD nNumberOfBytesToRead,  // number of bytes to read
  LPDWORD lpNumberOfBytesRead, // number of bytes read
  LPOVERLAPPED lpOverlapped    // overlapped buffer
)
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
			{
				retv = FALSE;
				goto _return;
			}
		}
		else if( hComFile != hFile )
		{
			retv = FALSE;
			goto _return;
		}
		(DWORD)hFile &= ~0xc0000000;
	}
	retv = ReadFile( hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped );

_return:

	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_WriteFile(
  HANDLE hFile,                    // handle to file
  LPCVOID lpBuffer,                // data buffer
  DWORD nNumberOfBytesToWrite,     // number of bytes to write
  LPDWORD lpNumberOfBytesWritten,  // number of bytes written
  LPOVERLAPPED lpOverlapped        // overlapped buffer
)
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
			{
				retv = FALSE;
				goto _return;
			}
		}
		else if( hComFile != hFile )
		{
			retv = FALSE;
			goto _return;
		}
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

_return:
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_CloseHandle(
  HANDLE hObject   // handle to object
)
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	if( hObject != INVALID_HANDLE_VALUE )
	{
		/*
		if( hComFileForData == hObject ||
			hComFile == hObject )
		{
			if( hComFileForData == hObject )
			{
				nDataModeCount--;
				if( nDataModeCount == 0 )
					bDataMode = FALSE;
			}
			retv = TRUE;
			goto _return;
		}
		*/
		if( hComFileForData == hObject ||
			hComFile == hObject )
		{
			if( bDataMode )
			{
				if( hComFileForData == hObject )
				{
					nDataModeCount--;
					nComRefCount--;
					if( nDataModeCount == 0 )
					{
						bDataMode = FALSE;
						hComFileForData = INVALID_HANDLE_VALUE;
					}
				}
				if( nComRefCount  )
				{
					retv = TRUE;
					goto _return;
				}
				else
				{
					hComFile = INVALID_HANDLE_VALUE;
				}
			}
			else if( hComFile != hObject )
			{
				retv = FALSE;
				goto _return;
			}
			(DWORD)hObject &= ~0xc0000000;
		}
	}
	retv = CloseHandle( hObject );
	hComFile = INVALID_HANDLE_VALUE;
_return:
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_GetCommState(
  HANDLE hFile,  // handle to communications device
  LPDCB lpDCB    // device-control block
)
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
			{
				retv = FALSE;
				goto _return;// FALSE;
			}
		}
		else if( hComFile != hFile )
		{
			retv = FALSE;
			goto _return;// FALSE;
		}
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = GetCommState( hFile, lpDCB );

_return:
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_SetCommState(
  HANDLE hFile,  // handle to communications device
  LPDCB lpDCB    // device-control block
)
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
			{
				goto _return;// FALSE;
			}
		}
		else if( hComFile != hFile )
		{
			goto _return;// FALSE;
		}
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = SetCommState( hFile, lpDCB );
_return:
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_PurgeComm(
  HANDLE hFile,  // handle to communications resource
  DWORD dwFlags  // action to perform
)
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();

	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = PurgeComm( hFile, dwFlags );

_return:
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_SetCommMask(
  HANDLE hFile,    // handle to communications device
  DWORD dwEvtMask  // mask that identifies enabled events
)
{

	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = SetCommMask( hFile, dwEvtMask );

_return:	
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_SetupComm(
  HANDLE hFile,     // handle to communications device
  DWORD dwInQueue,  // size of input buffer
  DWORD dwOutQueue  // size of output buffer
)
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = SetupComm( hFile, dwInQueue, dwOutQueue );
_return:	
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_ClearCommError(
  HANDLE hFile,     // handle to communications device
  LPDWORD lpErrors, // error codes
  LPCOMSTAT lpStat  // communications status
)
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = ClearCommError( hFile, lpErrors, lpStat );

_return:	
	UNLOCK_KINGMOS();
	return retv;
}

DWORD WINAPI Win32_SetFilePointer(
  HANDLE hFile,                // handle to file
  LONG lDistanceToMove,        // bytes to move pointer
  PLONG lpDistanceToMoveHigh,  // bytes to move pointer
  DWORD dwMoveMethod           // starting point
)
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	retv = SetFilePointer( hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod );
	UNLOCK_KINGMOS();
	return retv;
}

/*BOOL WINAPI Win32_SetEndOfFile(
  HANDLE hFile   // handle to file
)
{
	return SetEndOfFile( hFile );
}
*/

BOOL WINAPI Win32_SetCommTimeouts(
  HANDLE hFile,                  // handle to comm device
  LPCOMMTIMEOUTS lpCommTimeouts  // time-out values
)
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = SetCommTimeouts( hFile, lpCommTimeouts );

_return:	
	UNLOCK_KINGMOS();
	return retv;

}

BOOL WINAPI Win32_WaitCommEvent(
  HANDLE hFile,                // handle to comm device
  LPDWORD lpEvtMask,           // event type
  LPOVERLAPPED lpOverlapped  // overlapped structure
)
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = WaitCommEvent( hFile, lpEvtMask, lpOverlapped );
_return:	
	UNLOCK_KINGMOS();
	return retv;
}

//BOOL WINAPI Win32_SwitchToThread( VOID )
//{
//	return SwitchToThread();
//}

DWORD WINAPI Win32_GetTickCount( VOID )
{
	DWORD retv = FALSE;

	LOCK_KINGMOS();
	retv = GetTickCount();
	UNLOCK_KINGMOS();
	return retv;
}

HMODULE Win32_LoadLibrary( LPCTSTR lpFileName )
{
	HMODULE retv = FALSE;
	LOCK_KINGMOS();
	retv = LoadLibrary( lpFileName );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_FreeLibrary( HMODULE hModule )
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	retv = FreeLibrary( hModule );
	UNLOCK_KINGMOS();
	return retv;
}

FARPROC WINAPI Win32_GetProcAddress( HMODULE hModule, LPCSTR lpProcName )
{
	FARPROC retv = FALSE;
	LOCK_KINGMOS();
	retv = GetProcAddress( hModule, lpProcName );
	UNLOCK_KINGMOS();
	return retv;
}

DWORD	WINAPI Win32_GetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh )
{
	DWORD retv = FALSE;
	LOCK_KINGMOS();
	retv = GetFileSize( hFile, lpFileSizeHigh );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL	WINAPI Win32_GetCommTimeouts( HANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = GetCommTimeouts( hFile, lpCommTimeouts );
_return:
	UNLOCK_KINGMOS();
	return retv;
}

BOOL	WINAPI Win32_TransmitCommChar(HANDLE hFile, char cChar )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = TransmitCommChar( hFile, cChar );
_return:
	UNLOCK_KINGMOS();
	return retv;

}

BOOL	WINAPI Win32_SetCommBreak(HANDLE hFile )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv =  SetCommBreak( hFile );
_return:
	UNLOCK_KINGMOS();
	return retv;
}

BOOL	WINAPI Win32_ClearCommBreak(HANDLE hFile )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = ClearCommBreak( hFile );

_return:

	UNLOCK_KINGMOS();
	return retv;
}
BOOL	WINAPI Win32_GetCommModemStatus(HANDLE hFile, LPDWORD lpModemStat )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv = GetCommModemStatus( hFile, lpModemStat );
_return:
	UNLOCK_KINGMOS();
	return retv;
}
BOOL	WINAPI Win32_GetCommProperties(HANDLE hFile, LPCOMMPROP lpCommProp )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}

	retv  = GetCommProperties( hFile, lpCommProp );
_return:
	UNLOCK_KINGMOS();
	return retv;
}
BOOL	WINAPI Win32_EscapeCommFunction(HANDLE hFile, DWORD dwFunc )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	if( hComFileForData == hFile ||
		  hComFile == hFile )
	{
		if( bDataMode )
		{
			if( hComFileForData != hFile )
				goto _return;// FALSE;
		}
		else if( hComFile != hFile )
			goto _return;// FALSE;
		(DWORD)hFile &= ~0xc0000000;
	}
	retv = EscapeCommFunction( hFile, dwFunc );
_return:
	UNLOCK_KINGMOS();
	return retv;
}

HANDLE WINAPI Win32_FindFirstFile(
  LPCTSTR lpFileName,               // file name
  LPWIN32_FIND_DATA lpFindFileData  // data buffer
)
{
	HANDLE retv = FALSE;
	LOCK_KINGMOS();
	retv = FindFirstFile( lpFileName, lpFindFileData );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_FindNextFile(
  HANDLE hFindFile,                // search handle 
  LPWIN32_FIND_DATA lpFindFileData // data buffer
)
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	retv = FindNextFile( hFindFile, lpFindFileData );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_FindClose(
  HANDLE hFindFile   // file search handle
)
{
	BOOL retv = FALSE;
	LOCK_KINGMOS();
	retv = FindClose( hFindFile );
	UNLOCK_KINGMOS();
	return retv;
}

BOOL WINAPI Win32_DeleteFile(
  LPCTSTR lpFileName   // file name
)
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	retv = DeleteFile( lpFileName );
	UNLOCK_KINGMOS();
	return retv;
}

DWORD WINAPI Win32_GetLastError( void )
{
	BOOL retv = FALSE;

	LOCK_KINGMOS();
	retv = GetLastError();
	UNLOCK_KINGMOS();
	return retv;
}

VOID WINAPI Win32_EnterCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection  // critical section
)
{
	LOCK_KINGMOS();
	EnterCriticalSection( lpCriticalSection );
	UNLOCK_KINGMOS();
}

VOID WINAPI Win32_LeaveCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection  // critical section
)
{
	LOCK_KINGMOS();
	LeaveCriticalSection( lpCriticalSection );
	UNLOCK_KINGMOS();
}
