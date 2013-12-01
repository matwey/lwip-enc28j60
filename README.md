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
* Get a Gecko chip with UEXT connector (EM-32G880F128-H from Olimex)
* Get an SWD programmer (any of the EnergyMicro starter kits)
* Wire everything up the only way the connectors fit
* Get an `arm-none-eabi-gcc` (eg. from the Debian package `gcc-arm-none-eabi`)
* Get the EFM32 toolchain set up (FIXME: that could need some more details)
* Fetch the lwIP library and build the example code

      cd examples/netblink/
      git clone git://git.savannah.nongnu.org/lwip.git -b DEVEL-1_4_1
      make

* Upload the program:

      make upload

* Configure your Ethernet interface as 192.168.0.1/24
* Test everything:

      socat udp:192.168.0.2:1234  -

* Enjoy that the LED blinks every time you send a line.

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
