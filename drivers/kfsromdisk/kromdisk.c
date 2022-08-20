/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：kromdisk
版本号：1.0.0
开发时期：2001-06-21
作者：ll
修改记录：
******************************************************/

#include <edef.h>
#include <eassert.h>
#include <ecore.h>
#include <eerror.h>
#include <edevice.h>
#include <efile.h>
#include <efsdmgr.h>
#include <estring.h>
#include <devdrv.h>

//#define RETAILMSG
#undef MapPtrToProcess
#define MapPtrToProcess( pBuf, handle ) (pBuf)

#undef GetCallerProcess
#define GetCallerProcess

#define STATIC static 
STATIC DWORD ROM_Init( DWORD dwContext );
STATIC BOOL ROM_Deinit( DWORD handle );
STATIC DWORD ROM_Open( DWORD handle, DWORD dwAccess, DWORD dwShareMode );
STATIC BOOL  ROM_Close( DWORD handle );
STATIC DWORD ROM_Read( DWORD handle, LPVOID lpBuffer, DWORD dwNumBytes );
STATIC DWORD ROM_Write( DWORD handle, LPCVOID lpBuffer, DWORD dwNumBytes );
STATIC DWORD ROM_Seek( DWORD handle, long lDistance, DWORD dwMoveMethod );
STATIC VOID  ROM_PowerUp( DWORD handle );
STATIC VOID  ROM_PowerDown( DWORD handle );
STATIC BOOL  ROM_IOControl( DWORD handle,DWORD dwIoControlCode,LPVOID pInBuf,DWORD nInBufSize,LPVOID pOutBuf,DWORD nOutBufSize,PDWORD pBytesReturned);

static BOOL ClearRoot( char * lpSrcRootPath, char * lpSubPath );

static const DEVICE_DRIVER drvRomDisk={
    ROM_Init,
    ROM_Deinit,
    ROM_IOControl,
    ROM_Open,
    ROM_Close,
    ROM_Read,
    ROM_Write,
    ROM_Seek,
    ROM_PowerUp,
    ROM_PowerDown
};

#define BLOCK_SIZE 512l

#define ROM_SIZE (1024l * 512 * 16)

#define BLOCK_NUMBER  (ROM_SIZE/BLOCK_SIZE)
// align to dword
typedef struct __DISK_DATA
{
    int  iOpenCount;
	BYTE * lpRomDisk;
    WORD wState;
}_DISK_DATA, FAR * _LPDISK_DATA;

static _DISK_DATA  diskData;

#ifdef EML_WIN32
#define MEM_COMMIT           0x1000
#define MEM_RELEASE          0x8000     
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_NOACCESS          0x01


extern LPVOID WINAPI Win32_VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );
extern BOOL WINAPI Win32_VirtualProtect(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    );

extern BOOL WINAPI Win32_VirtualFree(
  LPVOID lpAddress,  // address of region of committed pages
  DWORD dwSize,      // size of region
  DWORD dwFreeType   // type of free operation
);
 
 
#endif

#ifdef EML_WIN32

#include <stdio.h>
static void LoadDataFromDisk(void * p)
{
    FILE *in;

    if ((in = fopen("filerom.dat", "rb"))
        == NULL)
    {
        return;
    }
    fread(p, ROM_SIZE, 1, in); /* write struct s to file */
    fclose(in);	
}

static void SaveDataToDisk(void *p)
{
    FILE *out;

    if ((out = fopen("filerom.dat", "wb"))
        == NULL)
    {
       return;
    }
    fwrite(p, ROM_SIZE, 1, out); /* write struct s to file */
    fclose(out);

}

#define STR_KINGMOS_ROOT_OUT ".\\___romdisk.out"
#define STR_KINGMOS_ROOT_IN ".\\___romdisk.in"
#define STR_KINGMOS_DEST_ROOT "\\Kingmos"


#endif  // EML_WIN32

#ifdef EML_WIN32

#include <io.h>
#include <direct.h>
/* c:\\abc */
/* \\ */

static void LoadSystemFile( char * lpSrcRootPath, char * lpSubPath )
{
	struct _finddata_t fd; 
	long handle;
	char buf[1024];

	strcpy( buf, lpSrcRootPath );
	strcat( buf, lpSubPath );
	strcat( buf, "*.*" );
	handle = _findfirst( buf, &fd );
	if( handle && handle != -1 )
	{
		do
		{			
			FILE * lpFile;

			if( strcmp( fd.name , "." ) == 0 ||
				strcmp( fd.name , ".." ) == 0 )
				continue;
			if( fd.attrib & _A_SUBDIR )
			{
				strcpy( buf, STR_KINGMOS_DEST_ROOT );
				strcat( buf, lpSubPath );
			    strcat( buf, fd.name );
				CreateDirectory( buf, NULL );
				strcat( buf, "\\" );
				LoadSystemFile( lpSrcRootPath, buf );
				continue;
			}

			strcpy( buf, lpSrcRootPath );
			strcat( buf, lpSubPath );
			strcat( buf, fd.name );
			//RETAILMSG( 1, ( "copy file:%s.\r\n", buf ) );
			lpFile = fopen( buf, "rb" );
			if( lpFile )
			{
				HANDLE hFile;
				strcpy( buf, STR_KINGMOS_DEST_ROOT );
				strcat( buf, lpSubPath );
				strcat( buf, fd.name );
				hFile = CreateFile( buf, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, 0, NULL );
				
				if( hFile == INVALID_HANDLE_VALUE )
				{
					if( GetLastError() ==  ERROR_ACCESS_DENIED )
					{
						SetFileAttributes( buf, FILE_ATTRIBUTE_NORMAL );
						hFile = CreateFile( buf, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, 0, NULL );
					}
				}
				if( hFile != INVALID_HANDLE_VALUE )
				{
					DWORD dwRead = 0;

					while(1)
					{
						dwRead = fread( buf, 1, sizeof( buf ), lpFile );
						if( dwRead > 0 )
							WriteFile( hFile, buf, dwRead, &dwRead, NULL );
						else
							break;
					}					
					CloseHandle( hFile );
					SetFileAttributes( buf, FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_READONLY );
				}
				fclose( lpFile );
			}
		}while( _findnext( handle, &fd ) == 0 );
		_findclose( handle );
	}
}

static BOOL ClearRoot( char * lpSrcRootPath, char * lpSubPath )
{
	struct _finddata_t fd; 
	long handle;
	char buf[128];

	strcpy( buf, lpSrcRootPath );
	strcat( buf, lpSubPath );
	strcat( buf, "*.*" );
	handle = _findfirst( buf, &fd );
	if( handle && handle != -1 )
	{
		do
		{			
			if( strcmp( fd.name , "." ) == 0 ||
				strcmp( fd.name , ".." ) == 0 )
				continue;
			if( fd.attrib & _A_SUBDIR )
			{
				strcpy( buf, lpSubPath );
			    strcat( buf, fd.name );
				strcat( buf, "\\" );
				ClearRoot( lpSrcRootPath, buf );
				continue;
			}

			strcpy( buf, lpSrcRootPath );
			strcat( buf, lpSubPath );
			strcat( buf, fd.name );
			unlink( buf );
		}while( _findnext( handle, &fd ) == 0 );
		_findclose( handle );
	}

	strcpy( buf, lpSrcRootPath );
	strcat( buf, lpSubPath );
	return 1;
}

#define FILE_ATTRIBUTE_DEVICE               0x00008000

static void RestoreSystemFile( char * lpRootPath, char * lpSubPath )
{
#ifndef KINGMOS_DEMO

	FILE_FIND_DATA wfd;
	HANDLE handle;
	char buf[128];

    strcpy( buf, lpSubPath );
	strcat( buf, "*.*" );

	handle = FindFirstFile( buf, &wfd );
	if( handle != INVALID_HANDLE_VALUE )
	{
		do
		{
			HANDLE hFile;

			if( strcmp( wfd.cFileName , "." ) == 0 ||
				strcmp( wfd.cFileName , ".." ) == 0 || 
				(wfd.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) )
				continue;

			if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				strcpy( buf, lpRootPath );
				strcat( buf, lpSubPath );
			    strcat( buf, wfd.cFileName );
				_mkdir( buf );
				strcpy( buf, lpSubPath );
			    strcat( buf, wfd.cFileName );
				strcat( buf, "\\" );
				RestoreSystemFile( lpRootPath, buf );
				continue;
			}

			strcpy( buf, lpSubPath );
			strcat( buf, wfd.cFileName );
			hFile = CreateFile( buf, 
				                GENERIC_READ, 
								0, 
								NULL, 
								OPEN_ALWAYS, 0, NULL );
			if( hFile != INVALID_HANDLE_VALUE )
			{
				FILE * lpFile;

				strcpy( buf, lpRootPath );
				strcat( buf, lpSubPath );
			    strcat( buf, wfd.cFileName );

				lpFile = fopen( buf, "w+b" );
				if( lpFile )
				{
					DWORD dwRead = 0;

					while(1)
					{
						dwRead = 0;
						ReadFile( hFile, buf, sizeof(buf), &dwRead, NULL );
						if( dwRead > 0 )
							fwrite( buf, 1, dwRead, lpFile );
						else
							break;
					}					
					fclose( lpFile );
				}
				CloseHandle( hFile );
			}
		}while( FindNextFile( handle, &wfd ) );
	}

#endif

}

#endif

BOOL _LoadRomdisk( void )
{
    HANDLE hDev = RegisterDriver( "ROM", 0, &drvRomDisk, 0 );
    HANDLE hFile;
    DISK_INIT di;
    
    RETAILMSG( 1, ( ".....................................................\r\n" ) );
	RETAILMSG( 1, ( "_LoadRomdisk:hDev=%x\r\n", hDev ) );
    hFile = CreateFile( "ROM0:", GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
					   0, NULL);    
	RETAILMSG( 1, ( "_LoadRomdisk:hFile=%x\r\n", hFile ) );
    if(hFile != INVALID_HANDLE_VALUE) {
        di.hDevice = hDev;
        DeviceIoControl(hFile, IOCTL_DISK_INITIALIZED, &di, sizeof(di), NULL, 0, NULL, NULL);
    }

    if( hFile != INVALID_HANDLE_VALUE ){
        CloseHandle(hFile);

#ifdef EML_WIN32
		LoadSystemFile( STR_KINGMOS_ROOT_IN, "\\" );
		hFile = CreateFile( "\\kingmos\\Vol:",  GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
		DeviceIoControl(hFile, IOCTL_DISK_READONLY, 0, 0, NULL, 0, NULL, NULL);
		
#endif
//
        return TRUE;
    }
    return FALSE;
}

static CRITICAL_SECTION csRomDisk;

DWORD ROM_Init( DWORD dwContext )
{
#ifdef EML_WIN32
    DWORD dwOldProc;

    LPVOID fileBuf = Win32_VirtualAlloc( NULL, ROM_SIZE, MEM_COMMIT, PAGE_READWRITE );
    memset( fileBuf, 0, ROM_SIZE );
    LoadDataFromDisk( fileBuf );
    Win32_VirtualProtect( fileBuf, ROM_SIZE, PAGE_READONLY, &dwOldProc );
    memset( &diskData, 0, sizeof( diskData ) );
	diskData.lpRomDisk = (LPBYTE)fileBuf;

	InitializeCriticalSection( &csRomDisk );
	csRomDisk.lpcsName = "CS-ROMDISK";

	if( diskData.lpRomDisk )
    {
        return (DWORD)&diskData; 
    }
	else
		return 0;

#else

    extern const unsigned char __diskRom[];

    RETAILMSG(1, (TEXT("ROMDISK: ROM_Init\r\n")));
	InitializeCriticalSection( &csRomDisk );
	csRomDisk.lpcsName = "CS-ROMDISK";

    memset( &diskData, 0, sizeof( diskData ) );
	diskData.lpRomDisk = (LPBYTE)__diskRom;//(LPBYTE)fileBuf;

	if( diskData.lpRomDisk )
		return (DWORD)&diskData; 
	else
		return 0;

#endif

}

BOOL ROM_Deinit( DWORD handle )
{
#ifdef EML_WIN32

    DWORD dwOldProcted;
    Win32_VirtualProtect( diskData.lpRomDisk, ROM_SIZE, PAGE_READWRITE, &dwOldProcted );
	SaveDataToDisk(diskData.lpRomDisk);
    Win32_VirtualProtect( diskData.lpRomDisk, ROM_SIZE, PAGE_NOACCESS, &dwOldProcted );

    Win32_VirtualFree( diskData.lpRomDisk, 0, MEM_RELEASE );
	DeleteCriticalSection( &csRomDisk );
    return TRUE;

#else

    RETAILMSG(1, (TEXT("ROMDISK: Deinit\r\n")));
	diskData.lpRomDisk = 0;
	DeleteCriticalSection( &csRomDisk );
    return TRUE;

#endif
}

#define ERROR_IO_PENDING  100
#define MAX_RW_BUF 1

static DWORD
DoDiskIO(
    HANDLE handle,
    DWORD Opcode,
    PDISK_RW pdrw
    )
{
    DWORD status = ERROR_SUCCESS;
    DWORD num_sg;
    DWORD curr_byte;
    DWORD dwBufSize;
    PRW_BUF prwBuf;
    PBYTE pBuf;
    volatile PBYTE pCard;

	EnterCriticalSection( &csRomDisk );


    pdrw->dwStatus = ERROR_IO_PENDING;

    if (pdrw->nrwNum > MAX_RW_BUF) {
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    //
    // Make sure request doesn't exceed the disk
    //
    if ((pdrw->dwStartSector + pdrw->dwSectorNumber - 1) > BLOCK_NUMBER) {
        status = ERROR_SECTOR_NOT_FOUND;
        goto ddi_exit;
    }
    status = ERROR_SUCCESS;
    curr_byte = pdrw->dwStartSector * BLOCK_SIZE;

    num_sg = pdrw->nrwNum;
    prwBuf = &(pdrw->rwBufs[0]);
    dwBufSize = prwBuf->dwSize;
    pBuf = MapPtrToProcess((LPVOID)prwBuf->lpBuf, GetCallerProcess());

#ifdef EML_WIN32
    {
        DWORD dwOldProcted;
        Win32_VirtualProtect( diskData.lpRomDisk, ROM_SIZE, PAGE_READWRITE, &dwOldProcted );
    }
#endif

    while (num_sg) {
        pCard = (LPBYTE)diskData.lpRomDisk + curr_byte;
        if (Opcode == IOCTL_DISK_READ) {
            memcpy(pBuf, pCard, dwBufSize);
        } else {
            memcpy(pCard, pBuf, dwBufSize);
        }
        
        num_sg--;
        if (num_sg == 0) {
            break;
        }
        prwBuf++;
        pBuf = MapPtrToProcess((LPVOID)prwBuf->lpBuf, GetCallerProcess());
        dwBufSize = prwBuf->dwSize;
    }   // while sg
#ifdef EML_WIN32
    {
        DWORD dwOldProcted;
        Win32_VirtualProtect( diskData.lpRomDisk, ROM_SIZE, PAGE_NOACCESS, &dwOldProcted );
    }
#endif


ddi_exit:
    pdrw->dwStatus = status;
	LeaveCriticalSection( &csRomDisk );
    return status;
}   // DoDiskIO

BOOL
ROM_IOControl(
    DWORD handle,
    DWORD dwIoControlCode,
    LPVOID pInBuf,
    DWORD nInBufSize,
    LPVOID pOutBuf,
    DWORD nOutBufSize,
    PDWORD pBytesReturned
    )
{
    PDISK_RW pdrw;
	DWORD dwStatus;

    //
    // Execute dwIoControlCode
    //
    switch (dwIoControlCode) {
    case IOCTL_DISK_READ:
        pdrw = (PDISK_RW)pInBuf;
		dwStatus = DoDiskIO( (HANDLE)handle,  dwIoControlCode, pdrw );
		if(  dwStatus != ERROR_SUCCESS )
		{
			dwStatus = FALSE;
		}
		else
			dwStatus = TRUE;
		return dwStatus;
    case IOCTL_DISK_WRITE:
#ifdef EML_WIN32
		{
			pdrw = (PDISK_RW)pInBuf;
			dwStatus = DoDiskIO( (HANDLE)handle,  dwIoControlCode, pdrw );
			if( dwStatus != ERROR_SUCCESS )
			{
				dwStatus = FALSE;
			}
			else
				dwStatus = TRUE;
			return dwStatus;
		}
		return FALSE;		
#else
		return FALSE;
#endif
		break;
    case IOCTL_DISK_GETINFO:
		{
			PDISK_INFO pdi = (PDISK_INFO)pInBuf;
			pdi->nTotalSectors = BLOCK_NUMBER;
			pdi->nBytesPerSector = BLOCK_SIZE;
			pdi->nCylinders = BLOCK_NUMBER;//16;
			pdi->nHeadsPerCylinder = 1;//2;//1;
			pdi->nSectorsPerTrack = pdi->nTotalSectors / 1;//(16*2);
			pdi->dwFlags = DISK_INFO_FLAG_CHS_UNCERTAIN | DISK_INFO_FLAG_UNFORMATTED;
		}
        return TRUE;
    case IOCTL_DISK_SETINFO:
		RETAILMSG(1, (TEXT("ROMDISK: DISK_IOCTL_SETINFO\r\n")));		
        return TRUE;
    case IOCTL_DISK_INITIALIZED:
        //
        // Load and initialize the associated file system driver
        //

		RETAILMSG(1, (TEXT("ROMDISK: DISK_IOCTL_INITIALIZED-ENTRY.\r\n")));
        if ( pInBuf && 
			( (LPDISK_INIT)pInBuf)->hDevice && 
             Dev_LoadFSD(((LPDISK_INIT)pInBuf)->hDevice, "KFSD") ) 
        {
            RETAILMSG(1, (TEXT("ROMDSK: InitFSD succeeded\r\n")));
        } else {
            RETAILMSG(1, (TEXT("ROMDSK: InitFSD failed\r\n")));
        }
        return TRUE;
    case IOCTL_DISK_GETNAME:
		{
			static const TCHAR tcVol[] = "Kingmos";
		if( nOutBufSize >= sizeof( tcVol ) ) 
		{
			strcpy( pOutBuf, tcVol );
			*pBytesReturned = sizeof( tcVol );
		}
		else
		    *pBytesReturned = 0;//(DWORD)"Kingmos";
		}
        return TRUE;    
    default:
        return FALSE;
    }
}

DWORD ROM_Open( DWORD handle, DWORD dwAccess, DWORD dwShareMode )
{
    _LPDISK_DATA lpdd = (_LPDISK_DATA)handle;

    RETAILMSG(1, (TEXT("ROMDISK: ROM_Open\r\n")));

    if( lpdd->iOpenCount == 0 )
    {
    }
    lpdd->iOpenCount++;
    return (DWORD)lpdd;
}

BOOL  ROM_Close( DWORD handle )
{
    _LPDISK_DATA lpdd = (_LPDISK_DATA)handle;

    RETAILMSG(1, (TEXT("ROMDISK: ROM_Close\r\n")));

    if( lpdd->iOpenCount > 0 )
    {
        lpdd->iOpenCount--;
    }

    return TRUE;
}

DWORD ROM_Read( DWORD handle, LPVOID lpBuffer, DWORD dwNumBytes )
{
    RETAILMSG(1, (TEXT("ROMDISK: ROM_Read\r\n")));
	return 0;
}

DWORD ROM_Write( DWORD handle, LPCVOID lpBuffer, DWORD dwNumBytes )
{
    RETAILMSG(1, (TEXT("ROMDISK: ROM_Write\r\n")));
	return 0;
}

DWORD ROM_Seek( DWORD handle, long lDistance, DWORD dwMoveMethod )
{
    RETAILMSG(1, (TEXT("ROMDISK: ROM_Seek\r\n")));    
	return 0;
}

VOID  ROM_PowerUp( DWORD handle )
{
    RETAILMSG(1, (TEXT("ROMDISK: ROM_PowerUp\r\n")));
}

VOID  ROM_PowerDown( DWORD handle )
{
    //RETAILMSG(1, (TEXT("ROMDISK: ROM_PowerDown\r\n")));
}





