/* Pinout definitions for the Olimex EM-32G880F128-H board */

#include <em_gpio.h>

#define UEXT3_PORT gpioPortD
#define UEXT3_PIN 4

#define UEXT4_PORT gpioPortD
#define UEXT4_PIN 5

#define UEXT5_PORT gpioPortD
#define UEXT5_PIN 7

#define UEXT6_PORT gpioPortD
#define UEXT6_PIN 6

#define UEXT7_PORT gpioPortD
#define UEXT7_PIN 1

#define UEXT8_PORT gpioPortD
#define UEXT8_PIN 0

#define UEXT9_PORT gpioPortD
#define UEXT9_PIN 2

#define UEXT10_PORT gpioPortD
#define UEXT10_PIN 3

#define LED1_PORT gpioPortE
#define LED1_PIN 1

#define LED2_PORT gpioPortE
#define LED2_PIN 2

#define BUTTON_PORT gpioPortE
#define BUTTON_PIN 0

#define UEXT_USART USART1
#define UEXT_USART_CLOCK cmuClock_USART1
#define UEXT_USART_LOCATION 1

void board_setup(void);
void led1_on(void);
void led2_on(void);
void led1_off(void);
void led2_off(void);
bool button_pressed(void);
