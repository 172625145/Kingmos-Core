/* ******************************************************
 * @copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
 * ******************************************************
 * 
 * ******************************************************
 * 文件说明：设置MMU
 * 版本号：1.0.0
 * 开发时期：2001-04-04
 * 作者：李林
 * 修改记录：
 * *******************************************************
 */

.nolist
.include "def.inc"
.include "linkage.inc"
.list

.equ MemoryMap , 0x2a4		@ From ksarm.h - to get this to compile

.equ BANK_SIZE	,		0x00100000	@ 1MB per bank in MemoryMap array
.equ BANK_SHIFT	,		20

.equ USER_MODE    ,   0x10        @2_10000
.equ FIQ_MODE     ,   0x11        @2_10001
.equ IRQ_MODE     ,   0x12        @2_10010
.equ SVC_MODE     ,   0x13        @2_10011
.equ ABORT_MODE   ,   0x17        @2_10111
.equ UNDEF_MODE   ,   0x1b        @2_11011
.equ SYS_MODE  ,   0x1f        @2_11111



@
@   Define RAM space for the Page Tables:
@

@ High memory layout:
@       FFFD0000 - first level page table (uncached) (2nd half is r/o)
@       FFFD4000 - second level page tables (uncached)
@       FFFE0000 - disabled for protection
@       FFFF0000 - exception vectors
@       FFFF03E0 - exception vector jump table
@       FFFF0400 - not used (r/o)
@       FFFF1000 - disabled for protection
@       FFFF2000 - r/o (physical overlaps with vectors)
@       FFFF2400 - Interrupt stack (1k)
@       FFFF2800 - r/o (physical overlaps with Abort stack)
@       FFFF3000 - disabled for protection
@       FFFF4000 - r/o (physical memory overlaps with vectors & intr. stack & FIQ stack)
@       FFFF4900 - Abort stack (2k - 256 bytes)
@       FFFF5000 - disabled for protection
@       FFFF6000 - r/o (physical memory overlaps with vectors & intr. stack)
@       FFFF6800 - FIQ stack (256 bytes)
@       FFFF6900 - r/o (physical memory overlaps with Abort stack)
@       FFFF7000 - disabled
@       FFFFC000 - kernel stack
@       FFFFC800 - KDataStruct
@       FFFFCC00 - r/o for protection (2nd level page table for 0xFFF00000)

@        ^ 0xFFFD0000
@FirstPT         # 0x4000
@PageTables      # 0x4000
@                # 0x8000
@                # 0x10000       @ not mapped
@ExVector        # 0x1000
@                # 0x1400        @ not mapped
@                # 0x0400        @ 1K interrupt stack
@IntStack        # 0x2000        @ not mapped                    (ffff2800)
@                # 0x0100        @ not mapped (FIQ stack)        (ffff4800)
@                # 0x0700        @ 2K-256 abort stack            (ffff4900)
@AbortStack      # 0x1800        @ not mapped                    (ffff5000)
@                # 0x0100        @ not mapped (FIQ stack)        (ffff6800)
@FIQStack        # 0xC000-0x6900 @ not mapped                    (ffff6900)
@KDBase          # 0x07E0        @ 2K-32 kernel stack
@KStack          # 0x0020        @ temporary register save area
@KData           # 0x0400         @ kernel data area

.set	BASE_PAGE_TABLE,	0xfffd0000
.set	FirstPT,			BASE_PAGE_TABLE 
@ .set	PageTables,			FirstPT + 0x4000
.set	UnusedPageTables,			FirstPT + 0x4000	@ 不需要全局第二级页表，放到每一个线程 -2004-12-28 修改
.set	_Notmaped1,			UnusedPageTables + 0x4000
.set	_Notmaped2,			_Notmaped1 + 0x8000
.set	ExVector,			_Notmaped2 + 0x10000
.set	_Notmaped3,			ExVector +	0x1000
.set	_Notmaped4,			_Notmaped3 + 0x1400
.set	IntStack,			_Notmaped4 + 0x0400
.set	_Notmaped5,			IntStack  + 0x2000
.set	_Notmaped6,			_Notmaped5 + 0x0100
.set	AbortStack,			_Notmaped6 + 0x0700
.set	_Notmaped7,			AbortStack+ 0x1800
.set	FIQStack,			_Notmaped7 + 0x0100
.set	KDBase,				FIQStack + 0xc000-0x6900+0x07e0
.set	KStack,				KDBase + 0x0020
.set	KData,				KStack + 0x0000


        @-----------------------------------------------------------------------
        @ .KDATA area is used to reserve physical memory for the above structures.
        @AREA |.KDATA|,DATA,NOINIT
        
        @EXPORT  ExceptionVectorsArea
		@EXPORT  KDataArea

		.globl	KDataArea
		.globl	ExceptionVectorsArea
		.section	".kdata", #alloc, #write
        
KDataArea:
PTs:    .space	0x4000          @ space for first-level page table
@ 不需要全局第二级页表，放到每一个线程-2004-12-28 修改
@        .space	0x4000          @ space for 2nd-level page tables
@
ExceptionVectorsArea: 
        .space	0x0400          @ space for exception vectors
        .space	0x0400          @ space for interrupt stack
        .space	0x0100          @ space for fiq stack
        .space	0x0700          @ space for abort stack
KPage:	.space	0x0c00          @ space for kernel stack & kdatastruct
HighPT:	.space	0x0400          @ space for 2nd level page table to map 0xfff00000
KDEnd:  

@KDataArea
@PTs     %       0x4000          @ space for first-level page table
@        %       0x4000          @ space for 2nd-level page tables
@ExceptionVectorsArea 
@        %       0x0400          @ space for exception vectors
@        %       0x0400          @ space for interrupt stack
@        %       0x0100          @ space for FIQ stack
@        %       0x0700          @ space for Abort stack
@KPage   %       0x0c00          @ space for kernel stack & KDataStruct
@HighPT  %       0x0400          @ space for 2nd level page table to map 0xFFF00000
@KDEnd   %       0

@	MACRO
@	mtc15	$cpureg, $cp15reg
@	mcr		p15,0,$cpureg,$cp15reg,c0,0
@	MEND

@	MACRO
@	mfc15	$cpureg, $cp15reg
@	mrc		p15,0,$cpureg,$cp15reg,c0,0
@	MEND

	.macro mtc15, cpureg, cp15reg
	    mcr		p15,0,\cpureg,\cp15reg,c0,0
	.endm

	.macro mfc15, cpureg, cp15reg
	    mrc		p15,0,\cpureg,\cp15reg,c0,0
	.endm

	.text
	
.extern		WriteByte
.extern		_InitKernel
.extern		TLBClear
.extern		InitUart
.extern		WriteString
.extern		PutHex
.extern		PutHex_Virtual
.extern		LED_FLASH 
.extern		ES_VectorTable

@	TEXTAREA
	
@	IMPORT  WriteByte
@	@IMPORT  ES_InstallVectorTable
@	IMPORT  ES_Entry
@	IMPORT  TLBClear
@	@IMPORT  main
@	@IMPORT	EbootKernelStart
@	@IMPORT	FlashCardLaunch
 @   IMPORT  InitUart
  @  IMPORT  WriteString
   @ IMPORT  PutHex
	@IMPORT	LED_FLASH 
@	IMPORT  ES_VectorTable



@=============================================================
@ Causes the SA1100 to transition to Idle mode
@=============================================================

	ENTRY SetIdleMode

	ldr	    r1, =0xA8000000
	mcr     p15,0,r0,c15,c2,2
	ldr     r0, [r1]
	mcr     p15,0,r0,c15,c8,2
	nop
	nop
	nop
	mov     pc, lr

@=============================================================
@ Enable Dcache Cache
@=============================================================
	ENTRY EnableDCache

	mov     r1, #0x0071             @ Enable: MMU
	orr     r1, r1, #0x1000         @ Enable icache, Dcache
	orr     r1, r1, #0x000C         @ Enable dcache=4, write buffer=8
	
	mtc15   r1, c1                  @ enable the MMU & Caches
	nop
	mov     pc, lr


@=============================================================
@ Disable Dcache Cache
@=============================================================
	ENTRY DisableDCache

	mov     r1, #0x0071             @ Enable: MMU
	orr     r1, r1, #0x1000         @ Enable icache, Dcache
	orr     r1, r1, #0x0008         @ Enable dcache=4, write buffer=8
	
	mtc15   r1, c1                  @ enable the MMU & Caches
	nop
	mov     pc, lr


@=============================================================
@ PhyAdrFromVirAdr: Figure out Physical Address of a Virtual Address
@
@ (r0) = VA
@ (r1) = ptr to OEMAddressTable
@
@ On return
@ (r0) = PA
@
@ Register used:
@ r0 - r3, r12
@
@=============================================================
        ENTRY PhyAdrFromVirAdr
        mov     r12, r0                         @ (r12) = VA
        mov     r0, #-1                         @ (r0) = -1 (initialize return value to an invalid PA)
101:
        ldr     r3, [r1]                        @ (r3) = OEMAddressTable[i].vaddr        
        mov     r2, r3, LSR #BANK_SHIFT         @ make 1MB aligned
        mov     r3, r2, LSL #BANK_SHIFT

        cmp     r3, #0                          @ EndOfTable?
        moveq   pc, lr                          @ INVALID OEMAddressTable, VA not found
        
        cmp     r12, r3                         @ Is (VA >= OEMAddressTable[i].vaddr)?
        blt     102f                           @ go to next entry if not

        ldr     r2, [r1, #8]                    @ (r2) = size in MB
        add     r2, r3, r2, LSL #BANK_SHIFT     @ (r2) = OEMAddressTable[i].vaddr + region size
        cmp     r12, r2                         @ Is (VA < OEMAddressTable[i].vaddr + region size)?
        blt     103f                           @ Found if it's true

102:
        add     r1, r1, #12                     @ i ++ (move to next entry)
        b       101b                           @ test next entry

        @ found the entry
        @ (r1) = &OEMAddressTable[i]
        @ (r3) = OEMAddressTable[i].vaddr
103:
        ldr     r0, [r1, #4]                    @ (r0) = OEMAddressTable[i].paddr
        mov     r0, r0, LSR #BANK_SHIFT         @ (r0) >>= 20
        add     r0, r12, r0, LSL #BANK_SHIFT    @ (r0) = VA + (r0 << 20)
        sub     r0, r0, r3                      @ (r0) -= OEMAddressTable[i].vaddr
        
        mov     pc, lr                          @ return
        

@=============================================================
@ KernelStart - Set up the MMU and Dcache for bootloader
@
@ This routine will initialize the first-level page table based up the contents
@ of the MemoryMap array and enable the MMU and caches.
@
@	Entry	(r0) = pointer to MemoryMap array in physical memory
@	Exit	returns if MemoryMap is invalid

@@ Now If we are booting from Flash we want to copy and jump to RAM
@@ Otherwise if we are already in RAM we want to skip this step 
@=============================================================

	    ENTRY _KernelStart

        mov     r11, r0                         @ (r11) = &MemoryMap (save pointer)
        ldr     r9, =PTs                        @ (r9) = "virtual address" of 1st level table

        ldr     r7, =0x1FF00000                 @ VA needs 512MB, 1MB aligned.
        ldr     r8, =0xFFF00000                 @ PA needs   4GB, 1MB aligned.

        and     r6, r9, r7                      @ (r6) = KDATA Virtual Address (1MB)

        mov     r1, r11                         @ (r1) = ptr to MemoryMap array

5:
        ldr     r2, [r1], #4                    @ (r2) = virtual address to map Bank at
        ldr     r3, [r1], #4                    @ (r3) = physical address to map from
        ldr     r4, [r1], #4                    @ (r4) = num MB to map

        cmp     r4, #0                          @ End of table?
        moveq   pc, lr                          @ RETURN!!!! INVALID MEMORY MAP. NO MAPPING FOR KDATA AREA!

        and     r2, r2, r7                      @ VA needs 512MB, 1MB aligned.
        and     r3, r3, r8                      @ PA needs 4GB, 1MB aligned.

        @
        @ For each MB in this table entry, compare with KDATA Virtal Address...
        @
8:
        cmp     r2, r6                          @ KDATA VA = Section VA?
        beq     9f                             @ Found it!!! (r3) is the PA

        add     r2, r2, #0x00100000             @ (r2) = VA + 1MB
        add     r3, r3, #0x00100000             @ (r3) = PA + 1MB

        sub     r4, r4, #1                      @ Decrement number of MB left
        cmp     r4, #0
        bne     8b                             @ Map next MB

        b       5b                             @ Get next element

9:
        mov     r4, r9
        mov     r1, r3

@       Found MemoryMap entry for .KDATA section which contains the kernel data page,
@ the first-level page table, exception vector page, etc.
@
@       (r1) = base address of bank
@       (r4) = virtual address of FirstPT
@       (r11) = ptr to MemoryMap array

15:
        mov     r10, #BANK_SIZE
        sub     r10, r10, #1                    @ (r10) = mask for offset
        and     r10, r10, r4                    @ (r10) = offset into bank for FirstPT
        orr     r10, r10, r1                    @ (r10) = ptr to FirstPT

@       Zero out page tables & kernel data page

        mov     r0, #0                          @ (r0-r3) = 0's to store
        mov     r1, #0
        mov     r2, #0
        mov     r3, #0
        mov     r4, r10                         @ (r4) = first address to clear
        add     r5, r10, #KDEnd-PTs             @ (r5) = last address + 1
18:
        stmia   r4!, {r0-r3}
        stmia   r4!, {r0-r3}
        cmp     r4, r5
        blo     18b


@       Setup 2nd level page table to map the high memory area which contains the
@ first level page table, 2nd level page tables, kernel data page, etc.

        add     r4, r10, #HighPT-PTs            @ (r4) = ptr to high page table
        orr     r0, r10, #0x051                 @ (r0) = PTE for 64K, kr/w kr/w r/o r/o page, uncached unbuffered
        str     r0, [r4, #0xD0*4]               @ store the entry into 8 consecutive slots
        str     r0, [r4, #0xD1*4]
        str     r0, [r4, #0xD2*4]
        str     r0, [r4, #0xD3*4]
@ 不需要全局第二级页表，放到每一个线程 -2004-12-28 修改        
@        str     r0, [r4, #0xD4*4]
@        str     r0, [r4, #0xD5*4]
@        str     r0, [r4, #0xD6*4]
@        str     r0, [r4, #0xD7*4]
@
        add     r8, r10, #ExceptionVectorsArea-PTs  @ (r8) = ptr to vector page

        @bl      OEMARMCacheMode                 @ places C and B bit values in r0 as set by OEM
		mov	r0, #0x0000000C

        mov     r2, r0
        orr     r0, r8, #0x002                  @ construct the PTE
        orr     r0, r0, r2
        str     r0, [r4, #0xF0*4]               @ store entry for exception vectors
        orr     r0, r0, #0x500                  @ (r0) = PTE for 4k r/o r/o kr/w kr/w C+B page
        str     r0, [r4, #0xF4*4]               @ store entry for abort stack
        str     r0, [r4, #0xF6*4]               @ store entry for FIQ stack  (access permissions overlap for abort and FIQ stacks, same 1k)
        orr     r0, r8, #0x042
        orr     r0, r0, r2                      @ (r0)= PTE for 4K r/o kr/w r/o r/o (C+B as set by OEM)
        str     r0, [r4, #0xF2*4]               @ store entry for interrupt stack
        add     r9, r10, #KPage-PTs             @ (r9) = ptr to kdata page
        orr     r0, r9, #0x002
        orr     r0, r0, r2                      @ (r0)=PTE for 4K (C+B as set by OEM)
        orr     r0, r0, #0x250                  @ (r0) = set perms kr/w kr/w kr/w+ur/o r/o
        str     r0, [r4, #0xFC*4]               @ store entry for kernel data page
        orr     r0, r4, #0x001                  @ (r0) = 1st level PTE for high memory section
        add     r1, r10, #0x4000
        str     r0, [r1, #-4]                   @ store PTE in last slot of 1st level table
.if 0
        mov     r0, r4
        mov     r1, #256                        @ dump 256 words
        CALL    WriteHex
.endif

@ Fill in first level page table entries to create "un-mapped" regions
@ from the contents of the MemoryMap array.
@
@       (r9) = ptr to KData page
@       (r10) = ptr to 1st level page table
@       (r11) = ptr to MemoryMap array

        add     r10, r10, #0x2000               @ (r10) = ptr to 1st PTE for "unmapped space"

        mov         r0, #0x02
        orr     r0, r0, r2                      @ (r0)=PTE for 0: 1MB (C+B as set by OEM)
        orr     r0, r0, #0x400                  @ set kernel r/w permission
20:
        mov     r1, r11                         @ (r1) = ptr to MemoryMap array


25:
        ldr     r2, [r1], #4                    @ (r2) = virtual address to map Bank at
        ldr     r3, [r1], #4                    @ (r3) = physical address to map from
        ldr     r4, [r1], #4                    @ (r4) = num MB to map

        cmp     r4, #0                          @ End of table?
        beq     29f

        ldr     r5, =0x1FF00000
        and     r2, r2, r5                      @ VA needs 512MB, 1MB aligned.

        ldr     r5, =0xFFF00000
        and     r3, r3, r5                      @ PA needs 4GB, 1MB aligned.

        add     r2, r10, r2, LSR #18
        add     r0, r0, r3                      @ (r0) = PTE for next physical page

28:
        str     r0, [r2], #4
        add     r0, r0, #0x00100000             @ (r0) = PTE for next physical page

        sub     r4, r4, #1                      @ Decrement number of MB left
        cmp     r4, #0
        bne     28b                            @ Map next MB

        bic     r0, r0, #0xF0000000             @ Clear Section Base Address Field
        bic     r0, r0, #0x0FF00000             @ Clear Section Base Address Field
        b       25b                            @ Get next element


29:
        tst     r0, #8
        bic     r0, r0, #0x0C                   @ clear cachable & bufferable bits in PTE
        add     r10, r10, #0x0800               @ (r10) = ptr to 1st PTE for "unmapped uncached space"
        bne     20b                            @ go setup PTEs for uncached space
        sub     r10, r10, #0x3000               @ (r10) = restore address of 1st level page table


@ Setup the vector area.
@
@       (r8) = ptr to exception vectors

        add     r7, pc, #VectorInstructions - (.+8)
        ldmia   r7!, {r0-r3}                    @ load 4 instructions
        stmia   r8!, {r0-r3}                    @ store the 4 vector instructions
        ldmia   r7!, {r0-r3}                    @ load 4 instructions
        stmia   r8!, {r0-r3}                    @ store the 4 vector instructions

@        add     r8, r8, #0x3E0-(8*4)
 @       ldmia   r7!, {r0-r3}
  @      stmia   r8!, {r0-r3}
   @     ldmia   r7!, {r0-r3}
    @    stmia   r8!, {r0-r3}

   @ convert VectorTable to Physical Address
        ldr     r0, =ES_VectorTable                @ (r0) = VA of VectorTable
        mov     r1, r11                         @ (r1) = &OEMAddressTable[0]
        bl      PhyAdrFromVirAdr
        mov     r7, r0                          @ (r7) = PA of VectorTable
        add     r8, r8, #0x3E0-(8*4)            @ (r8) = target location of the vector table
        ldmia   r7!, {r0-r3}
        stmia   r8!, {r0-r3}
        ldmia   r7!, {r0-r3}
        stmia   r8!, {r0-r3}



@ The page tables and exception vectors are setup. Initialize the MMU and turn it on.

@
        mov     r1, #1
        mtc15   r1, c3                          @ setup access to domain 0
        mtc15   r10, c2
        bl      TLBClear

    @   set mmu option
	@  about test the same code with different mmu seting( timer interrupt's period is 10 ms )
	@  if mmu, the cpu speed is about 88.910 seconds
	@  if mmu + data cache , cpu speed is about 80.760 seconds
	@  if mmu + icache,      cpu speed is about 4.610  seconds
	@  if mmu + dcache + icache + write buffer, cpu speed is about 3.620 seconds
	@  test by lilin

        mfc15   r1, c1
@		mov   r1, #0                @r1 = 0

@-----fast begin--------------------------        
        orr     r1, r1, #0x007F                 @ changed to read-mod-write for ARM920 Enable: MMU, Align, DCache, WriteBuffer
	    orr     r1, r1, #0x0200                 @ ROM protection enable
        orr     r1, r1, #0x1000                 @ ICache enable
@-----fast end--------------------------

@-----medium begin--------------------------
       @orr     r1, r1, #0x1                @ enable  mmu
       @orr     r1, r1, #0x0004         @ enable data cache
    	@orr     r1, r1, #0x1000         @ Enable icache
       @orr     r1, r1, #0x0008         @ Enable write buffer=8
    	@orr     r1, r1, #0x0200         @ ROM protection enable
@----medium end----------------------------

@-----slow begin--------------------------       
	   @orr     r1, r1, #0x1                @ enable  mmu
	   @orr     r1, r1, #0x0200         @ ROM protection enable
@-----slow end--------------------------

        orr     r1, r1, #0x2000         @ interrupt base address = 0xffff0000 

@ kkkk:    
 @   ldr r0, =0xaadefdd
 @   bl PutHex
 @   b kkkk
       
        ldr     r0, VirtualStart
        cmp     r0, #0                          @ make sure no stall on "mov pc,r0" below
        mtc15   r1, c1                          @ enable the MMU & Caches
        mov     pc, r0                          @  & jump to new virtual address
        nop

@ MMU & caches now enabled.
@
@       (r10) = physcial address of 1st level page table

VStart:
        ldr     sp, =KStack
        add     r4, sp, #KData-KStack           @ (r4) = ptr to KDataStruct
        
		@mov     r0, #0x80000000
        @str     r0, [r4, #hBase]                @ set HandleBase
        @ldr     r0, =DirectRet
        @str     r0, [r4, #pAPIReturn]           @ set DirectReturn address
        @add     r1, r10, #0x4000                @ (r1) = physical addr of 2nd page table pool
        @orr     r1, r1, #1                      @ (r1) = page table descriptor for pool entries
        @str     r1, [r4, #ptDesc]               @ save for LoadPageTable()

@ Initialize stacks for each mode.
        mov     r1, #ABORT_MODE | 0x80
        msr     cpsr_c, r1                      @ switch to Abort Mode w/IRQs disabled
        add     sp, r4, #AbortStack-KData
        mov     r1, sp                          @ r1 = abort sp
        mov     r2, #IRQ_MODE | 0x80
        msr     cpsr_c, r2                      @ switch to IRQ Mode w/IRQs disabled
        add     sp, r4, #IntStack-KData
        mov     r2, sp                          @ r2 = irq sp

        mov     r3, #FIQ_MODE | 0x80
        msr     cpsr_c, r3                      @ switch to FIQ Mode w/IRQs disabled
        add     sp, r4, #FIQStack-KData

        mov     r3,  #UNDEF_MODE | 0x80
        msr     cpsr_c, r3                      @ switch to Undefined Mode w/IRQs disabled
        mov     sp, r4                          @ (sp_undef) = &KData
        mov     r3, sp


@        mov     r3,  #SVC_MODE | 0x80
@        msr     cpsr_c, r3                      @ switch to svc Mode w/IRQs disabled
@        mov     r0, sp                          @ (sp_undef) = &KData
@		sub     r0, r0, #0x20          

		mov     r3, #SYS_MODE | 0x80
        msr     cpsr_c, r3                      @ switch to Supervisor Mode w/IRQs enabled
@		mov     sp, r0
        ldr     sp, =KDBase
		mov     r3, sp                          @ r3 = sys sp
		mov     r0, r10                         @ r0 = first levle page table phycial address
		b       _InitKernel

		.type	VirtualStart, #object
		VirtualStart: .word	VStart
		.size	VirtualStart, . - VirtualStart

@VirtualStart DCD VStart

VectorInstructions:
        ldr     pc, [pc, #0x3E0-8]              @ undefined instruction
        @BREAKPOINT                              @ undefined instruction

        ldr     pc, [pc, #0x3E0-8]              @ undefined instruction
        ldr     pc, [pc, #0x3E0-8]              @ SVC
        ldr     pc, [pc, #0x3E0-8]              @ Prefetch abort
        ldr     pc, [pc, #0x3E0-8]              @ data abort

        @BREAKPOINT                              @ unused vector location
		ldr     pc, [pc, #0x3E0-8]              @ unused vector location

        ldr     pc, [pc, #0x3E0-8]              @ IRQ
        ldr     pc, [pc, #0x3E0-8]              @ FIQ
