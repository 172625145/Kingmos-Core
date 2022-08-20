/* ******************************************************
 * @copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
 * ******************************************************
 * 
 * ******************************************************
 * �ļ�˵��������Cache����
 * �汾�ţ�1.0.0
 * ����ʱ�ڣ�2003-04-04
 * ���ߣ��ܱ�
 * �޸ļ�¼��
 * *******************************************************
 */

.nolist
.include "linkage.inc"
.include "def.inc"
.list

 	.text

@ ********************************************************************
@������void TLBClear( void )
@��������
@����ֵ����
@����������������ݺ�ָ���TLB
@����: 
@ ********************************************************************

	ENTRY TLBClear

	mcr 	p15, 0, r0, c8, c7, 0	@ flush I+D TLBs
    mov pc, lr          @ return


@ ********************************************************************
@������void FlushICache(void)@
@��������
@����ֵ����
@����������ˢ�²�����Чָ��Cache
@����: 
@ ********************************************************************

	ENTRY	FlushICache

	mcr 	p15, 0, r0, c7, c5, 0	@ flush the icache
	mov	pc, lr			@ return


@ ********************************************************************
@������void FlushDCache(void)@
@��������
@����ֵ����
@����������ˢ�²�����Ч����Cache
@����: 
@ ********************************************************************

	ENTRY	FlushDCache
	
@Coprocessor cache control 
@Clean DCache
@

	stmfd   sp!, {r4,r5,lr}				@ save r4-r5, and lr
		
	mrc p15, 0, r1, c0, c0 ,1			@ Get cache information

	mov	r2, #7							@ 3 bit mask
	and	r3, r2, r1, lsr #18				@ get cache size
	and	r4, r2, r1, lsr #15				@ get cache associativity
	and	r5, r2, r1, lsr #12				@ get base and line length
	movs	r2, r5, lsr #2				@ get base (and set flags)
	and	r5, r5, #3						@ get line length

	@ calculate lsb of index field
	@
	@ 32 - cache associativity - base
	rsb	r1, r4, #32
	sub	r1, r1, r2

	@ calculate msb of segment field
	@
	@ 8 + cache size - cache associativity
	add	r2, r3, #8
	sub	r2, r2, r4

	@ calculate lsb of segment field
	@
	@ line length + 3
	add	r3, r5, #3

	@ calculate max value for index field
	rsb	r4, r1, #32
	mov	r5, #1
	mov	r4, r5, lsl r4
	subne	r4, r4, r4, lsr #2
	sub	r4, r4, #1

	@ calculate max value for segment field
	sub	r2, r2, r3
	add	r2, r2, #1
	mov	r2, r5, lsl r2
	sub	r2, r2, #1

	@ now finally clean the cache
1:
	mov	r5, r2
2:	
	mov	r0, r4, lsl r1
	orr	r0, r0, r5, lsl r3
	mcr	p15, 0, r0, c7, c10, 2				@ clean line by index (not address)
	subs   r5, r5, #1

	bge	2b
	subs	r4, r4, #1
	bge	1b

    mcr p15,0,r0,c7,c6,0					@ now, invalidate data cache

	ldmfd   sp!, {r4,r5,lr}					@ save r4-r5, and lr	
	mov     pc, lr
	

@ ********************************************************************
@������DWORD OEMARMCacheMode(void)@
@��������
@����ֵ����
@��������������Cache(C) and Buffer(B) bits
@����: 
@ ********************************************************************

	ENTRY OEMARMCacheMode

	mov	r0, #0x0000000C
    mov pc, lr          @ return



