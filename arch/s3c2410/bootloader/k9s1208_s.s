
	.macro	LDR4STR1, src,tmp1,tmp2	
	ldrb	\tmp1,[\src]
	ldrb	\tmp2,[\src]
	orr	\tmp1,\tmp1,\tmp2,LSL #8
	ldrb	\tmp2,[\src]
	orr	\tmp1,\tmp1,\tmp2,LSL #16
	ldrb	\tmp2,[\src]
	orr	\tmp1,\tmp1,\tmp2,LSL #24
	.endm

@	AREA |C\\code|, CODE, READONLY
	
	.global __RdPage512	
__RdPage512:

	@input:a1(r0)=pPage
	stmfd	sp!,{r1-r11}

	ldr	r1,=0x4e00000c  @NFDATA
	mov	r2,#0x200
0:		
	LDR4STR1 r1,r4,r3
	LDR4STR1 r1,r5,r3
	LDR4STR1 r1,r6,r3
	LDR4STR1 r1,r7,r3
	LDR4STR1 r1,r8,r3
	LDR4STR1 r1,r9,r3
	LDR4STR1 r1,r10,r3
	LDR4STR1 r1,r11,r3
	stmia	r0!,{r4-r11}
	subs	r2,r2,#32
	bne	0b

	ldmfd	sp!,{r1-r11}
	mov	pc,lr
