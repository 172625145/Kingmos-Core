@===========================================
@ NAME: OPTION.A
@ DESC: Configuration options for .S files
@ HISTORY:
@ 02.28.2002: ver 0.0
@===========================================

@Start address of each stacks,
_STACK_BASEADDRESS	EQU 0x33ff8000  
_MMUTT_STARTADDRESS	EQU 0x33ff8000  
_ISR_STARTADDRESS	EQU 0x33ffff00

		GBLL 	PLL_ON_START  
PLL_ON_START	SETL 	{TRUE}


		GBLL	ENDIAN_CHANGE
ENDIAN_CHANGE	SETL	{FALSE}

		GBLA	ENTRY_BUS_WIDTH
ENTRY_BUS_WIDTH	SETA	16	


@BUSWIDTH = 16,32
		GBLA    BUSWIDTH	@max. bus width for the GPIO configuration
BUSWIDTH	SETA    32


		GBLA	FCLK
FCLK		SETA	50000000

	[ FCLK = 50000000	
M_MDIV		EQU	0x5c	@Fin=12.0MHz Fout=50.0MHz
M_PDIV		EQU	0x4
M_SDIV		EQU	0x2

U_MDIV		EQU	 40	@Fin=12.0MHz Fout=48.0MHz
U_PDIV		EQU	 1
U_SDIV		EQU	 2
    	]

	END
