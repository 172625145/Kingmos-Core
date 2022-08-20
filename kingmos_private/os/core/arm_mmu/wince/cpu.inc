; offset in tss struct

R_CPSR     EQU   0x0
R_PC       EQU   0x4

R_R0       EQU   0x8
R_R1       EQU   0xc
R_R2       EQU   0x10
R_R3       EQU   0x14
R_R4       EQU   0x18
R_R5       EQU   0x1c
R_R6       EQU   0x20
R_R7       EQU   0x24
R_R8       EQU   0x28
R_R9       EQU   0x2c
R_R10      EQU   0x30
R_R11      EQU   0x34
R_R12      EQU   0x38
R_R13      EQU   0x3c
R_SP       EQU   0x3c
R_R14      EQU   0x40
R_LR       EQU   0x40


; ARM CPU modes
USER_MODE   EQU 2_10000
FIQ_MODE    EQU 2_10001
IRQ_MODE    EQU 2_10010
SVC_MODE    EQU 2_10011
ABORT_MODE  EQU 2_10111
UNDEF_MODE  EQU 2_11011
SYSTEM_MODE EQU 2_11111

	END

