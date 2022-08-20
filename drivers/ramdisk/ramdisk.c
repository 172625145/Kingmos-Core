/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：ramdisk
版本号：1.0.0
开发时期：2001-07-02
作者：zb
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

#undef MapPtrToProcess
#define MapPtrToProcess( pBuf, handle ) (pBuf)
#undef GetCallerProcess
#define GetCallerProcess

#define STATIC static 
STATIC DWORD RAM_Init( DWORD dwContext );
STATIC BOOL RAM_Deinit( DWORD handle );
STATIC DWORD RAM_Open( DWORD handle, DWORD dwAccess, DWORD dwShareMode );
STATIC BOOL  RAM_Close( DWORD handle );
STATIC DWORD RAM_Read( DWORD handle, LPVOID lpBuffer, DWORD dwNumBytes );
STATIC DWORD RAM_Write( DWORD handle, LPCVOID lpBuffer, DWORD dwNumBytes );
STATIC DWORD RAM_Seek( DWORD handle, long lDistance, DWORD dwMoveMethod );
STATIC VOID  RAM_PowerUp( DWORD handle );
STATIC VOID  RAM_PowerDown( DWORD handle );
STATIC BOOL  RAM_IOControl( DWORD handle,DWORD dwIoControlCode,LPVOID pInBuf,DWORD nInBufSize,LPVOID pOutBuf,DWORD nOutBufSize,PDWORD pBytesReturned);

static BOOL ClearRoot( char * lpSrcRootPath, char * lpSubPath );

static const DEVICE_DRIVER drvRamDisk={
    RAM_Init,
    RAM_Deinit,
    RAM_IOControl,
    RAM_Open,
    RAM_Close,
    RAM_Read,
    RAM_Write,
    RAM_Seek,
    RAM_PowerUp,
    RAM_PowerDown
};

#define BLOCK_SIZE 512l

#define RAM_SIZE (1024l * 512 * 1)

#define BLOCK_NUMBER  (RAM_SIZE/BLOCK_SIZE)
// align to dword
typedef struct __DISK_DATA
{
    int  iOpenCount;
	BYTE * lpRamDisk;
    WORD wState;
}_DISK_DATA, FAR * _LPDISK_DATA;

static _DISK_DATA  diskData;



BOOL _LoadRamdisk( void )
{
    HANDLE hDev = RegisterDriver( "RAM", 0, &drvRamDisk, 0 );
    HANDLE hFile;
    DISK_INIT di;
    
    RETAILMSG( 1, ( ".....................................................\r\n" ) );
	RETAILMSG( 1, ( "_LoadRamdisk:hDev=%x\r\n", hDev ) );
    hFile = CreateFile( "RAM0:", GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
					   0, NULL);    
	RETAILMSG( 1, ( "_LoadRamdisk:hFile=%x\r\n", hFile ) );
    if(hFile != INVALID_HANDLE_VALUE) {
        di.hDevice = hDev;
        DeviceIoControl(hFile, IOCTL_DISK_INITIALIZED, &di, sizeof(di), NULL, 0, NULL, NULL);
    }

    if( hFile != INVALID_HANDLE_VALUE ){
        CloseHandle(hFile);

        return TRUE;
    }
    return FALSE;
}


static CRITICAL_SECTION csRamDisk;

DWORD RAM_Init( DWORD dwContext )
{
    static DWORD fileBuf[RAM_SIZE/4];

    RETAILMSG(1, (TEXT("RAMDISK: RAM_Init\r\n")));
	InitializeCriticalSection( &csRamDisk );
	csRamDisk.lpcsName = "CS-RAMDISK";

    memset( &diskData, 0, sizeof( diskData ) );
	diskData.lpRamDisk = (LPBYTE)fileBuf;

	memset( fileBuf, 0, RAM_SIZE );

	if( diskData.lpRamDisk )
		return (DWORD)&diskData; 
	else
		return 0;
}

BOOL RAM_Deinit( DWORD handle )
{
    RETAILMSG(1, (TEXT("RAMDISK: Deinit\r\n")));
	diskData.lpRamDisk = 0;
	DeleteCriticalSection( &csRamDisk );
    return TRUE;
}

#define ERROR_IO_PENDING  100
#define MAX_RW_BUF 1

static DWORD
DoDiskIO(
    HANDLE handle,
    DWORD Opcode,
    PDISK_RW prwd
    )
{
    DWORD status = ERROR_SUCCESS;
    DWORD nRwBufNum;
    DWORD curr_byte;
    DWORD dwBufSize;
    PRW_BUF prwBuf;
    PBYTE pBuf;
    volatile PBYTE pCard;

	EnterCriticalSection( &csRamDisk );


    prwd->dwStatus = ERROR_IO_PENDING;

    if (prwd->nrwNum > MAX_RW_BUF) {
        status = ERROR_INVALID_PARAMETER;
        goto ddi_exit;
    }

    if ((prwd->dwStartSector + prwd->dwSectorNumber - 1) > BLOCK_NUMBER) {
        status = ERROR_SECTOR_NOT_FOUND;
        goto ddi_exit;
    }
    status = ERROR_SUCCESS;
    curr_byte = prwd->dwStartSector * BLOCK_SIZE;

    nRwBufNum = prwd->nrwNum;
    prwBuf = &(prwd->rwBufs[0]);
    dwBufSize = prwBuf->dwSize;
    pBuf = MapPtrToProcess((LPVOID)prwBuf->lpBuf, GetCallerProcess());

    while (nRwBufNum) {
        pCard = (LPBYTE)diskData.lpRamDisk + curr_byte;
        if (Opcode == IOCTL_DISK_READ) {
            memcpy(pBuf, pCard, dwBufSize);
        } else {
            memcpy(pCard, pBuf, dwBufSize);
        }
        
        nRwBufNum--;
        if (nRwBufNum == 0) {
            break;
        }
        prwBuf++;
        pBuf = MapPtrToProcess((LPVOID)prwBuf->lpBuf, GetCallerProcess());
        dwBufSize = prwBuf->dwSize;
    }   // while sg


ddi_exit:
    prwd->dwStatus = status;
	LeaveCriticalSection( &csRamDisk );
    return status;
}   // DoDiskIO

BOOL
RAM_IOControl(
    DWORD handle,
    DWORD dwIoControlCode,
    LPVOID pInBuf,
    DWORD nInBufSize,
    LPVOID pOutBuf,
    DWORD nOutBufSize,
    PDWORD pBytesReturned
    )
{
    PDISK_RW pSG;
	DWORD dwStatus;

    // Execute dwIoControlCode

    switch (dwIoControlCode) {
    case IOCTL_DISK_READ:
        pSG = (PDISK_RW)pInBuf;
		dwStatus = DoDiskIO( (HANDLE)handle,  dwIoControlCode, pSG );
		if(  dwStatus != ERROR_SUCCESS )
		{
			dwStatus = FALSE;
		}
		else
			dwStatus = TRUE;
		return dwStatus;
    case IOCTL_DISK_WRITE:
        pSG = (PDISK_RW)pInBuf;
		dwStatus = DoDiskIO( (HANDLE)handle,  dwIoControlCode, pSG );
		if( dwStatus != ERROR_SUCCESS )
		{
			dwStatus = FALSE;
		}
		else
			dwStatus = TRUE;
		return dwStatus;
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
		RETAILMSG(1, (TEXT("RAMDISK: DISK_IOCTL_SETINFO\r\n")));		
        return TRUE;
    case IOCTL_DISK_INITIALIZED:

		// 将该disk与一个文件系统关联

		RETAILMSG(1, (TEXT("RAMDISK: DISK_IOCTL_INITIALIZED-ENTRY.\r\n")));
        if ( pInBuf && 
			( (LPDISK_INIT)pInBuf)->hDevice && 
             Dev_LoadFSD(((LPDISK_INIT)pInBuf)->hDevice, "KFSD") ) 
        {
            RETAILMSG(1, (TEXT("RAMDSK: InitFSD succeeded\r\n")));
        } else {
            RETAILMSG(1, (TEXT("RAMDSK: InitFSD failed\r\n")));
        }
        return TRUE;
    case IOCTL_DISK_GETNAME:
		*pBytesReturned = 0;
        return TRUE;
    
    default:
        return FALSE;
    }
}

DWORD RAM_Open( DWORD handle, DWORD dwAccess, DWORD dwShareMode )
{
    _LPDISK_DATA lpdd = (_LPDISK_DATA)handle;

    RETAILMSG(1, (TEXT("RAMDISK: RAM_Open\r\n")));

    if( lpdd->iOpenCount == 0 )
    {
    }
    lpdd->iOpenCount++;
    return (DWORD)lpdd;
}

BOOL  RAM_Close( DWORD handle )
{
    _LPDISK_DATA lpdd = (_LPDISK_DATA)handle;

    RETAILMSG(1, (TEXT("RAMDISK: RAM_Close\r\n")));

    if( lpdd->iOpenCount > 0 )
    {
        lpdd->iOpenCount--;
    }

    return TRUE;
}

DWORD RAM_Read( DWORD handle, LPVOID lpBuffer, DWORD dwNumBytes )
{
    RETAILMSG(1, (TEXT("RAMDISK: RAM_Read\r\n")));
	return 0;
}

DWORD RAM_Write( DWORD handle, LPCVOID lpBuffer, DWORD dwNumBytes )
{
    RETAILMSG(1, (TEXT("RAMDISK: RAM_Write\r\n")));
	return 0;
}

DWORD RAM_Seek( DWORD handle, long lDistance, DWORD dwMoveMethod )
{
    RETAILMSG(1, (TEXT("RAMDISK: RAM_Seek\r\n")));    
	return 0;
}

VOID  RAM_PowerUp( DWORD handle )
{
    RETAILMSG(1, (TEXT("RAMDISK: RAM_PowerUp\r\n")));
}

VOID  RAM_PowerDown( DWORD handle )
{
    //RETAILMSG(1, (TEXT("RAMDISK: RAM_PowerDown\r\n")));
}





