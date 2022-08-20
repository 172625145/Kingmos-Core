/******************************************************
Copyright(c) °æÈ¨ËùÓÐ£¬1998-2003Î¢Âß¼­¡£±£ÁôËùÓÐÈ¨Àû¡£
******************************************************/

/* Copyright © 1999-2000 Intel Corp.  */
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

    drv_glob.h

Abstract:  

    This file provides the C definitions for the driver globals area of
    shared memory, used to coordinate between ISRs and ISTs.

--*/
#ifndef _DRV_GLOB_H
#define _DRV_GLOB_H

// don't include all the c stuff for fwp2.s
#ifndef ASM_ONLY
//#include "tchaud.h"    // Touch/Audio defs
//#include "halether.h"  // Ethernet debug defs
//#include "p2.h"
#endif //ASM_ONLY
#pragma pack(1)

#ifndef PAD
#define PAD(label,amt)  UCHAR Pad##label[amt]
#endif
// Driver globals version (should be updated on *any* driver globals structure change).
//
#define DRVGLB_MAJOR_VER	1
#define DRVGLB_MINOR_VER	0

// Download/KITL device types.
//
typedef enum _DOWNLOAD_DEVICE_
{
	DOWNLOAD_DEVICE_CS8900,
    DOWNLOAD_DEVICE_PCMCIA
} DOWNLOAD_DEVICE, *PDOWNLOAD_DEVICE;

// Make sure these match up with the defs in config.bib, and drv_glob.inc
//#define DRIVER_GLOBALS_PHYSICAL_MEMORY_START  (DMA_BUFFER_BASE+0x59000)
//#define DRIVER_GLOBALS_PHYSICAL_MEMORY_START  (DMA_BUFFER_BASE+0x20000)
//#define DRIVER_GLOBALS_PHYSICAL_MEMORY_SIZE   0x1000  // 4K
#define DRIVER_GLOBALS_PHYSICAL_MEMORY_START	(DMA_BUFFER_BASE+0x10000) //:-) 
#define DRIVER_GLOBALS_PHYSICAL_MEMORY_SIZE     0x10000  // 64K //:-)

// In OEMInit, we zero out the region specified by the following defs
#define DRIVER_GLOBALS_ZEROINIT_START  DRIVER_GLOBALS_PHYSICAL_MEMORY_START
//#define DRIVER_GLOBALS_ZEROINIT_SIZE   0x800
//we don't want misc intialized either since we pass info from the kernel 
#define DRIVER_GLOBALS_ZEROINIT_SIZE   0x300	
// don't include all the c stuff for fwp2.s
#ifndef ASM_ONLY
// Power/Battery Management (PCF50606) Globals

// Power State
#define PCF50606_STATE_IDLE        0x0000
#define PCF50606_STATE_ADC_BUSY    0x0001
#define PCF50606_STATE_ADC_READY   0x0002
//...
#define PCF50606_STATE_ERROR       0x8000

typedef struct _POWER_GLOBALS 
{
    DWORD   State;
    DWORD   Error;  // Win32 error codes

    BYTE    ACLineStatus;
    BYTE    BatteryFlag;

    // AD Control / Status registers
    BYTE    bADCC1;
    BYTE    bADCC2;
    BYTE    bADCDAT1;
    BYTE    bADCDAT2;
    BYTE    bADCDAT3;

    PAD(0, 241);    // Pad to 256 bytes

} POWER_GLOBALS, *PPOWER_GLOBALS;

// Touch panel globals.  Note: it's important that buffer A and B are
// contiguous in memory and are 16 bytes apart.
typedef struct _TOUCH_GLOBALS {
    ULONG penUpFake;	// start offset 0x0004
    ULONG semaphore;   
	ULONG touchIrq;
    ULONG timerIrq;
    ULONG penUpMissed;
    ULONG status;
    PAD(2,228);          // Pad to 256 bytes
//    PAD(2,232);          // Pad to 256 bytes

} TOUCH_GLOBALS, *PTOUCH_GLOBALS;

// PCMCIA globals.  
typedef struct _PCMCIA_GLOBALS 
{
    // Used to let HAL know which level ints are enabled.
    UCHAR slot0Enable;   
    UCHAR slot1Enable;
    UCHAR slot0CDvalidint;   
    UCHAR slot1CDvalidint;
    UCHAR slot0BVDStschgint;   
    UCHAR slot1BVDStschgint;
    PAD(0,250);          // Pad to 256 bytes
    
} PCMCIA_GLOBALS, *PPCMCIA_GLOBALS;

// For timing interrupt latency
typedef struct _PROFILE_GLOBALS {
    ULONG itIndex;			// index of timer interrupt (log[0])
    ULONG itCounter;		// counter at isr start (log[1])
    ULONG itSpc;			// saved stack pointer (log[2])
    PAD(0,244);          	// Pad to 256 bytes
} PROFILE_GLOBALS, *PPROFILE_GLOBALS;

// Miscellaneous
typedef struct _MISC_GLOBALS {
    UCHAR offButton;        // Indicate to keyboard driver when OFF button pressed
    UCHAR EbootDevice;
    UCHAR numSlots;
    UCHAR dummy;
    PAD(0,252);
} MISC_GLOBALS, *PMISC_GLOBALS;

typedef struct _EDBG_ADDR {
	DWORD  dwIP;         // @field IP address (net byte order)
	USHORT wMAC[3];      // @field Ethernet address (net byte order)
	USHORT wPort;        // @field UDP port # (net byte order) - only used if appropriate
    
} EDBG_ADDR;

// For debugging over ethernet. Controls debug messages, ethernet shell
// and kernel debugger.  Note that this struct should not be zeroed
// out by OEMInit, as the eboot bootloader passes us state info.
typedef struct _DBG_ETH_GLOBALS 
{
    DWORD EbootMagicNum;    // To detect if ether bootloader is present
#define EBOOT_MAGIC_NUM 0x45424F54 // "EBOT"

    UCHAR etherEnabled;     // If non-zero, ethernet card present
    UCHAR etherFlags;       // Set by eboot loader. Controls which components are enabled over ether (see ethdbg.h)
    PAD(0,2);

    EDBG_ADDR TargetAddr;       // IP and ether address of Odo
    DWORD SubnetMask;       // Subnet mask

    EDBG_ADDR DownloadHostAddr; // IP and ether address of host who started us
    
    // The following addresses are assumed valid if the corresponding flag in
    // etherFlags is set.
    EDBG_ADDR DbgHostAddr;   // IP/ether addr and UDP port of host receiving dbg msgs
    EDBG_ADDR KdbgHostAddr;  // IP/ether addr and UDP port of host running kernel debugger
    EDBG_ADDR PpshHostAddr;  // IP/ether addr and UDP port of host running ether text shell
    DWORD DHCPLeaseTime;     // DHCP lease duration in seconds.
    DWORD EdbgFlags;         // Information about ethernet system
    WORD  KitlTransport;         // Transport for KITL communications.
    UCHAR fmtBuf[174];
} DBG_ETH_GLOBALS, *PDBG_ETH_GLOBALS;

#define DBG_ETH_GLOBALS_SIZE 256


#define USBD_GLOBALS_BUF_SIZE (0x4000)
#define USBDUIR_RESET   (0x4)
#define USBDUIR_RESUME	(0x2)
#define USBDUIR_SUSPEND	(0x1)
#define USBDEIR_EP0	(0x1)
#define USBDEIR_EP1	(0x2)
#define USBDEIR_EP2	(0x4)
#define USBDEIR_EP3	(0x8)
#define USBDEIR_EP4	(0x10)

typedef struct _USBD_GLOBALS {  // :-)
	BYTE usbdRxBuf[USBD_GLOBALS_BUF_SIZE];
	ULONG usbdRxWrPt;  //not used in USB-DMA mode
	ULONG usbdRxRdPt; 
	ULONG usbdRxCnt;   
	//ULONG usbdTxRdPt;
	//ULONG usbdTxWrPt;
	//ULONG usbdTxCnt;
	//ULONG usbdMddRxCnt;
    BYTE usbdEir;
	BYTE usbdUir;
	BYTE usbdDma3Int;
	PAD(0,(256-15));
} USBD_GLOBALS, *PUSBD_GLOBALS;


typedef struct _DRIVER_GLOBALS
{
	USHORT           MajorVer;				// Driver globals major version.
	USHORT           MinorVer;				// Driver globals minor version.
    TOUCH_GLOBALS    tch;					// Offset 0x0004
    PCMCIA_GLOBALS   pcm;					// Offset 0x0100
    PROFILE_GLOBALS  prof;					// Offset 0x0200
    MISC_GLOBALS     misc;					// Offset 0x0300
    POWER_GLOBALS    power;					// Offset 0x0400
    USBD_GLOBALS     usbd;					// Offset 0x0500
    DWORD            dwLastLaunchAddrValid;	// Offset 0x4600
    DWORD            dwLastLaunchAddr;		// Offset 0x4604
	BYTE			 baUDID[8];				// Offset 0x4608
    PAD(0, (0x200-4*sizeof(DWORD)));        // Offset 0x4800
	PAD(2,60);	// HMSEO strange pad... if remove this pad... system is not bootup... I'll debug later... at now I have no time... and It seems no problem.... :(
    // The following structs will not be zero initialized (see defs above)
    DBG_ETH_GLOBALS  eth;    // Offset 0x800
} DRIVER_GLOBALS, *PDRIVER_GLOBALS;
#endif //not ASM_ONLY

// And some defs to match above struct
#define TOUCHPANEL_PENSAMPLES_BUFA  (DRIVER_GLOBALS_PHYSICAL_MEMORY_START)
#define TOUCHPANEL_PENSAMPLES_BUFB  (DRIVER_GLOBALS_PHYSICAL_MEMORY_START + 0x10)

// Add pointer to off button flag
#ifdef MIPS
#define  MISC_GLOBALS_BASE          (DRIVER_GLOBALS_PHYSICAL_MEMORY_START + 0x300)       
#define  OFFBUTTON                  0
#endif

#ifndef ASM_ONLY
#pragma pack()

// Prototype functions from drvlib
#ifdef __cplusplus
extern "C" {
#endif
void DriverSleep(DWORD dwMS, BOOL bInPowerHandler);
#ifdef __cplusplus
}
#endif
#endif //not ASM_ONLY

#endif // _DRV_GLOB_H
