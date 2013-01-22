/* SD hardware configuration for the Olimex MOD-SDMMC as wired through a
 * straight-through UEXT cable to a UEXT board */

#include "board.h"

#define SS_PORT UEXT10_PORT
#define SS_PIN UEXT10_PIN
#define MOSI_PORT UEXT8_PORT
#define MOSI_PIN UEXT8_PIN
#define MISO_PORT UEXT7_PORT
#define MISO_PIN UEXT7_PIN
#define SCK_PORT UEXT9_PORT
#define SCK_PIN UEXT9_PIN

#define USART UEXT_USART
#define USART_CLOCK UEXT_USART_CLOCK
#define USART_LOCATION UEXT_USART_LOCATION
