/* ******************************************************
 * @copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
 * ******************************************************
 * 
 * ******************************************************
 * 文件说明：线程切换功能
 * 版本号：1.0.0
 * 开发时期：2001-04-04
 * 作者：李林
 * 修改记录：
 * *******************************************************
 */

.nolist
.include "def.inc"
.include "linkage.inc"
.include "cpu.inc"
.list

.equ	USER_MODE	,	0x10	@2_10000
.equ	FIQ_MODE	,	0x11	@2_10001
.equ	IRQ_MODE	,	0x12	@2_10010
.equ	SVC_MODE	,	0x13	@2_10011
.equ	ABORT_MODE	,	0x17	@2_10111
.equ	UNDEF_MODE	,	0x1b	@2_11011
.equ	SYS_MODE ,	0x1f	@2_11111

	.extern SwitchToProcessByHandle
	.extern SwitchBackProcess
	.extern DoImplementCallBack

	.text

@===========================================================
	@ r0  ->  	lpPrevTSS
	@ r1  ->    lpNextTSS
	@ r2  ->    &lpCurThread
	@ r3  ->    lpNext
@===========================================================
	ENTRY Switch

	str     r3, [r2]
	add     r0, r0, #0x8
	add     r1, r1, #0x8
	stmia   r0, { r0-r14 }

	ldmia   r1, { r0 - r14 }
	mov    pc, r14


@typedef struct _SYSCALL
@{
@	UINT uiCallMode;	// 输入之前的模式; 输出，将要切换到的新模式
@	DWORD dwCallInfo;
@	PFNVOID pfnRetAdress;
@	UINT uiArg[1];		//可变长度
@}SYSCALL, FAR * LPSYSCALL;
@PFNVOID DoImplementCallBack( LPSYSCALL lpCallInfo )

	ENTRY KL_ImplementCallBack
	b _ImplementCallBack
	
	ENTRY KL_ImplementCallBack4	

_ImplementCallBack:
	
	stmfd sp!, {r0-r3}
	sub   sp, sp, #12
	mov   r0, sp
	bl    DoImplementCallBack
	cmp   r0, #0
	moveq pc, lr					@假如无效，直接返回
    
	ldr  lr, =0xF0100000            @返回地址
	ldr  r12, [sp]                  @调用模式
	add  sp, sp, #12

	msr  cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQ disable
	msr  spsr_c, r12                @
	mov  lr,     r0                 @ 调用地址

	msr  cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQ disable
	ldmfd sp!, {r0-r3}

	msr  cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQ disable
	movs pc, lr



    ENTRY _CallInMode
    @调用从系统模式到用户模式
    @int _CallInMode( LPVOID (r0)lpParam, LPVOID (r1)lpIP, LPVOID (r2)sp, UINT (r3)mode )
    @
	msr  cpsr_c, #SYS_MODE        @ switch to sys_mode
	@mov  lr, #0x0
	ldr   lr, =0xF0100000         @return sys adr
    mov  sp, r2
	msr  cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQ disable
    msr  spsr, r3              @
    movs pc, r1                @ 决不返回
    
    ENTRY SwitchToStackSpace
    mov  sp, r0
    mov  pc, lr
        

@=======================================================
@VOID HandlerException( LPJMP_EXCEPTION jmp_data, int retv, UINT mode )
@
@=======================================================

    ENTRY HandlerException

	ldmia  r0!, {r4 - r12, sp, lr}
	
	ldmia  r0!, {r3}                  @ r3 = return pc

	movs   r0, r1					 @ return value
	moveq  r0, #1

	msr   cpsr_c, #SVC_MODE | 0x80    @ switch to svc_mode and IRQ disable
    msr   spsr, r2				    @					

	movs  pc, r3

	@内核调用功能，该部分代码需要与 callsrv.s 同步改动
	
    ENTRY KC_CaptureException    

    @ copy to EXCEPTION_CONTEXT struct
	mov    r12, #0
	add    r1, pc, #_KC_ExceptionRet-(.+8)
	stmfd  sp!, {r1}		@ return address	
	stmfd  sp!, {r4 - r12, sp, lr}

	@ call sys api
	mov  r0, sp	
	stmfd sp!, {lr}
    bl   KL_CaptureException
    ldmfd sp!, {lr}
   
    add  sp, sp, #48		@ popup EXCEPTION_CONTEXT struct 
    mov   pc, lr			@ common  return 

_KC_ExceptionRet:    
    mov   pc, lr			@ common  return 


	