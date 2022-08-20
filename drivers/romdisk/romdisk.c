#include <edef.h>
#include <eassert.h>
#include <ecore.h>
#include <eerror.h>
#include <edevice.h>
#include <efile.h>
//#include <efsdmgr.h>
#include <estring.h>
#include <devdrv.h>

#define STATIC static 

STATIC DWORD ROM_Init( DWORD dwContext );
STATIC BOOL  ROM_Deinit( DWORD handle );
STATIC DWORD ROM_Open( DWORD handle, DWORD dwAccess, DWORD dwShareMode );
STATIC BOOL  ROM_Close( DWORD handle );
STATIC DWORD ROM_Read( DWORD handle, LPVOID lpBuffer, DWORD dwNumBytes );
STATIC DWORD ROM_Write( DWORD handle, LPCVOID lpBuffer, DWORD dwNumBytes );
STATIC DWORD ROM_Seek( DWORD handle, long lDistance, DWORD dwMoveMethod );
STATIC VOID  ROM_PowerUp( DWORD handle );
STATIC VOID  ROM_PowerDown( DWORD handle );
STATIC BOOL  ROM_IOControl( DWORD handle,DWORD dwIoControlCode, LPVOID pInBuf,DWORD nInBufSize,LPVOID pOutBuf,DWORD nOutBufSize,PDWORD pBytesReturned);



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

//The ROM_SIZE is adopted for test by zb.
//#define ROM_SIZE (1024l * 512 * 6)
#define ROM_SIZE (1024l * 512 * 2)


#define BLOCK_NUMBER  (ROM_SIZE/BLOCK_SIZE)
// align to dword
typedef struct __DISK_DATA
{
    int  iOpenCount;
	BYTE * lpRamDisk;
    WORD wState;
}_DISK_DATA, FAR * _LPDISK_DATA;

static _DISK_DATA  diskData;

BOOL _LoadRomdisk( void )
{
    HANDLE hDev = RegisterDriver( "ROM", 0, &drvRomDisk, 0 );
    HANDLE hFile;
    DISK_INIT di;
    
    RETAILMSG( 1, ( ".....................................\r\n" ) );
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
        return TRUE;
    }
    return FALSE;
}

#define	ROM_DISK_HANDLE		0x0000000a;

DWORD ROM_Init( DWORD dwContext )
{

	RETAILMSG(1, (TEXT("RAMDISK: ROM_Init\r\n")));

	return ROM_DISK_HANDLE;
}

BOOL ROM_Deinit( DWORD handle )
{
	RETAILMSG(1, (TEXT("RAMDISK: ROM_DEInit\r\n")));

    return TRUE;
}


BOOL
ROM_IOControl(
    DWORD handle,
    DWORD dwIoControlCode,
    LPVOID pInBuf,
    DWORD  nInBufSize,
    LPVOID pOutBuf,
    DWORD  nOutBufSize,
    PDWORD pBytesReturned
    )
{

    //
    // Execute dwIoControlCode
    //
    switch (dwIoControlCode) {
    case IOCTL_DISK_GETNAME:
		RETAILMSG(1, (TEXT("RAMDISK: DISK_IOCTL_GETNAME\r\n")));

		strcpy( pOutBuf, TEXT("kingmos"));
		// system default disk , set to 0
		*pBytesReturned = strlen(TEXT("kingmos"));
        return TRUE;   
    case IOCTL_DISK_INITIALIZED:
        //
        // Load and initialize the associated file system driver
        //
		RETAILMSG(1, (TEXT("RAMDISK: DISK_IOCTL_INITIALIZED-ENTRY.\r\n")));

        if ( pInBuf && 
			( (LPDISK_INIT)pInBuf)->hDevice && 
			//FSDMGR_LoadFSD(((LPDISK_INIT)pInBuf)->hDevice, TEXT("RAMFSD.DLL"))) {
             Dev_LoadFSD(((LPDISK_INIT)pInBuf)->hDevice, "romfs")) 
        {
            RETAILMSG(1, (TEXT("ROMDSK: InitFSD succeeded\r\n")));
        } else {
            RETAILMSG(1, (TEXT("ROMDSK: InitFSD failed\r\n")));
        }
        return TRUE;
    default:
        return FALSE;
    }
}

DWORD ROM_Open( DWORD handle, DWORD dwAccess, DWORD dwShareMode )
{

    RETAILMSG(1, (TEXT("RAMDISK: ROM_Open\r\n")));

    return (DWORD)handle;
}

BOOL  ROM_Close( DWORD handle )
{
    RETAILMSG(1, (TEXT("RAMDISK: ROM_Close\r\n")));

    return TRUE;
}

DWORD ROM_Read( DWORD handle, LPVOID lpBuffer, DWORD dwNumBytes )
{
    RETAILMSG(1, (TEXT("RAMDISK: ROM_Read\r\n")));
	return 0;
}

DWORD ROM_Write( DWORD handle, LPCVOID lpBuffer, DWORD dwNumBytes )
{
    RETAILMSG(1, (TEXT("RAMDISK: ROM_Write\r\n")));
	return 0;
}

DWORD ROM_Seek( DWORD handle, long lDistance, DWORD dwMoveMethod )
{
    RETAILMSG(1, (TEXT("RAMDISK: ROM_Seek\r\n")));    
	return 0;
}

VOID  ROM_PowerUp( DWORD handle )
{
    RETAILMSG(1, (TEXT("RAMDISK: ROM_PowerUp\r\n")));
}

VOID  ROM_PowerDown( DWORD handle )
{
    RETAILMSG(1, (TEXT("RAMDISK: ROM_PowerDown\r\n")));
}


