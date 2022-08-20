@/* ******************************************************
@ * @copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
@ * ******************************************************
@ * 
@ * ******************************************************
@ * �ļ�˵�����߳��л�����
@ * �汾�ţ�1.0.0
@ * ����ʱ�ڣ�2001-04-04
@ * ���ߣ�����
@ * �޸ļ�¼��
@ * *******************************************************
@ */

.nolist
.include "linkage.inc"
.list

	.extern SwitchToProcessByHandle
	.extern SwitchBackProcess

	
 .text
   
   
	ENTRY	Switch

	@ r0  ->  	lpPrevTSS
	@ r1  ->    lpNextTSS
	add     r0, r0, #0x8
	add     r1, r1, #0x8
	stmia   r0, { r0-r14 }
	ldmia   r1, { r0-r14 }
	mov    pc, r14
	
	
	ENTRY	KL_ImplementCallBack

	@ r0 -> callstruct poiter, param0, param1,....
	stmfd   sp!, {r4, r5, lr}      @ save some register 

	mov    r4, r0
	ldr    r0, [r4]                    @ r1 = pointer to callbackdata
	ldr    r0, [r0]                    @ r0 =  hProcess

	sub    sp, sp, #0x14                @ c= 5 * 4 space for callback struct
	mov    r1, sp                      @ r1 = callstack

	bl SwitchToProcessByHandle                 @( hProcess, cs * )

	cmp  r0, #0
	beq  _ret0

	sub   sp, sp, #0x30                @12 * 4
	mov   r0, sp
	add   r1, r4, #0x14

	@ copy 12 int from [r1] to [r0]
	mov   r2, #0xc@
L1:
	ldr	  r3, [r1], #4
	str   r3, [r0], #4
	subs  r2, r2, #1
	bne	  L1
		
	ldr   r0, [r4, #0x04]        @arg0
	ldr   r1, [r4, #0x08]        @arg1
	ldr   r2, [r4, #0x0c]        @arg2
	ldr   r3, [r4, #0x10]        @arg3
								 @call the lpfn
	ldr    r4, [r4]              
	ldr    r4, [r4,#0x04]        @r4 =  lpfn

	mov    lr, pc
	mov    pc, r4
	@@bl     r4                  @call the lpfn

	add    sp, sp, #0x30         @clear sp 12 * 4	

	mov    r4, r0                @save retv
	bl     SwitchBackProcess
	mov    r0, r4	             @restore retv
_ret0:
	add    sp, sp, #0x14         @clear callstack

	ldmfd  sp!, {r4, r5, lr}     @save some register 
	mov    pc, lr
	@ENTRY_END
	
	

	ENTRY	KL_ImplementCallBack4
	@ r0 -> callstruct poiter, r1 = param1, r2 = param2, r3 = param3 
	stmfd sp!, {lr}
	stmfd sp!, {r0-r3}

	mov r0, sp
	bl _KL_ImplementCallBack4       @r0�Ƿ���ֵ,��Ҫ�ı�

	add sp, sp, #0x10               @���� r0-r3
	ldmfd sp!, {pc}
	
	
	ENTRY	_KL_ImplementCallBack4
	@ r0 -> sp of callstruct poiter, param1, param2, param3 
	stmfd  sp!, {r4, r5, lr}           @ save some register 
	
	mov    r4, r0		              @ ������r0���浽r4
	ldr    r0, [r4]                   @ r1 = pointer to CALLBACKDATA
	ldr    r0, [r0]                   @ r0 =  hProcess

	sub    sp, sp, #0x14              @ c = 20��ΪSwitchToProcessByHandle׼��CALLSTACK �ṹ�ռ�
	mov    r1, sp                     @ r1 = CALLSTACK �ṹ�ռ��ַ
	bl     SwitchToProcessByHandle            @ �л��� hProcess�Ľ��̿ռ䣬BOOL SwitchToProcessByHandle( hProcess, cs * )@ ����ɹ�����TRUE
	cmp    r0, #0                     @ �Ƿ�ɹ���
	beq    _ret1
	
	ldr    r0, [r4]                     @ r0 = ָ�� CALLBACKDATA �ṹ��ַ
	ldr    r5, [r0, #0x04]				@ r5 = CALLBACKDATA �ṹ��Ա lpfn
	ldr    r0, [r0, #0x08]				@ r0 = CALLBACKDATA �ṹ��Ա arg0����Ҫ���ݵĲ��� arg0 ��
	ldr    r1, [r4, #0x04]				@ r1 = ��Ҫ���ݵĲ��� arg1 
	ldr    r2, [r4, #0x08]				@ r2 = ��Ҫ���ݵĲ��� arg2
	ldr    r3, [r4, #0x0c]				@ r3 = ��Ҫ���ݵĲ��� arg3
        
    mov    lr, pc                       @ Ϊ���� lpfn׼�����ص�ַ
    mov    pc, r5		                @ ���ã�call��lpfn 
	mov    r5, r0                       @ ���� lpfn�ķ���ֵ r0
	
	bl     SwitchBackProcess            @ �л���������

	mov    r0, r5				        @ �ָ�����ֵ r0
_ret1:
	add    sp, sp, #0x14				@ ����ΪSwitchToProcessByHandle׼����CALLSTACK
	ldmfd  sp!, {r4, r5, pc}			@ save some register 
  	
 