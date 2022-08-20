/* ******************************************************
 * @copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
 * ******************************************************
 * 
 * ******************************************************
 * 文件说明：异常处理
 * 版本号：1.0.0
 * 开发时期：2003-04-04
 * 作者：周兵
 * 修改记录：AlexZeng 
 * *******************************************************
 */
.nolist
.include "linkage.inc"
.list


	.include "cpu.inc"
	
	.globl Undefined_Handler
	.globl Swi_Handler
	.globl Prefetch_Handler
	.globl DataAbort_Handler
	.globl FIQ_Handler
	.globl IRQ_Handler
	.globl GetInstructionBreakpoint


    .text

 ENTRY	Undefined_Handler
	stmfd	sp!, {r0-r3, r12, lr}
	mov     r0, #0x01
	bl		INTR_HandleErrorTrap
	ldmfd	sp!, {r0-r3, r12, pc}^

  ENTRY	FIQ_Handler
	stmfd   sp!, {r0-r3, lr}
	mov     r0, #0x02
	bl      INTR_HandleErrorTrap           @;没有处理,仅仅输出信息
	ldmfd   sp!, {r0-r3, lr}
	subs    pc,  lr,  #4

  ENTRY	Swi_Handler
	sub     r12, r12, #0xF0000000
	cmp     r12, #0x00200000                @;是否是系统 API调用 ？ must sync with api bit define in eapisrv.h
	blo     HandleAPICall                   @;是，到HandleAPICall

    add     r12, r12, #0xF0000000           @;否，处理错误，这里恢复r12的值
    
	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}           @; save r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @; switch back to svc_mode and IRQs disabled
	mov     r0, lr
   	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r1}                    @; 保存lr and spsr到sys_mode的stack里（ save lr and spsr）

    mov     r1, r12                         @; 为INTR_Software准备参数
	msr     cpsr_c, #SYS_MODE               @; IRQs enable
	bl      INTR_Software                   @; 调用C异常处理函数
    
    msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r1}                    @; restore lr and spsr
	msr     cpsr_c, #SVC_MODE | 0x80        @; switch back to svc_mode and IRQs disabled
    mov     lr, r0                          @; restore svc_mode lr and spsr
	msr     spsr_c, r1

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to svc_mode and IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}           @; restore r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @; switch to svc_mode and IRQs disabled
    movs    pc, lr                          @; 返回

HandleAPICall:

	msr     cpsr_c, #SYS_MODE              @; switch to sys_mode and IRQs disabled

@; 将r0-r3压入sp

	stmfd   sp!, {r0-r3}

     msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 mrs     r3, spsr                              @ 为MakeAPICall准备参数3

@;
@; sp[0] = r0 - 参数0(arg0)
@; sp[1] = r1 - 参数1(arg1)
@; sp[2] = r2 - 参数2(arg2)
@; sp[3] = r3 - 参数3(arg3)
@; sp[4] =  - 参数4(arg4)
@; sp[5] = - 参数5(arg5)
@; ...
@;



	 mov     r1, sp                @; 为MakeAPICall准备参数1 

	 stmfd   sp!, {r3,lr}             @; 保存 sys_mode下的 lr

	 mov     r0, r12               @; 为MakeAPICall准备参数0 = 调用信息（api call info）
	 mov     r2, lr                @; 为MakeAPICall准备参数2 = 返回地址（return address）
	 bl      MakeAPICall           @; MakeAPICall 将返回需要调用的系统功能地址并做其他切换

	 cmp     r0, #0x0              @; 系统功能是否有效 ？
	 beq     DirectRet             @; 否，到DirectRet

	 mov     r12,r0                @; 有效，r0是有效的系统功能地址
	 ldmfd   sp!, {r3,lr}             @; 恢复 sys_mode下的 lr
	 ldmfd   sp!, {r0-r3}          @; 从sp弹出 参数0 至 参数3 到 r0-r3
	 add     lr, pc, #APICallRet-(.+8)        @; lr = return address = APICallRet
	 mov     pc, r12               @; 调用系统功能

APICallRet:
								   @; 系统功能完成并返回到只这里，r0等于系统功能返回值
  	 stmfd   sp!, {r0}             @; 保存 r0

	 bl      DefaultHandler        @; 调用默认处理

	 sub     sp, sp, #0x4                 @ APICallReturn( LPDWORD * lpdwState ) 需要参数
	 mov     r0, sp
	 bl      APICallReturn         @; 默认处理完成，得到返回地址并做其他切换
								   @; r0 = 需要返回的PC地址
     ldmfd   sp!, {r1}             @ r1 = spsr

	 mov     r12, r0               @; 返回到调用者

	 ldmfd   sp!, {r0}             @; 恢复 r0

	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 msr     spsr, r1              @
	 movs    pc, r12

DirectRet:

     stmfd   sp!, {r0}             @; 保存 r0

     bl      DefaultHandler        @; 调用默认处理

	 ldmfd   sp!, {r0}             @; 恢复 r0

	 ldmfd   sp!, {r1,r2}          @ 恢复 r1=spsr 返回的r2=PC地址	 
     add     sp, sp, #0x10         @ 弹出 之前压入的 r0-r3 sp

     msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled
     msr     spsr, r1
	 mov     lr, r2
     movs    pc, lr                @ 返回到调用者


@	 ldmfd   sp!, {lr}             @; 恢复 返回的PC地址
@     add     sp, sp, #0x10         @; 弹出 之前压入的 r0-r3sp
@     mov     pc, lr                @; 返回到调用者

@;取指令异常
  ENTRY Prefetch_Handler
	sub     lr, lr, #4

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}           @; save r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @; switch back to abort_mode and IRQs disabled
	mov     r0, lr
	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r1}                    @; 保存lr and spsr到sys_mode的stack里（ save lr and spsr）

	msr     cpsr_c, #SYS_MODE               @; IRQs enable
	bl      INTR_PrefetchAbort              @; 调用C异常处理函数 r0 = lr

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r1}                    @; restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80        @; switch back to abort_mode and IRQs disabled
	mov     lr, r0                          @; restore svc_mode lr and spsr
	msr     spsr_c, r1

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}           @; restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @; switch to abort_mode and IRQs disabled
	movs    pc, lr                          @; 返回


@;取数据中止
  ENTRY DataAbort_Handler
	sub     lr, lr, #8
	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}           @; save r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @; switch back to abort mode w/IRQs disabled
	mov     r0, lr
   	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @; save lr and spsr

	msr     cpsr_c, #SYS_MODE              @; w/IRQs enable
	bl      INTR_DataAbort                 @; 调用C异常处理函数 (r0) = abort data address,(r1) = abort status, (r2)=abort pc address

    msr     cpsr_c, #SYS_MODE | 0x80      @; switch to sys mode w/IRQs disabed
	ldmfd   sp!, {r0-r1}                  @; restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80    @; switch back to abort mode w/IRQs disabled
    mov     lr, r0
	msr     spsr_c, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}          @; restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80     @; switch back to abort mode w/IRQs disabled
    movs    pc, lr                         @; 返回
       
@;中断处理
   ENTRY IRQ_Handler
	sub     lr, lr, #4
	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}          @; save r0-r3,r12, and lr

	msr     cpsr_c, #IRQ_MODE | 0x80       @; switch back to IRQ mode w/IRQs disabled
	mov     r0, lr
   	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @; save lr and spsr
	bl      INTR_Interrupt                 @; 调用C中断处理函数(参数r0=pc)

    msr     cpsr_c, #SYS_MODE              @; switch to supervisor mode w/IRQs enable
	bl      DefaultHandler                 @; 调用C默认处理函数

    msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys mode w/IRQs disabel
	ldmfd   sp!, {r0-r1}                    @; restore lr and spsr
	msr     cpsr_c, #IRQ_MODE | 0x80        @; switch back to IRQ mode w/IRQs disabled
    mov     lr, r0
	msr     spsr_c, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}           @; restore r0-r3,r12, and lr

	msr     cpsr_c, #IRQ_MODE | 0x80       @; switch back to IRQ mode w/IRQs disabled
    movs    pc, lr


@;/*********************************************************************************************************
@;** 函数名称: INTR_ON    
@;** 功能描述: 开中断
@;** 输　入:   无
@;** 输　出 :  无
@;** 全局变量: 无
@;** 调用模块: 无
@;** 
@;** 作　者: AlexZeng
@;** 日　期: 2003年11月4日
@;**-------------------------------------------------------------------------------------------------------
@;** 修　改: 
@;** 日　期: 
@;**-------------------------------------------------------------------------------------------------------
@;********************************************************************************************************/
  ENTRY	INTR_ON
    mrs     r0, cpsr                        @; (r0) = current status
    bic     r1, r0, #0x80                   @; clear interrupt disable bit
    msr     cpsr_c, r1                        @; update status register
    mov     pc, lr                                  @; return to caller
	

@;/*********************************************************************************************************
@;** 函数名称: INTR_OFF    
@;** 功能描述: 关中断
@;** 输　入:   无
@;** 输　出 :  无
@;** 全局变量: 无
@;** 调用模块: 无
@;** 
@;** 作　者: AlexZeng
@;** 日　期: 2003年11月4日
@;**-------------------------------------------------------------------------------------------------------
@;** 修　改: 
@;** 日　期: 
@;**-------------------------------------------------------------------------------------------------------
@;********************************************************************************************************/
  ENTRY  INTR_OFF
    mrs     r0, cpsr                        @; (r0) = current status
    orr     r1, r0, #0x80                   @; set interrupt disable bit
    msr     cpsr_c, r1                      @;  update status register
    mov     pc, lr                          @; return to caller
 
@;/*********************************************************************************************************
@;** 函数名称: SetDataBreakpoint 
@;** 功能描述: 设置数据地址断点
@;** 输　入:   无
@;** 输　出 :  无
@;** 全局变量: 无
@;** 调用模块: 无
@;** 
@;** 作　者: AlexZeng
@;** 日　期: 2003年11月4日
@;**-------------------------------------------------------------------------------------------------------
@;** 修　改: 
@;** 日　期: 
@;**-------------------------------------------------------------------------------------------------------
@;********************************************************************************************************/
  ENTRY SetDataBreakpoint
	@;@r0 = Access data breakpoint address register (DBAR).
	@;@r1 = Access data breakpoint value register (DBVR).
	@;@r2 = Access data breakpoint mask register (DBMR).
	@;@r3 = Load data breakpoint control register (DBCR).
	;mcr p15,0,r0,c14,c0,0
	;mcr p15,0,r1,c14,c1,0
	;mcr p15,0,r2,c14,c2,0
	;mcr p15,0,r3,c14,c3,0
	mov	pc, lr


@;@=======================================================
	ENTRY SetInstructionBreakpoint 
	@;@r0 = write instruction breakpoint address and control register (ibcr).
	@;cmp r0, #0x0
	@;orrne r0, r0, #0x1                   @ enable instructionbreakpoint
	@;mcrne p15,0,r0,c14,c8,0
	mov	pc, lr


	ENTRY GetInstructionBreakpoint
	@;mrc p15,0,r0,c14,c8,0
	mov	pc, lr
        

    .end




