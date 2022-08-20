@BWSCON

.equ DW8,				0x0
.equ DW16,				0x1
.equ DW32,				0x2
.equ WAIT,			(0x1<<2)
.equ UBLB,			(0x1<<3)



@BUSWIDTH=32
.equ B1_BWSCON,	 (DW32)
.equ B2_BWSCON,	 (DW16)
.equ B3_BWSCON,	 (DW16)
.equ B4_BWSCON,	 (DW16)
.equ B5_BWSCON,	 (DW16)
.equ B6_BWSCON,	 (DW32)
.equ B7_BWSCON,	 (DW32)

@BANK0CON 

.equ B0_Tacs,			0x0	@0clk
.equ B0_Tcos,			0x0	@0clk
.equ B0_Tacc,			0x7	@14clk
.equ B0_Tcoh,			0x0	@0clk
.equ B0_Tah,			0x0	@0clk
.equ B0_Tacp,			0x0	
.equ B0_PMC,			0x0	@normal


@BANK1CON

.equ B1_Tacs,			0x0	@0clk
.equ B1_Tcos,			0x0	@0clk
.equ B1_Tacc,			0x7	@14clk
.equ B1_Tcoh,			0x0	@0clk
.equ B1_Tah	,		0x0	@0clk
.equ B1_Tacp,			0x0	
.equ B1_PMC	,		0x0	@normal


@Bank 2 parameter

.equ B2_Tacs,			0x0	@0clk
.equ B2_Tcos,			0x0	@0clk
.equ B2_Tacc,			0x7	@14clk
.equ B2_Tcoh,			0x0	@0clk
.equ B2_Tah	,		0x0	@0clk
.equ B2_Tacp,			0x0	
.equ B2_PMC	,		0x0	@normal


@Bank 3 parameter
.equ B3_Tacs,			0x0	@0clk
.equ B3_Tcos,			0x0	@0clk
.equ B3_Tacc,			0x7	@14clk
.equ B3_Tcoh,			0x0	@0clk
.equ B3_Tah	,		0x0	@0clk
.equ B3_Tacp,			0x0	
.equ B3_PMC	,		0x0	@normal


@Bank 4 parameter

.equ B4_Tacs,			0x0	@0clk
.equ B4_Tcos,			0x0	@0clk
.equ B4_Tacc,			0x7	@14clk
.equ B4_Tcoh,			0x0	@0clk
.equ B4_Tah	,		0x0	@0clk
.equ B4_Tacp,			0x0	
.equ B4_PMC	,		0x0	@normal


@Bank 5 parameter

.equ B5_Tacs,			0x0	@0clk
.equ B5_Tcos,			0x0	@0clk
.equ B5_Tacc,			0x7	@14clk
.equ B5_Tcoh,			0x0	@0clk
.equ B5_Tah	,		0x0	@0clk
.equ B5_Tacp,			0x0	
.equ B5_PMC	,		0x0	@normal


@Bank 6 parameter
.equ B6_MT,			0x3	@SDRAM
@B6_Trcd			0x0	@2clk
.equ B6_Trcd,			0x1	@3clk
.equ B6_SCAN,			0x1	@9bit

@Bank 7 parameter
.equ B7_MT,			0x3	@SDRAM
@B7_Trcd			0x0	@2clk
.equ B7_Trcd,			0x1	@3clk
.equ B7_SCAN,			0x1	@9bit

@REFRESH parameter

.equ REFEN,			0x1	@Refresh enable
.equ TREFMD,			0x0	@CBR(CAS before RAS)/Auto refresh
.equ Trp	,		0x0	@2clk
.equ Trc	,		0x3	@7clk

		
.equ Tchr	,		0x2	@3clk
.equ REFCNT,			1113	@period=15.6us, HCLK=60Mhz, (2048+1-15.6*60)


@#endif

