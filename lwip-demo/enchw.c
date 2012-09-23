/* ENC28J60 hardware implementation for EMLIB devices */

#include "enchw.h"
#include <em_gpio.h>
#include <em_cmu.h>
#include <em_usart.h>

#include "enchw-config.c"

static USART_InitSync_TypeDef enc28j60_usart_config = \
  { enable: usartEnable, \
    refFreq: 0, \
    baudrate: 2000000,  /* 2mhz works well with the current pause commands, 10mhz doesnt */ \
    databits: usartDatabits8, \
    master: true, \
    msbf: true, \
    clockMode: usartClockMode0 \
  };

void enchw_setup(void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(USART_CLOCK, true);

	/* just an experiment: use *fast* clock */
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

	/* ss is active low */
	GPIO_PinModeSet(SS_PORT, SS_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(MOSI_PORT, MOSI_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(SCK_PORT, SCK_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(MISO_PORT, MISO_PIN, gpioModeInput, 0);

	USART_Reset(USART);
	USART_InitSync(USART, &enc28j60_usart_config);

        /* routing setup: cs is done manually */
        USART->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | (USART_LOCATION << 8);
}


volatile static uint8_t j=0;
#define pause() while(++j)

void enchw_select(void)
{
	/* this migh be relevant for t_{CSD}, especially when sending consecutive commands. */
	pause();
	GPIO_PinOutClear(SS_PORT, SS_PIN);
}

void enchw_unselect(void)
{
	/* if this pause is not observed, T_{CSH} will not be obeyed and writes
	 * to MIREGADR will fail */
	pause();
	GPIO_PinOutSet(SS_PORT, SS_PIN);
}

uint8_t enchw_exchangebyte(uint8_t byte)
{
	USART_Tx(USART, byte);
	return USART_Rx(USART);
}

static uint8_t enc_command(uint8_t first, uint8_t second)
{
	uint8_t result;
	enchw_select();
	enchw_exchangebyte(first);
	result = enchw_exchangebyte(second);
	enchw_unselect();
	return result;
}

uint8_t enc_RCR(uint8_t reg) { return enc_command(reg & 0x1f, 0); }
void enc_WCR(uint8_t reg, uint8_t data) { enc_command(0x40 | (reg & 0x1f), data); }
void enc_BFS(uint8_t reg, uint8_t data) { enc_command(0x80 | (reg & 0x1f), data); }
void enc_BFC(uint8_t reg, uint8_t data) { enc_command(0xa0 | (reg & 0x1f), data); }

void enc_SC(void) { enchw_exchangebyte(0xff); }

void enc_select_page(uint8_t page) { enc_WCR(ENC_ECON1, page & 0x03); }

void enc_wait(void) { while (!(enc_RCR(ENC_ESTAT) & ENC_ESTAT_CLKRDY));}

uint16_t enc_MII_read(uint8_t mireg)
{
	uint16_t result = 0;

	enc_select_page(2);
	enc_WCR(ENC_MIREGADR, mireg);
	enc_BFS(ENC_MICMD, ENC_MICMD_MIIRD);

	enc_select_page(3);
	while(enc_RCR(ENC_MISTAT) & ENC_MISTAT_BUSY);

	enc_select_page(2);
	result = enc_RCR(ENC_MIRDL);
	result |= enc_RCR(ENC_MIRDH)<<8;

	enc_BFC(ENC_MICMD, ENC_MICMD_MIIRD);

	return result;
}

void enc_MII_write(uint8_t mireg, uint16_t data)
{
	enc_select_page(3);
	while(enc_RCR(ENC_MISTAT) & ENC_MISTAT_BUSY);

	enc_select_page(2);
	enc_WCR(ENC_MIREGADR, mireg);
	enc_WCR(ENC_MIWRL, data & 0xff);
	enc_WCR(ENC_MIWRH, data>>8);
}
