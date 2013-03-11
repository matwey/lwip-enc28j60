/* SD hardware configuration for the Olimex MOD-ENC28J60 as wired through the
 * custom cable to a EFM32STK board.
 *
 * Wire like this:
 *
 *   STK EXT pin  ------  UEXT plug pin
 *        7        (SS)          10
 *        4       (MOSI)          8
 *        6       (MISO)          7
 *        8       (CLCK)          9
 *       19        (GND)          2
 *       20        (3V3)          1
 *
 * Don't use VMCU (the STK's pin 2) as 3V3 supply, it can't provide the current
 * the ENC28J60 needs.
 * */

#include "board.h"

#define SS_PORT EXT7_PORT
#define SS_PIN EXT7_PIN
#define MOSI_PORT EXT4_PORT
#define MOSI_PIN EXT4_PIN
#define MISO_PORT EXT6_PORT
#define MISO_PIN EXT6_PIN
#define SCK_PORT EXT8_PORT
#define SCK_PIN EXT8_PIN

#define HAS_RESET_PIN 1
#define RESET_PORT EXT5_PORT
#define RESET_PIN EXT5_PIN

#define USART EXT_USART
#define USART_CLOCK EXT_USART_CLOCK
#define USART_LOCATION EXT_USART_LOCATION
