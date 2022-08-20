#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   设置与平台相关的bsp 环境.
#
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


CMN_INCLUDES=$(__AMBO_OS_HOME)/include $(__AMBO_OS_HOME)/arch/s3c2410/include $(__AMBO_OS_HOME)/drivers/include $(__AMBO_OS_HOME)/kingmos_private/include


ifeq ($(PLAT_BOOTLOADER), 1)
CMN_ADEFINES += --defsym _BOOTLOADER=1
else
CMN_ADEFINES += --defsym _BOOTLOADER=0
endif


#CMN_ADEFINES += --defsym DEBUG_UART=3

CMN_CDEFINES += -DARM_CPU -DS3C2410 -DKINGMOS -DKINGMOS_CORE

