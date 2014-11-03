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
