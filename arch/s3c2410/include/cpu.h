#ifndef __CPU_H
#define __CPU_H

#define TIMER_PERIOD 1
#define OEM_CLOCK_FREQ (3686400)       // ticks/s
#define TIMER_COUNTDOWN 3686 // Inc to next 1ms tick (3.6864x10^6tick/sec * 1/1000sec/msec)
// Timer count value for 1 ms
#define OEM_COUNT_1MS (OEM_CLOCK_FREQ / 1000)
#define RESCHED_PERIOD (1)	// Reschedule period in ms
// Timer count value for rescheduler period (3686 for 1 ms system tick)
#define RESCHED_INCREMENT ((RESCHED_PERIOD * OEM_CLOCK_FREQ) / 1000)


#define R_CPSR  0x0
#define R_PC  0x4
#define R_R0  0x8
#define R_R1  0xc
#define R_R2  0x10
#define R_R3  0x14
#define R_R4  0x18
#define R_R5  0x1c
#define R_R6  0x20
#define R_R7  0x24
#define R_R8  0x28
#define R_R9  0x2c
#define R_R10 0x30
#define R_R11 0x34
#define R_R12 0x38
#define R_R13 0x3c
#define R_R14 0x40

typedef struct _TSS{
	UINT cpsr;
	UINT pc;   // 
	UINT r0;
	UINT r1;
	UINT r2;
	UINT r3;
	UINT r4;
	UINT r5;
	UINT r6;
	UINT r7;
	UINT r8;
	UINT r9;
	UINT r10;
	UINT r11;
	UINT r12;
	UINT r13;   //sp
	UINT r14;   //lr
}TSS;

#define INIT_TSS  { 0x00000013, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

void LockIRQSave( UINT * lpSave );
void UnlockIRQRestore( UINT * lpRestore );


#define PAGE_SIZE (4*1024)
#define PAGE_MASK (4*1024-1)
#define PAGE_SHIFT 12
#define PAGE_ALIGN_DOWN( dwAdr ) ( (dwAdr) & (~PAGE_MASK) )
#define PAGE_ALIGN_UP( dwAdr ) ( ( (dwAdr) + PAGE_MASK ) & (~PAGE_MASK) )
#define PAGE_TABLE_SIZE 1024

#define NEXT_PHYSICAL_PAGE( lpAdr ) ( ( (DWORD)lpAdr ) + PAGE_SIZE )

#define UNCACHE_TO_CACHE( lpAdr ) ( ((DWORD)lpAdr) - 0xA0000000 + 0x80000000 )
#define CACHE_TO_UNCACHE( lpAdr ) ( ((DWORD)lpAdr) - 0x80000000 + 0xA0000000 )
extern void FlushCacheAndClearTLB( void );

extern void INTR_ON( void );
extern void INTR_OFF( void );


#endif //__CPU_H




