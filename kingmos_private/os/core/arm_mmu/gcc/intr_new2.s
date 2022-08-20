/* ******************************************************
 * @copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
 * ******************************************************
 * 
 * ******************************************************
 * 文件说明：中断异常处理
 * 版本号：1.0.0
 * 开发时期：2001-04-04
 * 作者：周兵
 * 修改记录：
 * *******************************************************
 */

.nolist
.include "linkage.inc"
.list

.globl		INTR_OFF
.globl		INTR_ON
.globl		ES_VectorTable

.extern		INTR_Interrupt
.extern		INTR_Software
.extern		INTR_PrefetchAbort
.extern		INTR_DataAbort
.extern		INTR_FastIntr
.extern		DefaultHandler
.extern		DebugValue
.extern		GetTryFlagAdr
.extern		LoadFailurePage
.extern		MakeAPICall
.extern		APICallReturn

@ ARM processor modes
.equ	USER_MODE	,	0x10	@2_10000
.equ	FIQ_MODE	,	0x11	@2_10001
.equ	IRQ_MODE	,	0x12	@2_10010
.equ	SVC_MODE	,	0x13	@2_10011
.equ	ABORT_MODE	,	0x17	@2_10111
.equ	UNDEF_MODE	,	0x1b	@2_11011
.equ	SYSTEM_MODE ,	0x1f	@2_11111

	.macro mtc15, cpureg, cp15reg
	    mcr		p15,0,\cpureg,\cp15reg,c0,0
	.endm

	.macro mfc15, cpureg, cp15reg
	    mrc		p15,0,\cpureg,\cp15reg,c0,0
	.endm

	.text
	.type	 ES_VectorTable, #object	
ES_VectorTable:
        .word   ES_ResetHandler
        .word   ES_UndefExceptionHandler
        .word   ES_SWIHandler
        .word   ES_PrefetchAbortHandler
        .word   ES_DataAbortHandler
        .word   ES_UnusedHandler
        .word   ES_IRQHandler
        .word   ES_FIQHandler
	    .size	ES_VectorTable, . - ES_VectorTable

@=======================================================
@=======================================================
	ENTRY ES_ResetHandler
	nop

@=======================================================
@=======================================================
    ENTRY ES_UndefExceptionHandler
	nop

@=======================================================
@=======================================================
    ENTRY ES_SWIHandler
	stmfd   sp!, {r0-r3, r12, lr}           @ save some register 
    mrs     r1, spsr                        @ r1 = spsr
    stmfd   sp!, {r1}                       @ save SPSR onto the IRQ stack

	mov  r0, lr
	bl INTR_Software                             @ call c code

    ldmfd   sp!, {r1}                       @ restore IRQ SPSR from the IRQ stack
    msr     spsr, r1                        @ spsr = r1
	ldmfd   sp!, {r0-r3, r12, lr}           @ restore some register
	@subs pc, lr, #4		@ return
	movs pc, lr

@-----------------------------------------------------------
@-----------------------------------------------------------
    ENTRY ES_PrefetchAbortHandler
	sub     lr, lr, #0xF0000004
	cmp     lr, #0x00200000         @must sync with api bit define in eapisrv.h
	blo     HandleAPICall

	@  handle exception
	add     lr, lr, #0xF0000000


    stmfd   sp!, {r0-r3, r12, lr}
    mov     r0, lr                          @ (r0) = faulting address
    mfc15   r2, c13                         @ (r2) = process base address
    tst     r0, #0xFE000000                 @ slot 0 reference?
    orreq   r0, r0, r2                      @ (r0) = process slot based address
    bl      LoadFailurePage                   @ (r0) = !0 if entry loaded

	cmp     r0, #0x0	                    @ not handle the page failure
	ldmnefd sp!, {r0-r3,r12,pc}^

    
	@ now, handle exception
	ldmfd   sp!, {r0-r3,r12,lr}             @ 
	stmfd   sp!, {r0-r3, r12, lr}           @ save some register 
	ldmfd   sp!, {r0-r3,r12,lr}             @ 
	stmfd   sp!, {r4,r5}
	mov     r4,  lr
	mrs     r5,  spsr
	msr     cpsr_c, #SVC_MODE | 0x80       @ switch to supervisor mode w/IRQs disabled
	stmfd   sp!, {r0-r3,r4}                 @ r4 is lr(data_abort'lr)
	stmfd   sp!, {r5,r12}                   @ save spsr(r5) and r12
	stmfd   sp!, {lr}                       @ lr is svc's lr	
	mov     r0,  r4

	msr     cpsr_c, #ABORT_MODE | 0x80       @ switch back to abort mode w/IRQs disabled
	ldmfd   sp!, {r4,r5}                    @restore r4, r5

	msr     cpsr_c, #SVC_MODE | 0x80       @ switch to supervisor mode w/IRQs disabled

	bl      INTR_PrefetchAbort					@ call c code, handle exception

	ldmfd   sp!, {lr}             @ lr=svc's lr
	ldmfd   sp!, {r0,r12}         @ r0 = spsr
	msr     spsr, r0
	ldmfd   sp!, {r0-r3,pc}^

HandleAPICall:

	 msr     cpsr_c, #SVC_MODE | 0x80     @ switch to supervisor mode w/IRQs enable
     stmfd   sp!, {r0-r3}
	 msr     cpsr_c, #ABORT_MODE | 0x80       @ switch back to abort mode w/IRQs disabled
     mov     r0, lr               @ api call info
	 mrs     r3, spsr             @ return  mode

	 msr     cpsr_c, #SVC_MODE     @ switch to supervisor mode w/IRQs enable
     mov     r1, sp

	 stmfd   sp!, {r3,lr}

@     mov     r0, r12               @ api call info     
	 mov     r2, lr                @ return address
     bl      MakeAPICall

	 cmp     r0, #0x0              @ 2003-06-13
	 beq     DirectRet            @ 2003-06-13

     mov     r12,r0                @ r0 is call address
	 ldmfd   sp!, {r3,lr}
     ldmfd   sp!, {r0-r3}
     add     lr, pc, #APICallRet-(.+8)        @ lr = return address
     mov     pc, r12

APICallRet:

     stmfd   sp!, {r0}

     bl      DefaultHandler        @ if resche ?

     sub     sp, sp, #0x8          @ space for CALLRET struct
	 mov     r0, sp
     bl      APICallReturn

	 mov     r12, r0               @ r12 is return address
     ldr     r0, [sp, #0]          @ (r0) = return mode
	 msr     spsr, r0              @ spsr = r0

	 add     sp, sp, #0x8          @ popup space for CALLRET struct

@	 mov     r12, r0
	 ldmfd   sp!, {r0}             @ r0 = return vaue
	 movs     pc, r12

DirectRet:
     bl      DefaultHandler        @if resche ?
	 ldmfd   sp!, {r3,lr}
     msr     spsr, r3 
     ldmfd   sp!, {r0-r3}       @popup sp
     movs    pc, lr


    ENTRY CallInUserMode

     
	mov    pc, r0
@=======================================================
@=======================================================
    ENTRY ES_DataAbortHandler
	sub     lr, lr, #8
	stmfd   sp!, {r0-r3, r12, lr}           @ save some register

    mfc15   r0, c6                          @ (r0) = fault address
    mfc15   r1, c5                          @ (r1) = fault status

@	mov     r2, lr                          @ (r2) = abort address

@    mfc15   r2, c13                         @ (r2) = process base address
 @   tst     r0, #0xFE000000                 @ slot 0 reference?
  @  orreq   r0, r0, r2                      @ (r0) = process slot based address

	and     r1, r1, #0x0D                   @ type of data abort
    cmp     r1, #0x05                       @ translation error?
	movne   r0, #0x0
	bleq    LoadFailurePage

	cmp     r0, #0x0	                    @ not handle the page failure
	ldmnefd sp!, {r0-r3,r12,pc}^

@   now to handle exception
	ldmfd   sp!, {r0-r3,r12,lr}             @ 
	stmfd   sp!, {r4,r5}
	mov     r4,  lr
	mrs     r5,  spsr
	msr     cpsr_c, #SVC_MODE | 0x80       @ switch to supervisor mode w/IRQs disabled
	stmfd   sp!, {r0-r3,r4}                 @ r4 is lr(data_abort'lr)
	stmfd   sp!, {r5,r12}                   @ save spsr(r5) and r12
	stmfd   sp!, {lr}                       @ lr is svc's lr	
	mov     r2,  r4

	msr     cpsr_c, #ABORT_MODE | 0x80       @ switch back to abort mode w/IRQs disabled
	ldmfd   sp!, {r4,r5}                    @restore r4, r5

	msr     cpsr_c, #SVC_MODE | 0x80       @ switch to supervisor mode w/IRQs disabled

    mfc15   r0, c6                          @ (r0) = fault address
    mfc15   r1, c5                          @ (r1) = fault status

	bl    INTR_DataAbort					@ call c code, handle exception

	ldmfd   sp!, {lr}             @ lr=svc's lr
	ldmfd   sp!, {r0,r12}         @ r0 = spsr
	msr     spsr, r0
	ldmfd   sp!, {r0-r3,pc}^

@=======================================================
@=======================================================
    ENTRY ES_UnusedHandler

	stmfd   sp!, {r0-r3, r12, lr}           @ save some register 
    mrs     r1, spsr                        @ r1 = spsr
    stmfd   sp!, {r1}                       @ save SPSR onto the IRQ stack

    ldmfd   sp!, {r1}                       @ restore IRQ SPSR from the IRQ stack
    msr     spsr, r1                        @ spsr = r1
	ldmfd   sp!, {r0-r3, r12, lr}           @ restore some register
	@subs pc, lr, #4		@ return
	movs pc, lr

	
@=======================================================
@=======================================================
    ENTRY ES_IRQHandler
	sub     lr, lr, #4                      @ fix return address
	stmfd   sp!, {r4, r5}                   @ save some register to IRQ stack

	mov     r4,   lr                        @ r4 = spsr
	mrs     r5,   spsr

	msr     cpsr_c, #SVC_MODE | 0x80       @ switch to supervisor mode w/IRQs disabled
	stmfd   sp!, {r0-r3, r4}                   @ save r0-r3, and lr(irq_mode)
	stmfd   sp!, {r12, lr}                  @ save r12, lr(svc_mode)
	stmfd   sp!, {r5}                       @ save spsr
	

	mov     r0,  r4                         @ r0 = lr(irq_mode)
	mov     r1,  r5                         @ r1 = spsr(irq_mode)

	msr     cpsr_c, #IRQ_MODE | 0x80       @ switch back to IRQ mode w/IRQs disabled
	ldmfd   sp!, {r4, r5}                   @ restore r4, r5

	msr     cpsr_c, #SVC_MODE | 0x80       @ switch to supervisor mode w/IRQs disabled

    stmfd   sp!, {r0}                       @ save lr
	bl      INTR_Interrupt                      @ add by zenghao
	ldmfd   sp!, {r0}                       @ r0=lr

    msr     cpsr_c, #SVC_MODE       @ switch to supervisor mode w/IRQs enable
	bl      DefaultHandler
    msr     cpsr_c, #SVC_MODE | 0x80       @ switch to supervisor mode w/IRQs disabled	
	@bl      DebugValue

    ldmfd   sp!, {r0}                       @ restore spsr
	msr     spsr, r0
	
	ldmfd   sp!, {r12, lr}                       @ restore register
	ldmfd   sp!, {r0-r3, pc}^                   @ restore register

@=======================================================
@=======================================================
    ENTRY ES_FIQHandler
	stmfd   sp!, {r0-r3, r12, lr}           @ save some register 
    mrs     r1, spsr                        @ r1 = spsr
    stmfd   sp!, {r1}                       @ save SPSR onto the IRQ stack

	bl INTR_FastIntr					@ call c code

    ldmfd   sp!, {r1}                       @ restore IRQ SPSR from the IRQ stack
    msr     spsr, r1                        @ spsr = r1
	ldmfd   sp!, {r0-r3, r12, lr}           @ restore some register
	subs pc, lr, #4		@ return


@=======================================================
@INTR_ON - enable interrupts
@=======================================================
        ENTRY INTR_ON

        mrs     r0, cpsr                        @ (r0) = current status
        bic     r1, r0, #0x80                   @ clear interrupt disable bit
        msr     cpsr, r1                        @ update status register
        mov     pc, lr                                  @ return to caller


@=======================================================
@ INTR_OFF - disable interrupts
@=======================================================
        ENTRY INTR_OFF

        mrs     r0, cpsr                        @ (r0) = current status
        orr     r1, r0, #0x80                   @ set interrupt disable bit
        msr     cpsr, r1                        @ update status register
        mov     pc, lr                          @ return to caller


@=======================================================
@=======================================================

	ENTRY SetDataBreakpoint
	@r0 = Access data breakpoint address register (DBAR).
	@r1 = Access data breakpoint value register (DBVR).
	@r2 = Access data breakpoint mask register (DBMR).
	@r3 = Load data breakpoint control register (DBCR).
	mcr p15,0,r0,c14,c0,0
	mcr p15,0,r1,c14,c1,0
	mcr p15,0,r2,c14,c2,0
	mcr p15,0,r3,c14,c3,0
	mov	pc, lr			@ return


@=======================================================
@=======================================================

	ENTRY SetInstructionBreakpoint
	@r0 = Write instruction breakpoint address and control register (IBCR).
	cmp r0, #0x0
	orrne r0, r0, #0x1                   @ enable InstructionBreakpoint
	mcrne p15,0,r0,c14,c8,0
	mov	pc, lr			@ return

@=======================================================
@=======================================================

	ENTRY GetInstructionBreakpoint
	mrc p15,0,r0,c14,c8,0
	mov	pc, lr			@ return


@=======================================================
@=======================================================

    ENTRY SetProcessId

    mtc15   r0, c13                         @ set process base address register
	mov	pc, lr			@ return

@=======================================================
@=======================================================
    ENTRY GetProcessId

    mfc15   r0, c13                         @ set process base address register
	mov	pc, lr			@ return

@=======================================================
@=======================================================

    ENTRY KL_GetCPSR
    mrs     r0, cpsr                        @ (r0) = current status
    mov     pc, lr                                  @ return to caller


    @LEAF_ENTRY KL_SetCPSR
    @stmfd   sp!, {r0, r1, lr}           @ save some register 
    @msr     spsr, r0                               @ (r0) = new status
	@ldmfd   sp!, {r0, r1, pc}^
	@movs     pc, lr
	@ENTRY_END  


@=======================================================
@BOOL __TRY( int * lp, int len )
@=======================================================

	ENTRY _____TRY
	        @BOOL __TRY( int * lp, int len )
	stmfd   sp!, {r0-r1, lr}           @ save some register 
	
	bl GetTryFlagAdr

	mov r2, r0                         @ r2 = flag adr
	ldmfd   sp!, {r0-r1, lr}           @ save some register 

	mov r3, #0x01
	str r3, [r2]

	cmp r1, #0x0
	beq _try1
_try0:
	ldr r3, [r2]                       @ flag adr
	cmp r3, #0x0
	beq _try1

	ldr r3, [r0]
	add r0, r0, #0x4
	subs r1, r1, #0x4
	bne _try0

_try1:

    ldr r0, [r2]                         @return value
	mov r3,  #0x0
	str r3, [r2]

    @mov r0, lr
	@bl _Testr0

	mov pc, lr

	ENTRY ___GetCPUState
    mfc15   r0, c1
	mov pc, lr
