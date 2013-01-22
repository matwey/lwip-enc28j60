BOARD = efm32gg-stk3700
PARTNUMBER = EFM32GG990F1024
FAMILYNUMBER = EFM32GG

UPLOAD_MECHANISM = eacommander
EACOMMANDER_DIRECTION = mcu

BUILDDIR = build
SOURCEDIR = .

BOARD_OBJS = board.o
CFLAGS += -I ../../efm32/boards/${BOARD}/
VPATH += ../../efm32/boards/${BOARD}/

DRIVER_OBJS = enc28j60.o enchw.o
VPATH += ../../enc28j60driver ../../efm32/enchw
CFLAGS += -DENC28J60_USE_PBUF # configure the enc28j60 backend to build functions that involve lwip buffer mgmt
CFLAGS += -I../../enc28j60driver -I../../efm32/enchw

LWIP_OBJS = etharp.o mem.o memp.o netif.o pbuf.o raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o udp.o dhcp.o init.o def.o timers.o dns.o inet_chksum.o err.o icmp.o ip_frag.o ip4_addr.o ip4.o
# -I. for lwipopts.h
CFLAGS += -I./lwip/src/include/ipv4 -I./lwip/src/include/ipv6 -I./lwip/src/include -I.
VPATH += lwip/src/netif lwip/src/core lwip/src/api lwip/src/core/ipv4

NETIF_OBJS = mchdrv.o testapp.o
CFLAGS += -I../../lwip
VPATH += ../../lwip/netif

MY_OBJS = mch_main.o

OBJS += $(patsubst %,${BUILDDIR}/%,${BOARD_OBJS} ${MY_OBJS} ${DRIVER_OBJS} ${NETIF_OBJS} ${LWIP_OBJS})

include /usr/share/energymicrolib/Makefile.complete
