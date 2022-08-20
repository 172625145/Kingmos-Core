/*++

    Modified RWBulk.c

--*/


#include <windows.h>

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "devioctl.h"

#include <setupapi.h>
#include <basetyps.h>
#include "BulkUsb.h"
#include "GUID.h"

#include "usbdi.h"

void EB_Printf(TCHAR *fmt,...); //engine.h


#define NOISY(_x_) EB_Printf _x_ ;

char inPipe[32] = "PIPE00";     // pipe name for bulk input pipe on our test board
char outPipe[32] = "PIPE01";    // pipe name for bulk output pipe on our test board
char completeDeviceName[256] = "";  //generated from the GUID registered by the driver itself

/*
BOOL fDumpUsbConfig = FALSE;    // flags set in response to console command line switches
BOOL fDumpReadData = FALSE;
BOOL fRead = FALSE;
BOOL fWrite = FALSE;


int gDebugLevel = 1;      // higher == more verbose, default is 1, 0 turns off all

ULONG IterationCount = 1; //count of iterations of the test we are to perform
int WriteLen = 0;         // #bytes to write
int ReadLen = 0;          // #bytes to read
*/
// functions


HANDLE
OpenOneDevice (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_INTERFACE_DEVICE_DATA   DeviceInfoData,
    IN       char *devName
    )
/*++
Routine Description:

    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific usb device,
    open that device and fill in all the relevant information in the given
    USB_DEVICE_DESCRIPTOR structure.

Arguments:

    HardwareDeviceInfo:  handle to info obtained from Pnp mgr via SetupDiGetClassDevs()
    DeviceInfoData:      ptr to info obtained via SetupDiEnumInterfaceDevice()

Return Value:

    return HANDLE if the open and initialization was successfull,
    else INVLAID_HANDLE_VALUE.

--*/
{
    PSP_INTERFACE_DEVICE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
    HANDLE                               hOut = INVALID_HANDLE_VALUE;

    //
    // allocate a function class device data structure to receive the
    // goods about this particular device.
    //
    SetupDiGetInterfaceDeviceDetail (
	    HardwareDeviceInfo,
	    DeviceInfoData,
	    NULL, // probing so no output buffer yet
	    0, // probing so output buffer length of zero
	    &requiredLength,
	    NULL); // not interested in the specific dev-node


    predictedLength = requiredLength;
    // sizeof (SP_FNCLASS_DEVICE_DATA) + 512;

    functionClassDeviceData = malloc (predictedLength);
    functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

    //
    // Retrieve the information from Plug and Play.
    //
    if (! SetupDiGetInterfaceDeviceDetail (
	       HardwareDeviceInfo,
	       DeviceInfoData,
	       functionClassDeviceData,
	       predictedLength,
	       &requiredLength,
	       NULL)) {
	free( functionClassDeviceData );
	return INVALID_HANDLE_VALUE;
    }

    strcpy( devName,functionClassDeviceData->DevicePath) ;
    //EB_Printf( "Attempting to open %s\n", devName );

    hOut = CreateFile (
		  functionClassDeviceData->DevicePath,
		  GENERIC_READ | GENERIC_WRITE,
		  FILE_SHARE_READ | FILE_SHARE_WRITE,
		  NULL, // no SECURITY_ATTRIBUTES structure
		  OPEN_EXISTING, // No special create flags
		  0, // No special attributes
		  NULL); // No template file

    if (INVALID_HANDLE_VALUE == hOut) {
	//EB_Printf( "FAILED to open %s\n", devName );
    }
    free( functionClassDeviceData );
    return hOut;
}


HANDLE
OpenUsbDevice( LPGUID  pGuid, char *outNameBuf)
/*++
Routine Description:

   Do the required PnP things in order to find
   the next available proper device in the system at this time.

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated name for this device

Return Value:

    return HANDLE if the open and initialization was successful,
    else INVLAID_HANDLE_VALUE.
--*/
{
   ULONG NumberDevices;
   HANDLE hOut = INVALID_HANDLE_VALUE;
   HDEVINFO                 hardwareDeviceInfo;
   SP_INTERFACE_DEVICE_DATA deviceInfoData;
   ULONG                    i;
   BOOLEAN                  done;
   PUSB_DEVICE_DESCRIPTOR   usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   *UsbDevices = &usbDeviceInst;

   *UsbDevices = NULL;
   NumberDevices = 0;

   //
   // Open a handle to the plug and play dev node.
   // SetupDiGetClassDevs() returns a device information set that contains info on all
   // installed devices of a specified class.
   //
   hardwareDeviceInfo = SetupDiGetClassDevs (
			   pGuid,
			   NULL, // Define no enumerator (global)
			   NULL, // Define no
			   (DIGCF_PRESENT | // Only Devices present
			    DIGCF_INTERFACEDEVICE)); // Function class devices.

   //
   // Take a wild guess at the number of devices we have;
   // Be prepared to realloc and retry if there are more than we guessed
   //
   NumberDevices = 4;
   done = FALSE;
   deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

   i=0;
   while (!done) {
      NumberDevices *= 2;

      if (*UsbDevices) {
	 *UsbDevices =
	       realloc (*UsbDevices, (NumberDevices * sizeof (USB_DEVICE_DESCRIPTOR)));
      } else {
	 *UsbDevices = calloc (NumberDevices, sizeof (USB_DEVICE_DESCRIPTOR));
      }

      if (NULL == *UsbDevices) {

	 // SetupDiDestroyDeviceInfoList destroys a device information set
	 // and frees all associated memory.

	 SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
	 return INVALID_HANDLE_VALUE;
      }

      usbDeviceInst = *UsbDevices + i;

      for (; i < NumberDevices; i++) {

	 // SetupDiEnumDeviceInterfaces() returns information about device interfaces
	 // exposed by one or more devices. Each call returns information about one interface;
	 // the routine can be called repeatedly to get information about several interfaces
	 // exposed by one or more devices.

	 if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
					 0, // We don't care about specific PDOs
					 pGuid,
					 i,
					 &deviceInfoData)) {

	    hOut = OpenOneDevice (hardwareDeviceInfo, &deviceInfoData, outNameBuf);
	    if ( hOut != INVALID_HANDLE_VALUE ) {
	       done = TRUE;
	       break;
	    }
	 } else {
	    if (ERROR_NO_MORE_ITEMS == GetLastError()) {
	       done = TRUE;
	       break;
	    }
	 }
      }
   }

   NumberDevices = i;

   // SetupDiDestroyDeviceInfoList() destroys a device information set
   // and frees all associated memory.

   SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
   free ( *UsbDevices );
   return hOut;
}




BOOL
GetUsbDeviceFileName( LPGUID  pGuid, char *outNameBuf)
/*++
Routine Description:

    Given a ptr to a driver-registered GUID, give us a string with the device name
    that can be used in a CreateFile() call.
    Actually briefly opens and closes the device and sets outBuf if successfull;
    returns FALSE if not

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated zero-terminated name for this device

Return Value:

    TRUE on success else FALSE

--*/
{
    HANDLE hDev = OpenUsbDevice( pGuid, outNameBuf );
    if ( hDev != INVALID_HANDLE_VALUE )
    {
	CloseHandle( hDev );
	return TRUE;
    }
    return FALSE;

}

HANDLE
open_dev()
/*++
Routine Description:

    Called by dumpUsbConfig() to open an instance of our device

Arguments:

    None

Return Value:

    Device handle on success else NULL

--*/
{

    HANDLE hDEV = OpenUsbDevice( (LPGUID)&GUID_CLASS_I82930_BULK, completeDeviceName);


    if (hDEV == INVALID_HANDLE_VALUE) {
	//EB_Printf("Failed to open (%s) = %d", completeDeviceName, GetLastError());
    } else {
	//EB_Printf("DeviceName = (%s)\n", completeDeviceName);
    }       

    return hDEV;
}


HANDLE
open_file( char *filename)
/*++
Routine Description:

    Called by main() to open an instance of our device after obtaining its name

Arguments:

    None

Return Value:

    Device handle on success else NULL

--*/
{

    int success = 1;
    HANDLE h;

    if ( !GetUsbDeviceFileName(
	(LPGUID) &GUID_CLASS_I82930_BULK,
	completeDeviceName) )
    {
	//NOISY(("Failed to GetUsbDeviceFileName:%d\n", GetLastError()));
	return  INVALID_HANDLE_VALUE;
    }

    strcat (completeDeviceName,
	    "\\"
	    );          

    strcat (completeDeviceName,
	    filename
	    );                  

    //EB_Printf("completeDeviceName = (%s)\n", completeDeviceName);

    h = CreateFile(completeDeviceName,
	GENERIC_WRITE | GENERIC_READ,
	FILE_SHARE_WRITE | FILE_SHARE_READ,
	NULL,
	OPEN_EXISTING,
	0,
	NULL);

    if (h == INVALID_HANDLE_VALUE) {
	//NOISY(("Failed to open (%s) = %d", completeDeviceName, GetLastError()));
	success = 0;
    } else {
	    //NOISY(("Opened successfully.\n"));
    }       

    return h;
}



// Begin, routines for USB configuration dump (Cmdline "rwbulk -u" )


char
*usbDescriptorTypeString(UCHAR bDescriptorType )
/*++
Routine Description:

    Called to get ascii string of USB descriptor

Arguments:

    PUSB_ENDPOINT_DESCRIPTOR->bDescriptorType or
    PUSB_DEVICE_DESCRIPTOR->bDescriptorType or
    PUSB_INTERFACE_DESCRIPTOR->bDescriptorType or
    PUSB_STRING_DESCRIPTOR->bDescriptorType or
    PUSB_POWER_DESCRIPTOR->bDescriptorType or
    PUSB_CONFIGURATION_DESCRIPTOR->bDescriptorType

Return Value:

    ptr to string

--*/{

    switch(bDescriptorType) {

    case USB_DEVICE_DESCRIPTOR_TYPE:
	return "USB_DEVICE_DESCRIPTOR_TYPE";

    case USB_CONFIGURATION_DESCRIPTOR_TYPE:
	return "USB_CONFIGURATION_DESCRIPTOR_TYPE";
	

    case USB_STRING_DESCRIPTOR_TYPE:
	return "USB_STRING_DESCRIPTOR_TYPE";
	

    case USB_INTERFACE_DESCRIPTOR_TYPE:
	return "USB_INTERFACE_DESCRIPTOR_TYPE";
	

    case USB_ENDPOINT_DESCRIPTOR_TYPE:
	return "USB_ENDPOINT_DESCRIPTOR_TYPE";
	

#ifdef USB_POWER_DESCRIPTOR_TYPE // this is the older definintion which is actually obsolete
    // workaround for temporary bug in 98ddk, older USB100.h file
    case USB_POWER_DESCRIPTOR_TYPE:
	return "USB_POWER_DESCRIPTOR_TYPE";
#endif
	
#ifdef USB_RESERVED_DESCRIPTOR_TYPE  // this is the current version of USB100.h as in NT5DDK

    case USB_RESERVED_DESCRIPTOR_TYPE:
	return "USB_RESERVED_DESCRIPTOR_TYPE";

    case USB_CONFIG_POWER_DESCRIPTOR_TYPE:
	return "USB_CONFIG_POWER_DESCRIPTOR_TYPE";

    case USB_INTERFACE_POWER_DESCRIPTOR_TYPE:
	return "USB_INTERFACE_POWER_DESCRIPTOR_TYPE";
#endif // for current nt5ddk version of USB100.h

    default:
	return "??? UNKNOWN!!"; 
    }
}


char
*usbEndPointTypeString(UCHAR bmAttributes)
/*++
Routine Description:

    Called to get ascii string of endpt descriptor type

Arguments:

    PUSB_ENDPOINT_DESCRIPTOR->bmAttributes

Return Value:

    ptr to string

--*/
{
    UINT typ = bmAttributes & USB_ENDPOINT_TYPE_MASK;


    switch( typ) {
    case USB_ENDPOINT_TYPE_INTERRUPT:
	return "USB_ENDPOINT_TYPE_INTERRUPT";

    case USB_ENDPOINT_TYPE_BULK:
	return "USB_ENDPOINT_TYPE_BULK";    

    case USB_ENDPOINT_TYPE_ISOCHRONOUS:
	return "USB_ENDPOINT_TYPE_ISOCHRONOUS"; 
	
    case USB_ENDPOINT_TYPE_CONTROL:
	return "USB_ENDPOINT_TYPE_CONTROL"; 
	
    default:
	return "??? UNKNOWN!!"; 
    }
}


char
*usbConfigAttributesString(UCHAR bmAttributes)
/*++
Routine Description:

    Called to get ascii string of USB_CONFIGURATION_DESCRIPTOR attributes

Arguments:

    PUSB_CONFIGURATION_DESCRIPTOR->bmAttributes

Return Value:

    ptr to string

--*/
{
    UINT typ = bmAttributes & USB_CONFIG_POWERED_MASK;


    switch( typ) {

    case USB_CONFIG_BUS_POWERED:
	return "USB_CONFIG_BUS_POWERED";

    case USB_CONFIG_SELF_POWERED:
	return "USB_CONFIG_SELF_POWERED";
	
    case USB_CONFIG_REMOTE_WAKEUP:
	return "USB_CONFIG_REMOTE_WAKEUP";

	
    default:
	return "??? UNKNOWN!!"; 
    }
}


void
print_USB_CONFIGURATION_DESCRIPTOR(PUSB_CONFIGURATION_DESCRIPTOR cd)
/*++
Routine Description:

    Called to do formatted ascii dump to console of a USB config descriptor

Arguments:

    ptr to USB configuration descriptor

Return Value:

    none

--*/
{              
    EB_Printf("===== USB DEVICE STATUS =====\nUSB_CONFIGURATION_DESCRIPTOR\n");

    EB_Printf(
    "bLength = 0x%x, decimal %d\n", cd->bLength, cd->bLength
    );

    EB_Printf(
    "bDescriptorType = 0x%x ( %s )\n", cd->bDescriptorType, usbDescriptorTypeString( cd->bDescriptorType )
    );

    EB_Printf(
    "wTotalLength = 0x%x, decimal %d\n", cd->wTotalLength, cd->wTotalLength
    );

    EB_Printf(
    "bNumInterfaces = 0x%x, decimal %d\n", cd->bNumInterfaces, cd->bNumInterfaces
    );

    EB_Printf(
    "bConfigurationValue = 0x%x, decimal %d\n", cd->bConfigurationValue, cd->bConfigurationValue
    );

    EB_Printf(
    "iConfiguration = 0x%x, decimal %d\n", cd->iConfiguration, cd->iConfiguration
    );

    EB_Printf(
    "bmAttributes = 0x%x ( %s )\n", cd->bmAttributes, usbConfigAttributesString( cd->bmAttributes )
    );

    EB_Printf(
    "MaxPower = 0x%x, decimal %d\n", cd->MaxPower, cd->MaxPower
    );
    EB_Printf("-----------------------------\n");
}


void
print_USB_INTERFACE_DESCRIPTOR(PUSB_INTERFACE_DESCRIPTOR id, UINT ix)
/*++
Routine Description:

    Called to do formatted ascii dump to console of a USB interface descriptor

Arguments:

    ptr to USB interface descriptor

Return Value:

    none

--*/
{
    EB_Printf("USB_INTERFACE_DESCRIPTOR #%d\n", ix);


    EB_Printf(
    "bLength = 0x%x\n", id->bLength
    );


    EB_Printf(
    "bDescriptorType = 0x%x ( %s )\n", id->bDescriptorType, usbDescriptorTypeString( id->bDescriptorType )
    );


    EB_Printf(
    "bInterfaceNumber = 0x%x\n", id->bInterfaceNumber
    );
    EB_Printf(
    "bAlternateSetting = 0x%x\n", id->bAlternateSetting
    );
    EB_Printf(
    "bNumEndpoints = 0x%x\n", id->bNumEndpoints
    );
    EB_Printf(
    "bInterfaceClass = 0x%x\n", id->bInterfaceClass
    );
    EB_Printf(
    "bInterfaceSubClass = 0x%x\n", id->bInterfaceSubClass
    );
    EB_Printf(
    "bInterfaceProtocol = 0x%x\n", id->bInterfaceProtocol
    );
    EB_Printf(
    "bInterface = 0x%x\n", id->iInterface
    );
    EB_Printf("-----------------------------\n");
}


void
print_USB_ENDPOINT_DESCRIPTOR(PUSB_ENDPOINT_DESCRIPTOR ed, int i)
/*++
Routine Description:

    Called to do formatted ascii dump to console of a USB endpoint descriptor

Arguments:

    ptr to USB endpoint descriptor,
    index of this endpt in interface desc

Return Value:

    none

--*/
{
    EB_Printf(
    "USB_ENDPOINT_DESCRIPTOR for Pipe%02d\n", i
    );

    EB_Printf(
    "bLength = 0x%x\n", ed->bLength
    );

    EB_Printf(
    "bDescriptorType = 0x%x ( %s )\n", ed->bDescriptorType, usbDescriptorTypeString( ed->bDescriptorType )
    );


    if ( USB_ENDPOINT_DIRECTION_IN( ed->bEndpointAddress ) ) {
	EB_Printf(
	"bEndpointAddress= 0x%x ( INPUT )\n", ed->bEndpointAddress
	);
    } else {
	EB_Printf(
	"bEndpointAddress= 0x%x ( OUTPUT )\n", ed->bEndpointAddress
	);
    }

    EB_Printf(
    "bmAttributes= 0x%x ( %s )\n", ed->bmAttributes, usbEndPointTypeString ( ed->bmAttributes )
    );


    EB_Printf(
    "wMaxPacketSize= 0x%x, decimal %d\n", ed->wMaxPacketSize, ed->wMaxPacketSize
    );
    EB_Printf(
    "bInterval = 0x%x, decimal %d\n", ed->bInterval, ed->bInterval
    );
    EB_Printf("-----------------------------\n");
}

void
rw_dev( HANDLE hDEV )
/*++
Routine Description:

    Called to do formatted ascii dump to console of  USB
    configuration, interface, and endpoint descriptors
    (Cmdline "rwbulk -u" )

Arguments:

    handle to device

Return Value:

    none

--*/
{
    UINT success;
    int siz, nBytes;
    char buf[256];
    PUSB_CONFIGURATION_DESCRIPTOR cd;
    PUSB_INTERFACE_DESCRIPTOR id;
    PUSB_ENDPOINT_DESCRIPTOR ed;

    siz = sizeof(buf);

    if (hDEV == INVALID_HANDLE_VALUE) {
	//NOISY(("DEV not open"));
	return;
    }
    
    success = DeviceIoControl(hDEV,
	    IOCTL_BULKUSB_GET_CONFIG_DESCRIPTOR,
	    buf,
	    siz,
	    buf,
	    siz,
	    &nBytes,
	    NULL);

    //NOISY(("request complete, success = %d nBytes = %d\n", success, nBytes));
    
    if (success) {
	ULONG i;
	UINT  j, n;
	char *pch;

	pch = buf;
	n = 0;

	cd = (PUSB_CONFIGURATION_DESCRIPTOR) pch;

	print_USB_CONFIGURATION_DESCRIPTOR( cd );

	pch += cd->bLength;

	do {

	    id = (PUSB_INTERFACE_DESCRIPTOR) pch;

	    print_USB_INTERFACE_DESCRIPTOR(id, n++);

	    pch += id->bLength;
	    for (j=0; j<id->bNumEndpoints; j++) {

		ed = (PUSB_ENDPOINT_DESCRIPTOR) pch;

		print_USB_ENDPOINT_DESCRIPTOR(ed,j);

		pch += ed->bLength;
	    }
	    i = (ULONG)(pch - buf);
	} while (i<cd->wTotalLength);

    }
    
    return;

}


int  dumpUsbConfig()
/*++
Routine Description:

    Called to do formatted ascii dump to console of  USB
    configuration, interface, and endpoint descriptors
    (Cmdline "rwbulk -u" )

Arguments:

    none

Return Value:

    none

--*/
{

    HANDLE hDEV = open_dev();

    if ( hDEV )
    {
	rw_dev( hDEV );
	CloseHandle(hDEV);
    }

    return 0;
}
//  End, routines for USB configuration and pipe info dump  (Cmdline "rwbulk -u" )



