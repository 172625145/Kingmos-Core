

@#ifndef _S3C2410_REG_H
@#define _S3C2410_REG_H

@.equ GPIO_GPIO(Nb)  (0x00000001 << Nb)
@.equ GPIO_GPIO0      GPIO_GPIO (0)   /* GPIO  [0]                       */
@.equ GPIO_GPIO1      GPIO_GPIO (1)   /* GPIO  [1]                       */
@.equ GPIO_GPIO2      GPIO_GPIO (2)   /* GPIO  [2]                       */
@.equ GPIO_GPIO3      GPIO_GPIO (3)   /* GPIO  [3]                       */
@.equ GPIO_GPIO4      GPIO_GPIO (4)   /* GPIO  [4]                       */
@.equ GPIO_GPIO5      GPIO_GPIO (5)   /* GPIO  [5]                       */
@.equ GPIO_GPIO6      GPIO_GPIO (6)   /* GPIO  [6]                       */
@.equ GPIO_GPIO7      GPIO_GPIO (7)   /* GPIO  [7]                       */
@.equ GPIO_GPIO8      GPIO_GPIO (8)   /* GPIO  [8]                       */
@.equ GPIO_GPIO9      GPIO_GPIO (9)   /* GPIO  [9]                       */
@.equ GPIO_GPIO10     GPIO_GPIO (10)  /* GPIO [10]                       */
@.equ GPIO_GPIO11     GPIO_GPIO (11)  /* GPIO [11]                       */
@.equ GPIO_GPIO12     GPIO_GPIO (12)  /* GPIO [12]                       */
@.equ GPIO_GPIO13     GPIO_GPIO (13)  /* GPIO [13]                       */
@.equ GPIO_GPIO14     GPIO_GPIO (14)  /* GPIO [14]                       */
@.equ GPIO_GPIO15     GPIO_GPIO (15)  /* GPIO [15]                       */
@.equ GPIO_GPIO16     GPIO_GPIO (16)  /* GPIO [16]                       */
@.equ GPIO_GPIO17     GPIO_GPIO (17)  /* GPIO [17]                       */

@.equ GPIO_GPIO(Nb),  (0x00000001 << Nb)
.equ GPIO_GPIO0,         (0x00000001 << 0)
.equ GPIO_GPIO1,         (0x00000001 << 1)
.equ GPIO_GPIO2,         (0x00000001 << 2)
.equ GPIO_GPIO3,         (0x00000001 << 3)
.equ GPIO_GPIO4,         (0x00000001 << 4)
.equ GPIO_GPIO5,         (0x00000001 << 5)
.equ GPIO_GPIO6,         (0x00000001 << 6)
.equ GPIO_GPIO7,         (0x00000001 << 7)
.equ GPIO_GPIO8,         (0x00000001 << 8)
.equ GPIO_GPIO9,         (0x00000001 << 9)
.equ GPIO_GPIO10,         (0x00000001 << 10)
.equ GPIO_GPIO11,         (0x00000001 << 11)
.equ GPIO_GPIO12,         (0x00000001 << 12)
.equ GPIO_GPIO13,         (0x00000001 << 13)
.equ GPIO_GPIO14,         (0x00000001 << 14)
.equ GPIO_GPIO15,         (0x00000001 << 15)
.equ GPIO_GPIO16,         (0x00000001 << 16)
.equ GPIO_GPIO17,         (0x00000001 << 17)


@.equ INTMSK             0x4A000008
@.equ INTSUBMSK	   0x4A00001C
@.equ WTCON              0x53000000
@.equ LOCKTIME           0x4C000000
@.equ MPLLCON            0x4C000004
@.equ UPLLCON            0x4C000008
@.equ CLKDIVN            0x4C000014
@.equ BWSCON             0x48000000
@.equ REFRESH            0x48000024
@.equ BANKSIZE	   0x48000028
@.equ GPFCON   	   0x56000050
@.equ GPFDAT		   0x56000054
@.equ GPFUP		   0x56000058

.equ INTMSK,             0x4A000008
.equ INTSUBMSK,	   0x4A00001C
.equ WTCON     ,         0x53000000
.equ LOCKTIME   ,        0x4C000000
.equ MPLLCON     ,       0x4C000004
.equ UPLLCON    ,        0x4C000008
.equ CLKDIVN    ,        0x4C000014
.equ BWSCON     ,        0x48000000
.equ REFRESH    ,        0x48000024
.equ BANKSIZE	,   0x48000028
.equ GPFCON   	,   0x56000050
.equ GPFDAT		,   0x56000054
.equ GPFUP		,   0x56000058


.equ MISCCR,             0x56000080
.equ SCK_NORMAL,         (0x7<<17)

.equ GSTATUS0   ,        0x560000AC
.equ GSTATUS1   ,        0x560000B0
.equ GSTATUS2   ,        0x560000B4
.equ GSTATUS3   ,        0x560000B8
.equ GSTATUS4   ,        0x560000BC    

.equ PWRST       ,      GPIO_GPIO0
.equ OFFRST      ,      GPIO_GPIO1
.equ WDRST       ,      GPIO_GPIO2
.equ POWEROFFLED1,	  (0x2<<4)
.equ POWEROFFLED2,  	  (0x4<<4)
.equ POWEROFFLED3,  	  (0x8<<4)

@.equ FCLK_SPEED 1
@#if FCLK_SPEED==0   /*  FCLK=203Mhz, Fin=12Mhz for AUDIO        */
@        .equ M_MDIV,          0xc3
@        .equ M_PDIV,          0x4
@        .equ M_SDIV,          0x1
@#elif FCLK_SPEED==1        /* FCLK = 202.8Mhz */
        .equ M_MDIV,          0x5c
        .equ M_PDIV,          0x4
        .equ M_SDIV,          0x2
@#endif


@#endif
