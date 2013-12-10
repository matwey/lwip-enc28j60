## use this section with the energyicro stk board
#BOARD = efm32gg-stk3700
#PARTNUMBER = EFM32GG990F1024
#FAMILYNUMBER = EFM32GG
#EACOMMANDER_DIRECTION = mcu

# use this section with the olimex board
BOARD = olimex-efm32g880f128-h
PARTNUMBER = EFM32G880F128
FAMILYNUMBER = EFM32G
EACOMMANDER_DIRECTION = out

UPLOAD_MECHANISM = eacommander

BUILDDIR = build
SOURCEDIR = .

BOARD_OBJS = board.o
CFLAGS += -I ../../efm32/boards/${BOARD}/
vpath %.c ../../efm32/boards/${BOARD}/

DRIVER_OBJS = enc28j60.o enchw.o rtc.o
vpath %.c ../../enc28j60driver ../../efm32/enchw
CFLAGS += -DENC28J60_USE_PBUF # configure the enc28j60 backend to build functions that involve lwip buffer mgmt
CFLAGS += -I../../enc28j60driver -I../../efm32/enchw

# use this with lwip 1.5 development or later
#LWIP_OBJS = etharp.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o udp.o dhcp.o init.o def.o timers.o dns.o inet_chksum.o err.o icmp.o ip_frag.o ip4_addr.o ip4.o
# use this with lwip 1.4.1
LWIP_OBJS = etharp.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o udp.o dhcp.o init.o def.o timers.o dns.o inet_chksum.o err.o icmp.o ip_frag.o ip_addr.o ip.o
# -I. for lwipopts.h
CFLAGS += -I./lwip/src/include/ipv4 -I./lwip/src/include/ipv6 -I./lwip/src/include -I.
vpath %.c lwip/src/netif lwip/src/core lwip/src/api lwip/src/core/ipv4

NETIF_OBJS = mchdrv.o
CFLAGS += -I../../lwip
vpath %.c ../../lwip/netif

MY_OBJS = mch_main.o testapp.o

OBJS += ${BOARD_OBJS} ${MY_OBJS} ${DRIVER_OBJS} ${NETIF_OBJS} ${LWIP_OBJS}

CFLAGS += -pedantic -Wall -std=c99

# adaptions to debian gcc
CFLAGS += -I/usr/arm-none-eabi/include/ -ffreestanding -I/usr/include/newlib/
LDFLAGS += -L/usr/arm-none-eabi/lib/ -L/usr/lib/arm-none-eabi/newlib/armv7-m/

CFLAGS += -D${PARTNUMBER} -D${FAMILYNUMBER} -I/usr/arm-none-eabi/include/libopencmsis/
CFLAGS += -Os -g -Wall -Wextra -fno-common -mcpu=cortex-m3 -mthumb -msoft-float

LDFLAGS += -nostartfiles -Wl,--gc-sections -mthumb -msoft-float -mcpu=cortex-m3
LDFLAGS += -lc -lnosys
LDSCRIPT ?= libopencm3_efm32g880f128.ld
# gotta ditch that
LDFLAGS += -L /usr/arm-none-eabi/lib/ -lopencm3_efm32g  -T$(LDSCRIPT)

OBJS += em_cmu.o em_gpio.o em_usart.o em_rtc.o em_emu.o system_efm32g.o
vpath %.c /usr/lib/arm-none-eabi/src/energymicrolib/src/ ${HOME}/git/energymicrolib/Device/EnergyMicro/EFM32G/Source/

OBJS += nvic.o
vpath %.c ${HOME}/git/libopencm3/lib/cm3/

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc

# the explicit rule (a copy of the default rule plus "build/") seems to be the
# easiest way to get make to output to a subdirectory. vpath is required to
# make the implicit rule happy that links a binary from an object of the same
# name. using the opportunity to mkdir too.
./${BUILDDIR}/%.o: %.c
	@mkdir -p ./${BUILDDIR}/
	$(COMPILE.c) $(OUTPUT_OPTION) $<


BUILDOBJS = $(addprefix ./${BUILDDIR}/,${OBJS})

# name does not matter, but needs to be in objects for default rule to apply
./${BUILDDIR}/mch_main: ${BUILDOBJS}

# enable dependencies
-include $(BUILDOBJS:%.o=%.d)
CFLAGS += -MD -MP

./${BUILDDIR}/mch_main.bin: ./${BUILDDIR}/mch_main
	objcopy -Obinary $< $@

stlink_upload: mch_main.bin
	gdb-multiarch -x ./openocd-gdb.cfg \
		-ex "monitor halt" \
		-ex "monitor flash write_image erase $<" \
		-ex "monitor verify_image $<" \
		-ex "monitor reset" \
		-ex "disconnect" \
		-ex "quit"

gdb: build/main.elf
	gdb-multiarch -x ./openocd-gdb.cfg $<
