#include "board.h"
#include <em_gpio.h>
#include <em_cmu.h>

void board_setup(void)
{
	/* generic gpio activation */
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, 0);

	GPIO_PinModeSet(BUTTON0_PORT, BUTTON0_PIN, gpioModeInputPullFilter, 1);
	GPIO_PinModeSet(BUTTON1_PORT, BUTTON1_PIN, gpioModeInputPullFilter, 1);
}

void led0_on(void)
{
	GPIO_PinOutSet(LED0_PORT, LED0_PIN);
}
void led1_on(void)
{
	GPIO_PinOutSet(LED1_PORT, LED1_PIN);
}

void led0_off(void)
{
	GPIO_PinOutClear(LED0_PORT, LED0_PIN);
}
void led1_off(void)
{
	GPIO_PinOutClear(LED1_PORT, LED1_PIN);
}

bool button0_pressed(void)
{
	return !GPIO_PinInGet(BUTTON0_PORT, BUTTON0_PIN);
}
bool button1_pressed(void)
{
	return !GPIO_PinInGet(BUTTON1_PORT, BUTTON1_PIN);
}
