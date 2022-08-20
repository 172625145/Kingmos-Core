@/* ******************************************************
@ * @copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
@ * ******************************************************
@ * 
@ * ******************************************************
@ * 文件说明：线程切换功能
@ * 版本号：1.0.0
@ * 开发时期：2001-04-04
@ * 作者：李林
@ * 修改记录：
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
	bl _KL_ImplementCallBack4       @r0是返回值,不要改变

	add sp, sp, #0x10               @弹出 r0-r3
	ldmfd sp!, {pc}
	
	
	ENTRY	_KL_ImplementCallBack4
	@ r0 -> sp of callstruct poiter, param1, param2, param3 
	stmfd  sp!, {r4, r5, lr}           @ save some register 
	
	mov    r4, r0		              @ 将参数r0保存到r4
	ldr    r0, [r4]                   @ r1 = pointer to CALLBACKDATA
	ldr    r0, [r0]                   @ r0 =  hProcess

	sub    sp, sp, #0x14              @ c = 20，为SwitchToProcessByHandle准备CALLSTACK 结构空间
	mov    r1, sp                     @ r1 = CALLSTACK 结构空间地址
	bl     SwitchToProcessByHandle            @ 切换到 hProcess的进程空间，BOOL SwitchToProcessByHandle( hProcess, cs * )@ 假如成功返回TRUE
	cmp    r0, #0                     @ 是否成功？
	beq    _ret1
	
	ldr    r0, [r4]                     @ r0 = 指到 CALLBACKDATA 结构地址
	ldr    r5, [r0, #0x04]				@ r5 = CALLBACKDATA 结构成员 lpfn
	ldr    r0, [r0, #0x08]				@ r0 = CALLBACKDATA 结构成员 arg0（需要传递的参数 arg0 ）
	ldr    r1, [r4, #0x04]				@ r1 = 需要传递的参数 arg1 
	ldr    r2, [r4, #0x08]				@ r2 = 需要传递的参数 arg2
	ldr    r3, [r4, #0x0c]				@ r3 = 需要传递的参数 arg3
        
    mov    lr, pc                       @ 为调用 lpfn准备返回地址
    mov    pc, r5		                @ 调用（call）lpfn 
	mov    r5, r0                       @ 保存 lpfn的返回值 r0
	
	bl     SwitchBackProcess            @ 切换到调用者

	mov    r0, r5				        @ 恢复返回值 r0
_ret1:
	add    sp, sp, #0x14				@ 弹出为SwitchToProcessByHandle准备的CALLSTACK
	ldmfd  sp!, {r4, r5, pc}			@ save some register 
  	
 