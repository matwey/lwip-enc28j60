#include "board.h"
#include <em_gpio.h>
#include <em_cmu.h>

void led_setup(void)
{
	/* generic gpio activation */
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(LED2_PORT, LED2_PIN, gpioModePushPull, 1);
}

void led1_on(void)
{
	GPIO_PinOutClear(LED1_PORT, LED1_PIN);
}
void led2_on(void)
{
	GPIO_PinOutClear(LED2_PORT, LED2_PIN);
}

void led1_off(void)
{
	GPIO_PinOutSet(LED1_PORT, LED1_PIN);
}
void led2_off(void)
{
	GPIO_PinOutSet(LED2_PORT, LED2_PIN);
}
