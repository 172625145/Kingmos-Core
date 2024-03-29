/*
 *  From inpection of the TSS we know that NT's default IOPM offset is
 * 0x20AD.  From an inspection of a dump of a process structure, we
 * can find the bytes 'AD 20' at offset 0x30.  This is where NT stores
 * the IOPM offset for each process, so that I/O access can be granted
 * on a process-by-process basis.  This portion of the process
 * structure is not documented in the DDK.
 *
 *  This kernel mode driver fragment illustrates the brute force
 * method of poking the IOPM base into the process structure.
 */
void GiveIO()
{
	char *CurProc;

	CurProc = IoGetCurrentProcess();
	*((USHORT *)(CurProc + 0x30)) = 0x88;
}

