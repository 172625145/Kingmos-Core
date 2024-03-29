/*********************************************************************

  This code fragment illustrates the unsuccessful attempt to directly
modify the IOPM base address.  This code would appear in a kernel
mode device driver.  Refer to the GIVEIO.C listing for a complete
device driver example.

*********************************************************************/

/*
 *  Make sure our structure is packed properly, on byte boundary, not
 * on the default doubleword boundary.
 */
#pragma pack(push,1)

/*
 *  Structure of a GDT (global descriptor table) entry.  From
 * processor manual.
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

/*
 *  Structure of the 48 bits of the GDT register that are stored
 * by the SGDT instruction.
 */
typedef struct {
	unsigned short  limit;
	GDTENT  *base;
} GDTREG;

#pragma pack(pop)

/*
 *  This code demonstrates the brute force approach to modifying
 * the IOPM base.  The IOPM base is stored as a two byte integer
 * at offset 0x66 within the TSS, as documented in the processor
 * manual.  In Windows NT, the IOPM is stored within the TSS
 * starting at offset 0x88, and going for 0x2004 bytes.  This is
 * not documented anywhere, and was determined by inspection.
 * The code here puts some 0's into the IOPM so that we
 * can try to access some I/O ports, then modifies the IOPM base
 * address.
 *
 *  This code is unsuccessful because NT overwrites the IOPM
 * base on each process switch.
 */
void GiveIO()
{
	GDTREG gdtreg;
	GDTENT *g;
	short TaskSeg;
	char *TSSbase;
	int i;

	_asm str TaskSeg;                                       // get the TSS selector
	_asm sgdt gdtreg;                                       // get the GDT address

	g = gdtreg.base + (TaskSeg >> 3);       // get the TSS descriptor

										// get the TSS address
	TSSbase = (PVOID)(g->baselo | (g->basemid << 16) 
		       | (g->basehi << 24));

	for(i=0; i < 16; ++i)                           // poke some 0's into the
		TSSbase[0x88 + i] = 0;                  //   IOPM

										// set IOPM base to 0x88
	*((USHORT *)(TSSbase + 0x66)) = 0x88;
}

