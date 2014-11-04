/** 
 * @addtogroup rtc RTC
 * @{
 * @addtogroup rtc-efm32burtc-impl EFM32 BURTC implementation
 * @{
 *
 * An implementation of the interface described in `rtc.h` for EMLIB (Energy
 * Micro EFM32) devices, which does not need a high-priorized interrupt handler
 * routine and can be queried inside interrupts.
 *
 * This implementation uses the BURTC clock available on some EFM32 devices.
 *
 * This implementation persists the tick count over poweroff situations using
 * the backup capacitor, allows storing some data along with the backup-powered
 * counter, and detects overflows.
 *
 * Overflows are currently not handles well: If the device is powered off duing
 * an overflow (which happens at fixed points in time from the last resetting
 * start of the device), it will indicate an overflow condition -- that's
 * primarily because we guarantee that ticks increase monotonically modulo the
 * bit length, and we can't rewind the counter to somewhere where we can count
 * for long time after a poweroff.
 *
 * Possible future solutions are:
 *
 * * Use BURTC and RTC independently. BURTC can get a bigger prescaler (and
 *   thus survive 194 days) and be reset at will.
 * * Make a difference between the system-visible ticks and the BURTC ticks,
 *   eg. rewind the BURTC once its MSB becomes 1, and store that state to
 *   replay it to applications.
 * * Use the BURTC's COMP0 interrupt flag instead of the overflow indicator.
 *   COMP0 would, at runtime, be repeatedly set to be less than the count, and
 *   only when the comparator interrupt flag is treated as an overflow.
 */

#include "rtc.h"
#include <em_burtc.h>
#include <em_cmu.h>
#include <em_chip.h>

static volatile uint32_t high64; /**< The overflowing part of the 32bit value composed of high32 and the register */

/** If using this as an interrupt is not an option, it can just as well be
 * called in another fashion as rtc_maintenance. */
//void rtc_maintenance(void)
void RTC_IRQHandler(void)
{
	if (!(RTC->IF & RTC_IF_OF)) return;

        __asm__("CPSID I\n"); /* should be __disable_irq or cm_disable_interrupts */
	if (high32 == 0xff)
		high64 += 1;
	high32 += 1;
	RTC->IFC = RTC_IF_OF;
        __asm__("CPSIE I\n"); /* should be __enable_irq  or cm_enable_interrupts */
}

void rtc_setup(void)
{
	/** @TODO implement */
}

uint32_t rtc_get24(void)
{
	/** @TODO provide simple fallabck*/
}

uint32_t rtc_get32(void)
{
	/** @TODO implement */
}

uint64_t rtc_get64(void)
{
	/** @TODO implement */
}

uint32_t rtc_get_ms(void)
{
	/** @TODO implement */
}

uint64_t rtc_get_ms64(void)
{
	/** @TODO implement */
}

/** @} @} */
