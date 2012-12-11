#include "enchw.h"
#include "enc28j60.h"

/* Documentation references in here are relative to the ENC28J60 Data Sheet
 * DS39662D */

/** Stores the chip's active register page in order to avoid unnecessary
 * explicit changes. */
static enc_register_t last_used_register;

/** Initialize an ENC28J60 device. */
void enc_setup(void)
{
	enchw_setup();
	last_used_register = ENC_BANK_INDETERMINATE;
}

/** Run the built-in diagnostics. Returns 0 on success or an unspecified
 * error code. */
uint8_t enc_bist(void)
{
	/* according to 15.1 */
	enc_WCR(ENC_EDMASTL, 0);
	enc_WCR(ENC_EDMASTH, 0);
	enc_WCR(ENC_EDMANDL, 0xff);
	enc_WCR(ENC_EDMANDH, 0x1f);
	enc_WCR(ENC_ERXNDL, 0xff);
	enc_WCR(ENC_ERXNDH, 0x1f);
	enc_BFS(ENC_ECON1, ENC_ECON1_CSUMEN);
	/* skipping step 4 as we're using address fill mode */
	enc_WCR(ENC_EBSTCON, ENC_EBSTCON_ADDRESSFILL | ENC_EBSTCON_TME | ENC_EBSTCON_BIST);
	enc_BFS(ENC_ECON1, ENC_ECON1_DMAST);
	while(!enc_RCR(ENC_ECON1 & ENC_ECON1_DMAST));

	return 0;
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

/** 16-bit register read. This only applies to ENC28J60 registers whose low
 * byte is at an even offset and whose high byte is one above that. Can be
 * passed either L or H sub-register. */
/* FIXME: could use enc_register16_t */
uint16_t enc_RCR16(enc_register_t reg) { return (enc_RCR(reg|1) << 8) | enc_RCR(reg&~1);}
/** 16-bit register read. Compare enc_RCR16. Writes the lower byte first, then
 * the higher. */
void enc_WCR16(uint8_t reg, uint16_t data) { enc_WCR(reg&~1, data & 0xff); enc_WCR(reg|1, data >> 8); }

void enc_SRC(void) { enchw_exchangebyte(0xff); }

void enc_wait(void) { while (!(enc_RCR(ENC_ESTAT) & ENC_ESTAT_CLKRDY));}

uint16_t enc_MII_read(enc_register_t mireg)
{
	uint16_t result = 0;

	enc_WCR(ENC_MIREGADR, mireg);
	enc_BFS(ENC_MICMD, ENC_MICMD_MIIRD);

	while(enc_RCR(ENC_MISTAT) & ENC_MISTAT_BUSY);

	result = enc_RCR16(ENC_MIRDL);

	enc_BFC(ENC_MICMD, ENC_MICMD_MIIRD);

	return result;
}

void enc_MII_write(uint8_t mireg, uint16_t data)
{
	while(enc_RCR(ENC_MISTAT) & ENC_MISTAT_BUSY);

	enc_WCR(ENC_MIREGADR, mireg);
	enc_WCR16(ENC_MIWRL, data);
}
