@====================================================================
@ File Name : option.h
@ Function  : S3C2410 
@ Program   : Shin, On Pil (SOP)
@ Date      : March 4, 2002
@ Version   : 0.0
@ History
@   0.0 : Programming start (February 20,2002) -> SOP
@====================================================================

@#ifndef __OPTION_H__
@#define __OPTION_H__

.equ FCLK, 200000000
.equ HCLK, FCLK/2
.equ PCLK, HCLK/2

@ BUSWIDTH : 16,32
.equ BUSWIDTH,    (32)

@64MB
@ 0x30000000 ~ 0x30ffffff : Download Area (16MB) Cacheable
@ 0x31000000 ~ 0x33feffff : Non-Cacheable Area
@ 0x33ff0000 ~ 0x33ff47ff : Heap & RW Area
@ 0x33ff4800 ~ 0x33ff7fff : FIQ ~ User Stack Area
@ 0x33ff8000 ~ 0x33fffeff : Not Useed Area
@ 0x33ffff00 ~ 0x33ffffff : Exception & ISR Vector Table

.equ _RAM_STARTADDRESS, 	0x30000000
.equ _ISR_STARTADDRESS, 	0x33ffff00     
.equ _MMUTT_STARTADDRESS,	0x33ff8000
.equ _STACK_BASEADDRESS,  0x33ff8000
.equ HEAPEND		  ,		0x33ff0000

.equ STACK_SIZE        ,      0x00000400 /*  size of C stack */
.equ STACK_BASE        ,      0x32000000        

.equ U_MDIV,	 40	@Fin=12.0MHz Fout=48.0MHz
.equ U_PDIV		,	 1
.equ U_SDIV		,	 2


@#endif /*__OPTION_H__*/
