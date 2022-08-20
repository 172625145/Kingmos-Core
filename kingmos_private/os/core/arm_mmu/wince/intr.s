	OPT	2	; disable listing
	INCLUDE kxarm.h
	;INCLUDE sa11x0.inc 
	OPT	1	; reenable listing

	EXPORT  INTR_OFF
	EXPORT  INTR_ON
	EXPORT  ES_VectorTable

	IMPORT  INTR_Interrupt
	IMPORT  INTR_Software
	IMPORT  INTR_PrefetchAbort
	IMPORT  INTR_DataAbort
	IMPORT  INTR_FastIntr
	IMPORT  DefaultHandler
	IMPORT  DebugValue
	IMPORT  GetTryFlagAdr
	IMPORT  LoadFailurePage

	IMPORT  MakeAPICall
	IMPORT  APICallReturn

; ARM processor modes
USER_MODE	EQU	2_10000
FIQ_MODE	EQU 2_10001
IRQ_MODE	EQU 2_10010
SVC_MODE	EQU 2_10011
ABORT_MODE	EQU 2_10111
UNDEF_MODE	EQU 2_11011
SYSTEM_MODE EQU	2_11111

    MACRO
    mtc15   $cpureg, $cp15reg
    mcr     p15,0,$cpureg,$cp15reg,c0,0
    MEND

    MACRO
    mfc15   $cpureg, $cp15reg
    mrc     p15,0,$cpureg,$cp15reg,c0,0
    MEND

	TEXTAREA
 	
ES_VectorTable
        DCD     ES_ResetHandler
        DCD     ES_UndefExceptionHandler
        DCD     ES_SWIHandler
        DCD     ES_PrefetchAbortHandler
        DCD     ES_DataAbortHandler
        DCD     ES_UnusedHandler
        DCD     ES_IRQHandler
        DCD     ES_FIQHandler
		LTORG

;-----------------------------------------------------------


;-----------------------------------------------------------
	LEAF_ENTRY ES_ResetHandler
	nop
	ENTRY_END ES_ResetHandler
;-----------------------------------------------------------
;-----------------------------------------------------------
    LEAF_ENTRY ES_UndefExceptionHandler
	nop
	ENTRY_END ES_UndefExceptionHandler
;-----------------------------------------------------------
;-----------------------------------------------------------
    LEAF_ENTRY ES_SWIHandler
	stmfd   sp!, {r0-r3, r12, lr}           ; save some register 
    mrs     r1, spsr                        ; r1 = spsr
    stmfd   sp!, {r1}                       ; save SPSR onto the IRQ stack

	mov  r0, lr
	bl INTR_Software                             ; call c code

    ldmfd   sp!, {r1}                       ; restore IRQ SPSR from the IRQ stack
    msr     spsr, r1                        ; spsr = r1
	ldmfd   sp!, {r0-r3, r12, lr}           ; restore some register
	;subs pc, lr, #4		; return
	movs pc, lr
    ENTRY_END ES_SWIHandler
;-----------------------------------------------------------
;-----------------------------------------------------------
    LEAF_ENTRY ES_PrefetchAbortHandler
	sub     lr, lr, #0xF0000004
	cmp     lr, #0x00200000         ;must sync with api bit define in eapisrv.h
	blo     HandleAPICall

	;  handle exception
	add     lr, lr, #0xF0000000


    stmfd   sp!, {r0-r3, r12, lr}
    mov     r0, lr                          ; (r0) = faulting address
    mfc15   r2, c13                         ; (r2) = process base address
    tst     r0, #0xFE000000                 ; slot 0 reference?
    orreq   r0, r0, r2                      ; (r0) = process slot based address
    bl      LoadFailurePage                   ; (r0) = !0 if entry loaded

	cmp     r0, #0x0	                    ; not handle the page failure
	ldmnefd sp!, {r0-r3,r12,pc}^

    
	; now, handle exception
	ldmfd   sp!, {r0-r3,r12,lr}             ; 
	stmfd   sp!, {r0-r3, r12, lr}           ; save some register 
	ldmfd   sp!, {r0-r3,r12,lr}             ; 
	stmfd   sp!, {r4,r5}
	mov     r4,  lr
	mrs     r5,  spsr
	msr     cpsr_c, #SVC_MODE:OR:0x80       ; switch to supervisor mode w/IRQs disabled
	stmfd   sp!, {r0-r3,r4}                 ; r4 is lr(data_abort'lr)
	stmfd   sp!, {r5,r12}                   ; save spsr(r5) and r12
	stmfd   sp!, {lr}                       ; lr is svc's lr	
	mov     r0,  r4

	msr     cpsr_c, #ABORT_MODE:OR:0x80       ; switch back to abort mode w/IRQs disabled
	ldmfd   sp!, {r4,r5}                    ;restore r4, r5

	msr     cpsr_c, #SVC_MODE:OR:0x80       ; switch to supervisor mode w/IRQs disabled

	bl      INTR_PrefetchAbort					; call c code, handle exception

	ldmfd   sp!, {lr}             ; lr=svc's lr
	ldmfd   sp!, {r0,r12}         ; r0 = spsr
	msr     spsr, r0
	ldmfd   sp!, {r0-r3,pc}^

HandleAPICall

     mov     r12, lr               ; api call info
	 msr     cpsr_c, #SVC_MODE     ; switch to supervisor mode w/IRQs disabled

     stmfd   sp!, {r0-r3}
	 mov     r1, sp

	 stmfd   sp!, {lr}

     mov     r0, r12               ; api call info     
	 mov     r2, lr                ; return address
     bl      MakeAPICall

	 cmp     r0, #0x0              ; 2003-06-13
	 beq     DirectRet            ; 2003-06-13

     mov     r12,r0                ; r0 is call address
	 ldmfd   sp!, {lr}
     ldmfd   sp!, {r0-r3}
     add     lr, pc, #APICallRet-(.+8)        ; lr = return address
     mov     pc, r12

APICallRet

     stmfd   sp!, {r0}
     
     bl      DefaultHandler        ; if resche ?
     bl      APICallReturn

	 mov     r12, r0
	 ldmfd   sp!, {r0}  
	 mov     pc, r12

DirectRet
     stmfd   sp!, {r0}            ; push return value
     bl      DefaultHandler        ;if resche ?

	 ldmfd   sp!, {r0}
	 ldmfd   sp!, {lr}
     add     sp, sp, #0x10
	 ;ldmfd   sp!, {r0-r3}       ;popup sp
     mov     pc, lr

    ENTRY_END ES_PrefetchAbortHandler

;-----------------------------------------------------------
;-----------------------------------------------------------

    LEAF_ENTRY ES_DataAbortHandler
	sub     lr, lr, #8
	stmfd   sp!, {r0-r3, r12, lr}           ; save some register

    mfc15   r0, c6                          ; (r0) = fault address
    mfc15   r1, c5                          ; (r1) = fault status

;	mov     r2, lr                          ; (r2) = abort address

;    mfc15   r2, c13                         ; (r2) = process base address
 ;   tst     r0, #0xFE000000                 ; slot 0 reference?
  ;  orreq   r0, r0, r2                      ; (r0) = process slot based address

	and     r1, r1, #0x0D                   ; type of data abort
    cmp     r1, #0x05                       ; translation error?
	movne   r0, #0x0
	bleq    LoadFailurePage

	cmp     r0, #0x0	                    ; not handle the page failure
	ldmnefd sp!, {r0-r3,r12,pc}^

;   now to handle exception
	ldmfd   sp!, {r0-r3,r12,lr}             ; 
	stmfd   sp!, {r4,r5}
	mov     r4,  lr
	mrs     r5,  spsr
	msr     cpsr_c, #SVC_MODE:OR:0x80       ; switch to supervisor mode w/IRQs disabled
	stmfd   sp!, {r0-r3,r4}                 ; r4 is lr(data_abort'lr)
	stmfd   sp!, {r5,r12}                   ; save spsr(r5) and r12
	stmfd   sp!, {lr}                       ; lr is svc's lr	
	mov     r2,  r4

	msr     cpsr_c, #ABORT_MODE:OR:0x80       ; switch back to abort mode w/IRQs disabled
	ldmfd   sp!, {r4,r5}                    ;restore r4, r5

	msr     cpsr_c, #SVC_MODE:OR:0x80       ; switch to supervisor mode w/IRQs disabled

    mfc15   r0, c6                          ; (r0) = fault address
    mfc15   r1, c5                          ; (r1) = fault status

	bl    INTR_DataAbort					; call c code, handle exception

	ldmfd   sp!, {lr}             ; lr=svc's lr
	ldmfd   sp!, {r0,r12}         ; r0 = spsr
	msr     spsr, r0
	ldmfd   sp!, {r0-r3,pc}^

	ENTRY_END ES_DataAbortHandler

;-----------------------------------------------------------
;-----------------------------------------------------------
    LEAF_ENTRY ES_UnusedHandler
	stmfd   sp!, {r0-r3, r12, lr}           ; save some register 
    mrs     r1, spsr                        ; r1 = spsr
    stmfd   sp!, {r1}                       ; save SPSR onto the IRQ stack

    ldmfd   sp!, {r1}                       ; restore IRQ SPSR from the IRQ stack
    msr     spsr, r1                        ; spsr = r1
	ldmfd   sp!, {r0-r3, r12, lr}           ; restore some register
	;subs pc, lr, #4		; return
	movs pc, lr

	ENTRY_END ES_UnusedHandler
;-----------------------------------------------------------

;-----------------------------------------------------------

    LEAF_ENTRY ES_IRQHandler
	sub     lr, lr, #4                      ; fix return address
	stmfd   sp!, {r4, r5}                   ; save some register to IRQ stack

	mov     r4,   lr                        ; r4 = spsr
	mrs     r5,   spsr

	msr     cpsr_c, #SVC_MODE:OR:0x80       ; switch to supervisor mode w/IRQs disabled
	stmfd   sp!, {r0-r3, r4}                   ; save r0-r3, and lr(irq_mode)
	stmfd   sp!, {r12, lr}                  ; save r12, lr(svc_mode)
	stmfd   sp!, {r5}                       ; save spsr
	

	mov     r0,  r4                         ; r0 = lr(irq_mode)
	mov     r1,  r5                         ; r1 = spsr(irq_mode)

	msr     cpsr_c, #IRQ_MODE:OR:0x80       ; switch back to IRQ mode w/IRQs disabled
	ldmfd   sp!, {r4, r5}                   ; restore r4, r5

	msr     cpsr_c, #SVC_MODE:OR:0x80       ; switch to supervisor mode w/IRQs disabled

    stmfd   sp!, {r0}                       ; save lr
	bl      INTR_Interrupt                      ; add by zenghao
	ldmfd   sp!, {r0}                       ; r0=lr

    msr     cpsr_c, #SVC_MODE       ; switch to supervisor mode w/IRQs enable
	bl      DefaultHandler
    msr     cpsr_c, #SVC_MODE:OR:0x80       ; switch to supervisor mode w/IRQs disabled
	;bl      DebugValue

    ldmfd   sp!, {r0}                       ; restore spsr
	msr     spsr, r0
	
	ldmfd   sp!, {r12, lr}                       ; restore register
	ldmfd   sp!, {r0-r3, pc}^                   ; restore register

	ENTRY_END

;------------------------------------------------------------

;-----------------------------------------------------------
    LEAF_ENTRY ES_FIQHandler
	stmfd   sp!, {r0-r3, r12, lr}           ; save some register 
    mrs     r1, spsr                        ; r1 = spsr
    stmfd   sp!, {r1}                       ; save SPSR onto the IRQ stack

	bl INTR_FastIntr					; call c code

    ldmfd   sp!, {r1}                       ; restore IRQ SPSR from the IRQ stack
    msr     spsr, r1                        ; spsr = r1
	ldmfd   sp!, {r0-r3, r12, lr}           ; restore some register
	subs pc, lr, #4		; return
	ENTRY_END ES_FIQHandler

;-----------------------------------------------------------
;

;	LEAF_ENTRY ES_InstallVectorTable
	
;	stmfd sp!, {r0-r3, lr}

	; setup destination of VectorTable at address 0x20 ~ 0x20 + 8 * 4
;	ldr r0, =0xFFFF0020        ; dest address
;    ldr r1, =VectorTable       ; source address
;	ldr r3, =0x8               ; count 

;20	ldr r2, [r1], #4           ; now , copy from source to dest
;	str r2, [r0], #4
;	subs r3, r3, #1
;	bgt %B20

	; setup interrupt vector jmp table
;	ldr r0, =VectorJmpTable	    ; source
;	ldr r1, =0xFFFF0000			; dest 
;	ldr r3, =0x8                ; count
;10	ldr r2, [r0], #4            ; now copy from source to dest
;	str r2, [r1], #4
;	subs r3, r3, #1
;	bgt %B10

	;ok , init finish, return 
;	ldmfd sp!, {r0-r3, pc}
;	ENTRY_END  


;-------------------------------------------------------------------------------
; INTR_ON - enable interrupts
;-------------------------------------------------------------------------------
        LEAF_ENTRY INTR_ON
        mrs     r0, cpsr                        ; (r0) = current status
        bic     r1, r0, #0x80                   ; clear interrupt disable bit
        msr     cpsr, r1                        ; update status register
        mov     pc, lr                                  ; return to caller
		ENTRY_END INTR_ON
		


;-------------------------------------------------------------------------------
; INTR_OFF - disable interrupts
;-------------------------------------------------------------------------------
        LEAF_ENTRY INTR_OFF
        mrs     r0, cpsr                        ; (r0) = current status
        orr     r1, r0, #0x80                   ; set interrupt disable bit
        msr     cpsr, r1                        ; update status register
        mov     pc, lr                          ; return to caller
		ENTRY_END INTR_OFF


;-----------------------------------------------------------
	LEAF_ENTRY SetDataBreakpoint
	;r0 = Access data breakpoint address register (DBAR).
	;r1 = Access data breakpoint value register (DBVR).
	;r2 = Access data breakpoint mask register (DBMR).
	;r3 = Load data breakpoint control register (DBCR).
	mcr p15,0,r0,c14,c0,0
	mcr p15,0,r1,c14,c1,0
	mcr p15,0,r2,c14,c2,0
	mcr p15,0,r3,c14,c3,0
	mov	pc, lr			; return
	ENTRY_END  

	LEAF_ENTRY SetInstructionBreakpoint
	;r0 = Write instruction breakpoint address and control register (IBCR).
	cmp r0, #0x0
	orrne r0, r0, #0x1                   ; enable InstructionBreakpoint
	mcrne p15,0,r0,c14,c8,0
	mov	pc, lr			; return
	ENTRY_END  

	LEAF_ENTRY GetInstructionBreakpoint
	mrc p15,0,r0,c14,c8,0
	mov	pc, lr			; return
	ENTRY_END  

    LEAF_ENTRY SetProcessId

    mtc15   r0, c13                         ; set process base address register
	mov	pc, lr			; return

	ENTRY_END

    LEAF_ENTRY GetProcessId

    mfc15   r0, c13                         ; set process base address register
	mov	pc, lr			; return

	ENTRY_END


    LEAF_ENTRY KL_GetCPSR
    mrs     r0, cpsr                        ; (r0) = current status
    mov     pc, lr                                  ; return to caller
	ENTRY_END  


    ;LEAF_ENTRY KL_SetCPSR
    ;stmfd   sp!, {r0, r1, lr}           ; save some register 
    ;msr     spsr, r0                               ; (r0) = new status
	;ldmfd   sp!, {r0, r1, pc}^
	;movs     pc, lr
	;ENTRY_END  

    LEAF_ENTRY GetiPC
	mov r0, lr
	mov pc, lr
	ENTRY_END  

	
	LEAF_ENTRY _____TRY
	        ;BOOL __TRY( int * lp, int len )
	stmfd   sp!, {r0-r1, lr}           ; save some register 
	
	bl GetTryFlagAdr

	mov r2, r0                         ; r2 = flag adr
	ldmfd   sp!, {r0-r1, lr}           ; save some register 

	mov r3, #0x01
	str r3, [r2]

	cmp r1, #0x0
	beq _try1
_try0
	ldr r3, [r2]                       ; flag adr
	cmp r3, #0x0
	beq _try1

	ldr r3, [r0]
	add r0, r0, #0x4
	subs r1, r1, #0x4
	bne _try0

_try1

    ldr r0, [r2]                         ;return value
	mov r3,  #0x0
	str r3, [r2]

    ;mov r0, lr
	;bl _Testr0

	mov pc, lr

	ENTRY_END

	END
