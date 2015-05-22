ENC28J60 driver and lwIP port
=============================

This repository contains a generic ENC28J60 driver with hardware abstraction
and an lwIP network interface port. An example implementation for the hardware
abstraction is provided for the EnergyMicro Gecko microcontroller, on which the
example of an lwIP project using the driver can be run.

It provides all the functionality required for stable send/receive operation
(was tested against concurrent flood pings and TCP requests), but has room for
extension, especially with respect to error reporting and optimization.
(Current code relies on polling only, and utilizes neither DMA nor interrupts.)

Quick start
-----------

* Get a ENC28J60 chip with UEXT connector (MOD-ENC28J60 from Olimex)
* Get a Gecko chip with UEXT connector (EM-32G880F128-H from Olimex) with a
  power supply (the programmer only measures and does not power)
* Get an SWD programmer (ST-Link/V2)
* Wire everything up the only way the connectors fit
* Get `openocd` (eg. from Debian, version 0.8.0-131-gbd0409a-0~exp1 or later[1])
* Get an `arm-none-eabi-gcc` (eg. from the Debian package `gcc-arm-none-eabi`)
* Get the EFM32 emlib and CMSIS sources (FIXME: that could need some more details)
* Fetch the lwIP library and build the example code

        cd examples/netblink/
        git clone git://git.savannah.nongnu.org/lwip.git -b DEVEL-1_4_1
        make

* Start `openocd`:

        openocd -f openocd.cfg

  Leave this running and continue in another terminal.

* Upload the program:

        make stlink_upload

* Configure your Ethernet interface as 192.168.0.1/24
* Test everything:

        socat udp:192.168.0.2:1234  -

* Enjoy that the LED blinks every time you send a line.

Need debug output?

* In a dedicated terminal, run

        socat pipe:/tmp/readswo-in - | ./swodecoder.py /dev/stdin

* make stlink_readswo

* Make the LED blink several times; some buffering is still involved in the
  output path.

[1] needs to include http://openocd.zylin.com/#/c/1664

ENC28J60 driver
---------------

The driver itself resides in `./enc28j60driver/`. It is basically stack
agnostic, ie. it just implements typical operations conducted with the ENC28J60
(eg. setup, doing a self test, reading a received frame into pre-allocated
memory).

If compiled with `-DENC28J60_USE_PBUF`, two small additional functions,
@ref enc_read_received_pbuf and @ref enc_transmit_pbuf, are added, which are
designed to use the lwIP pbuf memory management system. This is to keep the
interfaces simple.

lwIP port
---------

The lwIP netif implementation for the driver resides in lwip/netif, following
the naming structure of the lwIP project (at least I hope so).

It is suitable for lwIP version 1.4.1 and the current (as of 2013-01-22) 1.5
development version.

Due to the interfaces provided by the driver itself, it is rather minimal, and
only consists of a init and a polling routine (which, as the name implies, is
to be called as often as possible).

EFM32 backend
-------------

An implementation of the hardware backend for the EFM32 Gecko platform can be
found in `efm32/enchw`. It utilizes the free (zlib-style licensed) emlib
library provided by EnergyMicro, and manages transmission of bytes over the
chips' SPI peripherials. The very pinouts are defined in the `enchw-config.h`
files, which are provided for particular development boards in `efm32/boards/`,
along with very simple board drivers that are used in the examples.
