#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

static DWORD g_dwSectorSize=512;

typedef struct _FSD_DISK_DATA
{ 
    DWORD nSectors;
    DWORD nBytesPerSector;
    DWORD nCylinders;
    DWORD nHeadsPerCylinder;
    DWORD nSectorsPerTrack;
    DWORD dwFlags;
} FSD_DISK_DATA, * PFDD;


DWORD WINAPI WinReadDisk(HANDLE  hDsk, DWORD dwSector, DWORD cSectors, void * pBuffer, DWORD cbBuffer)
{
	DWORD dwRet;
	DWORD dwError;
	int		iCount=0;

	if( g_dwSectorSize * cSectors > cbBuffer )
		return -1;
	if( SetFilePointer( hDsk, dwSector*g_dwSectorSize,NULL, FILE_BEGIN ) == dwSector*g_dwSectorSize){

		while(!ReadFile( hDsk,pBuffer, cSectors*g_dwSectorSize,&dwRet ,NULL ) ){

			iCount++;
			if( iCount >3)
				return -1;

			if( SetFilePointer( hDsk, dwSector*g_dwSectorSize,NULL, FILE_BEGIN ) != dwSector*g_dwSectorSize){

				return -1;
			}
		}
		if( dwRet == g_dwSectorSize * cSectors ){
			return 0;
		}else{

			dwError=GetLastError( );
		}
	}
	dwError=GetLastError( );
	return -1;
}

DWORD WINAPI WinWriteDisk(HANDLE  hDsk, DWORD dwSector, DWORD cSectors, const void * pBuffer, DWORD cbBuffer)
{
	DWORD dwRet ,dwError;
	int		iCount=0;

	if( g_dwSectorSize * cSectors > cbBuffer )
		return -1;

//	SetFilePointer( hDsk, dwSector*g_dwSectorSize,NULL, FILE_BEGIN );
	if( SetFilePointer( hDsk, dwSector*g_dwSectorSize,NULL, FILE_BEGIN ) == dwSector*g_dwSectorSize){

		while( !WriteFile( hDsk,pBuffer, cSectors*g_dwSectorSize,&dwRet ,NULL ) ){

			DWORD	dwError=GetLastError( );
			if( dwError==19 ){

				MessageBox( NULL,"Write protected","Error:",MB_OK|MB_SETFOREGROUND); 
			}
			iCount++;
			if( iCount >3)
				return -1;
			if( SetFilePointer( hDsk, dwSector*g_dwSectorSize,NULL, FILE_BEGIN ) != dwSector*g_dwSectorSize){

				return -1;
			}
		}
		if( dwRet == g_dwSectorSize * cSectors ){

			if(! FlushFileBuffers( hDsk) ){

				dwError=GetLastError( );
			}
			return 0;
		}else{

			dwError=GetLastError( );
		}
	}
	dwError=GetLastError( );
	return -1;
}


DWORD WINAPI WinGetDiskInfo( HANDLE  hDsk, void* pDiskInfo )
{
	DISK_GEOMETRY	dg_Info;
	DWORD			dwRet;
	PFDD pfdd =(PFDD)pDiskInfo;

	if( DeviceIoControl( hDsk, 
				  IOCTL_DISK_GET_DRIVE_GEOMETRY, // dwIoControlCode
				  NULL,                          // lpInBuffer
				  0,                             // nInBufferSize
				  (LPVOID) &dg_Info,          // output buffer
				  (DWORD) sizeof(dg_Info),        // size of output buffer
				  (LPDWORD) &dwRet,     // number of bytes returned
				  (LPOVERLAPPED) NULL// OVERLAPPED structure
				  ) ){

		pfdd->nBytesPerSector= dg_Info.BytesPerSector;
		pfdd->nSectors=dg_Info.Cylinders.LowPart *dg_Info.SectorsPerTrack*dg_Info.TracksPerCylinder;
		pfdd->dwFlags=0;
		g_dwSectorSize=dg_Info.BytesPerSector;
		return 0;
	}
	return -1;
}


static HANDLE    g_hDiskDevice=NULL;

HANDLE  InitDisk( void )
{
	int  i;
	char	csName[100];

	for( i=5;i>0 ;i--){

		sprintf( csName,"\\\\.\\PHYSICALDRIVE%d",i);
		g_hDiskDevice= CreateFile( csName, GENERIC_READ | GENERIC_WRITE, 0,//FILE_SHARE_READ | FILE_SHARE_WRITE,
												   NULL, OPEN_EXISTING,  FILE_FLAG_NO_BUFFERING,  NULL );

		if( g_hDiskDevice !=INVALID_HANDLE_VALUE ){
			break;
		}
	}
	if( g_hDiskDevice ==INVALID_HANDLE_VALUE ){
		g_hDiskDevice =GetLastError( );
		g_hDiskDevice =0;
		return g_hDiskDevice;
	}
	if( SetFilePointer( g_hDiskDevice, 0, NULL, FILE_BEGIN ) ){

		CloseHandle( g_hDiskDevice );
		g_hDiskDevice=NULL;
		return g_hDiskDevice;
	}
	return g_hDiskDevice;
}
void DeinitATADisk( HANDLE handle ) 
{
	CloseHandle( handle);
}
