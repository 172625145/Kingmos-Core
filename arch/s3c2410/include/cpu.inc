/* ******************************************************
 * ;copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
 * ******************************************************
 */

@ offset in tss struct

.equ R_CPSR      ,    0x0
.equ R_PC        ,    0x4

.equ R_R0        ,    0x8
.equ R_R1        ,    0xc
.equ R_R2        ,    0x10
.equ R_R3        ,    0x14
.equ R_R4        ,    0x18
.equ R_R5        ,    0x1c
.equ R_R6        ,    0x20
.equ R_R7        ,    0x24
.equ R_R8        ,    0x28
.equ R_R9        ,    0x2c
.equ R_R10       ,    0x30
.equ R_R11       ,    0x34
.equ R_R12       ,    0x38
.equ R_R13       ,    0x3c
.equ R_SP        ,    0x3c
.equ R_R14       ,    0x40
.equ R_LR        ,    0x40


@ ARM CPU modes

.equ USER_MODE    ,   0x10        @2_10000
.equ FIQ_MODE     ,   0x11        @2_10001
.equ IRQ_MODE     ,   0x12        @2_10010
.equ SVC_MODE     ,   0x13        @2_10011
.equ ABORT_MODE   ,   0x17        @2_10111
.equ UNDEF_MODE   ,   0x1b        @2_11011
.equ SYSTEM_MODE  ,   0x1f        @2_11111

