#include "log-itm.h"

static char buffer[LOGSIZE];

static void logitm_message_impl(char *message, va_list argp)
{
	size_t length;

	length = vsniprintf(buffer, LOGSIZE, message, argp);

	for (size_t i = 0; i < length; ++i)
		ITM_SendChar(buffer[i]);
}

static void logitm_disable(void)
{
	/* In theory, this should undo everything configured in logitm_start.
	 * As others are likely to use some functionality of this as well,
	 * we'll just disable the GPIO pin routing */

	GPIO->ROUTE &= ~GPIO_ROUTE_SWOPEN;
}

static void logitm_start(void)
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
