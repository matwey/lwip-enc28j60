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
 *
 * This implementation requires access to RMU->RSTCAUSE, be careful not to
 * initialize it after clearing that.
 */

#include "rtc.h"
#include <em_emu.h>
#include <em_burtc.h>
#include <em_cmu.h>
#include <em_chip.h>
#include <em_rmu.h>

static volatile uint32_t high64; /**< The overflowing part of the 32bit value composed of high32 and the register */

/** @addtogroup rfc-efm32burtc-impl-regs Retention registers
 *
 * This is a simplistic approach to checksummed doublebuffered registers to
 * avoid issues when writing during power loss.
 *
 * The 128 registers can be accessed by offset and length; for each offset
 * used, (length + 1) * 2 bytes are used in two groups (a primary and a
 * secondary), each with a primitive checksum.
 *
 * @{ */

#define REG(index) BURTC->RET[index].REG

static uint32_t regs_checksum(uint8_t n, uint32_t *data)
{
	uint32_t result = 0;
	while (n--) result ^= *(data++);
	return ~result; /* negate to avoid empty memory being valid */
}

static bool regs_valid(uint8_t index, uint8_t n)
{
	return regs_checksum(n, &(REG(index))) == REG(index + n);
}

void rtc_regs_store(uint8_t index, uint8_t n, uint32_t *value)
{
	uint32_t *writecursor;
	if (regs_valid(index, n)) {
		writecursor = &(REG(index + n + 1));
	} else {
		writecursor = &(REG(index));
	}
	uint32_t checksum = regs_checksum(n, value);
	while (n--) *(writecursor++) = *(value++);
	*writecursor = checksum;
}

bool rtc_regs_retrieve(uint8_t index, uint8_t n, uint32_t *value)
{
	uint32_t *readcursor = 0;
	if (regs_valid(index, n)) {
		readcursor = &(REG(index));
	} else if (regs_valid(index + n + 1, n)) {
		readcursor = &(REG(index + n + 1));
	}
	if (readcursor == 0) return false;
	while (n--) *(value++) = *(readcursor++);
	return true;
}

/** @} */

/** If using this as an interrupt is not an option, it can just as well be
 * called in another fashion as rtc_maintenance. */
//void rtc_maintenance(void)
void BURTC_IRQHandler(void)
{
	if (!(BURTC->IF & BURTC_IF_OF)) return;

        __asm__("CPSID I\n"); /* should be __disable_irq or cm_disable_interrupts */
	BURTC->IFC = BURTC_IF_OF;

	high64 += 1;
	rtc_regs_store(0, 1, &high64);

        __asm__("CPSIE I\n"); /* should be __enable_irq  or cm_enable_interrupts */
}

void rtc_setup(void)
{
	CMU_ClockEnable(cmuClock_CORELE, true);

	/* this is a better indicator than querying the resetcause as suggested
	 * in AN0041 and the burtc example, for their check fails to detect an
	 * "unplug, press reset, plug in" situation, where their burtcSetup is
	 * never called after a reset. RSTEN on the other hand has shown to be
	 * pretty reliable.
	 *
	 * see also:
	 * http://community.silabs.com/t5/32-bit-MCU/Retention-registers-of-BURTC-loose-data/td-p/110231
	 */
	bool bu_is_ready = (BURTC->CTRL & BURTC_CTRL_RSTEN) == 0;

	bool timestamp_is_usable = false;

	if (bu_is_ready) {
		/* do i really have to do this? LFXORDY is false when coming
		 * out of backup mode clearly, but it ticked all the time!
		 *
		 * see also:
		 * http://community.silabs.com/t5/32-bit-MCU/resuming-on-LFXO-after-wakeup-from-BUPD-with-BURTC-running/m-p/133720
		 * */
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

		timestamp_is_usable = true;

		if (BURTC->IF & BURTC_IF_OF) {
			timestamp_is_usable = false;
		}

		if (timestamp_is_usable && !rtc_regs_retrieve(0, 1, &high64)) {
			timestamp_is_usable = false;
		}

		/* Check if retention registers were being written to when backup mode was entered */
		if (BURTC->STATUS & BURTC_STATUS_RAMWERR) {
			/* so what. we'll probably always ignore that, for
			 * writers will prefer to do their own
			 * doublebuffering/checksumming than fail just because
			 * they had bad timing */
		}
		if (BURTC->STATUS & BURTC_STATUS_BUMODETS) {
			/* we might want to pass BURTC_TimestampGet() on --
			 * otoh, that can be queried any time later too, just
			 * ->STATUS will be gone. */
		} else {
			/* everything is fine */
		}

		BURTC_StatusClear();
	} else {
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

		EMU_EM4Init_TypeDef em4Init = {
			.lockConfig = true,
			.osc = emuEM4Osc_LFXO,
			.buRtcWakeup = false,
			.vreg = true,
		};
		EMU_BUPDInit_TypeDef bupdInit = {
			.probe = emuProbe_Disable,
			.bodCal = false,
			.statusPinEnable = false,
			.resistor = emuRes_Res3,
			.voutStrong = false,
			.voutMed = false,
			.voutWeak = false,
			.inactivePower = emuPower_MainBU,
			.activePower = emuPower_None,
			.enable = true,
		};

		EMU_EM4Lock(false);
		EMU_BUPDInit(&bupdInit);
		EMU_BUReady();
		/* wake up bu domain */
		RMU_ResetControl(rmuResetBU, false);

		EMU_EM4Init(&em4Init);

		BURTC_Init_TypeDef burtcInit = {
			.enable = true,
			.mode = burtcModeEM4,
			.debugRun = false,
			.clkSel = burtcClkSelLFXO,
			.clkDiv = 64,
			.lowPowerComp = 0,
			.timeStamp = true,
			.compare0Top = false,
			.lowPowerMode = burtcLPDisable,
		};

		BURTC_Init(&burtcInit);
	}

	if (!timestamp_is_usable)
	{
		/* better not look for patterns where none are; also, if our
		 * timestamp is not usable any more, everything else is
		 * potentially outdated too.
		 *
		 * (admittedly, this is the very specific opinion on usable
		 * data of someone who uses this for keeping rtc offsts, but
		 * hey, this is a clock module after all.) */
		for (uint8_t i = 0; i < 128; ++i)
		{
			BURTC->RET[i].REG = 0;
		}

		high64 = 0;
		rtc_regs_store(0, 1, &high64);

		/* be careful not to clear this early -- if we wake up to an
		 * overflow condition, the above invalidation must have
		 * occurred before this gets cleared, or we might wake up
		 * briefly to clear this and later have a wrong time. */
		BURTC->IFC = BURTC_IF_OF;
	}

	BURTC_IntEnable(RTC_IF_OF);
	NVIC_EnableIRQ(BURTC_IRQn);
}

uint32_t rtc_get24(void)
{
	return BURTC_CounterGet() & 0xffffff;
}

uint32_t rtc_get32(void)
{
	return BURTC_CounterGet();
}

uint64_t rtc_get64(void)
{
	/** see rtc-efm32rtc.c's rtc_get32 for a more readable version */
	uint64_t my_high64;
	uint32_t overflew;
	uint32_t first, second;

        __asm__("CPSID I\n"); /* should be __disable_irq or cm_disable_interrupts */
	first = BURTC_CounterGet();
	my_high64 = high64;
	overflew = BURTC->IF & BURTC_IF_OF;
        __asm__("CPSIE I\n"); /* should be __enable_irq  or cm_enable_interrupts */

	second = BURTC_CounterGet();

	return ((my_high64 + (overflew || (second < first)))<< 32) | second;
}

uint32_t rtc_get_ms(void)
{
	uint64_t bigproduct = rtc_get32() * (uint64_t)1000;
	return bigproduct / rtc_get_ticks_per_second();
}

uint64_t rtc_get_ms64(void)
{
	/** FIXME this is copied from etc-efm32rtc.c, should use some kind of common fallback mechanism */
	return rtc_get64() * 1000 / rtc_get_ticks_per_second();
}

uint32_t rtc_get_ticks_per_second(void)
{
	return 512;
}

/** @} @} */
