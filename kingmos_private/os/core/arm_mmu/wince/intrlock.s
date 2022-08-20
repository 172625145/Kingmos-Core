	OPT	2	; disable listing
	INCLUDE kxarm.h
	;INCLUDE sa11x0.inc 
	OPT	1	; reenable listing



    TEXTAREA

    LEAF_ENTRY KL_InterlockedExchange
	mov    r2, r0
    swp    r0, r1, [r2]
    mov    pc, lr
	ENTRY_END  KL_InterlockedExchange


	LEAF_ENTRY KL_InterlockedCompareExchange
	;r0 = lpTarge
	;r1 = new Value
	;r2 = compare Value

    stmfd   sp!, {r4}

    mrs     r3, cpsr                        ; (r3) = current status
    orr     r4, r3, #0x80                   ; set interrupt disable bit
    msr     cpsr, r4                        ; update status register

	ldr     r4, [r0]
	cmp     r4, r2
	streq   r1, [r0]	
	mov     r0, r4

	msr     cpsr, r3                        ; restore status register

	ldmfd   sp!, {r4}           ; restore some register
	mov     pc, lr

	ENTRY_END  KL_InterlockedCompareExchange


	LEAF_ENTRY KL_InterlockedExchangeAdd
	;r0 = lpAdd to be
	;r1 = add value

    mrs     r2, cpsr                        ; (r3) = current status
    orr     r3, r2, #0x80                   ; set interrupt disable bit
    msr     cpsr, r3                        ; update status register

	ldr     r3, [r0]
	add     r1, r1, r3
	str     r1, [r0]	
	mov     r0, r3

	msr     cpsr, r2                        ; restore status register

	mov     pc, lr

	ENTRY_END  KL_InterlockedExchangeAdd


    LEAF_ENTRY KL_InterlockedIncrement
	;r0 = lpAdd to be
    ;stmfd   sp!, {r0-r3,lr}

    mrs     r1, cpsr                        ; (r1) = current status
    orr     r2, r1, #0x80                   ; set interrupt disable bit
    msr     cpsr, r2                        ; update status register

	ldr     r2, [r0]
	add     r3, r2, #1                        ; add 1
	str     r3, [r0]

	mov     r0,  r3                        ; return current value
	msr     cpsr, r1                        ; restore status register

	;ldmfd   sp!, {r0-r3, lr}           ; restore some register
	mov     pc, lr

	ENTRY_END KL_InterlockedIncrement


    LEAF_ENTRY KL_InterlockedDecrement
	;r0 = lpDec to be
    ;stmfd   sp!, {r0-r3,lr}

    mrs     r1, cpsr                        ; (r1) = current status
    orr     r2, r1, #0x80                   ; set interrupt disable bit
    msr     cpsr, r2                        ; update status register

	ldr     r2, [r0]
	sub     r3, r2, #1                      ; sub 1
	str     r3, [r0]

    mov     r0, r3                          ; return currnent value
	msr     cpsr, r1                        ; restore status register

	;ldmfd   sp!, {r0-r3, lr}           ; restore some register
	mov     pc, lr

	ENTRY_END KL_InterlockedDecrement

	LEAF_ENTRY LockIRQSave
	;r0 = address to save

    mrs     r1, cpsr                        ; (r1) = current status
    orr     r2, r1, #0x80                   ; set interrupt disable bit
    msr     cpsr, r2                        ; update status register
	str     r1, [r0]                        ; save the cpsr   
	mov     pc, lr

	ENTRY_END LockIRQSave

	LEAF_ENTRY UnlockIRQRestore
	;r0 = address to be restored

	ldr     r0, [r0]
    msr     cpsr, r0                        ; update status register
	mov     pc, lr

	ENTRY_END UnlockIRQRestore

	END
