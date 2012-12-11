#include "enchw.h"
#include "enc28j60.h"

/** Stores the chip's active register page in order to avoid unnecessary
 * explicit changes. */
static enc_register_t last_used_register;

/** Initialize an ENC28J60 device. */
void enc_setup(void)
{
	enchw_setup();
	last_used_register = ENC_BANK_INDETERMINATE;
}

static uint8_t command(uint8_t first, uint8_t second)
{
	uint8_t result;
	enchw_select();
	enchw_exchangebyte(first);
	result = enchw_exchangebyte(second);
	enchw_unselect();
	return result;
}

/* this would recurse infinitely if ENC_ECON1 was not ENC_BANKALL */
static void select_page(uint8_t page) { enc_WCR(ENC_ECON1, page & 0x03); }

static void ensure_register_accessible(enc_register_t r)
{
	if ((r & ENC_BANKMASK) == ENC_BANKALL) return;
	if ((r & ENC_BANKMASK) == last_used_register) return;

	select_page(r >> 6);
}

uint8_t enc_RCR(enc_register_t reg) { ensure_register_accessible(reg); return command(reg & 0x1f, 0); }
void enc_WCR(uint8_t reg, uint8_t data) { ensure_register_accessible(reg); command(0x40 | (reg & 0x1f), data); }
void enc_BFS(uint8_t reg, uint8_t data) { ensure_register_accessible(reg); command(0x80 | (reg & 0x1f), data); }
void enc_BFC(uint8_t reg, uint8_t data) { ensure_register_accessible(reg); command(0xa0 | (reg & 0x1f), data); }

void enc_SC(void) { enchw_exchangebyte(0xff); }

void enc_wait(void) { while (!(enc_RCR(ENC_ESTAT) & ENC_ESTAT_CLKRDY));}

uint16_t enc_MII_read(enc_register_t mireg)
{
	uint16_t result = 0;

	enc_WCR(ENC_MIREGADR, mireg);
	enc_BFS(ENC_MICMD, ENC_MICMD_MIIRD);

	while(enc_RCR(ENC_MISTAT) & ENC_MISTAT_BUSY);

	result = enc_RCR(ENC_MIRDL);
	result |= enc_RCR(ENC_MIRDH)<<8;

	enc_BFC(ENC_MICMD, ENC_MICMD_MIIRD);

	return result;
}

void enc_MII_write(uint8_t mireg, uint16_t data)
{
	while(enc_RCR(ENC_MISTAT) & ENC_MISTAT_BUSY);

	enc_WCR(ENC_MIREGADR, mireg);
	enc_WCR(ENC_MIWRL, data & 0xff);
	enc_WCR(ENC_MIWRH, data>>8);
}
