#include "log-itm.h"
#include "log.h"

#include <stdio.h>

#include <em_cmu.h>
#include <core_cm3.h>

static char buffer[LOGSIZE];

static void logitm_message_impl(char *message, va_list argp)
{
	size_t length;
	bool overflow = false;

	length = vsniprintf(buffer, LOGSIZE, message, argp);

	if (length > LOGSIZE) {
		length = LOGSIZE;
		overflow = true;
	}

	for (size_t i = 0; i < length; ++i)
		ITM_SendChar(buffer[i]);

	if (overflow) {
		const char *oftext = "(overflow)\n";
		while (*oftext != '\0') ITM_SendChar(*(oftext++));
	}
}

static void logitm_disable(void)
{
	/* In theory, this should undo everything configured in logitm_start.
	 *
	 * To keep the pin usable when logitm_start is called later again, we're not
	 * disabling anything here. (When any other application comes along that uses
	 * the same channel, I'll hopefully know how to coordinate a handover.)
	 */
}

void logitm_start(void)
{
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;

  /* Set default locations as recommended by EnergyMicro tools */
#if defined(_EFM32_GIANT_FAMILY) || defined(_EFM32_LEOPARD_FAMILY) || defined(_EFM32_WONDER_FAMILY)
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;

  GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
  GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
#else
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) |GPIO_ROUTE_SWLOCATION_LOC1;
  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#endif

  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
  while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

  log_backend_set(logitm_message_impl, logitm_disable);
}
