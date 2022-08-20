/* ******************************************************
 * @copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
 * ******************************************************
 * 
 * ******************************************************
 * �ļ�˵�����ж��쳣����
 * �汾�ţ�1.0.0
 * ����ʱ�ڣ�2001-04-04
 * ���ߣ��ܱ�
 * �޸ļ�¼��
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
	cmp     r12, #0x00200000                @ �Ƿ���ϵͳ API���� �� must sync with api bit define in eapisrv.h
	blo     HandleAPICall                   @ �ǣ���HandleAPICall

    add     r12, r12, #0xF0000000           @ �񣬴����������ָ�r12��ֵ
    
	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}           @ save r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @ switch back to svc_mode and IRQs disabled
	mov     r0, lr
    mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r1}                    @ ����lr and spsr��sys_mode��stack� save lr and spsr��

    mov     r1, r12                         @ ΪINTR_Software׼������
	msr     cpsr_c, #SYS_MODE               @ IRQs enable
	bl      INTR_Software                   @ ����C�쳣������
    
    msr     cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r1}                    @ restore lr and spsr
	msr     cpsr_c, #SVC_MODE | 0x80        @ switch back to svc_mode and IRQs disabled
    mov     lr, r0                          @ restore svc_mode lr and spsr
	msr     spsr, r1

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to svc_mode and IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}           @ restore r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQs disabled
    movs    pc, lr                          @ ����


HandleAPICall:

@typedef struct _SYSCALL
@{
@	UINT uiCallMode;	// ����֮ǰ��ģʽ; �������Ҫ�л�������ģʽ
@	DWORD dwCallInfo;
@	PFNVOID pfnRetAdress;
@	UINT uiArg[1];		//�ɱ䳤��
@}SYSCALL, FAR * LPSYSCALL;

	 msr     cpsr_c, #SYS_MODE | 0x80              @ switch to sys_mode and IRQs disabled

@ ��r0-r3ѹ��sp

	 stmfd   sp!, {r0-r3}
	 
	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 
	 mrs     r1, spsr                              @ ΪSYSCALL׼������ = ֮ǰ��ģʽ
	 
     msr     cpsr_c, #SYS_MODE                     @ switch to sys_mode and IRQs enable

@
@ sp[0] = r0 - ����0(arg0)
@ sp[1] = r1 - ����1(arg1)
@ sp[2] = r2 - ����2(arg2)
@ sp[3] = r3 - ����3(arg3)
@ sp[4] =  - ����4(arg4)
@ sp[5] = - ����5(arg5)
@ ...
@

     stmfd   sp!, { r1, r12, lr}    @ ׼�� SYSCALL �ṹ��Ա����

	 mov     r0, sp                @ r0 = SYSCALL ptr
     bl      MakeSysCall           @ MakeSysCall ��������Ҫ���õ�ϵͳ���ܵ�ַ���������л�

	 cmp     r0, #0x0              @ r0 = ���õ�ַ�� ϵͳ�����Ƿ���Ч ��
	 beq     DirectRet             @ �񣬵�DirectRet

	 ldr     r12, [sp]             @ r12  = ����ģʽ , �ض���SYS or USER mode
     add     sp, sp, #12		   @ ���sp

	 ldr     lr, =0xF0100000       @ ���ص�ַ

	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 msr     spsr_c, r12                   @����ģʽ 
	 mov     lr, r0

     msr     cpsr_c, #SYS_MODE | 0x80              @ switch to sys_mode and IRQs disabled	 
	 ldmfd   sp!, {r0-r3}          @ ��sp���� ����0 �� ����3 �� r0-r3
     
	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 

     movs    pc, lr               @ ����ϵͳ����

APICallRet:
                                   @ ϵͳ������ɲ����ص�ֻ���r0����ϵͳ���ܷ���ֵ
     stmfd   sp!, {r0}             @ ���� r0

     bl      DefaultHandler        @ ����Ĭ�ϴ���
     
	 sub     sp, sp, #0x4                 @ APICallReturn( LPDWORD * lpdwState ) ��Ҫ����
	 mov     r0, sp
     bl      APICallReturn         @ Ĭ�ϴ�����ɣ��õ����ص�ַ���������л�
	                               @ r0 = ��Ҫ���ص�PC��ַ
	 ldmfd   sp!, {r1}             @ r1 = spsr
	 mov     r12, r0               @ r12=���ص�������
     
	 ldmfd   sp!, {r0}             @ �ָ� r0

	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 msr     spsr, r1              @
	 movs    pc, r12

DirectRet:

     stmfd   sp!, {r0}             @ ���� r0

     bl      DefaultHandler        @ ����Ĭ�ϴ���

	 ldmfd   sp!, {r0}             @ �ָ� r0
	 ldmfd   sp!, { r1, r2, r3}    @ �ָ� cpsr, callinfo, return adr

     add     sp, sp, #0x10         @ ���� ֮ǰѹ��� r0-r3 sp
     
     msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled
     
     msr     spsr, r1
	 mov     lr, r3
     movs    pc, lr                @ ���ص�������

@-----------------------------------------------------------
@-----------------------------------------------------------
    ENTRY ES_PrefetchAbortHandler

	sub     lr, lr, #0xF0000004
	cmp     lr, #0x00200000                @ �Ƿ���ϵͳ API���÷��� �� must sync with api bit define in eapisrv.h
	blo     _HandleCallReturn                   @ �ǣ���_HandleCallReturn

    add     lr, lr, #0xF0000000
    stmfd   sp!, {r0-r3, r12, lr}

    mov     r0, lr                          @ (r0) = faulting address
	mov     r1, lr                          @ (r1) = faulting address
    mfc15   r2, c13                         @ (r2) = process base address
    tst     r0, #0xFE000000                 @ slot 0 reference?
    orreq   r0, r0, r2                      @ (r0) = process slot based address
    bl      LoadFailurePage                 @ װ��ҳ(r0) = !0 if entry loaded

	cmp     r0, #0x0	                    @ not handle the page failure
	ldmnefd sp!, {r0-r3,r12,pc}^            @
   
	ldmfd   sp!, {r0-r3,r12,lr}             @ �ָ�r0-r3,r12,lr
   	                                        @ װ��ҳ����, ���´������
	stmfd   sp!, {r4}           			@ save r4

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to system mode w/IRQs disabled
	
	@ �л��� sys sp	
	ldr     r4, =lpCurThread
	ldr     r4, [r4]
	add     sp, r4, #0x400        @=0xC00-4
	sub     sp, sp, #4
	
	
	stmfd   sp!, {r0-r3, r12, lr}           @ save r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @ switch back to abort mode w/IRQs disabled
	
	ldmfd   sp!, {r4}             	@ �ָ�r4
	
	mov     r0, lr
	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @ save lr and spsr

	msr     cpsr_c, #SYS_MODE             @ w/IRQs enable
	bl      INTR_PrefetchAbort             @ ����C�쳣������

    msr     cpsr_c, #SYS_MODE | 0x80      @ switch to sys mode w/IRQs disabed
	ldmfd   sp!, {r0-r1}                  @ restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80       @ switch back to abort mode w/IRQs disabled
    mov     lr, r0
	msr     spsr, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}          @ restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80     @ switch back to abort mode w/IRQs disabled
    movs    pc, lr                         @ ����
    
@ �������÷����˳�
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
	ldmfd   sp!, {r0-r3,r12,lr}             @ �ָ�r0-r3,r12,lr
   	                                        @ װ��ҳ����, ���´������
	stmfd   sp!, {r4}           			@ save r4

	msr     cpsr_c, #SYS_MODE | 0x80        @ switch to system mode w/IRQs disabled
	
	@ �л��� sys sp	
	ldr     r4, =lpCurThread
	ldr     r4, [r4]
	add     sp, r4, #0x400        @=0x400-4
	sub     sp, sp, #4
	
	stmfd   sp!, {r0-r3, r12, lr}           @ save r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @ switch back to abort mode w/IRQs disabled
	
	ldmfd   sp!, {r4}           			@ �ָ�r4
		
	mov     r0, lr
	mrs     r1, spsr
    mfc15   r3, c6                          @ (r3) = abort address
    mfc15   r12, c5                         @ (r12) = abort status

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @ save lr and spsr

    mov     r1, r3                         @ΪINTR_DataAbort׼������
	mov     r2, r12                        @ΪINTR_DataAbort׼������   
	msr     cpsr_c, #SYS_MODE             @ w/IRQs enable
	bl      INTR_DataAbort                 @ ����C�쳣������ (r0)=abort pc address (r1) = abort data address,(r2) = abort status, 

    msr     cpsr_c, #SYS_MODE | 0x80      @ switch to sys mode w/IRQs disabed
	ldmfd   sp!, {r0-r1}                  @ restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80    @ switch back to abort mode w/IRQs disabled
    mov     lr, r0
	msr     spsr, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @ switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}          @ restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80     @ switch back to abort mode w/IRQs disabled
    movs    pc, lr                         @ ����

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
	bl      INTR_Interrupt                 @ ����C�жϴ�����(����r0=pc)

    msr     cpsr_c, #SYS_MODE              @ switch to supervisor mode w/IRQs enable
	bl      DefaultHandler                 @ ����CĬ�ϴ�����

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

