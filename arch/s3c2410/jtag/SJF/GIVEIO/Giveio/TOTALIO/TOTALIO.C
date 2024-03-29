/*********************************************************************

Author:     Dale Roberts
Date:       8/30/95
Program:    TOTALIO.SYS
Compile:    Use DDK BUILD facility

Purpose:    Give direct port I/O access to the whole system.

  This driver grants total system-wide I/O access to all applications.
Very dangerous, but useful for short tests.  Note that no test
application is required.  Just use control panel or
"net start totalio" to start the device driver.  When the driver
is stopped, total I/O is removed.  Because no Win32 app needs to
communicate with the driver, we don't have to create a device object.
So we have a very tiny driver here.

Since we can safely extend the TSS only to the end of the physical
memory page in which it lies, the I/O access is granted only up to
port 0xf00.  Accesses beyond this port address will still generate
exceptions.

*********************************************************************/
#include <ntddk.h>

/*
 *  Make sure our structure is packed properly, on byte boundary, not
 * on the default doubleword boundary.
*/
#pragma pack(push,1)

/*
 *  Structures for manipulating the GDT register and a GDT segment
 * descriptor entry.  Documented in Intel processor handbooks.
 */
typedef struct {
	unsigned limit : 16;
	unsigned baselo : 16;
	unsigned basemid : 8;
	unsigned type : 4;
	unsigned system : 1;
	unsigned dpl : 2;
	unsigned present : 1;
	unsigned limithi : 4;
	unsigned available : 1;
	unsigned zero : 1;
	unsigned size : 1;
	unsigned granularity : 1;
	unsigned basehi : 8;
} GDTENT;

typedef struct {
	unsigned short	limit;
	GDTENT	*base;
} GDTREG;

#pragma pack(pop)

/*
 *  This is the lowest level for setting the TSS segment descriptor
 * limit field.  We get the selector ID from the STR instruction,
 * index into the GDT, and poke in the new limit.  In order for the
 * new limit to take effect, we must then read the task segment
 * selector back into the task register (TR).
 */
void SetTSSLimit(int size)
{
	GDTREG gdtreg;
	GDTENT *g;
	short TaskSeg;

	_asm cli;							// don't get interrupted!
	_asm sgdt gdtreg;					// get GDT address
	_asm str TaskSeg;					// get TSS selector index
	g = gdtreg.base + (TaskSeg >> 3);	// get ptr to TSS descriptor
	g->limit = size;					// modify TSS segment limit
//
//  MUST set selector type field to 9, to indicate the task is
// NOT BUSY.  Otherwise the LTR instruction causes a fault.
//
	g->type = 9;						// mark TSS as "not busy"
//
//  We must do a load of the Task register, else the processor
// never sees the new TSS selector limit.
//
	_asm ltr TaskSeg;					// reload task register (TR)
	_asm sti;							// let interrupts continue
}
	
/*
 *  This routine gives total I/O access across the whole system.
 * It does this by modifying the limit of the TSS segment by direct
 * modification of the TSS descriptor entry in the GDT.
 * This descriptor is set up just once at sysetem init time.  Once we
 * modify it, it stays untouched across all processes.
 */
void GiveTotalIO(void)
{
	SetTSSLimit(0x20ab + 0xf00);
}

/*
 *  This returns the TSS segment to its normal size of 0x20ab, which
 * is two less than the default I/O map base address of 0x20ad.
 */
void RemoveTotalIO(void)
{
	SetTSSLimit(0x20ab);
}

/****************************************************************************
  Release all memory 'n' stuff.
****************************************************************************/
VOID
TotalIOdrvUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
{
	RemoveTotalIO();
}

/****************************************************************************
  Entry routine.  Set everything up.
****************************************************************************/
NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
	DriverObject->DriverUnload = TotalIOdrvUnload;
	GiveTotalIO();
    return STATUS_SUCCESS;
}

