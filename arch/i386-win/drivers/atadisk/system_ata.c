#include <ewindows.h>
#include <edevice.h>


extern DWORD WINAPI WinReadDisk(HANDLE hDsk, DWORD dwSector, DWORD cSectors, void * pBuffer, DWORD cbBuffer);
extern DWORD WINAPI WinWriteDisk(HANDLE hDsk, DWORD dwSector, DWORD cSectors, const void * pBuffer, DWORD cbBuffer);
extern DWORD WINAPI WinGetDiskInfo( HANDLE  hDsk, void* pfdd );


HANDLE    g_hDiskDevice=NULL;
static	DWORD g_dwSectorSize=512;
HANDLE  InitDisk( void );

typedef struct _SG_BUF {
	LPBYTE sb_buf;        // pointer to buffer
	DWORD  sb_len;        // length of buffer
} SG_BUF, *PSG_BUF;

typedef struct _SG_REQ {
    DWORD sr_start;     // starting sector number
	DWORD sr_num_sec;   // number of sectors
	DWORD sr_num_sg;    // number of scatter/gather buffers
    DWORD sr_status;    // request status
	SG_BUF sr_sglist[1];   // first scatter/gather buffer
} SG_REQ, * PSG_REQ;


DWORD
DSK_Init(
    DWORD dwContext
    )
{
/*	if( !g_hDiskDevice ) //PHYSICALDRIVEx PHYSICALDRIVE1  H:
		g_hDiskDevice= CreateFile("\\\\.\\PHYSICALDRIVE1", GENERIC_READ | GENERIC_WRITE, 0,//FILE_SHARE_READ | FILE_SHARE_WRITE,
											   NULL, OPEN_EXISTING,  FILE_FLAG_NO_BUFFERING,  NULL );

	if( g_hDiskDevice ==INVALID_HANDLE_VALUE ){
		g_hDiskDevice=NULL;
		return NULL;
	}
	if( SetFilePointer( g_hDiskDevice, 0, NULL, FILE_BEGIN ) ){

		CloseHandle( g_hDiskDevice );
		g_hDiskDevice=NULL;
		return g_hDiskDevice;
	}*/
	g_hDiskDevice=InitDisk( );
	return (DWORD)g_hDiskDevice;

}   // DSK_Init


BOOL
DSK_Close(
    DWORD handle
    )
{
	return TRUE;
}   // DSK_Close


//
// Device deinit - devices are expected to close down.
// The device manager does not check the return code.
//
BOOL
DSK_Deinit(
     DWORD handle      // pointer to the per disk structure
    )
{
	void DeinitATADisk( HANDLE handle );

	DeinitATADisk( (HANDLE)handle );
	return TRUE;
}   // DSK_Deinit

//
// Returns handle value for the open instance.
//
DWORD
DSK_Open(
    DWORD handle,
    DWORD dwAccess,
    DWORD dwShareMode
    )
{
	return handle;
}   // DSK_Open


//
// I/O Control function - responds to info, read and write control codes.
// The read and write take a scatter/gather list in pInBuf
//
BOOL
DSK_IOControl(
	DWORD handle, 
	DWORD dwCode, 
	LPVOID lpvInBuf, 
	DWORD nInBufSize, 
	LPVOID lpvOutBuf, 
	DWORD nOutBufSize, 
	LPDWORD lpdwReturned 
	)
{
    PSG_REQ pSG;

	switch( dwCode ){
		case IOCTL_DISK_READ:
			pSG = (PSG_REQ)lpvInBuf;
			return WinReadDisk((HANDLE)handle,pSG->sr_start, pSG->sr_num_sec,pSG->sr_sglist[0].sb_buf,pSG->sr_sglist[0].sb_len) ?FALSE: TRUE; 

		case IOCTL_DISK_WRITE:

			pSG = (PSG_REQ)lpvInBuf;
			return WinWriteDisk((HANDLE)handle,pSG->sr_start, pSG->sr_num_sec,pSG->sr_sglist[0].sb_buf,pSG->sr_sglist[0].sb_len) ?FALSE: TRUE; 
		case IOCTL_DISK_GETINFO:
			return WinGetDiskInfo((HANDLE)handle ,lpvInBuf) ? FALSE:TRUE;
		case IOCTL_DISK_INITIALIZED:
			FSDMGR_LoadFSD( lpvInBuf,"FATFSD") ;

	}
	return FALSE;
}   // DSK_IOControl




DWORD DSK_Read(DWORD handle, LPVOID pBuffer, DWORD dwNumBytes){return 0;}
DWORD DSK_Write(DWORD handle, LPCVOID pBuffer, DWORD dwNumBytes){return 0;}
DWORD DSK_Seek(DWORD handle, long lDistance, DWORD dwMoveMethod){return 0;}
void DSK_PowerUp(DWORD handle){}
void DSK_PowerDown(DWORD handle){}


BOOL Load_ATA_Driver( void )
{

	static 	DEVICE_DRIVER   driver_ne2000={
			DSK_Init,
			DSK_Deinit,
			DSK_IOControl,
			DSK_Open,
			DSK_Close,
			DSK_Read,
			DSK_Write,
			DSK_Seek,
			DSK_PowerUp,
			DSK_PowerDown
	};

    HANDLE hDev = RegisterDriver( "DSK", 5, &driver_ne2000, 0 );
	if( hDev != INVALID_HANDLE_VALUE){

		HANDLE hFile=CreateFile("DSK5:",GENERIC_READ|GENERIC_WRITE,0,FILE_SHARE_READ | FILE_SHARE_WRITE,OPEN_EXISTING,0,0);
		if( hFile != INVALID_HANDLE_VALUE){

			DeviceIoControl(hFile,IOCTL_DISK_INITIALIZED,hDev,4,0,0,0,0);
		}

		hDev=GetLastError( );

		return TRUE;
	}
  
	return FALSE;
}




