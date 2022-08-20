	OPT	2	; disable listing
	INCLUDE kxarm.h
	;INCLUDE sa11x0.inc 
	INCLUDE cpu.inc
	OPT	1	; reenable listing

	EXPORT  Switch
	IMPORT  SwitchToProcess
	IMPORT  SwitchBackProcess


    TEXTAREA

	LEAF_ENTRY Switch
	; r0  ->  	lpPrevTSS
	; r1  ->    lpNextTSS


	add     r0, r0, #0x8
	add     r1, r1, #0x8
	stmia   r0, { r0-r14 }

	mrs     r2,  cpsr                        ; r2 = cpsr	
    bic	    r2, r2, #0x80	; clear interrupt disable bit
	stmdb   r0, { r2, r14 }

	ldmdb   r1, { r0, r14 }
	msr     spsr, r0                        ; spsr = r0	
	ldmia   r1, { r0 - r14 }
	movs    pc, r14

	ENTRY_END

	LEAF_ENTRY KL_ImplementCallBack
	; r0 -> callstruct poiter, param0, param1,....

	stmfd   sp!, {r4, r5, lr}           ; save some register 

	mov    r4, r0
	ldr    r0, [r4]                    ; r1 = pointer to callbackdata
	ldr    r0, [r0]                    ; r0 =  hProcess

	sub    sp, sp, #0x14                ; sapce for callback struct  
	mov    r1, sp                      ; r1 = callstack

	bl SwitchToProcess                 ;( hProcess, cs * )

	cmp  r0, #0
	beq  _ret0

	sub   sp, sp, #0x30                   ;12 * 4
	mov   r0, sp
	add   r1, r4, #0x14

	; copy 12 int from [r1] to [r0]

	ldr   r2, [r1]
	ldr   r3, [r1, #0x4]
	str   r2, [r0]
	str   r3, [r0, #0x4]

	ldr   r2, [r1, #0x8]
	ldr   r3, [r1, #0xc]
	str   r2, [r0, #0x8]
	str   r3, [r0, #0xc]

	ldr   r2, [r1, #0x10]
	ldr   r3, [r1, #0x14]
	str   r2, [r0, #0x10]
	str   r3, [r0, #0x14]

	ldr   r2, [r1, #0x18]
	ldr   r3, [r1, #0x1c]
	str   r2, [r0, #0x18]
	str   r3, [r0, #0x1c]

	ldr   r2, [r1, #0x20]
	ldr   r3, [r1, #0x24]
	str   r2, [r0, #0x20]
	str   r3, [r0, #0x24]

	ldr   r2, [r1, #0x28]
	ldr   r3, [r1, #0x2c]
	str   r2, [r0, #0x28]
	str   r3, [r0, #0x2c]

    ldr   r0, [r4, #0x04]        ;arg0
	ldr   r1, [r4, #0x08]        ;arg1
	ldr   r2, [r4, #0x0c]        ;arg2
	ldr   r3, [r4, #0x10]        ;arg3

                                       ;call the lpfn
	ldr    r4, [r4]                    ;
	ldr    r4, [r4,#0x04]              ; r5 =  lpfn
	bl     r4                          ;call the lpfn
	add    sp, sp, #0x30               ;48     ;clear sp 12 * 4	

	mov    r4, r0                    ; save retv
	bl     SwitchBackProcess
	mov    r0, r4	                 ; restore retv

_ret0

	add    sp, sp, #0x14                ; clear callstack

	ldmfd  sp!, {r4, r5, lr}           ; save some register 
	mov    pc, lr

	ENTRY_END


	LEAF_ENTRY KL_ImplementCallBack4
	; r0 -> callstruct poiter, r1 = param1, r2 = param2, r3 = param3 

	stmfd sp!, {lr}
	stmfd sp!, {r0-r3}

	mov r0, sp
	bl _KL_ImplementCallBack4

	add sp, sp, #0x10             ;ldmfd sp!, {r0-r3}
	ldmfd sp!, {lr}
	mov   pc, lr
	ENTRY_END

_KL_ImplementCallBack4
	; r0 -> sp of callstruct poiter, param1, param2, param3 
	
	stmfd  sp!, {r4, r5, lr}           ; save some register 

	mov    r4, r0                      ; r4 pointer stack
	ldr    r0, [r4]                    ; r1 = pointer to callbackdata

	ldr    r0, [r0]                    ; r0 =  hProcess

	sub    sp, sp, #0x14                ;c = 20
	mov    r1, sp                      ; r1 = callstack

	bl     SwitchToProcess             ;( hProcess, cs * )

	cmp    r0, #0

	beq    _ret1
	
	

    ldr    r0, [r4]
	ldr    r5, [r0, #0x04]         ;r5 = lpfn
	ldr    r0, [r0, #0x08]         ;arg0
	ldr    r1, [r4, #0x04]        ;arg1
	ldr    r2, [r4, #0x08]        ;arg2
	ldr    r3, [r4, #0x0c]        ;arg3	
    
    mov       lr, pc
    mov       pc, r5               ; call the lpfn 

	mov       r5, r0                    ; save retv

	bl     SwitchBackProcess

	mov    r0, r5	                 ; restore retv

_ret1

	add    sp, sp, #0x14               ;12                ; clear callstack

	ldmfd  sp!, {r4, r5, lr}           ; save some register 
	mov    pc, lr

	;ENTRY_END


	LEAF_ENTRY _Switch
	; r0  ->  	lpPrevTSS
	; r1  ->    lpNextTSS


	str     r0, [r0, #R_R0]
	str     r1, [r0, #R_R1]
    str     r2, [r0, #R_R2]

	mrs     r2, cpsr                        ; r2 = cpsr	
    bic	    r2, r2, #0x80	; clear interrupt disable bit
	str     r2, [r0, #R_CPSR]       ; save cpsr
	str     lr, [r0, #R_PC]         ; save return address 

	add     r0, r0, #0x14
	stmia   r0, { r3-r14 }          ; save r3->r14

; now switch to next thread

    ldr     r0, [r1, #R_CPSR]
;    bic	    r0, r0, #0x80	           ; clear interrupt disable bit
	msr     spsr, r0                   ; spsr = r0
	ldr     lr, [r1, #R_PC]            ; lr = return address
	ldr     r0, [r1, #R_R0]            ; restroe r0
	ldr     r2, [r1, #R_R2]            ; restroe r2

	add     r1, r1, #0x14              ;pointer to r3 
	ldmia   r1, { r3-r14 }             ;restore r3-r14
	sub     r1, r1, #0x14           
	ldr     r1, [r1,#R_R1]             ;restroe r1
	movs    pc, lr

	ENTRY_END
;--------------------------------------------------------------

	LEAF_ENTRY __Switch

	str     r0, [r0, #R_R0]               ;
	str     r1, [r0, #R_R1]               ;

	str     r2, [r0, #R_R2]               ;
	str     r3, [r0, #R_R3]               ;

	str     r4, [r0, #R_R4]               ;
	str     r5, [r0, #R_R5]               ;
	str     r6, [r0, #R_R6]               ;
	str     r7, [r0, #R_R7]               ;
	str     r8, [r0, #R_R8]               ;
	str     r9, [r0, #R_R9]               ;
	str     r10, [r0, #R_R10]               ;
	str     r11, [r0, #R_R11]               ;
	str     r12, [r0, #R_R12]               ;

	str     r13, [r0, #R_SP]               ; save sp to prev thread
	str     lr,  [r0, #R_LR]               ; save pc to prev thread

	mrs     r2,  cpsr                        ; r2 = cpsr
	bic	    r2, r2, #0x80	; clear interrupt disable bit
	str     r2,  [r0, #R_CPSR]

	; restore next thread's register

	ldr     r4, [r1, #R_R4]         
	ldr     r5, [r1, #R_R5]         
	ldr     r6, [r1, #R_R6]         
	ldr     r7, [r1, #R_R7]         
	ldr     r8, [r1, #R_R8]         
	ldr     r9, [r1, #R_R9]         
	ldr     r10, [r1, #R_R10]       
	ldr     r11, [r1, #R_R11]       
	ldr     r12, [r1, #R_R12]       
	
	ldr     r13,  [r1, #R_SP]       
	ldr     lr,   [r1, #R_LR]        

	ldr     r2,  [r1, #R_CPSR]
	bic	    r2, r2, #0x80	; clear interrupt disable bit
	msr     spsr, r2                        ; spsr = r2	
   
	ldr     r2,  [r1, #R_R2]               ; 
	ldr     r3,  [r1, #R_R3]               ; 
	ldr     r0,  [r1, #R_R0]               ; start lParam when first run
	ldr     r1,  [r1, #R_R1]               ; 

	movs     pc, lr                        ; update pc and cpsr
	nop
	ENTRY_END

;	LEAF_ENTRY GetTestCodeAdr
 ;   add     r0, pc, #MyTestCode-(.+8)        ; lr = return address
  ;  mov     pc, lr

;MyTestCode     
 ;    ;stmfd   sp!, {lr}
;	 mov r0, #0x1
;	 mov r1, #0x2
;	 mov r3, #0x3
;	 mov r4, #0x4 
;     
;     ;bl      OutMyDebugString
;	 
;	 add     r0, r0, #0xF0000004
;
;	 ;ldmfd   sp!, {lr}  
;	 mov     pc, lr
;	ENTRY_END

	END
