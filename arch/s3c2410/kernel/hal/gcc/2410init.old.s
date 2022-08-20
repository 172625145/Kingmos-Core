/* 
 *  Copyright (C) 2002 Samsung Electronics SW.LEE  <hitchcar@sec.samsung.com>
 *  Copyright (c) 2001  Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002  Alex Züpke <azu@sysgo.de>
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

.equ SysStack, (_STACK_BASEADDRESS-0x2000)	 @;0x33ff4800 ~ 0x33ff6000
.equ SVCStack, (_STACK_BASEADDRESS-0x1000) 	@;4k 0x33ff5800 ~
.equ UndefStack, (_STACK_BASEADDRESS-0xc00) 	@;1k 0x33ff5c00 ~
.equ AbortStack, (_STACK_BASEADDRESS-0x800) 	@;1k 0x33ff5000 ~
.equ IRQStack, (_STACK_BASEADDRESS-0x400)	@;1k 0x33ff7c00 ~
.equ FIQStack, (_STACK_BASEADDRESS-0x0)	@;1k 0x33ff8000 ~ 

.globl  _startup
	.section .start, #alloc, #execinstr
	.type _startup, #function

_startup:

    mov     r1, #SVC_MODE|NOINT 	@ svc mode, no interrupts
    msr     cpsr, r1
	b	   reset				      

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

.include "table.s"    
    	
1001:
	.align 2	
	.global call_main
call_main:

    @because the romstart is defined as virtual address as:0x8c200000 ,
    @so we must mask the high half-word
    @memory.con's
    @RAMIMAGE  =:	0x8C200000   0X00900000
    @if you change memory.con's RAMIMAGE ,you must change 0x30200000 too
    @
    
	ldr		r2, =Restart_Address	
	ldr		r1, =0x000fffff
	and		r2,r2,r1
	ldr		r1, =0x30200000          @download address
	add		r1,r1,r2
	cmp        r1, #0                  @ make sure no stall on "mov pc,r1" below

	mov		pc,r1

Restart_Address:
	nop
    mov     fp,#0           /* no previous frame, so fp=0*/
	add	 r0, pc, #OEMAddressTable - (. + 8)            
    bl      _KernelStart		@   _KernelStart should never return
    b .
    
