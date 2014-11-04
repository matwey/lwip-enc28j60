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

/** If using this as an interrupt is not an option, it can just as well be
 * called in another fashion as rtc_maintenance. */
//void rtc_maintenance(void)
void BURTC_IRQHandler(void)
{
}

void rtc_setup(void)
{
	uint32_t startup_counter;

	CMU_ClockEnable(cmuClock_CORELE, true);

	startup_counter = BURTC_CounterGet();

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

	/* this is a better indicator than querying the resetcause as suggested
	 * in AN0041 and the burtc example, for their check fails to detect an
	 * "unplug, press reset, plug in" situation, where their burtcSetup is
	 * never called after a reset. RSTEN on the other hand has shown to be
	 * pretty reliable.
	 *
	 * see also
	 * http://community.silabs.com/t5/32-bit-MCU/Retention-registers-of-BURTC-loose-data/td-p/110231
	 * .
	 */
	bool bu_is_ready = (BURTC->CTRL & BURTC_CTRL_RSTEN) == 0;

	/* do i really have to do this? LFXORDY is false when coming out of backup mode clearly, but it ticked all the time! */
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

	if (bu_is_ready) {
		log_message("there might be a state\n");
		/* Check if retention registers were being written to when backup mode was entered */
		if (BURTC->STATUS & BURTC_STATUS_RAMWERR) {
			log_message("written to while entering mode, still using the configured clock (it's just not a valid time)\n");
		}
		if (BURTC->STATUS & BURTC_STATUS_BUMODETS) {
			log_message("Last timestamp present: %d. Continue counting from %d (now %d)\n", BURTC_TimestampGet(), startup_counter, BURTC_CounterGet());
		} else {
			log_message("No last timestmap. Continue counting from %d (now %d)\n", startup_counter, BURTC_CounterGet());
		}
		BURTC_StatusClear();
	} else {
		log_message("it's a fresh boot\n");

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
			.clkDiv = 1,
			.lowPowerComp = 0,
			.timeStamp = true,
			.compare0Top = false,
			.lowPowerMode = burtcLPDisable,
		};

		BURTC_Init(&burtcInit);
		log_message("configured\n");

		BURTC->RET[0].REG = 0x1000;
	}

	log_message("register value %x\n", BURTC->RET[0].REG++);
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

uint32_t rtc_get_ticks_per_second(void)
{
	return 512;
}

/** @} @} */
