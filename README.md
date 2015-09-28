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
* Get `openocd` (eg. from Debian, tested with version 0.9.0)
* Get an `arm-none-eabi-gcc` and `-gdb` (eg. from the Debian package `gcc-arm-none-eabi` and `gdb-arm-none-eabi`)

* Get the EFM32 emlib and CMSIS sources. Not everything contained in there is
  free software, but the parts used in the netblink example are:

        cd examples/netblink
        wget http://www.silabs.com/Support%20Documents/Software/Gecko_SDK.zip
        unzip Gecko_SDK.zip -d Gecko_SDK Device/\*.\[hc\] Device/\*/GCC/\* emlib/\*.\[hc\] CMSIS/\*.h

* In the same directory, fetch the lwIP library ...

        git clone git://git.savannah.nongnu.org/lwip.git -b DEVEL-1_4_1

* ... and build the example code:

        make

* Start `openocd`:

        openocd -f openocd.cfg

  Leave this running and continue in another terminal.

* Upload the program and let it run independt of the debugger:

        make upload reset

* Configure your Ethernet interface as 192.168.0.1/24
* Test everything:

        socat udp:192.168.0.2:1234  -

* Enjoy that the LED blinks every time you send a line.

### Debugging

* You can easily start a debuggin session with `make gdb`. The program can be
  flashed from inside gdb as using the `load` command, a reset using `run`.

### Links

[1]: http://www.silabs.com/
[2]: https://github.com/mbedmicro/mbed/tree/master/libraries/mbed/targets/hal/TARGET_Silicon_Labs/TARGET_EFM32/emlib

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
