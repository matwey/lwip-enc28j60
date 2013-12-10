/** 
 * @addtogroup rtc RTC
 * @{
 * @addtogroup rtc-efm32impl EFM32 implementation
 * @{
 *
 * An implementation of the interface described in `rtc.h` for EMLIB (Energy
 * Micro EFM32) devices, which does not need a high-priorized interrupt handler
 * routine and can be queried inside interrupts.
 */

#include "rtc.h"
#include <em_rtc.h>
#include <em_cmu.h>
#include <em_chip.h>

const RTC_Init_TypeDef my_rtc_settings = {
	.enable = true,
	.debugRun = true,
	.comp0Top = false,
};

static volatile uint8_t high32; /**< The overflowing part of the 24bit RTC register */
static volatile uint32_t high64; /**< The overflowing part of the 32bit value composed of high32 and the register */

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
	CMU_ClockEnable(cmuClock_CORELE, true);

	CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFRCO); /* using LFRCO instead of LFXO because LFXO won't come on -- possibly hardware fault on demo board */
	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_64); /* with _64, this gives 2ms ticks */
	CMU_ClockEnable(cmuClock_RTC, true);

	high32 = 0;
	high64 = 0;

	RTC_Init(&my_rtc_settings);

	RTC_IntEnable(RTC_IF_OF);

	NVIC_EnableIRQ(RTC_IRQn);

	/** @todo put an assert here on the 512Hz used below */
}

uint32_t rtc_get24(void)
{
	return RTC_CounterGet();
}

uint32_t rtc_get32(void)
{
	uint32_t my_high32;
	uint32_t overflew;
	uint32_t first, second;

        __asm__("CPSID I\n"); /* should be __disable_irq or cm_disable_interrupts */
	first = RTC_CounterGet();
	my_high32 = high32;
	overflew = RTC->IF & RTC_IF_OF;
        __asm__("CPSIE I\n"); /* should be __enable_irq  or cm_enable_interrupts */

	/* we need two time measurements, for the measurement from before
	 * reading the overflow flag may or not have wrapped, and the
	 * measurement from after reading the overflow flag may have already
	 * wrapped in the meantime (and we have to compare to a reading from
	 * before to see that). */
	second = RTC_CounterGet();

	/* this is a more concise version of the below explanations */
	return ((my_high32 + (overflew || (second < first)))<< 24) | second;
#if 0
	if (overflew)
	{
		/* there was an original unacknowledged overflow, either from
		 * before we entered the functionor during it. as we rely on
		 * the flag to always be cleared in time, we can be sure that
		 * we are not near the next wrap, and first (and just as well
		 * the second) is wrapped exactly once after the last write to
		 * high32 */
		return (my_high32 << 24) + second + (1 << 24);
	}

	if (second < first)
	{
		/* a wrap occurred in the middle of this function, after
		 * reading the overflow flag. we treat it as unaccounted for in
		 * high32 (for we cached that value beforehand; an interrupt
		 * routine might have changed the original value of high32
		 * after we enabled interrupts again), so we add 1, and take
		 * second as the precise value (for it has wrapped for sure).
		 * */
		return (my_high32 << 24) + second + (1 << 24);
	}

	/* no wraps in sight */
	return (my_high32 << 24) + second;
#endif
}

uint64_t rtc_get64(void)
{
	/* see rtc_get64 -- this is just longer, but follows its structure. */
	uint32_t my_high32;
	uint32_t my_high64;
	uint32_t overflew;
	uint32_t first, second;

	__asm__("CPSID I\n"); /* should be __disable_irq or cm_disable_interrupts */
	first = RTC_CounterGet();
	my_high32 = high32;
	my_high64 = high64;
	overflew = RTC->IF & RTC_IF_OF;
	__asm__("CPSIE I\n"); /* should be __enable_irq  or cm_enable_interrupts */

	second = RTC_CounterGet();

	return (((uint64_t)my_high64 << 32) + (((uint64_t)my_high32 + (overflew || (second < first))) << 24)) | second;
}

uint32_t rtc_get_ms(void)
{
	/* FIXME this can probably be expressed with less force, and if we know
	 * the frequency this operates on, we can further optimize this.
	 *
	 * anyway, this routine is more for display purposes than for real
	 * calculations. */
	uint64_t bigproduct = RTC_CounterGet() * (uint64_t)1000;
	return bigproduct / CMU_ClockFreqGet(cmuClock_RTC);
}

uint64_t rtc_get_ms64(void)
{
	/* this assumes the 512Hz ticks configured above */
	return rtc_get64() * 1000 / 512;
}

/** @} @} */
