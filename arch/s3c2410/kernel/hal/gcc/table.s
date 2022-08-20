	.globl	OEMAddressTable
	.type	OEMAddressTable, #object

OEMAddressTable:	
    @-------------------------------------------------------------
    @     Virt Addr   Phys Addr   MB
    @-------------------------------------------------------------
    .word 0x80000000, 0x02000000, 30  @ 30 MB SROM(SRAM/ROM) BANK 0
    .word 0x82000000, 0x08000000, 32  @ 32 MB SROM(SRAM/ROM) BANK 1
    .word 0x84000000, 0x10000000, 32  @ 32 MB SROM(SRAM/ROM) BANK 2
    .word 0x86000000, 0x18000000, 32  @ 32 MB SROM(SRAM/ROM) BANK 3
    .word 0x88000000, 0x20000000, 32  @ 32 MB SROM(SRAM/ROM) BANK 4
    .word 0x8A000000, 0x28000000, 32  @ 32 MB SROM(SRAM/ROM) BANK 5
    .word 0x8C000000, 0x30000000, 64  @ 64 MB DRAM BANK 0,1
    .word 0x90800000, 0x48000000,  1  @ Memory control register
    .word 0x90900000, 0x49000000,  1  @ USB Host register
    .word 0x90A00000, 0x4A000000,  1  @ Interrupt Control register
    .word 0x90B00000, 0x4B000000,  1  @ DMA control register
    .word 0x90C00000, 0x4C000000,  1  @ Clock & Power register
    .word 0x90D00000, 0x4D000000,  1  @ LCD control register
    .word 0x90E00000, 0x4E000000,  1  @ NAND flash control register
    .word 0x91000000, 0x50000000,  1  @ UART control register
    .word 0x91100000, 0x51000000,  1  @ PWM timer register
    .word 0x91200000, 0x52000000,  1  @ USB device register
    .word 0x91300000, 0x53000000,  1  @ Watchdog Timer register
    .word 0x91400000, 0x54000000,  1  @ IIC control register
    .word 0x91500000, 0x55000000,  1  @ IIS control register
    .word 0x91600000, 0x56000000,  1  @ I/O Port register
    .word 0x91700000, 0x57000000,  1  @ RTC control register
    .word 0x91800000, 0x58000000,  1  @ A/D convert register
    .word 0x91900000, 0x59000000,  1  @ SPI register
    .word 0x91A00000, 0x5A000000,  1  @ SD Interface register
	.word 0x92000000, 0x00000000,  2  @  2 MB SROM(SRAM/ROM) BANK 0
    .word 0x00000000, 0x00000000,  0  @ End of Table (MB MUST BE ZERO!)

    .size	OEMAddressTable, . - OEMAddressTable

