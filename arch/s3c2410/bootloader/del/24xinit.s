;=========================================
; NAME: 24XINIT.S
; DESC: C start up codes
;       Configure memory, ISR ,stacks
;	Initialize C-variables
; HISTORY:
; 2001.02.06:purnnamu: draft ver 0.0
; 2001.03.19:purnnamu: AsyncBusMode is used instead of SyncBusMode
; 2001.03.21:purnnamu: EnterPWDN is corrected for S3C2400.
; 2001.03.22:purnnamu: For SL_IDLE mode, EnterPWDN is corrected. 
;                      CLKDIVN is configured.
; 2001.04.03:purnnamu: There should be some delay between MPLL and UPLL.
; 2001.04.19:purnnamu: EnterPWDN is changed for the 'nGCS=active@STOP' problem.
; 2001.05.04:purnnamu: Big-Endian mode is supported.
; 2001.05.23:purnnamu: EnterPWDN is corrected for SL-IDLE mode.
; 2001.06.05:purnnamu: big-endian change code has some align error. one DCD is added for align.
; 2001.06.16:purnnamu: IRQ/FIQ/SVC stack size is enlarged from 0x400 to 0x1000.
;                      The problem, BWSCON settings in memcfg.a was not applied, is cleared. 
; 2001.06.25:purnnamu: There is problem in SDRAM self-refresh enterance.
;		       SDRAM self-refresh should be entered before CLKCON=SL_IDLE. Why?
; 2001.08.09:purnnamu: support SDRAM CL=3 for the 100Mhz CL=3 low speed SDRAM
;=========================================

	GET option.a
	GET memcfg.a

;Interrupt control register
INTOFFSET	EQU	0x14400014
INTMSK		EQU	0x14400008

;Watchdog timer
WTCON		EQU 	0x15300000

;LCD conttroller
LCDCON1		EQU 	0x14a00000

;Clock Controller
LOCKTIME	EQU	0x14800000
MPLLCON		EQU 	0x14800004
UPLLCON      	EQU	0x14800008
CLKCON		EQU	0x1480000c
CLKDIVN		EQU	0x14800014
    
;Memory Controller
BWSCON		EQU	0x14000000
REFRESH     	EQU 	0x14000024
BIT_SELFREFRESH EQU	(1<<22)

;Pre-defined constants
USERMODE    EQU 0x10
FIQMODE     EQU 0x11
IRQMODE     EQU 0x12
SVCMODE     EQU 0x13
ABORTMODE   EQU 0x17
UNDEFMODE   EQU 0x1b
MODEMASK    EQU 0x1f
NOINT       EQU 0xc0


;The location of stacks
UserStack	EQU	(_STACK_BASEADDRESS-0x3800)	;0x0cff4800
SVCStack    	EQU	(_STACK_BASEADDRESS-0x2800) 	;0x0cff5800
UndefStack	EQU  	(_STACK_BASEADDRESS-0x2400) 	;0x0cff5c00
AbortStack	EQU  	(_STACK_BASEADDRESS-0x2000) 	;0x0cff6000
IRQStack        EQU	(_STACK_BASEADDRESS-0x1000)	;0x0cff7000
FIQStack	EQU	(_STACK_BASEADDRESS-0x0)	;0x0cff8000


;Check if tasm.exe(armasm -16 ...@ADS 1.0) is used.
	GBLL    THUMBCODE
    [ {CONFIG} = 16 
THUMBCODE SETL  {TRUE}
	CODE32
    |   
THUMBCODE SETL  {FALSE}
    ]

    	MACRO
	MOV_PC_LR
	[ THUMBCODE
    	    bx	    lr
    	|
    	    mov	    pc,lr
    	]
	MEND

    	MACRO
	MOVEQ_PC_LR
	[ THUMBCODE
    	    bxeq    lr
    	|
    	    moveq    pc,lr
    	]
	MEND

    	MACRO
$HandlerLabel HANDLER $HandleLabel

$HandlerLabel
       [ ISR_2ADDR
	sub     sp,sp,#4        ;decrement sp(to store jump address)
	stmfd   sp!,{r0,r1}        ;PUSH the work register to stack(lr does't push because it return to original address)

	ldr     r0,=$HandleLabel;load the address of HandleXXX to r0
        ldr	r1,=0xdfffffc	;*
	ldr	r1,[r1]		;*
        cmp	r1,#0x44	;*
	subne	r0,r0,#0x1000000;*
	
	ldr     r0,[r0]         ;load the contents(service routine start address) of HandleXXX
	str     r0,[sp,#8]      ;store the contents(ISR) of HandleXXX to stack
	ldmfd   sp!,{r0,r1,pc}     ;POP the work register and pc(jump to ISR)
       |	
	sub     sp,sp,#4        ;decrement sp(to store jump address)
	stmfd   sp!,{r0}        ;PUSH the work register to stack(lr does't push because it return to original address)
	ldr     r0,=$HandleLabel;load the address of HandleXXX to r0
	ldr     r0,[r0]         ;load the contents(service routine start address) of HandleXXX
        str     r0,[sp,#4]      ;store the contents(ISR) of HandleXXX to stack
	ldmfd   sp!,{r0,pc}     ;POP the work register and pc(jump to ISR)
       ]

	MEND
	
	

	IMPORT  |Image$$RO$$Limit|  ; End of ROM code (=start of ROM data)
	IMPORT  |Image$$RW$$Base|   ; Base of RAM to initialise
	IMPORT  |Image$$ZI$$Base|   ; Base and limit of area
	IMPORT  |Image$$ZI$$Limit|  ; to zero initialise
	
	IMPORT  Main    ; The main entry of mon program 
	
	AREA    Init,CODE,READONLY

	ENTRY 


	;1)The code, which converts to Big-endian, should be in little endian code.
	;2)The following little endian code will be compiled in Big-Endian mode. 
	;  The code byte order should be changed as the memory bus width.
	;3)The pseudo instruction,DCD can't be used here because the linker generates error.
	ASSERT	:DEF:ENDIAN_CHANGE
	[ ENDIAN_CHANGE
	    ASSERT  :DEF:ENTRY_BUS_WIDTH
	    [ ENTRY_BUS_WIDTH=32
		b ChangeBigEndian	  ;DCD 0xea000007 
	    ]
	    [ ENTRY_BUS_WIDTH=16
		andeq r14,r7,r0,lsl #20   ;DCD 0x0007ea00
	    ]
	    [ ENTRY_BUS_WIDTH=8
            	streq r0,[r0,-r10,ror #1] ;DCD 0x070000ea
            ]
	|
	b ResetHandler  
    	]
	b HandlerUndef  ;handler for Undefined mode
	b HandlerSWI    ;handler for SWI interrupt
	b HandlerPabort ;handler for PAbort
	b HandlerDabort ;handler for DAbort
	b .     	;reserved
	b HandlerIRQ	;handler for IRQ interrupt 
	b HandlerFIQ	;handler for FIQ interrupt

;@0x20
	b EnterPWDN
ChangeBigEndian
;@0x24
	[ ENTRY_BUS_WIDTH=32
	    DCD 0xee110f10	;0xee110f10 => mrc p15,0,r0,c1,c0,0
	    DCD 0xe3800080	;0xe3800080 => orr r0,r0,#0x80;  //Big-endian
	    DCD 0xee010f10	;0xee010f10 => mcr p15,0,r0,c1,c0,0
	]
	[ ENTRY_BUS_WIDTH=16
	    DCD 0x0f10ee11
	    DCD 0x0080e380	
	    DCD 0x0f10ee01	
	]
	[ ENTRY_BUS_WIDTH=8
	    DCD 0x100f11ee	
	    DCD 0x800080e3	
	    DCD 0x100f01ee	
        ]
	DCD 0xffffffff  ;swinv 0xffffff is similar with NOP and run well in both endian mode. 
	DCD 0xffffffff
	DCD 0xffffffff
	DCD 0xffffffff  ;This line was skipped. fixed at 5.JUN.2001
;@0x40  	
	DCD 0xffffffff
	DCD 0xffffffff
	DCD 0xffffffff
	DCD 0xffffffff
	DCD 0xffffffff
	DCD 0xffffffff
	DCD 0xffffffff
	b ResetHandler

	
	
;Function for entering power down mode
; 1. SDRAM should be in self-refresh mode.
; 2. All interrupt should be maksked for SDRAM/DRAM self-refresh.
; 3. LCD controller should be disabled for SDRAM/DRAM self-refresh.
; 4. The I-cache should be turned on for the STOP/IDLE WORK-AROUND.
; 5. The location of the following code should not be changed for the STOP/IDLE WORK-AROUND.

;void EnterPWDN(int CLKCON); 
EnterPWDN       ;@0x60
	mov     r2,r0		;r2=rCLKCON
	and	r0,r0,#0x7
	cmp 	r0,#0x6		;SL_IDLE mode?
	beq	_SL_IDLE
	cmp	r0,#0x4		;IDLE mode?
	beq	_IDLE	
	ldr     r0,=REFRESH     ;STOP mode
	ldr     r3,[r0]		;r3=rREFRESH	
;@0x80  ----- critical section -----
	mov     r1, r3
	orr     r1, r1, #BIT_SELFREFRESH
	str     r1, [r0]	;Enable SDRAM self-refresh
	nop	;wait until self-refresh is issued. may not be needed.
        nop
        nop
_IDLE	mov     r1,#16	   
	ldr     r0,=CLKCON
;@0xa0 ----- critical section -----
	str     r2,[r0]    
0	subs    r1,r1,#1
	bne     %B0
	nop
	nop
	nop
	nop
	nop
;@0xc0 ----- critical section -----
	and	r2,r2,#0x7
	cmp	r2,#0x4	        ;IDLE mode?
	MOVEQ_PC_LR		;In case of IDLE mode, exiting self-refrsh isn't needed?
	ldr     r0,=REFRESH     ;exit from SDRAM self refresh mode.
	str     r3,[r0]
	MOV_PC_LR
_SL_IDLE
	ldr     r0,=REFRESH     
	ldr     r3,[r0]		;r3=rREFRESH	
;@0xe0 ----- critical section -----	
	mov     r1, r3
	orr     r1, r1, #BIT_SELFREFRESH
	str     r1, [r0]	;Enable SDRAM self-refresh
	ldr     r0,=CLKCON
	str     r2,[r0]    
	nop	
        ldr	r1,=0xffc0000
	ldr	r2,=LCDCON1
;@0x100 ----- critical section -----
; In case of SL_IDLE mode, wait until the current LCD frame is completed.
0       ldr	r0,[r2]
        ands	r0,r0,r1
        beq	%B0
        nop
        nop
        nop
        nop
        nop
;@0x120 ----- critical section -----
;wait until enter SL_IDLE mode. may not be needed
	mov     r0,#16
0	subs    r0,r0,#1
	bne     %B0
	ldr     r0,=REFRESH 	;exit from SDRAM self refresh mode.
	str     r3,[r0]
	MOV_PC_LR

	LTORG   
HandlerFIQ      HANDLER HandleFIQ
HandlerIRQ      HANDLER HandleIRQ
HandlerUndef    HANDLER HandleUndef
HandlerSWI      HANDLER HandleSWI
HandlerDabort   HANDLER HandleDabort
HandlerPabort   HANDLER HandlePabort


IsrIRQ  
	sub     sp,sp,#4       ;reserved for PC
	stmfd   sp!,{r8-r9}   
	
	ldr     r9,=INTOFFSET
	ldr     r9,[r9]
	ldr     r8,=HandleEINT0
	add     r8,r8,r9,lsl #2
	ldr     r8,[r8]
	str     r8,[sp,#8]
	ldmfd   sp!,{r8-r9,pc}


;=======
; ENTRY  
;=======
ResetHandler
	ldr     r0,=WTCON       ;watch dog disable 
	ldr     r1,=0x0         
	str     r1,[r0]

	ldr     r0,=INTMSK
	ldr     r1,=0xffffffff  ;all interrupt disable
	str     r1,[r0]

	;To reduce PLL lock time, adjust the LOCKTIME register. 
	ldr	r0,=LOCKTIME
	ldr	r1,=0xffffff
	str	r1,[r0]
        
        [ PLL_ON_START
	;Configure MPLL
	ldr	r0,=MPLLCON          
	ldr	r1,=((M_MDIV<<12)+(M_PDIV<<4)+M_SDIV)  ;Fin=12MHz,Fout=50MHz
	str	r1,[r0]
	]

	;Set memory control registers
        ldr     r0,=SMRDATA
	ldr     r1,=BWSCON	;BWSCON Address
	add	r2, r0, #52	;End address of SMRDATA
0       
	ldr     r3, [r0], #4    
	str     r3, [r1], #4    
	cmp     r2, r0		
	bne     %B0
	
    	;Initialize stacks
	bl      InitStacks

	ldr	r0,=0xdfffffc	;If HandleXXX is at 0xdffffxx,*(0xdfffffc) should be 0x44
	mov	r1,#0x44
	str	r1,[r0]
	
  	; Setup IRQ handler
	ldr     r0,=HandleIRQ       ;This routine is needed
	ldr     r1,=IsrIRQ          ;if there isn't 'subs pc,lr,#4' at 0x18, 0x1c
	str     r1,[r0]

    	;Copy and paste RW data/zero initialized data
	LDR     r0, =|Image$$RO$$Limit| ; Get pointer to ROM data
	LDR     r1, =|Image$$RW$$Base|  ; and RAM copy
	LDR     r3, =|Image$$ZI$$Base|  
	
	;Zero init base => top of initialised data
	CMP     r0, r1      ; Check that they are different
	BEQ     %F2
1       
	CMP     r1, r3      ; Copy init data
	LDRCC   r2, [r0], #4    ;--> LDRCC r2, [r0] + ADD r0, r0, #4         
	STRCC   r2, [r1], #4    ;--> STRCC r2, [r1] + ADD r1, r1, #4
	BCC     %B1
2       
	LDR     r1, =|Image$$ZI$$Limit| ; Top of zero init segment
	MOV     r2, #0
3       
	CMP     r3, r1      ; Zero init
	STRCC   r2, [r3], #4
	BCC     %B3

    [ :LNOT:THUMBCODE
    	BL  Main        ;Don't use main() because ......
    	B   .                       
    ]

    [ THUMBCODE         ;for start-up code for Thumb mode
    	orr     lr,pc,#1
    	bx      lr
    	CODE16
    	bl      Main        ;Don't use main() because ......
    	b       .
    	CODE32
    ]


;function initializing stacks
InitStacks
	;Don't use DRAM,such as stmfd,ldmfd......
	;SVCstack is initialized before
	;Under toolkit ver 2.5, 'msr cpsr,r1' can be used instead of 'msr cpsr_cxsf,r1'
	mrs     r0,cpsr
	bic     r0,r0,#MODEMASK
	orr     r1,r0,#UNDEFMODE|NOINT
	msr     cpsr_cxsf,r1		;UndefMode
	ldr     sp,=UndefStack
	
	orr     r1,r0,#ABORTMODE|NOINT
	msr     cpsr_cxsf,r1		;AbortMode
	ldr     sp,=AbortStack

	orr     r1,r0,#IRQMODE|NOINT
	msr     cpsr_cxsf,r1            ;IRQMode
	ldr     sp,=IRQStack
    
	orr     r1,r0,#FIQMODE|NOINT
	msr     cpsr_cxsf,r1            ;FIQMode
	ldr     sp,=FIQStack

	bic     r0,r0,#MODEMASK|NOINT
	orr     r1,r0,#SVCMODE
	msr     cpsr_cxsf,r1            ;SVCMode
	ldr     sp,=SVCStack
	
	;USER mode has not be initialized.
	
	mov     pc,lr 
	    ;The LR register won't be valid if the current mode is not SVC mode.
	

	LTORG

SMRDATA DATA
; Memory configuration should be optimized for best performance 
; The following parameter is not optimized.                     
; Memory access cycle parameter strategy
; 1) The memory settings is  safe parameters even at HCLK=75Mhz.
; 2) SDRAM refresh period is for HCLK=75Mhz. 

        DCD (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
    	DCD ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))   ;GCS0
    	DCD ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))   ;GCS1 
    	DCD ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))   ;GCS2
    	DCD ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))   ;GCS3
    	DCD ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))   ;GCS4
    	DCD ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))   ;GCS5
    	DCD ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))    ;GCS6
    	DCD ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))    ;GCS7
    	DCD ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)    

    [ BUSWIDTH=16
    	DCD 0x17            ;SCLK power saving mode, BANKSIZE 16M/16M    
    | ;BUSWIDTH=32
	DCD 0x10            ;SCLK power saving mode, BANKSIZE 32M/32M
    ]
    	DCD 0x30            ;MRSR6 CL=3clk
    	DCD 0x30            ;MRSR7

    	ALIGN


    	AREA RamData, DATA, READWRITE

        ^   _ISR_STARTADDRESS
HandleReset 	#   4
HandleUndef 	#   4
HandleSWI   	#   4
HandlePabort    #   4
HandleDabort    #   4
HandleReserved  #   4
HandleIRQ   	#   4
HandleFIQ   	#   4

;Don't use the label 'IntVectorTable',
;The value of IntVectorTable is different with the address you think it may be.
;IntVectorTable
HandleEINT0   	#   4
HandleEINT1   	#   4
HandleEINT2   	#   4
HandleEINT3   	#   4
HandleEINT4   	#   4
HandleEINT5   	#   4
HandleEINT6   	#   4
HandleEINT7   	#   4
HandleTICK   	#   4
HandleWDT 	#   4
HandleTIMER0 	#   4
HandleTIMER1 	#   4
HandleTIMER2 	#   4
HandleTIMER3 	#   4
HandleTIMER4 	#   4
HandleUERR01  	#   4
HandleNOTUSED 	#   4
HandleDMA0 	#   4
HandleDMA1 	#   4
HandleDMA2	#   4
HandleDMA3	#   4
HandleMMC	#   4
HandleSPI	#   4
HandleURXD0	#   4
HandleURXD1	#   4
HandleUSBD	#   4
HandleUSBH	#   4
HandleIIC   	#   4
HandleUTXD0 	#   4
HandleUTXD1 	#   4
HandleRTC 	#   4
HandleADC 	#   4

        END
