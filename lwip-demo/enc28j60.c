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

	enc_wait();

	last_used_register = ENC_BANK_INDETERMINATE;
}

/** Run the built-in diagnostics. Returns 0 on success or an unspecified
 * error code.
 *
 * FIXME: doesn't produce correct results (waits indefinitely on DMA, doesn't
 * alter pattern, addressfill produces correct checksum but wrong data (0xff
 * everywhere)
 * */
uint8_t enc_bist(void)
{
	/* according to 15.1 */
	/* 1. */
	enc_WCR16(ENC_EDMASTL, 0);
	/* 2. */
	enc_WCR16(ENC_EDMANDL, 0x1fff);
	enc_WCR16(ENC_ERXNDL, 0x1fff);
	/* 3. */
	enc_BFS(ENC_ECON1, ENC_ECON1_CSUMEN);
	/* 4. */
	enc_WCR(ENC_EBSTSD, 0x0c);

	/* 5. */
	enc_WCR(ENC_EBSTCON, ENC_EBSTCON_PATTERNSHIFTFILL | (1 << 5) | ENC_EBSTCON_TME);
//	enc_WCR(ENC_EBSTCON, ENC_EBSTCON_ADDRESSFILL | ENC_EBSTCON_PSEL | ENC_EBSTCON_TME);
	/* 6. */
	enc_BFS(ENC_EBSTCON, ENC_EBSTCON_BISTST);
	/* wait a second -- never took any time yet */
	while(enc_RCR(ENC_EBSTCON) & ENC_EBSTCON_BISTST)
		log_message("(%02x)", enc_RCR(ENC_EBSTCON));
	/* 7. */
	enc_BFS(ENC_ECON1, ENC_ECON1_DMAST);
	/* 8. */
	while(enc_RCR(ENC_ECON1) & ENC_ECON1_DMAST)
		log_message("[%02x]", enc_RCR(ENC_ECON1));

	/* 9.: FIXME pull this in */

	return 0;
}

/* Similar check to enc_bist, but doesn't rely on the BIST of the chip but
 * doesn some own reading and writing */
uint8_t enc_bist_manual(void)
{
	uint16_t address;
	uint8_t buffer[256];
	int i;

	enc_WCR16(ENC_ERXNDL, 0x1fff);

	for (address = 0; address < ENC_RAMSIZE; address += 256)
	{
		for (i = 0; i < 256; ++i)
			buffer[i] = ((address >> 8) + i) % 256;

		enc_WBM(buffer, address, 256);
	}

	for (address = 0; address < ENC_RAMSIZE; address += 256)
	{
		enc_RBM(buffer, address, 256);

		for (i = 0; i < 256; ++i)
			if (buffer[i] != ((address >> 8) + i) % 256)
				return 1;
	}

	/* dma checksum */

	/* we don't use dma at all, so we can just as well not test it.
	enc_WCR16(ENC_EDMASTL, 0);
	enc_WCR16(ENC_EDMANDL, ENC_RAMSIZE-1);

	enc_BFS(ENC_ECON1, ENC_ECON1_CSUMEN | ENC_ECON1_DMAST);

	while (enc_RCR(ENC_ECON1) & ENC_ECON1_DMAST) log_message(".");

	log_message("csum %08x", enc_RCR16(ENC_EDMACSL));
	*/

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
static void select_page(uint8_t page) { enc_WCR(ENC_ECON1, (page & 0x03) | (enc_RCR(ENC_ECON1) & ~0x03)); }

static void ensure_register_accessible(enc_register_t r)
{
	if ((r & ENC_BANKMASK) == ENC_BANKALL) return;
	if ((r & ENC_BANKMASK) == last_used_register) return;

	select_page(r >> 6);
}

/* FIXME: applies only to eth registers, not to mii ones */
uint8_t enc_RCR(enc_register_t reg) { ensure_register_accessible(reg); return command(reg & 0x1f, 0); }
void enc_WCR(uint8_t reg, uint8_t data) { ensure_register_accessible(reg); command(0x40 | (reg & 0x1f), data); }
void enc_BFS(uint8_t reg, uint8_t data) { ensure_register_accessible(reg); command(0x80 | (reg & 0x1f), data); }
void enc_BFC(uint8_t reg, uint8_t data) { ensure_register_accessible(reg); command(0xa0 | (reg & 0x1f), data); }

static void RBM_raw(uint8_t *dest, uint16_t length)
{
	enchw_select();
	enchw_exchangebyte(0x3a);
	while(length--)
		*(dest++) = enchw_exchangebyte(0);
	enchw_unselect();
}

static void RBM_discard(uint16_t length)
{
	enchw_select();
	enchw_exchangebyte(0x3a);
	while(length--)
		enchw_exchangebyte(0);
	enchw_unselect();
}

void enc_RBM(uint8_t *dest, uint16_t start, uint16_t length)
{
	enc_WCR16(ENC_ERDPTL, start);
	enc_BFS(ENC_ECON2, ENC_ECON2_AUTOINC);

	RBM_raw(dest, length);
}

void enc_WBM(uint8_t *src, uint16_t start, uint16_t length)
{
	enc_WCR16(ENC_EWRPTL, start);
	enc_BFS(ENC_ECON2, ENC_ECON2_AUTOINC);

	enchw_select();
	enchw_exchangebyte(0x7a);
	while(length--)
		enchw_exchangebyte(*(src++));
	enchw_unselect();
	/* FIXME: this is actually just triggering another pause */
	enchw_unselect();
}

/** 16-bit register read. This only applies to ENC28J60 registers whose low
 * byte is at an even offset and whose high byte is one above that. Can be
 * passed either L or H sub-register. */
/* FIXME: could use enc_register16_t */
uint16_t enc_RCR16(enc_register_t reg) { return (enc_RCR(reg|1) << 8) | enc_RCR(reg&~1);}
/** 16-bit register read. Compare enc_RCR16. Writes the lower byte first, then
 * the higher, as required for the MII interfaces as well as for ERXRDPT. */
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


void enc_LED_set(enc_lcfg_t ledconfig, enc_led_t led)
{
	uint16_t state;
	state = enc_MII_read(ENC_PHLCON);
	state = (state & ~(ENC_LCFG_MASK << led)) | (ledconfig << led);
	enc_MII_write(ENC_PHLCON, state);
}

/** Configure the ENC28J60 for network operation, whose initial parameters get
 * passed as well. */
void enc_operation_setup(enc_operation_t *op, uint16_t rxbufsize, uint8_t mac[6])
{
	/* practical consideration: we don't come out of clean reset, better do
	 * this -- discard all previous packages */

	enc_BFS(ENC_ECON1, ENC_ECON1_TXRST | ENC_ECON1_RXRST);
	while(enc_RCR(ENC_EPKTCNT)) enc_BFS(ENC_ECON2, ENC_ECON2_PKTDEC);
	enc_BFC(ENC_ECON1, ENC_ECON1_TXRST | ENC_ECON1_RXRST); /* FIXME: this should happen later, but when i don't do it here, things won't come up again. probably a problem in the startup sequence. */

	/********* receive buffer setup according to 6.1 ********/

	op->rxbufsize = rxbufsize;
	enc_WCR16(ENC_ERXSTL, 0); /* see errata, must be 0 */
	enc_WCR16(ENC_ERXRDPTL, 0);
	enc_WCR16(ENC_ERXNDL, rxbufsize);

	enc_WCR16(ENC_ERDPTL, 0); /* during regular operation, we'll just read through the ring buffer automatically; setting the read pointer up appropriately*/

	/******** for the moment, the receive filters are good as they are (6.3) ******/

	/******** waiting for ost (6.4) already happened in _setup ******/

	/******** mac initialization acording to 6.5 ************/

	/* enable reception and flow control (shouldn't hurt in simplex either) */
	enc_BFS(ENC_MACON1, ENC_MACON1_MARXEN | ENC_MACON1_TXPAUS | ENC_MACON1_RXPAUS);

	/* generate checksums for outgoing frames and manage padding automatically */
	enc_WCR(ENC_MACON3, ENC_MACON3_TXCRCEN | ENC_MACON3_FULLPADDING | ENC_MACON3_FRMLEN);

	/* setting defer is mandatory for 802.3, but it seems the default is reasonable too */

	/* MAMXF has reasonable default */

	/* it's not documented in detail what these do, just how to program them */
	enc_WCR(ENC_MAIPGL, 0x12);
	enc_WCR(ENC_MAIPGH, 0x0C);

	/* MACLCON registers have reasonable defaults */

	/* set the mac address */
	enc_WCR(ENC_MAADR1, mac[0]);
	enc_WCR(ENC_MAADR2, mac[1]);
	enc_WCR(ENC_MAADR3, mac[2]);
	enc_WCR(ENC_MAADR4, mac[3]);
	enc_WCR(ENC_MAADR5, mac[4]);
	enc_WCR(ENC_MAADR6, mac[5]);

	/*************** enabling reception as per 7.2 ***********/

	/* enable reception */
	enc_BFS(ENC_ECON1, ENC_ECON1_RXEN);

	/* pull transmitter and receiver out of reset */
	enc_BFC(ENC_ECON1, ENC_ECON1_TXRST | ENC_ECON1_RXRST);
}

void enc_transmit(enc_operation_t *op, uint8_t *data, uint16_t length)
{
	/* according to section 7.1 */
	/* FIXME: we only send a single frame blockingly, starting at the end of rxbuf */
	uint16_t start = op->rxbufsize;
	uint8_t control_byte = 0; /* no overrides */

	/* 1. */
	enc_WCR16(ENC_ETXSTL, start);
	/* 2. */
	enc_WBM(&control_byte, start, 1);
	enc_WBM(data, start+1, length);

	/* calculate checksum */

//	enc_WCR16(ENC_EDMASTL, start + 1);
//	enc_WCR16(ENC_EDMANDL, start + 1 + length - 3);
//	enc_BFS(ENC_ECON1, ENC_ECON1_CSUMEN | ENC_ECON1_DMAST);
//	while (enc_RCR(ENC_ECON1) & ENC_ECON1_DMAST);
//	uint16_t checksum = enc_RCR16(ENC_EDMACSL);
//	checksum = ((checksum & 0xff) << 8) | (checksum >> 8);
//	enc_WBM(&checksum, start + 1 + length - 2, 2);

	/* 3. */
	enc_WCR16(ENC_ETXNDL, start+1+length-1);
	
	/* 4. */
	/* skipped because not using interrupts yet */
	/* 5. */
	enc_BFS(ENC_ECON1, ENC_ECON1_TXRTS);

	/* block */
	while (enc_RCR(ENC_ECON1) & ENC_ECON1_TXRTS);

	uint16_t stored_read_position = enc_RCR16(ENC_ERDPTL);

	uint8_t result[7];
	enc_RBM(result, start + 1 + length, 7);
	log_message("transmitted. %02x %02x %02x %02x %02x %02x %02x\n", result[0], result[1], result[2], result[3], result[4], result[5], result[6]);

	enc_WCR16(ENC_ERDPTL, stored_read_position);

	/* FIXME: parse that and return reasonable state */
}

/** Read a received frame into data; may only be called when one is
 * available. Writes up to maxlength bytes and returns the total length of the
 * frame. (If the return value is > maxlength, parts of the frame were
 * discarded.) */
uint16_t enc_read_received(enc_operation_t *op, uint8_t *data, uint16_t maxlength)
{
	uint8_t header[6];
	uint16_t length;

	/* function documentation says that mustn't be the case anyway, but its
	 * safer and maybe we'll remove it */
	if (enc_RCR(ENC_EPKTCNT) == 0)
		return 0;

	log_message("reading from %04x... ", enc_RCR16(ENC_ERDPTL));

	RBM_raw(header, 6);
	length = header[2] | (header[3] << 8);

	if (length > maxlength)
	{
		RBM_raw(data, maxlength);
		RBM_discard(length - maxlength);
	} else {
		RBM_raw(data, length);
	}

	uint16_t next_location = header[0] + (header[1] << 8);
	enc_WCR16(ENC_ERXRDPTL, next_location);
	enc_WCR16(ENC_ERDPTL, next_location); /* due to wrapping, we should be there anyway. explicitly setting this because otherwise we'd have to do the two-byte-aligning ourselves, and then we might have to take the wrapping boundaries into consideration, and i tried to avoid that */
	enc_BFS(ENC_ECON2, ENC_ECON2_PKTDEC);

	log_message("until %04x, ", enc_RCR16(ENC_ERDPTL));
	log_message("and header is %02x %02x  %02x %02x %02x %02x.\n", header[0], header[1], header[2], header[3], header[4], header[5]);

	return length;
}

#ifdef ENC28J60_USE_PBUF
/** Like enc_read_received, but allocate a pbuf buf. FIXME: deduplication, error reporting */
void enc_read_received_pbuf(enc_operation_t *op, struct pbuf **buf)
{
	uint8_t header[6];
	uint16_t length;

	if (*buf != NULL)
		return 1;

	/* function documentation says that mustn't be the case anyway, but its
	 * safer and maybe we'll remove it */
	if (enc_RCR(ENC_EPKTCNT) == 0)
	{
		log_message("no packages pending\n");
		return;
	}

	RBM_raw(header, 6);
	length = header[2] | (header[3] << 8);

	*buf = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
	if (*buf == NULL)
	{
		log_message("failed to allocate buf of length %u.", length);
		return;
	}
	RBM_raw((*buf)->payload, length);

	uint16_t next_location = header[0] + (header[1] << 8);
	enc_WCR16(ENC_ERXRDPTL, next_location);
	enc_WCR16(ENC_ERDPTL, next_location); /* due to wrapping, we should be there anyway. explicitly setting this because otherwise we'd have to do the two-byte-aligning ourselves, and then we might have to take the wrapping boundaries into consideration, and i tried to avoid that */
	enc_BFS(ENC_ECON2, ENC_ECON2_PKTDEC);

	log_message("read with header %02x %02x  %02x %02x %02x %02x to pbuf.\n", header[0], header[1], header[2], header[3], header[4], header[5]);
}
#endif
