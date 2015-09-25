## use this section with the energyicro stk board
#BOARD = efm32gg-stk3700
#PARTNUMBER = EFM32GG990F1024
#FAMILYNUMBER = EFM32GG

# use this section with the olimex board
BOARD = olimex-efm32g880f128-h
PARTNUMBER = EFM32G880F128
FAMILYNUMBER = EFM32G

BUILDDIR = build


##############################################################
#################### libraries involved ######################
##############################################################

# enc28j60 board driver and driver

BOARD_OBJS = board.o
CFLAGS += -I ../../efm32/boards/${BOARD}/
vpath %.c ../../efm32/boards/${BOARD}/

DRIVER_OBJS = enc28j60.o enchw.o rtc-efm32rtc.o log.o log-itm.o
vpath %.c ../../enc28j60driver ../../efm32/enchw
CFLAGS += -DENC28J60_USE_PBUF # configure the enc28j60 backend to build functions that involve lwip buffer mgmt
CFLAGS += -I../../enc28j60driver -I../../efm32/enchw


# lwip

# use this with lwip 1.5 development or later
#LWIP_OBJS = etharp.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o udp.o dhcp.o init.o def.o timers.o dns.o inet_chksum.o err.o icmp.o ip_frag.o ip4_addr.o ip4.o
# use this with lwip 1.4.1
LWIP_OBJS = etharp.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o udp.o dhcp.o init.o def.o timers.o dns.o inet_chksum.o err.o icmp.o ip_frag.o ip_addr.o ip.o
# -I. for lwipopts.h
CFLAGS += -I./lwip/src/include/ipv4 -I./lwip/src/include/ipv6 -I./lwip/src/include -I.
vpath %.c lwip/src/netif lwip/src/core lwip/src/api lwip/src/core/ipv4


# enc28j60 lwip infrastructure

NETIF_OBJS = mchdrv.o
CFLAGS += -I../../lwip
vpath %.c ../../lwip/netif


familynumber = $(shell echo $(FAMILYNUMBER) | tr A-Z a-z)

CMSISPATH = ./mbed/libraries/mbed/targets/cmsis/
EMPATH_CODE = mbed/libraries/mbed/targets/hal/TARGET_Silicon_Labs/TARGET_EFM32
EMPATH_CMSIS = mbed/libraries/mbed/targets/cmsis/TARGET_Silicon_Labs/TARGET_EFM32/TARGET_${FAMILYNUMBER}_STK3700/
EMPATH_LD = ${EMPATH_CMSIS}/TOOLCHAIN_GCC_ARM/

# hardware configuration -- these constants do the dispatching in em_device.h
CFLAGS += -D${PARTNUMBER} -D${FAMILYNUMBER}

OBJS += em_cmu.o em_gpio.o em_usart.o em_rtc.o em_emu.o system_${familynumber}.o
CFLAGS += -I${EMPATH_CODE}/emlib/inc/ -I${EMPATH_CMSIS} -I${CMSISPATH}
vpath %.c ${EMPATH_CODE}/emlib/src/ ${EMPATH_CMSIS}

OBJS += startup_${familynumber}.o
vpath %.S ${EMPATH_LD}

# linker script from energymicro
LDFLAGS += -T${familynumber}.ld -L ${EMPATH_LD}


# use newlib as the libc

CFLAGS += -I/usr/include/newlib/
LDFLAGS += -L/usr/lib/arm-none-eabi/newlib/armv7-m/
LDFLAGS += -B/usr/lib/arm-none-eabi/newlib/armv7-m/
LDFLAGS += -lc -specs=nosys.specs


# the main application

MY_OBJS = mch_main.o testapp.o


##############################################################
###################### compiler setup ########################
##############################################################


OBJS += ${BOARD_OBJS} ${MY_OBJS} ${DRIVER_OBJS} ${NETIF_OBJS} ${LWIP_OBJS}

# architecture specific cflags
CFLAGS += -mcpu=cortex-m3 -mthumb -msoft-float
LDFLAGS += -mthumb -msoft-float -mcpu=cortex-m3

# proper compiler behavior
CFLAGS += -pedantic -Wall -Wextra -std=gnu99 -fno-common

# optimizations and debuggability
CFLAGS += -Os -g3
LDFLAGS += -Wl,--gc-sections


CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc

# the explicit rule (a copy of the default rule plus "build/") seems to be the
# easiest way to get make to output to a subdirectory. vpath is required to
# make the implicit rule happy that links a binary from an object of the same
# name. using the opportunity to mkdir too.
# 
# if you don't want to do this, drop all references to BUILDDIR and things are
# automagic
./${BUILDDIR}/%.o: %.c
	@mkdir -p ./${BUILDDIR}/
	$(COMPILE.c) $(OUTPUT_OPTION) $<
./${BUILDDIR}/%.o: %.S
	@mkdir -p ./${BUILDDIR}/
	$(COMPILE.S) $(OUTPUT_OPTION) $<


BUILDOBJS = $(addprefix ./${BUILDDIR}/,${OBJS})

# name does not matter, but needs to be in objects for default rule to apply
ELFFILE = ./${BUILDDIR}/mch_main
${ELFFILE}: ${BUILDOBJS}

# enable dependencies
-include $(BUILDOBJS:%.o=%.d)
CFLAGS += -MD -MP

all: $(ELFFILE)

upload: ${ELFFILE}
	arm-none-eabi-gdb -batch -x ./openocd-gdb.cfg -ex "load" $<

reset: ${ELFFILE}
	arm-none-eabi-gdb -batch -x ./openocd-gdb.cfg -ex "monitor reset" $<

gdb: ${ELFFILE}
	arm-none-eabi-gdb -x ./openocd-gdb.cfg $<
