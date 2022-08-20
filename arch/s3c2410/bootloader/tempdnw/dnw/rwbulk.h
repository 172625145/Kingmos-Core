#ifndef __RWBULK_H__
#define __RWBULK_H__

#include <windows.h>
#include <setupapi.h>

#ifdef __cplusplus
extern "C" {
#endif

HANDLE OpenOneDevice (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_INTERFACE_DEVICE_DATA   DeviceInfoData,
    IN       char *devName
    );

HANDLE OpenUsbDevice( LPGUID  pGuid, char *outNameBuf);

BOOL GetUsbDeviceFileName( LPGUID  pGuid, char *outNameBuf);

HANDLE open_dev();

HANDLE open_file( char *filename);

int  dumpUsbConfig();

extern char inPipe[32];
extern char outPipe[32]; 

#ifdef __cplusplus
}
#endif

#endif //__RWBULK_H__

