/* ******************************************************
 * @copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
 * ******************************************************
 * 
 * ******************************************************
 * �ļ�˵�����쳣����
 * �汾�ţ�1.0.0
 * ����ʱ�ڣ�2003-04-04
 * ���ߣ��ܱ�
 * �޸ļ�¼��AlexZeng 
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
	bl      INTR_HandleErrorTrap           @;û�д���,���������Ϣ
	ldmfd   sp!, {r0-r3, lr}
	subs    pc,  lr,  #4

  ENTRY	Swi_Handler
	sub     r12, r12, #0xF0000000
	cmp     r12, #0x00200000                @;�Ƿ���ϵͳ API���� �� must sync with api bit define in eapisrv.h
	blo     HandleAPICall                   @;�ǣ���HandleAPICall

    add     r12, r12, #0xF0000000           @;�񣬴����������ָ�r12��ֵ
    
	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}           @; save r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @; switch back to svc_mode and IRQs disabled
	mov     r0, lr
   	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r1}                    @; ����lr and spsr��sys_mode��stack� save lr and spsr��

    mov     r1, r12                         @; ΪINTR_Software׼������
	msr     cpsr_c, #SYS_MODE               @; IRQs enable
	bl      INTR_Software                   @; ����C�쳣������
    
    msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r1}                    @; restore lr and spsr
	msr     cpsr_c, #SVC_MODE | 0x80        @; switch back to svc_mode and IRQs disabled
    mov     lr, r0                          @; restore svc_mode lr and spsr
	msr     spsr_c, r1

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to svc_mode and IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}           @; restore r0-r3,r12, and lr

	msr     cpsr_c, #SVC_MODE | 0x80        @; switch to svc_mode and IRQs disabled
    movs    pc, lr                          @; ����

HandleAPICall:

	msr     cpsr_c, #SYS_MODE              @; switch to sys_mode and IRQs disabled

@; ��r0-r3ѹ��sp

	stmfd   sp!, {r0-r3}

     msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 mrs     r3, spsr                              @ ΪMakeAPICall׼������3

@;
@; sp[0] = r0 - ����0(arg0)
@; sp[1] = r1 - ����1(arg1)
@; sp[2] = r2 - ����2(arg2)
@; sp[3] = r3 - ����3(arg3)
@; sp[4] =  - ����4(arg4)
@; sp[5] = - ����5(arg5)
@; ...
@;



	 mov     r1, sp                @; ΪMakeAPICall׼������1 

	 stmfd   sp!, {r3,lr}             @; ���� sys_mode�µ� lr

	 mov     r0, r12               @; ΪMakeAPICall׼������0 = ������Ϣ��api call info��
	 mov     r2, lr                @; ΪMakeAPICall׼������2 = ���ص�ַ��return address��
	 bl      MakeAPICall           @; MakeAPICall ��������Ҫ���õ�ϵͳ���ܵ�ַ���������л�

	 cmp     r0, #0x0              @; ϵͳ�����Ƿ���Ч ��
	 beq     DirectRet             @; �񣬵�DirectRet

	 mov     r12,r0                @; ��Ч��r0����Ч��ϵͳ���ܵ�ַ
	 ldmfd   sp!, {r3,lr}             @; �ָ� sys_mode�µ� lr
	 ldmfd   sp!, {r0-r3}          @; ��sp���� ����0 �� ����3 �� r0-r3
	 add     lr, pc, #APICallRet-(.+8)        @; lr = return address = APICallRet
	 mov     pc, r12               @; ����ϵͳ����

APICallRet:
								   @; ϵͳ������ɲ����ص�ֻ���r0����ϵͳ���ܷ���ֵ
  	 stmfd   sp!, {r0}             @; ���� r0

	 bl      DefaultHandler        @; ����Ĭ�ϴ���

	 sub     sp, sp, #0x4                 @ APICallReturn( LPDWORD * lpdwState ) ��Ҫ����
	 mov     r0, sp
	 bl      APICallReturn         @; Ĭ�ϴ�����ɣ��õ����ص�ַ���������л�
								   @; r0 = ��Ҫ���ص�PC��ַ
     ldmfd   sp!, {r1}             @ r1 = spsr

	 mov     r12, r0               @; ���ص�������

	 ldmfd   sp!, {r0}             @; �ָ� r0

	 msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled	 
	 msr     spsr, r1              @
	 movs    pc, r12

DirectRet:

     stmfd   sp!, {r0}             @; ���� r0

     bl      DefaultHandler        @; ����Ĭ�ϴ���

	 ldmfd   sp!, {r0}             @; �ָ� r0

	 ldmfd   sp!, {r1,r2}          @ �ָ� r1=spsr ���ص�r2=PC��ַ	 
     add     sp, sp, #0x10         @ ���� ֮ǰѹ��� r0-r3 sp

     msr     cpsr_c, #SVC_MODE | 0x80              @ switch to svc_mode and IRQs disabled
     msr     spsr, r1
	 mov     lr, r2
     movs    pc, lr                @ ���ص�������


@	 ldmfd   sp!, {lr}             @; �ָ� ���ص�PC��ַ
@     add     sp, sp, #0x10         @; ���� ֮ǰѹ��� r0-r3sp
@     mov     pc, lr                @; ���ص�������

@;ȡָ���쳣
  ENTRY Prefetch_Handler
	sub     lr, lr, #4

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}           @; save r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @; switch back to abort_mode and IRQs disabled
	mov     r0, lr
	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	stmfd   sp!, {r0-r1}                    @; ����lr and spsr��sys_mode��stack� save lr and spsr��

	msr     cpsr_c, #SYS_MODE               @; IRQs enable
	bl      INTR_PrefetchAbort              @; ����C�쳣������ r0 = lr

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r1}                    @; restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80        @; switch back to abort_mode and IRQs disabled
	mov     lr, r0                          @; restore svc_mode lr and spsr
	msr     spsr_c, r1

	msr     cpsr_c, #SYS_MODE | 0x80        @; switch to sys_mode and IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}           @; restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80      @; switch to abort_mode and IRQs disabled
	movs    pc, lr                          @; ����


@;ȡ������ֹ
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
	bl      INTR_DataAbort                 @; ����C�쳣������ (r0) = abort data address,(r1) = abort status, (r2)=abort pc address

    msr     cpsr_c, #SYS_MODE | 0x80      @; switch to sys mode w/IRQs disabed
	ldmfd   sp!, {r0-r1}                  @; restore lr and spsr
	msr     cpsr_c, #ABORT_MODE | 0x80    @; switch back to abort mode w/IRQs disabled
    mov     lr, r0
	msr     spsr_c, r1

	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	ldmfd   sp!, {r0-r3, r12, lr}          @; restore r0-r3,r12, and lr

	msr     cpsr_c, #ABORT_MODE | 0x80     @; switch back to abort mode w/IRQs disabled
    movs    pc, lr                         @; ����
       
@;�жϴ���
   ENTRY IRQ_Handler
	sub     lr, lr, #4
	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r3, r12, lr}          @; save r0-r3,r12, and lr

	msr     cpsr_c, #IRQ_MODE | 0x80       @; switch back to IRQ mode w/IRQs disabled
	mov     r0, lr
   	mrs     r1, spsr

	msr     cpsr_c, #SYS_MODE | 0x80       @; switch to system mode w/IRQs disabled
	stmfd   sp!, {r0-r1}                   @; save lr and spsr
	bl      INTR_Interrupt                 @; ����C�жϴ�����(����r0=pc)

    msr     cpsr_c, #SYS_MODE              @; switch to supervisor mode w/IRQs enable
	bl      DefaultHandler                 @; ����CĬ�ϴ�����

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
@;** ��������: INTR_ON    
@;** ��������: ���ж�
@;** �䡡��:   ��
@;** �䡡�� :  ��
@;** ȫ�ֱ���: ��
@;** ����ģ��: ��
@;** 
@;** ������: AlexZeng
@;** �ա���: 2003��11��4��
@;**-------------------------------------------------------------------------------------------------------
@;** �ޡ���: 
@;** �ա���: 
@;**-------------------------------------------------------------------------------------------------------
@;********************************************************************************************************/
  ENTRY	INTR_ON
    mrs     r0, cpsr                        @; (r0) = current status
    bic     r1, r0, #0x80                   @; clear interrupt disable bit
    msr     cpsr_c, r1                        @; update status register
    mov     pc, lr                                  @; return to caller
	

@;/*********************************************************************************************************
@;** ��������: INTR_OFF    
@;** ��������: ���ж�
@;** �䡡��:   ��
@;** �䡡�� :  ��
@;** ȫ�ֱ���: ��
@;** ����ģ��: ��
@;** 
@;** ������: AlexZeng
@;** �ա���: 2003��11��4��
@;**-------------------------------------------------------------------------------------------------------
@;** �ޡ���: 
@;** �ա���: 
@;**-------------------------------------------------------------------------------------------------------
@;********************************************************************************************************/
  ENTRY  INTR_OFF
    mrs     r0, cpsr                        @; (r0) = current status
    orr     r1, r0, #0x80                   @; set interrupt disable bit
    msr     cpsr_c, r1                      @;  update status register
    mov     pc, lr                          @; return to caller
 
@;/*********************************************************************************************************
@;** ��������: SetDataBreakpoint 
@;** ��������: �������ݵ�ַ�ϵ�
@;** �䡡��:   ��
@;** �䡡�� :  ��
@;** ȫ�ֱ���: ��
@;** ����ģ��: ��
@;** 
@;** ������: AlexZeng
@;** �ա���: 2003��11��4��
@;**-------------------------------------------------------------------------------------------------------
@;** �ޡ���: 
@;** �ա���: 
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




