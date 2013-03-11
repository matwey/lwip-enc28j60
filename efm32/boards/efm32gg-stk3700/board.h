/* Pinout definitions for the EnergyMicro EFM32GG STK3700 board */

#include <em_gpio.h>

#define EXT4_PORT gpioPortD
#define EXT4_PIN 0
#define EXT5_PORT gpioPortC
#define EXT5_PIN 3
#define EXT6_PORT gpioPortD
#define EXT6_PIN 1
#define EXT7_PORT gpioPortC
#define EXT7_PIN 4
#define EXT8_PORT gpioPortD
#define EXT8_PIN 2
#define EXT12_PORT gpioPortD
#define EXT12_PIN 4
#define EXT14_PORT gpioPortD
#define EXT14_PIN 5
#define EXT15_PORT gpioPortD
#define EXT15_PIN 7

#define LED0_PORT gpioPortE
#define LED0_PIN 2

#define LED1_PORT gpioPortE
#define LED1_PIN 3

#define BUTTON0_PORT gpioPortB
#define BUTTON0_PIN 9

#define BUTTON1_PORT gpioPortB
#define BUTTON1_PIN 10

#define EXT_USART USART1
#define EXT_USART_CLOCK cmuClock_USART1
#define EXT_USART_LOCATION 1

#define EXT_LEUART LEUART0
#define EXT_LEUART_CLOCK cmuClock_LEUART0
#define EXT_LEUART_ROUTELOCATION LEUART_ROUTE_LOCATION_LOC0

void board_setup(void);
void led0_on(void);
void led1_on(void);
void led0_off(void);
void led1_off(void);
bool button0_pressed(void);
bool button1_pressed(void);
