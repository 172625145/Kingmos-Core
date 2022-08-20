/* ******************************************************
 * @copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
 * ******************************************************
 * 
 * ******************************************************
 * �ļ�˵�����߳��л�����
 * �汾�ţ�1.0.0
 * ����ʱ�ڣ�2001-04-04
 * ���ߣ�����
 * �޸ļ�¼��
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
@	UINT uiCallMode;	// ����֮ǰ��ģʽ; �������Ҫ�л�������ģʽ
@	DWORD dwCallInfo;
@	PFNVOID pfnRetAdress;
@	UINT uiArg[1];		//�ɱ䳤��
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
	moveq pc, lr					@������Ч��ֱ�ӷ���
    
	ldr  lr, =0xF0100000            @���ص�ַ
	ldr  r12, [sp]                  @����ģʽ
	add  sp, sp, #12

	msr  cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQ disable
	msr  spsr_c, r12                @
	mov  lr,     r0                 @ ���õ�ַ

	msr  cpsr_c, #SYS_MODE | 0x80        @ switch to sys_mode and IRQ disable
	ldmfd sp!, {r0-r3}

	msr  cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQ disable
	movs pc, lr



    ENTRY _CallInMode
    @���ô�ϵͳģʽ���û�ģʽ
    @int _CallInMode( LPVOID (r0)lpParam, LPVOID (r1)lpIP, LPVOID (r2)sp, UINT (r3)mode )
    @
	msr  cpsr_c, #SYS_MODE        @ switch to sys_mode
	@mov  lr, #0x0
	ldr   lr, =0xF0100000         @return sys adr
    mov  sp, r2
	msr  cpsr_c, #SVC_MODE | 0x80        @ switch to svc_mode and IRQ disable
    msr  spsr, r3              @
    movs pc, r1                @ ��������
    
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

	@�ں˵��ù��ܣ��ò��ִ�����Ҫ�� callsrv.s ͬ���Ķ�
	
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


	