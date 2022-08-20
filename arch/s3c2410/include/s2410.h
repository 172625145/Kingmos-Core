/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2001. Samsung Electronics, co. ltd  All rights reserved.

Module Name:  

Abstract:

    ARM920(S3C2410) definitions

rev:
	2002.5.20	: add define 	CE_MAJOR_VER == 0x0004 for CE.Net    by pcj ( bestworld@samsung.com )
	
	2002.5.16 	: change OEM_CLOCK_FREQ value for Audio ( bestworld@samsung.com )
	2002.4.3	: S3C2410 Support (SOC)

	2002.1.29	: change Timer values
	2002.1.29	: CE.NET port
	2002.1.22	: Add USBD definitions (kwangyoon LEE, kwangyoon@samsung.com)

Notes: 
--*/

#define BOARD_TYPE		1	// 1 : Aiji System, 2 : Meritech

#define ARM920

// Board timer constants.
//
#define S2410FCLK       	(203 * 1000 * 1000)		// 203MHz (FCLK).
//#define S2410FCLK       	(180 * 1000 * 1000)		// 180MHz (FCLK).
#define PCLKDIV         	4						// P-clock (PCLK) divisor.
#define S2410PCLK       	(S2410FCLK / PCLKDIV)	// PCLK.
#define S2410UCLK       	48000000				// 48MHz - for serial UARTs.

#define PRESCALER			24 // 200
#define D1_2				0x0
#define D1_4				0x1
#define D1_8				0x2
#define D1_16				0x3
#define D2					2
#define D4					4
#define D8					8
#define D16					16

#define SYS_TIMER_DIVIDER	D4 //D2 // D4
#define OEM_CLOCK_FREQ      (S2410PCLK / (PRESCALER+1) / SYS_TIMER_DIVIDER)
#define OEM_COUNT_1MS       (OEM_CLOCK_FREQ / 1000)				// Timer count for 1ms.
#define RESCHED_PERIOD      1 									// Reschedule period in ms.
#define RESCHED_INCREMENT   (RESCHED_PERIOD * OEM_COUNT_1MS)	// Number of ticks per reschedule period.


// Define LCD type of S3C2400X01
#define STN8BPP     1
#define TFT16BPP    2

#define LCDTYPE     TFT16BPP   // define LCD type as upper definition.


#define AUDIO_CODEC_CLOCK  384
#if (S2410FCLK == 112800000)
#define AUDIO_CODEC_CLOCK  256
#else
#define AUDIO_CODEC_CLOCK  384
#endif


// by pcj for ce.net power management.
#define CE_MAJOR_VER  0x0004


//=======================================================================
// Define S3C2410 Special Registers 
//=======================================================================
#ifndef __2410X_H__
#define __2410X_H__

#define CS8900DBG_IOBASE				(0xa7000300)
#define CS8900DBG_MEMBASE				(0xa6000000)
#define CS8900DBG_IP					((165 << 0) | (213 << 8) | (206 << 16) | (216 << 24))
//#define CS8900DBG_IP					((165 << 0) | (213 << 8) | (172 << 16) | (102 << 24))
#define CS8900DBG_MASK					((255 << 0) | (255 << 8) | (255 << 16) | (0   << 24))

#define CS8900DBG_MAC0					0x22
#define CS8900DBG_MAC1					0x33
#define CS8900DBG_MAC2					0x44
#define CS8900DBG_MAC3					0x55
#define CS8900DBG_MAC4					0x66
#define CS8900DBG_MAC5					0x77

#define CS8900DBG_USHORT(l, h)			(l | (h << 8))

/*
 * Registers : NAND Controller
 */

#define NAND_BASE      0xB0E00000   /* 0x4E000000           */
#define NFC_BASE       0x90E00000      //  0x4E000000
#define NFC_BASE_PHYSICAL 0x4E000000
typedef struct  
{
    unsigned int  rNFCONF;          /* 0x00                 */
    unsigned char rNFCMD;           /* 0x04                 */
    unsigned char d0[3];            
    unsigned char rNFADDR;          /* 0x08                 */
    unsigned char d1[3];            
    unsigned char rNFDATA;          /* 0x0c                 */
    unsigned char d2[3];
    unsigned int  rNFSTAT;          /* 0x10                 */
    unsigned char rNFECC0;          /* 0x14                 */
    unsigned char rNFECC1;          /* 0x15                 */
    unsigned char rNFECC2;          /* 0x16                 */
} NANDreg;    

//
// Memory Controller Register
//

#define MEMCTRL_BASE    0xB0800000 // 0x49000000

typedef struct  {
    unsigned long       rBWSCON;    // 0
    unsigned long       rBANKCON0;  // 4
    unsigned long       rBANKCON1;  // 8
    unsigned long       rBANKCON2;  // c
    unsigned long       rBANKCON3;  // 10
    unsigned long       rBANKCON4;  // 1c
    unsigned long       rBANKCON5;  // 18
    unsigned long       rBANKCON6;  // 1c
    unsigned long       rBANKCON7;  // 20
    unsigned long       rREFRESH;   // 24
    unsigned long       rBANKSIZE;  // 28
    unsigned long       rMRSRB6;    // 2c
    unsigned long       rMRSRB7;     // 30
}MEMreg;


//
// Clock & Power Management Special Register

#define CLKPWR_BASE    0xB0C00000 // 0x4C000000

typedef struct {
    unsigned long       rLOCKTIME;
    unsigned long       rMPLLCON;
    unsigned long       rUPLLCON;
    unsigned long       rCLKCON;
    unsigned long       rCLKSLOW;
    unsigned long       rCLKDIVN;
}CLKPWRreg;



// Make sure this matches entry in config.bib
// These buffs are now offset via a constant
#define DMA_BUFFER_BASE			0xAC000000
#define DMA_PHYSICAL_BASE		0x30000000  // S3C2410X01

#define AUDIO_DMA_BUFFER_BASE		(DMA_BUFFER_BASE + 0x00002000)
#define AUDIO_DMA_BUFFER_PHYS		(DMA_PHYSICAL_BASE + 0x00002000)

#define SDI_DMA_BUFFER_BASE			(DMA_BUFFER_BASE + 0x00028000)
#define SDI_DMA_BUFFER_PHYS			(DMA_PHYSICAL_BASE + 0x00028000)

#define FRAMEBUF_BASE				(DMA_BUFFER_BASE + 0x00100000)
#define FRAMEBUF_DMA_BASE			(DMA_PHYSICAL_BASE + 0x00100000)


//
// DMA Register
//


#define DMA_BASE    0xB0B00000 // 0x4B0000000

typedef struct {
        unsigned int    rDISRC0;        // 00
        unsigned int	rDISRCC0;		// 04
        unsigned int    rDIDST0;        // 08
        unsigned int 	rDIDSTC0;		// 0C
        unsigned int    rDCON0;         // 10
        unsigned int    rDSTAT0;        // 14
        unsigned int    rDCSRC0;        // 18
        unsigned int    rDCDST0;        // 1C
        unsigned int    rDMASKTRIG0;    // 20
        unsigned int	rPAD1[7];		// 24 - 3C

        unsigned int    rDISRC1;        // 40
        unsigned int	rDISRCC1;		// 44
        unsigned int    rDIDST1;        // 48
        unsigned int 	rDIDSTC1;		// 4C
        unsigned int    rDCON1;         // 50
        unsigned int    rDSTAT1;        // 54
        unsigned int    rDCSRC1;        // 58
        unsigned int    rDCDST1;        // 5C
        unsigned int    rDMASKTRIG1;    // 60
        unsigned int	rPAD2[7];		// 64 - 7C

        unsigned int    rDISRC2;        // 80
        unsigned int	rDISRCC2;		// 84
        unsigned int    rDIDST2;        // 88
        unsigned int 	rDIDSTC2;		// 8C
        unsigned int    rDCON2;         // 90
        unsigned int    rDSTAT2;        // 94
        unsigned int    rDCSRC2;        // 98
        unsigned int    rDCDST2;        // 9C
        unsigned int    rDMASKTRIG2;    // A0
        unsigned int	rPAD3[7];		// A4 - BC

        unsigned int    rDISRC3;        // C0
        unsigned int	rDISRCC3;		// C4
        unsigned int    rDIDST3;        // C8
        unsigned int 	rDIDSTC3;		// CC
        unsigned int    rDCON3;         // D0
        unsigned int    rDSTAT3;        // D4
        unsigned int    rDCSRC3;        // D8
        unsigned int    rDCDST3;        // DC
        unsigned int    rDMASKTRIG3;    // E0
 }DMAreg;


//
// Registers : I/O port
//

#define IOP_BASE      0xB1600000 // 0x56000000
typedef struct  {
    unsigned int  rGPACON;		// 00
    unsigned int  rGPADAT;
    unsigned int  rPAD1[2];
    
    unsigned int  rGPBCON;		// 10
    unsigned int  rGPBDAT;
    unsigned int  rGPBUP;
    unsigned int  rPAD2;
    
    unsigned int  rGPCCON;		// 20
    unsigned int  rGPCDAT;
    unsigned int  rGPCUP;
    unsigned int  rPAD3;
    
    unsigned int  rGPDCON;		// 30
    unsigned int  rGPDDAT;
    unsigned int  rGPDUP; 
    unsigned int  rPAD4;
    
    unsigned int  rGPECON;		// 40
    unsigned int  rGPEDAT;
    unsigned int  rGPEUP;
    unsigned int  rPAD5;
    
    unsigned int  rGPFCON;		// 50
    unsigned int  rGPFDAT;
    unsigned int  rGPFUP; 
    unsigned int  rPAD6;
    
    unsigned int  rGPGCON;		// 60
    unsigned int  rGPGDAT;
    unsigned int  rGPGUP; 
    unsigned int  rPAD7;
    
    unsigned int  rGPHCON;		// 70
    unsigned int  rGPHDAT;
    unsigned int  rGPHUP; 
    unsigned int  rPAD8;
    
    unsigned int  rMISCCR;		// 80
    unsigned int  rDCKCON;		
    unsigned int  rEXTINT0;
    unsigned int  rEXTINT1;		
    unsigned int  rEXTINT2;		// 90
	unsigned int  rEINTFLT0;
	unsigned int  rEINTFLT1;
	unsigned int  rEINTFLT2;
	unsigned int  rEINTFLT3;		// A0
	unsigned int  rEINTMASK;
	unsigned int  rEINTPEND;
	unsigned int  rGSTATUS0;		// AC
	unsigned int  rGSTATUS1;		// B0
	unsigned int  rGSTATUS2;		// B4
	unsigned int  rGSTATUS3;		// B8
	unsigned int  rGSTATUS4;		// BC
	
}IOPreg;  
 

//
// Registers : PWM
//

#define PWM_BASE      0xB1100000 // 0x51000000
typedef struct  {
    unsigned int  rTCFG0;
    unsigned int  rTCFG1;
    unsigned int  rTCON;
    unsigned int  rTCNTB0;
    unsigned int  rTCMPB0;
    unsigned int  rTCNTO0;
    unsigned int  rTCNTB1;
    unsigned int  rTCMPB1;
    unsigned int  rTCNTO1;
    unsigned int  rTCNTB2;
    unsigned int  rTCMPB2;
    unsigned int  rTCNTO2;
    unsigned int  rTCNTB3;
    unsigned int  rTCMPB3;
    unsigned int  rTCNTO3;
    unsigned int  rTCNTB4;
    unsigned int  rTCNTO4;
}PWMreg ;



//
// Registers : UART
//

#define UART0_BASE      0xB1000000 // 0x50000000
#define UART1_BASE      0xB1004000
#define UART2_BASE      0xB1008000

typedef struct  {
    unsigned int  rULCON;
    unsigned int  rUCON;
    unsigned int  rUFCON;
    unsigned int  rUMCON;
    unsigned int  rUTRSTAT;
    unsigned int  rUERSTAT;
    unsigned int  rUFSTAT;
    unsigned int  rUMSTAT;
    unsigned int  rUTXH;
    unsigned int  rURXH;
    unsigned int  rUBRDIV;
}UART0reg, UART1reg, UART2reg, UARTreg, S2410_UART_REG, *PS2410_UART_REG;


// 2410 USB DEVICE Function (Written by Seung-han, Lim)
// Little-Endian	

struct udcFARBits {				// function address reg
	BYTE func_addr		:7;		// function_address
	BYTE addr_up		:1;		// addr_update
};

struct PMRBits {				// power management reg
	BYTE sus_en		:1;		// suspend_en
	BYTE sus_mo		:1;		// suspend_mode
	BYTE muc_res		:1;		// mcu_resume
	BYTE usb_re		:1;		// usb_reset
	BYTE rsvd1		:3;		
	BYTE iso_up		:1;		// iso_update
};

struct EIRBits {				// ep interrupt reg
	BYTE ep0_int		:1;		// ep0_interrupt
	BYTE ep1_int		:1;		// ep1_interrupt
	BYTE ep2_int		:1;		// ep2_interrupt
	BYTE ep3_int		:1;		// ep3_interrupt
	BYTE ep4_int		:1;		// ep4_interrupt
	BYTE rsvd0		:3;
};

struct UIRBits {				// usb interrupt reg
	BYTE sus_int		:1;		// suspend inaterrupt
	BYTE resume_int	:1;			// resume interrupt
	BYTE reset_int		:1;		// reset interrupt
	BYTE rsvd0		:5;
};

struct EIERBits {				// interrupt mask reg
	BYTE ep0_int_en	:1;			// ep1_int_reg
	BYTE ep1_int_en	:1;			// ep1_int_reg
	BYTE ep2_int_en	:1;			// ep2_int_reg
	BYTE ep3_int_en	:1;			// ep3_int_reg
	BYTE ep4_int_en	:1;			// ep4_int_reg
	BYTE rsvd0		:3;
};

struct UIERBits {
	BYTE sus_int_en	:1;			// suspend_int_en
	BYTE rsvd1		:1;		
	BYTE reset_int_en	:1;		// reset_enable_reg
	BYTE rsvd0		:5;
};

struct FNR1Bits {				// frame number1 register
	BYTE fr_n1		:8;		// frame_num1_reg
};

struct FNR2Bits {				// frame number2 register
	BYTE fr_n2		:8;		// frame_num2_reg
};

struct INDEXBits {				// index register
	BYTE index		:8;		// index_reg
};

struct EP0ICSR1Bits				// EP0 & ICSR1 shared
{
	BYTE opr_ipr		:1;
	BYTE ipr_		:1;
	BYTE sts_ur		:1;
	BYTE de_ff		:1;
	BYTE se_sds		:1;
	BYTE sds_sts		:1;
	BYTE sopr_cdt		:1;
	BYTE sse_		:1;
};

struct ICSR2Bits {				// in csr2 areg
	BYTE rsvd1		:4;
	BYTE in_dma_int_en	:1;		// in_dma_int_en
	BYTE mode_in		:1;		// mode_in
	BYTE iso		:1; 		// iso/bulk mode
	BYTE auto_set		:1;		// auto_set
};

struct OCSR1Bits {				// out csr1 reg
	BYTE out_pkt_rdy	:1;		// out packet reday
	BYTE rsvd1		:1;
	BYTE ov_run		:1;		// over_run
	BYTE data_error	:1;			// data_error
	BYTE fifo_flush	:1;			// fifo_flush
	BYTE send_stall	:1;			// send_stall
	BYTE sent_stall	:1; 			// sent_stall
	BYTE clr_data_tog	:1;		// clear data toggle
};

struct OCSR2Bits {				// out csr2 reg
	BYTE rsvd1		:5;
	BYTE out_dma_int_en	:1;		// out_dma_int_en
	BYTE iso		:1;		// iso/bulk mode
	BYTE auto_clr		:1;		// auto_clr
};

struct EP0FBits {				// ep0 fifo reg
	BYTE fifo_data		:8;		// fifo data
};

struct EP1FBits {				// ep0 fifo reg
	BYTE fifo_data		:8;		// fifo data
};

struct EP2FBits {				// ep0 fifo reg
	BYTE fifo_data		:8;		// fifo data
};

struct EP3FBits {				// ep0 fifo reg
	BYTE fifo_data		:8;		// fifo data
};

struct EP4FBits {				// ep0 fifo reg
	BYTE fifo_data		:8;		// fifo data
};

struct MAXPBits {
	BYTE maxp		:4;		// max packet reg
	BYTE rsvd0		:4;
};

struct OFCR1Bits {				// out_fifo_cnt1_reg
	BYTE out_cnt_low	:8;		// out_cnt_low
};

struct OFCR2Bits {				// out_fifo_cnt2_reg
	BYTE out_cnt_high	:8;		// out_cnt_high
};

struct EP1DCBits {				// ep1 dma interface control
	BYTE dma_mo_en		:1;		// dma_mode_en
	BYTE in_dma_run		:1;		// in_dma_run
	BYTE orb_odr		:1;		// out_run_ob/out_dma_run
	BYTE demand_mo		:1;		// demand_mode
	BYTE state		:3;		// state
	BYTE in_run_ob		:1;		// in_run_ob
};

struct EP2DCBits {				// ep2 dma interface control
	BYTE dma_mo_en		:1;		// dma_mode_en
	BYTE in_dma_run		:1;		// in_dma_run
	BYTE orb_odr		:1;		// out_run_ob/out_dma_run
	BYTE demand_mo		:1;		// demand_mode
	BYTE state		:3;		// state
	BYTE in_run_ob		:1;		// in_run_ob
};

struct EP3DCBits {				// ep3 dma interface control
	BYTE dma_mo_en		:1;		// dma_mode_en
	BYTE in_dma_run		:1;		// in_dma_run
	BYTE orb_odr		:1;		// out_run_ob/out_dma_run
	BYTE demand_mo		:1;		// demand_mode
	BYTE state		:3;		// state
	BYTE in_run_ob		:1;		// in_run_ob
};

struct EP4DCBits {				// ep4 dma interface control
	BYTE dma_mo_en		:1;		// dma_mode_en
	BYTE in_dma_run		:1;		// in_dma_run
	BYTE orb_odr		:1;		// out_run_ob/out_dma_run
	BYTE demand_mo		:1;		// demand_mode
	BYTE state		:3;		// state
	BYTE in_run_ob		:1;		// in_run_ob
};

struct EP1DUBits {
	BYTE ep1_unit_cnt	:8;		// ep0_unit_cnt
};

struct EP2DUBits {
	BYTE ep2_unit_cnt	:8;		// ep0_unit_cnt
};

struct EP3DUBits {
	BYTE ep3_unit_cnt	:8;		// ep0_unit_cnt
};

struct EP4DUBits {
	BYTE ep4_unit_cnt	:8;		// ep0_unit_cnt
};

struct EP1DFBits {
	BYTE ep1_fifo_cnt	:8;
};

struct EP2DFBits {
	BYTE ep2_fifo_cnt	:8;
};

struct EP3DFBits {
	BYTE ep3_fifo_cnt	:8;
};

struct EP4DFBits {
	BYTE ep4_fifo_cnt	:8;
};

struct EP1DTLBits {
	BYTE ep1_ttl_l		:8;
};

struct EP1DTMBits {
	BYTE ep1_ttl_m		:8;
};

struct EP1DTHBits {
	BYTE ep1_ttl_h		:8;
};

struct EP2DTLBits {
	BYTE ep2_ttl_l		:8;
};

struct EP2DTMBits {
	BYTE ep2_ttl_m		:8;
};

struct EP2DTHBits {
	BYTE ep2_ttl_h		:8;
};

struct EP3DTLBits {
	BYTE ep3_ttl_l		:8;
};

struct EP3DTMBits {
	BYTE ep3_ttl_m		:8;
};

struct EP3DTHBits {
	BYTE ep3_ttl_h		:8;
};

struct EP4DTLBits {
	BYTE ep4_ttl_l		:8;
};

struct EP4DTMBits {
	BYTE ep4_ttl_m		:8;
};

struct EP4DTHBits {
	BYTE ep4_ttl_h		:8;
};

struct udcreg {						// PHY BASE : 0x52000140(Little Endian)
	struct udcFARBits	udcFAR;			// 0x140
	BYTE			rsvd0;				// 0x141
	BYTE			rsvd1;				// 0x142
	BYTE			rsvd2;				// 0x143
	struct PMRBits		PMR;			// 0x144
	BYTE			rsvd3;				// 0x145
	BYTE			rsvd4;				// 0x146
	BYTE			rsvd5;				// 0x147
	struct EIRBits		EIR;				// 0x148
	BYTE			rsvd6;				// 0x149
	BYTE			rsvd7;				// 0x14a
	BYTE			rsvd8;				// 0x14b
	BYTE			rsvd9;				// 0x14C
	BYTE			rsvd10;				// 0x14d
	BYTE			rsvd11;				// 0x14e
	BYTE			rsvd12;				// 0x14f

	BYTE			rsvd13;				// 0x150
	BYTE			rsvd14;				// 0x151
	BYTE			rsvd15;				// 0x152
	BYTE			rsvd16;				// 0x153
	BYTE			rsvd17;				// 0x154
	BYTE			rsvd18;				// 0x155
	BYTE			rsvd19;				// 0x156
	BYTE			rsvd20;				// 0x157
	struct UIRBits		UIR;				// 0x158
	BYTE			rsvd21;				// 0x159
	BYTE			rsvd22;				// 0x15a
	BYTE			rsvd23;				// 0x15b
	struct EIERBits		EIER;			// 0x15C
	BYTE			rsvd24;				// 0x15d
	BYTE			rsvd25;				// 0x15e
	BYTE			rsvd26;				// 0x15f

	BYTE			rsvd27;				// 0x160
	BYTE			rsvd28;				// 0x161
	BYTE			rsvd29;				// 0x162
	BYTE			rsvd30;				// 0x163
	BYTE			rsvd31;				// 0x164
	BYTE			rsvd32;				// 0x165
	BYTE			rsvd33;				// 0x166
	BYTE			rsvd34;				// 0x167
	BYTE			rsvd35;				// 0x168
	BYTE			rsvd36;				// 0x169
	BYTE			rsvd37;				// 0x16a
	BYTE			rsvd38;			// 0x16b
	struct UIERBits		UIER;				// 0x16c
	BYTE			rsvd39;				// 0x16d
	BYTE			rsvd40;				// 0x16e
	BYTE			rsvd41;				// 0x16f

	struct FNR1Bits		FNR1;			// 0x170
	BYTE			rsvd42;				// 0x171
	BYTE			rsvd43;				// 0x172
	BYTE			rsvd44;				// 0x173
	struct FNR2Bits		FNR2;			// 0x174
	BYTE			rsvd45;				// 0x175
	BYTE			rsvd46;				// 0x176
	BYTE			rsvd47;				// 0x177
	struct INDEXBits	INDEX;				// 0x178
	BYTE			rsvd48;			// 0x179
	BYTE			rsvd49;				// 0x17a
	BYTE			rsvd50;				// 0x17b
	BYTE			rsvd51;				// 0x17C
	BYTE			rsvd52;				// 0x17d
	BYTE			rsvd53;				// 0x17e
	BYTE			rsvd54;				// 0x17f

	struct MAXPBits		MAXP;			// 0x180
	BYTE			rsvd56;				// 0x181
	BYTE			rsvd57;				// 0x182
	BYTE			rsvd58;				// 0x183
	struct EP0ICSR1Bits	EP0ICSR1;		// 0x184
	// struct ICSR1Bits	ICSR1;			// 0x184
	// struct EP0CSRBits	EP0CSR;			// mapped to the in_csr1 reg 
	BYTE			rsvd59;				// 0x185
	BYTE			rsvd60;				// 0x186
	BYTE			rsvd61;				// 0x187
	struct ICSR2Bits	ICSR2;			// 0x188
	BYTE			rsvd63;				// 0x189
	BYTE			rsvd64;				// 0x18a
	BYTE			rsvd65;				// 0x18b
	BYTE			rsvd55;				// 0x180
	BYTE			rsvd66;				// 0x18d
	BYTE			rsvd67;				// 0x18e
	BYTE			rsvd68;				// 0x18f

	struct OCSR1Bits	OCSR1;			// 0x190
	BYTE			rsvd69;				// 0x191
	BYTE			rsvd70;				// 0x192
	BYTE			rsvd71;				// 0x193
	struct OCSR2Bits	OCSR2;			// 0x194
	BYTE			rsvd73;				// 0x195
	BYTE			rsvd74;				// 0x196
	BYTE			rsvd75;				// 0x197
	struct OFCR1Bits	OFCR1;			// 0x198
	BYTE			rsvd76;				// 0x199
	BYTE			rsvd77;				// 0x19a
	BYTE			rsvd78;				// 0x19b
	struct OFCR2Bits	OFCR2;			// 0x19C
	BYTE			rsvd79;				// 0x19d
	BYTE			rsvd80;				// 0x19e
	BYTE			rsvd81;				// 0x19f

	BYTE			rsvd82;				// 0x1A0	
	BYTE			rsvd83;				// 0x1a1
	BYTE			rsvd84;				// 0x1a2
	BYTE			rsvd85;				// 0x1a3
	BYTE			rsvd86;				// 0x1A4
	BYTE			rsvd87;				// 0x1a5
	BYTE			rsvd88;				// 0x1a6
	BYTE			rsvd89;				// 0x1a7
	BYTE			rsvd90;				// 0x1A8
	BYTE			rsvd91;				// 0x1a9
	BYTE			rsvd92;				// 0x1aa
	BYTE			rsvd93;				// 0x1ab
	BYTE			rsvd94;				// 0x1AC
	BYTE			rsvd95;				// 0x1ad
	BYTE			rsvd96;				// 0x1ae
	BYTE			rsvd97;				// 0x1af

	BYTE			rsvd98;				// 0x1B0
	BYTE			rsvd99;				// 0x1B1
	BYTE			rsvd100;				// 0x1B2
	BYTE			rsvd101;				// 0x1B3
	BYTE			rsvd102;				// 0x1B4
	BYTE			rsvd103;				// 0x1B5
	BYTE			rsvd104;				// 0x1B6
	BYTE			rsvd105;				// 0x1B7
	BYTE			rsvd106;				// 0x1B8
	BYTE			rsvd107;				// 0x1B9
	BYTE			rsvd108;				// 0x1Ba
	BYTE			rsvd109;				// 0x1Bb
	BYTE			rsvd110;				// 0x1BC
	BYTE			rsvd111;				// 0x1Bd
	BYTE			rsvd112;				// 0x1Be
	BYTE			rsvd113;				// 0x1Bf

	struct EP0FBits		EP0F;				// 0x1C0
	BYTE			rsvd114;				// 0x1c1
	BYTE			rsvd115;				// 0x1c2
	BYTE			rsvd116;				// 0x1c3
	struct EP1FBits		EP1F;				// 0x1C4
	BYTE			rsvd117;				// 0x1c5
	BYTE			rsvd118;				// 0x1c6
	BYTE			rsvd119;				// 0x1c7
	struct EP2FBits		EP2F;				// 0x1C8
	BYTE			rsvd120;				// 0x1c9
	BYTE			rsvd121;				// 0x1ca
	BYTE			rsvd122;				// 0x1cb
	struct EP3FBits		EP3F;				// 0x1CC
	BYTE			rsvd123;				// 0x1cd
	BYTE			rsvd124;				// 0x1ce
	BYTE			rsvd125;				// 0x1cf

	struct EP4FBits		EP4F;				// 0x1D0
	BYTE			rsvd126;				// 0x1d1
	BYTE			rsvd127;				// 0x1d2
	BYTE			rsvd128;				// 0x1d3
	BYTE			rsvd169;				// 0x1D4
	BYTE			rsvd170;				// 0x1d5
	BYTE			rsvd171;				// 0x1d6
	BYTE			rsvd172;				// 0x1d7
	BYTE			rsvd173;				// 0x1D8
	BYTE			rsvd174;				// 0x1d9
	BYTE			rsvd175;				// 0x1da
	BYTE			rsvd176;				// 0x1db
	BYTE			rsvd177;				// 0x1DC
	BYTE			rsvd178;				// 0x1dd
	BYTE			rsvd179;				// 0x1de
	BYTE			rsvd180;				// 0x1df

	BYTE			rsvd1e0;				// 0x1e0
	BYTE			rsvd1e1;				// 0x1e1
	BYTE			rsvd1e2;				// 0x1e2
	BYTE			rsvd1e3;				// 0x1e3
	BYTE			rsvd1e4;				// 0x1e4
	BYTE			rsvd1e5;				// 0x1e5
	BYTE			rsvd1e6;				// 0x1e6
	BYTE			rsvd1e7;				// 0x1e7
	BYTE			rsvd1e8;				// 0x1e8
	BYTE			rsvd1e9;				// 0x1e9
	BYTE			rsvd1ea;				// 0x1ea
	BYTE			rsvd1eb;				// 0x1eb
	BYTE			rsvd1ec;				// 0x1ec
	BYTE			rsvd1ed;				// 0x1ed
	BYTE			rsvd1ee;				// 0x1ee
	BYTE			rsvd1ef;				// 0x1ef

	BYTE			rsvd181;				// 0x1F0
	BYTE			rsvd182;				// 0x1F1
	BYTE			rsvd183;				// 0x1F2
	BYTE			rsvd184;				// 0x1F3
	BYTE			rsvd185;				// 0x1F4
	BYTE			rsvd186;				// 0x1F5
	BYTE			rsvd187;				// 0x1F6
	BYTE			rsvd188;				// 0x1F7
	BYTE			rsvd189;				// 0x1F8
	BYTE			rsvd190;				// 0x1F9
	BYTE			rsvd191;				// 0x1Fa
	BYTE			rsvd192;				// 0x1Fb
	BYTE			rsvd193;				// 0x1FC
	BYTE			rsvd194;				// 0x1Fd
	BYTE			rsvd195;				// 0x1Fe
	BYTE			rsvd196;				// 0x1Ff

	struct EP1DCBits	EP1DC;				// 0x200
	BYTE			rsvd197;				// 0x201
	BYTE			rsvd198;				// 0x202
	BYTE			rsvd199;				// 0x203
	struct EP1DUBits	EP1DU;				// 0x204
	BYTE			rsvd200;				// 0x205
	BYTE			rsvd201;				// 0x206
	BYTE			rsvd202;				// 0x207
	struct EP1DFBits	EP1DF;				// 0x208
	BYTE			rsvd203;				// 0x209
	BYTE			rsvd204;				// 0x20a
	BYTE			rsvd205;				// 0x20b
	struct EP1DTLBits	EP1DTL;				// 0x20C
	BYTE			rsvd206;				// 0x20d
	BYTE			rsvd207;				// 0x20e
	BYTE			rsvd208;				// 0x20f

	struct EP1DTMBits	EP1DTM;				// 0x210
	BYTE			rsvd209;				// 0x211
	BYTE			rsvd210;				// 0x212
	BYTE			rsvd211;				// 0x213
	struct EP1DTHBits	EP1DTH;				// 0x214
	BYTE			rsvd212;				// 0x215
	BYTE			rsvd213;				// 0x216
	BYTE			rsvd214;				// 0x217
	struct EP2DCBits	EP2DC;				// 0x218
	BYTE			rsvd215;				// 0x219
	BYTE			rsvd216;				// 0x21a
	BYTE			rsvd217;				// 0x21b
	struct EP2DUBits	EP2DU;				// 0x21C
	BYTE			rsvd218;				// 0x21d
	BYTE			rsvd219;				// 0x21e
	BYTE			rsvd220;				// 0x21f

	struct EP2DFBits	EP2DF;				// 0x220
	BYTE			rsvd221;				// 0x221
	BYTE			rsvd222;				// 0x222
	BYTE			rsvd223;				// 0x223
	struct EP2DTLBits	EP2DTL;				// 0x224
	BYTE			rsvd224;				// 0x225
	BYTE			rsvd225;				// 0x226
	BYTE			rsvd226;				// 0x227
	struct EP2DTMBits	EP2DTM;				// 0x228
	BYTE			rsvd227;				// 0x229
	BYTE			rsvd228;				// 0x22a
	BYTE			rsvd229;				// 0x22b
	struct EP2DTHBits	EP2DTH;				// 0x22C
	BYTE			rsvd230;				// 0x22d
	BYTE			rsvd231;				// 0x22e
	BYTE			rsvd232;				// 0x22f

	BYTE			rsvd233;				// 0x230
	BYTE			rsvd234;				// 0x231
	BYTE			rsvd235;				// 0x232
	BYTE			rsvd236;				// 0x233
	BYTE			rsvd237;				// 0x234
	BYTE			rsvd238;				// 0x235
	BYTE			rsvd239;				// 0x236
	BYTE			rsvd240;				// 0x237
	BYTE			rsvd241;				// 0x238
	BYTE			rsvd242;				// 0x239
	BYTE			rsvd243;				// 0x23a
	BYTE			rsvd244;				// 0x23b
	BYTE			rsvd245;				// 0x23C
	BYTE			rsvd246;				// 0x23d
	BYTE			rsvd247;				// 0x23e
	BYTE			rsvd248;				// 0x23f

	struct EP3DCBits	EP3DC;				// 0x240
	BYTE			rsvd249;				// 0x241
	BYTE			rsvd250;				// 0x242
	BYTE			rsvd251;				// 0x243
	struct EP3DUBits	EP3DU;				// 0x244
	BYTE			rsvd252;				// 0x245
	BYTE			rsvd253;				// 0x246
	BYTE			rsvd254;				// 0x247
	struct EP3DFBits	EP3DF;				// 0x248
	BYTE			rsvd255;				// 0x249
	BYTE			rsvd256;				// 0x24a
	BYTE			rsvd257;				// 0x24b
	struct EP3DTLBits	EP3DTL;				// 0x24C
	BYTE			rsvd258;				// 0x24d
	BYTE			rsvd259;				// 0x24e
	BYTE			rsvd260;				// 0x24f

	struct EP3DTMBits	EP3DTM;				// 0x250
	BYTE			rsvd261;				// 0x251
	BYTE			rsvd262;				// 0x252
	BYTE			rsvd263;				// 0x253
	struct EP3DTHBits	EP3DTH;				// 0x254
	BYTE			rsvd264;				// 0x255
	BYTE			rsvd265;				// 0x256
	BYTE			rsvd266;				// 0x257
	struct EP4DCBits	EP4DC;				// 0x258
	BYTE			rsvd267;				// 0x259
	BYTE			rsvd268;				// 0x25a
	BYTE			rsvd269;				// 0x25b
	struct EP4DUBits	EP4DU;				// 0x25C
	BYTE			rsvd270;				// 0x25d
	BYTE			rsvd271;				// 0x25e
	BYTE			rsvd272;				// 0x25f

	struct EP4DFBits	EP4DF;				// 0x260
	BYTE			rsvd273;				// 0x261
	BYTE			rsvd274;				// 0x262
	BYTE			rsvd275;				// 0x263
	struct EP4DTLBits	EP4DTL;				// 0x264
	BYTE			rsvd276;				// 0x265
	BYTE			rsvd277;				// 0x266
	BYTE			rsvd278;				// 0x267
	struct EP4DTMBits	EP4DTM;				// 0x268
	BYTE			rsvd279;				// 0x269
	BYTE			rsvd280;				// 0x26a
	BYTE			rsvd281;				// 0x26b
	struct EP4DTHBits	EP4DTH;				// 0x26C
};





//
// Registers : Interrupt Controller
//

#define INT_BASE      0xB0A00000 // 0x4A000000
typedef struct  {
    unsigned int  rSRCPND;
    unsigned int  rINTMOD;
    unsigned int  rINTMSK;
    unsigned int  rPRIORITY;
    unsigned int  rINTPND;
    unsigned int  rINTOFFSET;
    unsigned int  rSUBSRCPND;
    unsigned int  rINTSUBMSK;
}INTreg ;    


// S3C2410X01 Interrupt controller bit positions

#define    BIT_EINT0        (0x1)
#define    BIT_EINT1        (0x1<<1)
#define    BIT_EINT2        (0x1<<2)
#define    BIT_EINT3        (0x1<<3)
#define    BIT_EINT4_7      (0x1<<4)
#define    BIT_EINT8_23     (0x1<<5)
#define    BIT_RSV1         (0x1<<6)
#define    BIT_BAT_FLT      (0x1<<7)
#define    BIT_TICK         (0x1<<8)
#define    BIT_WDT          (0x1<<9)
#define    BIT_TIMER0       (0x1<<10)
#define    BIT_TIMER1       (0x1<<11)
#define    BIT_TIMER2       (0x1<<12)
#define    BIT_TIMER3       (0x1<<13)
#define    BIT_TIMER4       (0x1<<14)
#define    BIT_UART2        (0x1<<15)
#define    BIT_LCD          (0x1<<16)
#define    BIT_DMA0         (0x1<<17)
#define    BIT_DMA1         (0x1<<18)
#define    BIT_DMA2         (0x1<<19)
#define    BIT_DMA3         (0x1<<20)
#define    BIT_MMC	        (0x1<<21)
#define    BIT_SPI0	        (0x1<<22)
#define    BIT_UART1        (0x1<<23)
#define    BIT_RSV2         (0x1<<24)
#define    BIT_USBD         (0x1<<25)
#define    BIT_USBH         (0x1<<26)
#define    BIT_IIC	        (0x1<<27)
#define    BIT_UART0        (0x1<<28)
#define    BIT_SPI1         (0x1<<29)
#define    BIT_RTC	        (0x1<<30)
#define    BIT_ADC	        (0x1<<31)
#define    BIT_ALLMSK       (0xffffffff)

#define	   BIT_SUB_ADC		(0x1<<10)
#define    BIT_SUB_TC		(0x1<<9)
#define    BIT_SUB_ERR2		(0x1<<8)
#define    BIT_SUB_TXD2		(0x1<<7)
#define    BIT_SUB_RXD2		(0x1<<6)
#define    BIT_SUB_ERR1		(0x1<<5)
#define    BIT_SUB_TXD1		(0x1<<4)
#define    BIT_SUB_RXD1		(0x1<<3)
#define    BIT_SUB_ERR0		(0x1<<2)
#define    BIT_SUB_TXD0		(0x1<<1)
#define    BIT_SUB_RXD0		(0x1<<0)
#define    BIT_SUB_ALLMSK	(0x7ff)


// S3C2410X01 Interrupt controller source number
#define    INTSRC_EINT0     0
#define    INTSRC_EINT1     1
#define    INTSRC_EINT2     2
#define    INTSRC_EINT3     3
#define    INTSRC_EINT4_7   4
#define    INTSRC_EINT8_23  5
#define    INTSRC_RSV1      6
#define    INTSRC_BAT_FLT   7
#define    INTSRC_TICK      8
#define    INTSRC_WDT       9
#define    INTSRC_TIMER0    10
#define    INTSRC_TIMER1    11
#define    INTSRC_TIMER2    12
#define    INTSRC_TIMER3    13
#define    INTSRC_TIMER4    14
#define    INTSRC_UART2     15
#define    INTSRC_LCD       16
#define    INTSRC_DMA0      17
#define    INTSRC_DMA1      18
#define    INTSRC_DMA2      19
#define    INTSRC_DMA3      20
#define    INTSRC_MMC	    21
#define    INTSRC_SPI0	    22
#define    INTSRC_UART1     23
#define    INTSRC_RSV2      24
#define    INTSRC_USBD      25
#define    INTSRC_USBH      26
#define    INTSRC_IIC	    27
#define    INTSRC_UART0     28
#define    INTSRC_SPI1      29
#define    INTSRC_RTC	    30
#define    INTSRC_ADC	    31
#define    INTSRC_ALLMSK    (0xffffffff)

// S3C2410X01 Interrupt controller bit positions
// For SUB source pending bit.

#define 	INTSUB_RXD0		(0x1 << 0)
#define		INTSUB_TXD0		(0x1 << 1)
#define		INTSUB_ERR0		(0x1 << 2)
#define		INTSUB_RXD1		(0x1 << 3)
#define 	INTSUB_TXD1		(0x1 << 4)
#define		INTSUB_ERR1		(0x1 << 5)
#define		INTSUB_RXD2		(0x1 << 6)
#define 	INTSUB_TXD2		(0x1 << 7)
#define		INTSUB_ERR2		(0x1 << 8)
#define		INTSUB_TC		(0x1 << 9)
#define		INTSUB_ADC		(0x1 << 10)
#define 	INTSUB_SLLMSK	(0xFFFFFFFF)

#define		INTSUB_BASE     0xB0A00018 // 0x4A000018 // serial
#define		INTSUB_MSK      0xB0A0001C // 0x4A00001C // serial
#define		BIT_SUB_ALLMSK	(0x7ff)



//
// Registers : LCD Controller
//

#define LCD_BASE      0xB0D00000 // 0x4D000000
typedef struct  {
    unsigned int  rLCDCON1;		// 00
    unsigned int  rLCDCON2;		// 04
    unsigned int  rLCDCON3;		// 08
    unsigned int  rLCDCON4;		// 0C
    unsigned int  rLCDCON5;		// 10
    unsigned int  rLCDSADDR1;	// 14
    unsigned int  rLCDSADDR2;	// 18
    unsigned int  rLCDSADDR3;	// 1C
    unsigned int  rREDLUT;		// 20
    unsigned int  rGREENLUT;	// 24
    unsigned int  rBLUELUT;		// 28
    unsigned int  rPAD[8];		// 2C - 48
    unsigned int  rDITHMODE;	// 4C
    unsigned int  rTPAL;		// 50
    unsigned int  rLCDINTPND;	// 54
    unsigned int  rLCDSRCPND;	// 58
    unsigned int  rLCDINTMSK;	// 5C	
    unsigned int  rLPCSEL;		// 60
}LCDreg ;    

// LCD register value...    
#define MODE_STN_1BIT 	    (1)
#define MODE_STN_2BIT  	    (2)
#define MODE_STN_4BIT  	    (4)
#define MODE_CSTN_8BIT 	    (108)
#define MODE_CSTN_12BIT     (112)
#define MODE_TFT_1BIT       (201)
//#define MODE_TFT_2BIT	    (202)
//#define MODE_TFT_4BIT     (204)
#define MODE_TFT_8BIT       (208)
#define MODE_TFT_16BIT      (216)

#define SCR_XSIZE           (640)   //for virtual screen  
#define SCR_YSIZE           (480)
//#define SCR_XSIZE_TFT       (1280)   //for virtual screen  
//#define SCR_YSIZE_TFT       (960)
#define SCR_XSIZE_TFT       (480)   //for virtual screen  
#define SCR_YSIZE_TFT       (640)


#define LCD_XSIZE_STN       (320)
#define LCD_YSIZE_STN       (240)
#define LCD_XSIZE_CSTN      (320)
#define LCD_YSIZE_CSTN      (240)
//#define LCD_XSIZE_STN       (240)
//#define LCD_YSIZE_STN       (320)
//#define LCD_XSIZE_CSTN      (240)
//#define LCD_YSIZE_CSTN      (320)
//#define LCD_XSIZE_TFT       (640)	
//#define LCD_YSIZE_TFT       (480)
#define LCD_XSIZE_TFT       (240)	
#define LCD_YSIZE_TFT       (320)


#define ARRAY_SIZE_STN_1BIT     (SCR_XSIZE/8*SCR_YSIZE)
#define ARRAY_SIZE_STN_2BIT     (SCR_XSIZE/4*SCR_YSIZE)
#define ARRAY_SIZE_STN_4BIT     (SCR_XSIZE/2*SCR_YSIZE)
#define ARRAY_SIZE_CSTN_8BIT    (SCR_XSIZE/1*SCR_YSIZE)
#define ARRAY_SIZE_CSTN_12BIT   (SCR_XSIZE*2*SCR_YSIZE)
#define ARRAY_SIZE_TFT_8BIT     (SCR_XSIZE/1*SCR_YSIZE)
#define ARRAY_SIZE_TFT_16BIT    (SCR_XSIZE*2*SCR_YSIZE)

#define HOZVAL_STN          (LCD_XSIZE_STN/4-1)
#define HOZVAL_CSTN         (LCD_XSIZE_CSTN*3/8-1)
#define HOZVAL_TFT          (LCD_XSIZE_TFT-1)
#define LINEVAL_STN         (LCD_YSIZE_STN-1)
#define LINEVAL_CSTN        (LCD_YSIZE_CSTN-1)
#define LINEVAL_TFT         (LCD_YSIZE_TFT-1)

#define MVAL                (13)
#define MVAL_USED           (0)

//STN/CSTN timing parameter for LCBHBT161M(NANYA)
#define WLH                 (3)
#define WDLY                (3)
#define LINEBLANK           (1 &0xff)

//TFT timing parameter for V16C6448AB(PRIME VIEW) 
/*
#define VBPD                ((33-1)&0xff)
#define VFPD                ((10-1)&0xff)
#define VSPW                ((2-1) &0x3f)

#define HBPD                ((48-1)&0x7f)
#define HFPD                ((16-1)&0xff)
#define HSPW                ((96-1)&0xff)
*/
#define VBPD                ((1)&0xff)
#define VFPD                ((2)&0xff)
#define VSPW                ((1) &0x3f)

#define HBPD                ((6)&0x7f)
// 
#define HFPD                ((2)&0xff)
//#define HSPW                ((3)&0xff)
#define HSPW                ((4)&0xff)

#define CLKVAL_STN_MONO     (22) 	
    //69.14hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=4 
#define CLKVAL_STN_GRAY     (12) 	
    //124hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=4  
#define CLKVAL_CSTN         (8) 	
    //135hz @60Mhz,WLH=16clk,WDLY=16clk,LINEBLANK=1*8,VD=8  
//#define CLKVAL_TFT          (1)
#define CLKVAL_TFT          (6)
//#define CLKVAL_TFT          (7)
    //NOTE: 1)SDRAM should have 32-bit bus width. 
    //      2)HBPD,HFPD,HSPW should be optimized. 
    //44.6hz @75Mhz
    //VSYNC,HSYNC should be inverted
    //HBPD=48VCLK,HFPD=16VCLK,HSPW=96VCLK
    //VBPD=33HSYNC,VFPD=10HSYNC,VSPW=2HSYNC

#define M5D(n)              ((n) & 0x1fffff)



//
// ADC
//

#define ADC_BASE      0xB1800000 // 0x58000000

typedef struct {
        unsigned int 	rADCCON;
        unsigned int 	rADCTSC;
        unsigned int	rADCDLY;
        unsigned int 	rADCDAT0;
        unsigned int 	rADCDAT1;
}ADCreg ;        


//
// Registers : RTC
//



#define RTC_BASE      0xB1700000 // 0x57000000

typedef struct {
	unsigned int  rPAD1[16]; 	// 00 - 3C
    unsigned int  rRTCCON;		// 40
    unsigned int  rTICINT;
    unsigned int  rPAD2[2];      
    unsigned int  rRTCALM;
    unsigned int  rALMSEC;
    unsigned int  rALMMIN;
    unsigned int  rALMHOUR;
    unsigned int  rALMDAY;
    unsigned int  rALMMON;
    unsigned int  rALMYEAR;
    unsigned int  rRTCRST;
    unsigned int  rBCDSEC;
    unsigned int  rBCDMIN;
    unsigned int  rBCDHOUR;
    unsigned int  rBCDDAY;
    unsigned int  rBCDDATE;
    unsigned int  rBCDMON;
    unsigned int  rBCDYEAR;
} RTCreg;    

#if 0 // _BIG_ENDIAN
#define rRTCCON		(*(volatile unsigned char *)0xB1700043)
#define rRTCALM		(*(volatile unsigned char *)0xB1700053)
#define rALMSEC		(*(volatile unsigned char *)0xB1700057)
#define rALMMIN		(*(volatile unsigned char *)0xB170005b)
#define rALMHOUR	(*(volatile unsigned char *)0xB170005f)
#define rALMDAY		(*(volatile unsigned char *)0xB1700063)
#define rALMMON		(*(volatile unsigned char *)0xB1700067)
#define rALMYEAR	(*(volatile unsigned char *)0xB170006b)
#define rRTCRST		(*(volatile unsigned char *)0xB170006f)
#define rBCDSEC		(*(volatile unsigned char *)0xB1700073)
#define rBCDMIN		(*(volatile unsigned char *)0xB1700077)
#define rBCDHOUR	(*(volatile unsigned char *)0xB170007b)
#define rBCDDAY		(*(volatile unsigned char *)0xB170007f)
#define rBCDDATE	(*(volatile unsigned char *)0xB1700083)
#define rBCDMON		(*(volatile unsigned char *)0xB1700087)
#define rBCDYEAR	(*(volatile unsigned char *)0xB170008b)
#define rTICINT		(*(volatile unsigned char *)0xB1700047)

#endif


//
// Watch-Dog Timver
//

#define WATCH_BASE 0xB1300000 // 0x53000000

typedef struct {
    unsigned long   rWTCON;
    unsigned long   rWTDAT;
    unsigned long   rWTCNT;
} WATCHreg;

    
//
// SD / MMC 
//

#define MMC_BACE        0xB1A00000 // 0x5A000000

typedef struct {
    unsigned int   	rSDICON;
    unsigned int   	rSDIPRE;
    unsigned int   	rSDICMDARG;
    unsigned int   	rSDICMDCON;
    unsigned int   	rSDICMDSTA;
    unsigned int   	rSDIRSP0;
    unsigned int   	rSDIRSP1;
    unsigned int   	rSDIRSP2;
    unsigned int   	rSDIRSP3;
    unsigned int 	rSDIDTIMER;
    unsigned int	rSDIBSIZE;
    unsigned int 	rSDIDATCON;
    unsigned int	rSDIDATCNT;
    unsigned int	rSDIDATSTA;
    unsigned int	rSDIFSTA;
    unsigned int	rSDIDAT;
    unsigned int	rSDIINTMSK;
} MMCreg;




//
// IIC 
//


#define IIC_BASE        0xB1400000 // 54000000

#define IICFIF_PHYS     0x54000000  // physical address of IIC

typedef struct {
        unsigned int    rIICCON;
        unsigned int    rIICSTAT;
        unsigned int    rIICADD;
        unsigned int    rIICDS;
}IICreg;        



//
// IIS 
//


#define IIS_BASE        0xB1500000

#define IISFIF_PHYS     0x55000010  // physical address of IISFIF for DMA

typedef struct {
        unsigned int    rIISCON;
        unsigned int    rIISMOD;
        unsigned int    rIISPSR;
        unsigned int    rIISFCON;
        unsigned int    rIISFIF;
}IISreg;        


//
// SPI 
//

#define SSP_BASE 0xB1900000  // 0x59000000

typedef struct  {
    unsigned int rSPCON0; 	// 00
    unsigned int rSPSTA0;
    unsigned int rSPPIN0;
    unsigned int rSPPRE0;
    unsigned int rSPTDAT0;	// 10
    unsigned int rSPRDAT0;
    unsigned int rPAD[2];
    unsigned int rSPCON1; 	// 20
    unsigned int rSPSTA1;
    unsigned int rSPPIN1;
    unsigned int rSPPRE1;
    unsigned int rSPTDAT1; 	// 30
    unsigned int rSPRDAT1;
    
}SSPreg ; 

typedef struct {
    USHORT XSample;     //@field X Coordinate.
    USHORT YSample;     //@field Y Coordinate.
} TOUCHPANEL_POINT_SAMPLE, *PTOUCHPANEL_POINT_SAMPLE;

#define TOUCH_PEN_DOWN 0
#define TOUCH_PEN_UP 1
#define TOUCH_PEN_SAMPLE 2

#define READ_REGISTER_ULONG(reg) (*(volatile unsigned long * const)(reg))
#define WRITE_REGISTER_ULONG(reg, val) (*(volatile unsigned long * const)(reg)) = (val)
#define READ_REGISTER_USHORT(reg) (*(volatile unsigned short * const)(reg))
#define WRITE_REGISTER_USHORT(reg, val) (*(volatile unsigned short * const)(reg)) = (val)
#define READ_REGISTER_UCHAR(reg) (*(volatile unsigned char * const)(reg))
#define WRITE_REGISTER_UCHAR(reg, val) (*(volatile unsigned char * const)(reg)) = (val)


#endif


