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
.extern        MakeSysCall
.extern		APICallReturn
.extern        KL_ExitThread
@.extern        INTR_GetAbortStack
.extern        lpCurThread
@.extern 		AllocThreadKernelStackPage

@ ARM processor modes
.equ	USER_MODE	,	0x10	@2_10000
.equ	FIQ_MODE	,	0x11	@2_10001
.equ	IRQ_MODE	,	0x12	@2_10010
.equ	SVC_MODE	,	0x13	@2_10011
.equ	ABORT_MODE	,	0x17	@2_10111
.equ	UNDEF_MODE	,	0x1b	@2_11011
.equ	SYS_MODE ,	0x1f	@2_11111

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

	sub     r12, r12, #0xF0000000
	cmp     r12, #0x00200000                @ 是否是系统 API调用 ？ must sync with api bit define in eapisrv.h
	blo     HandleAPICall                   @ 是，到HandleAPICall

    add     r12, r12, #0xF0000000           @ 否，处理错误，这里恢复r12的值
    
	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}           @ save r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @ switch back to svc_mode and IRQs disabled
	mov     r0, lr
    mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r1}                    @ 保存lr and spsr到sys_mode的stack里（ save lr and spsr）

    mov     r1, r12                         @ 为INTR_Software准备参数
	msr     cpsr_c, #SYS_MODE               @ IRQs enable
	bl      INTR_Software                   @ 调用C异常处理函数
    
    msr     cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r1}                    @ restore lr and spsr
	msr     cpsr_c, #SVC_MODE | 0x80        @ switch back to svc_mode and IRQs disabled
    mov     lr, r0                          @ restore svc_mode lr and spsr
	msr     spsr, r1

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to svc_mode and IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}           @ restore r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQs disabled
    movs    pc, lr                          @ 返回


HandleAPICall:

@typedef struct _SYSCALL
@{
@	UINT uiCallMode;	// 输入之前的模式; 输出，将要切换到的新模式
@	DWORD dwCallInfo;
@	PFNVOID pfnRetAdress;
@	UINT uiArg[1];		//可变长度
@}SYSCALL, FAR * LPSYSCALL;

	 msr     cpsr_c, #SYS_MODE | 0x80              @ switch to sys_mode and IRQs disabled

@ 将r0-r3压入sp

	 stmfd   sp!, {r0-r3}
	 
	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 
	 mrs     r1, spsr                              @ 为SYSCALL准备数据 = 之前的模式
	 
     msr     cpsr_c, #SYS_MODE                     @ switch to sys_mode and IRQs enable

@
@ sp[0] = r0 - 参数0(arg0)
@ sp[1] = r1 - 参数1(arg1)
@ sp[2] = r2 - 参数2(arg2)
@ sp[3] = r3 - 参数3(arg3)
@ sp[4] =  - 参数4(arg4)
@ sp[5] = - 参数5(arg5)
@ ...
@

     stmfd   sp!, { r1, r12, lr}    @ 准备 SYSCALL 结构成员数据

	 mov     r0, sp                @ r0 = SYSCALL ptr
     bl      MakeSysCall           @ MakeSysCall 将返回需要调用的系统功能地址并做其他切换

	 cmp     r0, #0x0              @ r0 = 调用地址， 系统功能是否有效 ？
	 beq     DirectRet             @ 否，到DirectRet

	 ldr     r12, [sp]             @ r12  = 返回模式 , 必定是SYS or USER mode
     add     sp, sp, #12		   @ 清除sp

	 ldr     lr, =0xF0100000       @ 返回地址

	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 msr     spsr_c, r12                   @返回模式 
	 mov     lr, r0

     msr     cpsr_c, #SYS_MODE | 0x80              @ switch to sys_mode and IRQs disabled	 
	 ldmfd   sp!, {r0-r3}          @ 从sp弹出 参数0 至 参数3 到 r0-r3
     
	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 

     movs    pc, lr               @ 调用系统功能

APICallRet:
                                   @ 系统功能完成并返回到只这里，r0等于系统功能返回值
     stmfd   sp!, {r0}             @ 保存 r0

     bl      DefaultHandler        @ 调用默认处理
     
	 sub     sp, sp, #0x4                 @ APICallReturn( LPDWORD * lpdwState ) 需要参数
	 mov     r0, sp
     bl      APICallReturn         @ 默认处理完成，得到返回地址并做其他切换
	                               @ r0 = 需要返回的PC地址
	 ldmfd   sp!, {r1}             @ r1 = spsr
	 mov     r12, r0               @ r12=返回到调用者
     
	 ldmfd   sp!, {r0}             @ 恢复 r0

	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 msr     spsr, r1              @
	 movs    pc, r12

DirectRet:

     stmfd   sp!, {r0}             @ 保存 r0

     bl      DefaultHandler        @ 调用默认处理

	 ldmfd   sp!, {r0}             @ 恢复 r0
	 ldmfd   sp!, { r1, r2, r3}    @ 恢复 cpsr, callinfo, return adr

     add     sp, sp, #0x10         @ 弹出 之前压入的 r0-r3 sp
     
     msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled
     
     msr     spsr, r1
	 mov     lr, r3
     movs    pc, lr                @ 返回到调用者

@-----------------------------------------------------------
@-----------------------------------------------------------
    ENTRY ES_PrefetchAbortHandler

	sub     lr, lr, #0xF0000004
	cmp     lr, #0x00200000                @ 是否是系统 API调用返回 ？ must sync with api bit define in eapisrv.h
	blo     _HandleCallReturn                   @ 是，到_HandleCallReturn

    add     lr, lr, #0xF0000000
    stmfd   sp!, {r0-r3, r12, lr}

    mov     r0, lr                          @ (r0) = faulting address
	mov     r1, lr                          @ (r1) = faulting address
    mfc15   r2, c13                         @ (r2) = process base address
    tst     r0, #0xFE000000                 @ slot 0 reference?
    orreq   r0, r0, r2                      @ (r0) = process slot based address
    bl      LoadFailurePage                 @ 装入页(r0) = !0 if entry loaded

	cmp     r0, #0x0	                    @ not handle the page failure
	ldmnefd sp!, {r0-r3,r12,pc}^            @
   
	ldmfd   sp!, {r0-r3,r12,lr}             @ 恢复r0-r3,r12,lr
   	                                        @ 装入页错误, 以下处理错误
	stmfd   sp!, {r4}           			@ save r4

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to system mode w/IRQs disabled
	
	@ 切换到 sys sp	
	ldr     r4, =lpCurThread
	ldr     r4, [r4]
	add     sp, r4, #0x400        @=0xC00-4
	sub     sp, sp, #4
	
	
	stmfd   sp!, {r0-r3, r12, lr}           @ save r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @ switch back to abort mode w/IRQs disabled
	
	ldmfd   sp!, {r4}             	@ 恢复r4
	
	mov     r0, lr
	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @ save lr and spsr

	msr     cpsr_c, #SYS_MODE             @ w/IRQs enable
	bl      INTR_PrefetchAbort             @ 调用C异常处理函数

    msr     cpsr_c, #SYS_MODE | 0x80      @ switch to sys mode w/IRQs disabed
	ldmfd   sp!, {r0-r1}                  @ restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80       @ switch back to abort mode w/IRQs disabled
    mov     lr, r0
	msr     spsr, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}          @ restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80     @ switch back to abort mode w/IRQs disabled
    movs    pc, lr                         @ 返回
    
@ 正常调用返回退出
_HandleCallReturn:
    msr    cpsr_c, #SYS_MODE       @ switch to system mode w/IRQs enable
    b      APICallRet              @
    
@=======================================================
@=======================================================
    ENTRY ES_DataAbortHandler
	sub     lr, lr, #8
	stmfd   sp!, {r0-r3, r12, lr}           @ save some register

    mfc15   r0, c6                          @ (r0) = fault address
    mfc15   r1, c5                          @ (r1) = fault status

	and     r1, r1, #0x0D                   @ type of data abort
    cmp     r1, #0x05                       @ translation error?
	movne   r0, #0x0
	bleq    LoadFailurePage

	cmp     r0, #0x0	                    @ not handle the page failure
	ldmnefd sp!, {r0-r3,r12,pc}^

@   now to handle exception
	ldmfd   sp!, {r0-r3,r12,lr}             @ 恢复r0-r3,r12,lr
   	                                        @ 装入页错误, 以下处理错误
	stmfd   sp!, {r4}           			@ save r4

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to system mode w/IRQs disabled
	
	@ 切换到 sys sp	
	ldr     r4, =lpCurThread
	ldr     r4, [r4]
	add     sp, r4, #0x400        @=0x400-4
	sub     sp, sp, #4
	
	stmfd   sp!, {r0-r3, r12, lr}           @ save r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @ switch back to abort mode w/IRQs disabled
	
	ldmfd   sp!, {r4}           			@ 恢复r4
		
	mov     r0, lr
	mrs     r1, spsr
    mfc15   r3, c6                          @ (r3) = abort address
    mfc15   r12, c5                         @ (r12) = abort status

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @ save lr and spsr

    mov     r1, r3                         @为INTR_DataAbort准备参数
	mov     r2, r12                        @为INTR_DataAbort准备参数   
	msr     cpsr_c, #SYS_MODE             @ w/IRQs enable
	bl      INTR_DataAbort                 @ 调用C异常处理函数 (r0)=abort pc address (r1) = abort data address,(r2) = abort status, 

    msr     cpsr_c, #SYS_MODE | 0x80      @ switch to sys mode w/IRQs disabed
	ldmfd   sp!, {r0-r1}                  @ restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80    @ switch back to abort mode w/IRQs disabled
    mov     lr, r0
	msr     spsr, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}          @ restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80     @ switch back to abort mode w/IRQs disabled
    movs    pc, lr                         @ 返回

@=======================================================
@=======================================================
    ENTRY ES_UnusedHandler

	stmfd   sp!, {r0-r3, r12, lr}           @ save some register 
    mrs     r1, spsr                        @ r1 = spsr
    stmfd   sp!, {r1}                       @ save SPSR onto the IRQ stack

    mov  r0, #0x01
	bl INTR_HandleErrorTrap

    ldmfd   sp!, {r1}                       @ restore IRQ SPSR from the IRQ stack
    msr     spsr, r1                        @ spsr = r1
	ldmfd   sp!, {r0-r3, r12, lr}           @ restore some register
	@subs pc, lr, #4		@ return
	movs pc, lr

	
@=======================================================
@=============================================================
    ENTRY ES_IRQHandler
	sub     lr, lr, #4
	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}          @ save r0-r3,r12, and lr

	msr     cpsr_c, #IRQ_MODE | 0x80       @ switch back to IRQ mode w/IRQs disabled
	mov     r0, lr
   	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @ save lr and spsr
	bl      INTR_Interrupt                 @ 调用C中断处理函数(参数r0=pc)

    msr     cpsr_c, #SYS_MODE              @ switch to supervisor mode w/IRQs enable
	bl      DefaultHandler                 @ 调用C默认处理函数

    msr     cpsr_c, #SYS_MODE | 0x80        @ switch to sys mode w/IRQs disabel
	ldmfd   sp!, {r0-r1}                    @ restore lr and spsr
	msr     cpsr_c, #IRQ_MODE | 0x80        @ switch back to IRQ mode w/IRQs disabled
    mov     lr, r0
	msr     spsr, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}                   @ restore r0-r3,r12, and lr

	msr     cpsr_c, #IRQ_MODE | 0x80       @ switch back to IRQ mode w/IRQs disabled
    movs    pc, lr

@=======================================================
@=======================================================
    ENTRY ES_FIQHandler
	stmfd   sp!, {r0-r3, r12, lr}           @ save some register 
    mrs     r1, spsr                        @ r1 = spsr
    stmfd   sp!, {r1}                       @ save SPSR onto the IRQ stack

	mov r0, #0x02                       @ r0 = trap message
	bl INTR_HandleErrorTrap				@ call c code

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

    ENTRY SetCPUId

    mtc15   r0, c13                         @ set process base address register
    nop
    nop
	mov	pc, lr			@ return

@=======================================================
@=======================================================
    ENTRY GetCPUId

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

