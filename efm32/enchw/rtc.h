/** 
 * @addtogroup rtc RTC
 * @{
 *
 * A simple real time clock interface that provides up to 64bit tick counts of
 * implementation-defined ticks, and a millisecond interface.
 *
 * This consists of hardware clock setup and maintenance (`rtc_setup`,
 * `rtc_maintenance`), functions suitable to get the current number of system
 * ticks (`rtc_get24`, `rtc_get32` and `rtc_get64`) and functions for getting
 * milliseconds (`rtc_get_ms64`). Depending on the implementation, the lower
 * the bit lengths, the faster the execution. (For example, a 24bit counter
 * could be fetched from the hardware RTC, while the longer lengths might
 * require locking).
 *
 * If it is provided by the implementation, the `rtc_maintenance` function has
 * to be called at least once after every clock wrap. All functions are safe to
 * be called from interrupts too. (It is expected that there will be an
 * additional implementation provided header file, which will allow inlining of
 * functions or defining constant ticks per second later on; this will allow
 * the user not to depend on knowledge of the implementation in order to
 * determine whether or not `rtc_maintenance` has to be called.)
 *
 * No part of the ENC28J60 driver (EMLIB hardware implementation, ENC28J60
 * driver logic and lwIP port) currently use this, but it is used in the
 * example provided (along with simple code gluing `sys_ticks` to the RTC
 * interface).
 *
 * It is expected that both the hardware implementation and the driver logic
 * will resort to using this for timeouts sooner or later.
 *
 * @file
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

/** @} */
