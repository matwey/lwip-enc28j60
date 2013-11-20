/** @file
 *
 * A simple real time clock interface that provides up to 64bit tick counts of
 * implementation-defined ticks.
 *
 * @@@ obsolete
 *
 * Apart from hardware clock setup (`rtc_setup`) and something suitable for
 * system ticks (`rtc_get`), this provides facilities for a 64bit high
 * precision clock that is usable for inside interrupts and does not depend on
 * having a highest-priority interrupt handler by itself. (@todo)
 *
 * No part of the ENC28J60 driver (EMLIB hardware implementation, ENC28J60
 * driver logic and lwIP port) currently use this, but it is provided for
 * convenience (and the lwIP port hooks the sys_now function up to it, even
 * though it is not used in the examples currently).
 *
 * It is expected that both the hardware implementation and the driver logic
 * will resort to using this for timeouts sooner or later.
 * */

#include <stdint.h>

/** Call this once and early to configure the real time clock. Take care that
 * this might mess with your timing hardware depending on implementation. */
void rtc_setup(void);

/** Maintenance function that has to be called in regular intervals depending
 * on the implementation. Some implementations set up interrupt routines for
 * this and do not require calling it; anyway, it never does any harm.
 */
void rtc_maintenance(void);

/** Get the number of "ticks" expired since startup, module 2^24. This is very
 * efficient on the Energy Micro implementation, but unsuitable for everything
 * that needs full 32 bit. */
uint32_t rtc_get24(void);

/** Get the number of "ticks" expired since startup, module 2^32. */
uint32_t rtc_get32(void);

/** Get the number of "ticks" expired since startup, modulo 2^64. (Will not
 * wrap unless your ticks go crazily often or you run until after the predicted
 * end of the universe). */
uint64_t rtc_get64(void);

/** Get the number of ms expired since the start of the system. */
uint64_t rtc_get_ms64(void)/* __attribute__((optimize("O3")))*/;
