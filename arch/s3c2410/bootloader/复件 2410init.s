/* 
 *  Copyright (C) 2002 Samsung Electronics SW.LEE  <hitchcar@sec.samsung.com>
 *  Copyright (c) 2001  Marius Gr鰃er <mag@sysgo.de>
 *  Copyright (c) 2002  Alex Z黳ke <azu@sysgo.de>
 *  Copyright (c) 2002  Gary Jennejohn <gj@denx.de>
 * 
 *  ARMBOOT boot startup code 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

.include "memcfg.inc"
.include "option.inc"	
.include "register.inc"

.equ	USER_MODE	,	0x10	@2_10000
.equ	FIQ_MODE	,	0x11	@2_10001
.equ	IRQ_MODE	,	0x12	@2_10010
.equ	SVC_MODE	,	0x13	@2_10011
.equ	ABORT_MODE	,	0x17	@2_10111
.equ	UNDEF_MODE	,	0x1b	@2_11011
.equ	SYS_MODE    ,   0x1f	@2_11111
.equ    MODEMASK    ,   0x1f
.equ    NOINT       ,   0xc0

.equ SysStack, (_STACK_BASEADDRESS-0x3800)	@;0x33ff4800 ~ 
.equ SVCStack, (_STACK_BASEADDRESS-0x3000) 	@;0x33ff5800 ~
.equ UndefStack, (_STACK_BASEADDRESS-0x2c00) 	@;0x33ff5c00 ~
.equ AbortStack, (_STACK_BASEADDRESS-0x1c00) 	@;0x33ff5000 ~
.equ IRQStack, (_STACK_BASEADDRESS-0x400)	@;0x33ff7c00 ~
.equ FIQStack, (_STACK_BASEADDRESS-0x0)	@;0x33ff8000 ~

.globl  _startup
	.section .start, #alloc, #execinstr
	.type _startup, #function

@	.global _startup
_startup:
	b	reset				@ Supervisor Mode
        ldr     pc, _undefined_instruction	@ 0x4
        ldr     pc, _software_interrupt		@ 0x8 
        ldr     pc, _prefetch_abort		@ 0xc
        ldr     pc, _data_abort			@ 0x10
        ldr     pc, _not_used			@ 0x14
        ldr     pc, _irq			@ 0x18
        ldr     pc, _fiq			@ 0x1c	
	ldr	pc,_direct			@ 0x20

_undefined_instruction: .word undefined_instruction
_software_interrupt:    .word software_interrupt
_prefetch_abort:        .word prefetch_abort
_data_abort:            .word data_abort
_not_used:              .word not_used
_irq:                   .word irq
_fiq:                   .word fiq
_direct:		.word direct
	.balign 4

	.size	_startup, . - _startup
	.align
	
	.text
	
reset:

	ldr	r0,=WTCON       /*watch dog disable  */
	ldr	r1,=0x0         
	str	r1,[r0]

	ldr	r0,=INTMSK
	ldr	r1,=0xffffffff  /*all interrupt disable */
	str	r1,[r0]

	ldr	r0,=INTSUBMSK
	ldr	r1,=0x3ff		/*all sub interrupt disable */
	str	r1,[r0]
	
	/*	Initialize Ports...for display LED. */
	ldr     r0, =GPFCON
	ldr     r1, =0x55aa
	str     r1, [r0]
	
	ldr     r0, =GPFUP
	ldr     r1, =0xff
	str     r1, [r0]

	ldr	r0,=GPFDAT
	ldr	r1,=POWEROFFLED1
	str	r1,[r0]

 	/* Setup clock Divider control register  
	 * you must configure CLKDIVN before LOCKTIME or MPLL UPLL
	 * because default CLKDIVN 1,1,1 set the SDMRAM Timing Conflict
	nop
	 * FCLK:HCLK:PCLK = 1:2:4  in this case
	 */
	ldr	r0,=CLKDIVN
	ldr	r1,=0x3		
	str	r1,[r0]

	/*To reduce PLL lock time, adjust the LOCKTIME register. */
	ldr	r0,=LOCKTIME
	ldr	r1,=0xffffff
	str	r1,[r0]	

/*Configure MPLL */
    ldr     r0,=MPLLCON
@    (0xa1<<12+0x3<<4+0x1)   
    ldr     r1,= (0xa1<<12+0x3<<4+0x1)   @((M_MDIV<<12)+(M_PDIV<<4)+M_SDIV)  //Fin=12MHz,Fout=203MHz
    str     r1,[r0]

	ldr	r1,=GSTATUS2
	ldr	r10,[r1]
	tst	r10,#OFFRST
	bne	1000f

	/* MEMORY C0NTROLLER(MC)  SETTING */
	add	r0,pc,#MCDATA - (.+8)
	ldr	r1,=BWSCON		
	add	r2,r0,#52	@  End address of MEMORY CONTROLLER
1:	
	ldr	r3,[r0],#4
	str	r3,[r1],#4
	cmp	r2,r0
	bne	1b

 /***************************************** 
  * Power on reset 
  * POWER_OFF reset
  * Watchdog reset
  */
1000:
	tst	r10,#OFFRST
	beq	1001f

	ldr	r1,=MISCCR
	ldr	r0,[r1]
	bic	r0,r0,#SCK_NORMAL
	str	r0,[r1]

	/* MEMORY C0NTROLLER(MC)  SETTING */
	add	r0,pc,#MCDATA - (.+8)
	ldr	r1,=BWSCON		
	add	r2,r0,#52	@  End address of MEMORY CONTROLLER
1:	
	ldr	r3,[r0],#4
	str	r3,[r1],#4
	cmp	r2,r0
	bne	1b
	
	mov	r1,#254		@ Wait until SDRAM self refresh is released
loop1:
	subs	r1,r1,#1
	bne	loop1

direct:
	/***  LED  TEST ***********************/
	ldr	r0,=GPFDAT
	ldr	r1,=POWEROFFLED3
	str	r1,[r0]

	ldr	r1,=GSTATUS3   	/* cpu_s3c2410_resume address*/
	ldr	r6,[r1,#0]
	mov	pc,r6
	nop	

	.pool		

    /***
     * Memory configuration should be optimized for best performance 
     * The following parameter is not optimized.                     
     * Memory access cycle parameter strategy
     * 1) The memory settings is safe parameters even at HCLK=75Mhz.
     * 2) SDRAM refresh period is for HCLK=75Mhz. 
     */

	.align 2
		
MCDATA:
    .word (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
    .word ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))   
    .word ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))  
    .word ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))  
    .word ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))  
    .word ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))  
    .word ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))
    .word ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))    
    .word ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))    
    .word ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)    
    .word 0xB2          /* REFRESH Control Register  */ 
    .word 0x30          /* BANKSIZE Register : Burst Mode       */ 
    .word 0x30          /* SDRAM Mode Register       */ 

	
1001:
	.align 2
	.global call_main
call_main:



/* 初始化 MODE stack */
    mrs	r0, cpsr
	bic	r0, r0, #MODEMASK	
	orr	r1, r0, #(IRQ_MODE|NOINT)
	msr	cpsr_c, R1
@	ldr     sp,STACK_START	
    ldr	sp, =IRQStack
	
	orr	r1, r0, #(FIQ_MODE|NOINT)
	msr	cpsr_c, r1
	ldr	sp, =FIQStack
	
@    ldr     sp,STACK_START
@    sub     sp, sp, #0x4000

	orr	r1, r0, #(ABORT_MODE|NOINT)
	msr	cpsr_c, r1
	ldr	sp, =AbortStack
@    ldr     sp,STACK_START
@    sub     sp, sp, #0x5000


	orr    r1, r0, #(UNDEF_MODE|NOINT)
	msr    cpsr_c, r1
	ldr	sp, =UndefStack
@    ldr     sp,STACK_START
@    sub     sp, sp, #0x6000

	
	orr	r1, r0, #(SYS_MODE|NOINT)
	msr	cpsr_c, r1
	ldr	sp, =SysStack
@    ldr     sp,STACK_START
@    sub     sp, sp, #0x7000

	
	orr	r1, r0, #(SVC_MODE|NOINT)
	msr	cpsr_c, r1
	ldr	sp, =SVCStack
@    ldr     sp,STACK_START
@    sub     sp, sp, #0x8000
@    	ldr     sp,STACK_START
        mov     fp,#0           /* no previous frame, so fp=0*/
        
        mov     a1, #0          /* set argc to 0*/
        mov     a2, #0          /* set argv to NUL*/
        bl      _InitKernel            /* call main*/

STACK_START:
        .word   STACK_BASE



undefined_instruction:
    stmfd	sp!, {r0-r3, r12, lr}
	mov      r0, #0x01
	bl	   INTR_HandleErrorTrap
	ldmfd	sp!, {r0-r3, r12, pc}^
	
software_interrupt:	
    stmfd	sp!, {r0-r3, r12, lr}
	mov      r0, #0x02
	bl	   INTR_HandleErrorTrap
	ldmfd	sp!, {r0-r3, r12, pc}^

prefetch_abort:	
    stmfd	sp!, {r0-r3, r12, lr}
	mov      r0, #0x03
	bl	   INTR_HandleErrorTrap
	ldmfd	sp!, {r0-r3, r12, pc}^
	
data_abort:	
    stmfd	sp!, {r0-r3, r12, lr}
	mov      r0, #0x04
	bl	   INTR_HandleErrorTrap
	ldmfd	sp!, {r0-r3, r12, pc}^

not_used:	
    stmfd	sp!, {r0-r3, r12, lr}
	mov      r0, #0x05
	bl	   INTR_HandleErrorTrap
	ldmfd	sp!, {r0-r3, r12, pc}^

irq:						  /* 中断异常的处理 */  		
    sub     lr, lr, #4
	stmfd	sp!, {r0-r3, r12, lr}
	mrs     r1, spsr
	stmfd	sp!, {r1}
	
	@call 
	bl HandleIRQ

	ldmfd	sp!, {r1}	
	msr     spsr, r1	
	ldmfd	sp!, {r0-r3, r12, lr}
	movs     pc, lr	
fiq:
	stmfd   sp!, {r0-r3, lr}
	mov     r0, #0x06
	bl      INTR_HandleErrorTrap           @;没有处理,仅仅输出信息
	ldmfd   sp!, {r0-r3, lr}
	subs    pc,  lr,  #4
	



	
