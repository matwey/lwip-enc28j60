/**
 * @addtogroup enc28j60 ENC28J60 driver
 * @{
 * @addtogroup enc28j60-impl ENC28J60 driver implementation
 * @{
 *
 * .
 *
 * Documentation references in here are relative to the ENC28J60 Data Sheet
 * DS39662D
 * */

#include "enchw.h"
#include "enc28j60.h"

#if defined(ENC28J60_USE_PBUF) && !defined(DEBUG)
#define DEBUG(...) LWIP_DEBUGF(NETIF_DEBUG, (__VA_ARGS__))
#endif

#ifndef DEBUG
#error "Please provide a DEBUG(...) macro that behaves like a printf."
#endif

/** This access/cast happens too often to be written out explicitly */
#define HWDEV (enchw_device_t*)dev->hwdev

/** Initialize an ENC28J60 device. Returns 0 on success, or an unspecified
 * error code if something goes wrong.
 *
 * This function needs to be called first whenever the MCU or the network
 * device is powered up. It will not configure transmission or reception; use
 * @ref enc_ethernet_setup for that, possibly after having run self tests.
 * */
int enc_setup_basic(enc_device_t *dev)
{
	enchw_setup(HWDEV);

	if (enc_wait(dev))
		return 1;

	dev->last_used_register = ENC_BANK_INDETERMINATE;
	dev->rxbufsize = ~0;
	dev->rdpt = ~0;

	enc_BFS(dev, ENC_ECON2, ENC_ECON2_AUTOINC);

	return 0;
}

static void set_erxnd(enc_device_t *dev, uint16_t erxnd)
{
	if (erxnd != dev->rxbufsize) {
		dev->rxbufsize = erxnd;
		enc_WCR16(dev, ENC_ERXNDL, erxnd);
	}
}

static void set_erdpt(enc_device_t *dev, uint16_t erdpt)
{
	if (erdpt != dev->rdpt) {
		dev->rdpt = erdpt;
		enc_WCR16(dev, ENC_ERDPTL, erdpt);
	}
}

/** Run the built-in diagnostics. Returns 0 on success or an unspecified
 * error code.
 *
 * @todo doesn't produce correct results (waits indefinitely on DMA, doesn't
 * alter pattern, addressfill produces correct checksum but wrong data (0xff
 * everywhere)
 * */
uint8_t enc_bist(enc_device_t *dev)
{
	/* according to 15.1 */
	/* 1. */
	enc_WCR16(dev, ENC_EDMASTL, 0);
	/* 2. */
	enc_WCR16(dev, ENC_EDMANDL, 0x1fff);
	set_erxnd(dev, 0x1fff);
	/* 3. */
	enc_BFS(dev, ENC_ECON1, ENC_ECON1_CSUMEN);
	/* 4. */
	enc_WCR(dev, ENC_EBSTSD, 0x0c);

	/* 5. */
	enc_WCR(dev, ENC_EBSTCON, ENC_EBSTCON_PATTERNSHIFTFILL | (1 << 5) | ENC_EBSTCON_TME);
//	enc_WCR(dev, ENC_EBSTCON, ENC_EBSTCON_ADDRESSFILL | ENC_EBSTCON_PSEL | ENC_EBSTCON_TME);
	/* 6. */
	enc_BFS(dev, ENC_EBSTCON, ENC_EBSTCON_BISTST);
	/* wait a second -- never took any time yet */
	while(enc_RCR(dev, ENC_EBSTCON) & ENC_EBSTCON_BISTST)
		DEBUG("(%02x)", enc_RCR(dev, ENC_EBSTCON));
	/* 7. */
	enc_BFS(dev, ENC_ECON1, ENC_ECON1_DMAST);
	/* 8. */
	while(enc_RCR(dev, ENC_ECON1) & ENC_ECON1_DMAST)
		DEBUG("[%02x]", enc_RCR(dev, ENC_ECON1));

	/* 9.: @todo pull this in */

	return 0;
}

/* Similar check to enc_bist, but doesn't rely on the BIST of the chip but
 * doesn some own reading and writing */
uint8_t enc_bist_manual(enc_device_t *dev)
{
	uint16_t address;
	uint8_t buffer[256];
	int i;

	set_erxnd(dev, ENC_RAMSIZE-1);

	for (address = 0; address < ENC_RAMSIZE; address += 256)
	{
		for (i = 0; i < 256; ++i)
			buffer[i] = ((address >> 8) + i) % 256;

		enc_WBM(dev, buffer, address, 256);
	}

	for (address = 0; address < ENC_RAMSIZE; address += 256)
	{
		enc_RBM(dev, buffer, address, 256);

		for (i = 0; i < 256; ++i)
			if (buffer[i] != ((address >> 8) + i) % 256)
				return 1;
	}

	/* dma checksum */

	/* we don't use dma at all, so we can just as well not test it.
	enc_WCR16(ENC_EDMASTL, 0);
	enc_WCR16(ENC_EDMANDL, ENC_RAMSIZE-1);

	enc_BFS(ENC_ECON1, ENC_ECON1_CSUMEN | ENC_ECON1_DMAST);

	while (enc_RCR(ENC_ECON1) & ENC_ECON1_DMAST) DEBUG(".");

	DEBUG("csum %08x", enc_RCR16(ENC_EDMACSL));
	*/

	return 0;
}

static uint8_t command(enc_device_t *dev, uint8_t first, uint8_t second)
{
	uint8_t result;
	enchw_select(HWDEV);
	enchw_exchangebyte(HWDEV, first);
	result = enchw_exchangebyte(HWDEV, second);
	enchw_unselect(HWDEV);
	return result;
}

/* this would recurse infinitely if ENC_ECON1 was not ENC_BANKALL */
static void select_page(enc_device_t *dev, uint8_t page)
{
	uint8_t set = page & 0x03;
	uint8_t clear = (~page) & 0x03;
	if(set)
		enc_BFS(dev, ENC_ECON1, set);
	if(clear)
		enc_BFC(dev, ENC_ECON1, clear);
}

static void ensure_register_accessible(enc_device_t *dev, enc_register_t r)
{
	if ((r & ENC_BANKMASK) == ENC_BANKALL) return;
	if ((r & ENC_BANKMASK) == dev->last_used_register) return;

	select_page(dev, r >> 6);
}

/** @todo applies only to eth registers, not to mii ones */
uint8_t enc_RCR(enc_device_t *dev, enc_register_t reg) {
	ensure_register_accessible(dev, reg);
	return command(dev, reg & ENC_REGISTERMASK, 0);
}
void enc_WCR(enc_device_t *dev, uint8_t reg, uint8_t data) {
	ensure_register_accessible(dev, reg);
	command(dev, 0x40 | (reg & ENC_REGISTERMASK), data);
}
void enc_BFS(enc_device_t *dev, uint8_t reg, uint8_t data) {
	ensure_register_accessible(dev, reg);
	command(dev, 0x80 | (reg & ENC_REGISTERMASK), data);
}
void enc_BFC(enc_device_t *dev, uint8_t reg, uint8_t data) {
	ensure_register_accessible(dev, reg);
	command(dev, 0xa0 | (reg & ENC_REGISTERMASK), data);
}

void enc_RBM(enc_device_t *dev, uint8_t *dest, uint16_t start, uint16_t length)
{
	if (start != ENC_READLOCATION_ANY)
		set_erdpt(dev, start);

	enchw_select(HWDEV);
	enchw_exchangebyte(HWDEV, 0x3a);
	while(length--)
		*(dest++) = enchw_exchangebyte(HWDEV, 0);
	enchw_unselect(HWDEV);

	/* returning to 0 as ERXST has to be 0 according to errata */
	dev->rdpt = (dev->rdpt + length) % (dev->rxbufsize);
}

static void WBM_raw(enc_device_t *dev, uint8_t *src, uint16_t length)
{
	enchw_select(HWDEV);
	enchw_exchangebyte(HWDEV, 0x7a);
	while(length--)
		enchw_exchangebyte(HWDEV, *(src++));
	enchw_unselect(HWDEV);
	/** @todo this is actually just triggering another pause */
	enchw_unselect(HWDEV);
}

void enc_WBM(enc_device_t *dev, uint8_t *src, uint16_t start, uint16_t length)
{
	enc_WCR16(dev, ENC_EWRPTL, start);

	WBM_raw(dev, src, length);
}

/** 16-bit register read. This only applies to ENC28J60 registers whose low
 * byte is at an even offset and whose high byte is one above that. Can be
 * passed either L or H sub-register.
 *
 * @todo could use enc_register16_t
 * */
uint16_t enc_RCR16(enc_device_t *dev, enc_register_t reg) {
	return (enc_RCR(dev, reg|1) << 8) | enc_RCR(dev, reg&~1);
}
/** 16-bit register write. Compare enc_RCR16. Writes the lower byte first, then
 * the higher, as required for the MII interfaces as well as for ERXRDPT. */
void enc_WCR16(enc_device_t *dev, uint8_t reg, uint16_t data) {
	enc_WCR(dev, reg&~1, data & 0xff); enc_WCR(dev, reg|1, data >> 8);
}

void enc_SRC(enc_device_t *dev) {
	enchw_exchangebyte(HWDEV, 0xff);
}

/** Wait for the ENC28J60 clock to be ready. Returns 0 on success,
 * and an unspecified non-zero integer on timeout. */
int enc_wait(enc_device_t *dev)
{
	int i = 0;
	while (!(enc_RCR(dev, ENC_ESTAT) & ENC_ESTAT_CLKRDY))
		/** @todo as soon as we need a clock somewhere else, make this
		 * time and not iteration based */
		if (i++ == 100000) return 1;
	return 0;
}

uint16_t enc_MII_read(enc_device_t *dev, enc_register_t mireg)
{
	uint16_t result = 0;

	enc_WCR(dev, ENC_MIREGADR, mireg);
	enc_BFS(dev, ENC_MICMD, ENC_MICMD_MIIRD);

	while(enc_RCR(dev, ENC_MISTAT) & ENC_MISTAT_BUSY);

	result = enc_RCR16(dev, ENC_MIRDL);

	enc_BFC(dev, ENC_MICMD, ENC_MICMD_MIIRD);

	return result;
}

void enc_MII_write(enc_device_t *dev, uint8_t mireg, uint16_t data)
{
	while(enc_RCR(dev, ENC_MISTAT) & ENC_MISTAT_BUSY);

	enc_WCR(dev, ENC_MIREGADR, mireg);
	enc_WCR16(dev, ENC_MIWRL, data);
}


void enc_LED_set(enc_device_t *dev, enc_lcfg_t ledconfig, enc_led_t led)
{
	uint16_t state;
	state = enc_MII_read(dev, ENC_PHLCON);
	state = (state & ~(ENC_LCFG_MASK << led)) | (ledconfig << led);
	enc_MII_write(dev, ENC_PHLCON, state);
}

/** Configure the ENC28J60 for network operation, whose initial parameters get
 * passed as well. */
void enc_ethernet_setup(enc_device_t *dev, uint16_t rxbufsize, uint8_t mac[6])
{
	/* practical consideration: we don't come out of clean reset, better do
	 * this -- discard all previous packages */

	enc_BFS(dev, ENC_ECON1, ENC_ECON1_TXRST | ENC_ECON1_RXRST);
	while(enc_RCR(dev, ENC_EPKTCNT))
	{
		enc_BFS(dev, ENC_ECON2, ENC_ECON2_PKTDEC);
	}
	enc_BFC(dev, ENC_ECON1, ENC_ECON1_TXRST | ENC_ECON1_RXRST); /** @todo this should happen later, but when i don't do it here, things won't come up again. probably a problem in the startup sequence. */

	/********* receive buffer setup according to 6.1 ********/

	enc_WCR16(dev, ENC_ERXSTL, 0); /* see errata, must be 0 */
	set_erxnd(dev, rxbufsize);
	enc_WCR16(dev, ENC_ERXRDPTL, 0);

	dev->next_frame_location = 0;

	/******** for the moment, the receive filters are good as they are (6.3) ******/

	/******** waiting for ost (6.4) already happened in _setup ******/

	/******** mac initialization acording to 6.5 ************/

	/* enable reception and flow control (shouldn't hurt in simplex either) */
	enc_BFS(dev, ENC_MACON1, ENC_MACON1_MARXEN | ENC_MACON1_TXPAUS | ENC_MACON1_RXPAUS);

	/* generate checksums for outgoing frames and manage padding automatically */
	enc_WCR(dev, ENC_MACON3, ENC_MACON3_TXCRCEN | ENC_MACON3_FULLPADDING | ENC_MACON3_FRMLEN);

	/* setting defer is mandatory for 802.3, but it seems the default is reasonable too */

	/* MAMXF has reasonable default */

	/* it's not documented in detail what these do, just how to program them */
	enc_WCR(dev, ENC_MAIPGL, 0x12);
	enc_WCR(dev, ENC_MAIPGH, 0x0C);

	/* MACLCON registers have reasonable defaults */

	/* set the mac address */
	enc_WCR(dev, ENC_MAADR1, mac[0]);
	enc_WCR(dev, ENC_MAADR2, mac[1]);
	enc_WCR(dev, ENC_MAADR3, mac[2]);
	enc_WCR(dev, ENC_MAADR4, mac[3]);
	enc_WCR(dev, ENC_MAADR5, mac[4]);
	enc_WCR(dev, ENC_MAADR6, mac[5]);

	/*************** enabling reception as per 7.2 ***********/

	/* enable reception */
	enc_BFS(dev, ENC_ECON1, ENC_ECON1_RXEN);

	/* pull transmitter and receiver out of reset */
	enc_BFC(dev, ENC_ECON1, ENC_ECON1_TXRST | ENC_ECON1_RXRST);
}

/* Partial function of enc_transmit. Always call this as transmit_start /
 * {transmit_partial * n} / transmit_end -- and use enc_transmit or
 * enc_transmit_pbuf unless you're just implementing those two */
void transmit_start(enc_device_t *dev)
{
	/* according to section 7.1 */
	uint8_t control_byte = 0; /* no overrides */

	/* 1. */
	/** @todo we only send a single frame blockingly, starting at the end of rxbuf */
	enc_WCR16(dev, ENC_ETXSTL, dev->rxbufsize);
	/* 2. */
	enc_WBM(dev, &control_byte, dev->rxbufsize, 1);
}

void transmit_partial(enc_device_t *dev, uint8_t *data, uint16_t length)
{
	WBM_raw(dev, data, length);
}

void transmit_end(enc_device_t *dev, uint16_t length)
{
	/* calculate checksum */

//	enc_WCR16(dev, ENC_EDMASTL, start + 1);
//	enc_WCR16(dev, ENC_EDMANDL, start + 1 + length - 3);
//	enc_BFS(dev, ENC_ECON1, ENC_ECON1_CSUMEN | ENC_ECON1_DMAST);
//	while (enc_RCR(dev, ENC_ECON1) & ENC_ECON1_DMAST);
//	uint16_t checksum = enc_RCR16(dev, ENC_EDMACSL);
//	checksum = ((checksum & 0xff) << 8) | (checksum >> 8);
//	enc_WBM(dev, &checksum, start + 1 + length - 2, 2);

	/* 3. */
	enc_WCR16(dev, ENC_ETXNDL, dev->rxbufsize+1+length-1);
	
	/* 4. */
	/* skipped because not using interrupts yet */
	/* 5. */
	enc_BFS(dev, ENC_ECON1, ENC_ECON1_TXRTS);

	/* block */
	while (enc_RCR(dev, ENC_ECON1) & ENC_ECON1_TXRTS);

	uint8_t result[7];
	enc_RBM(dev, result, dev->rxbufsize + 1 + length, 7);
	DEBUG("transmitted. %02x %02x %02x %02x %02x %02x %02x\n", result[0], result[1], result[2], result[3], result[4], result[5], result[6]);

	/** @todo parse that and return reasonable state */
}

void enc_transmit(enc_device_t *dev, uint8_t *data, uint16_t length)
{
	/** @todo check buffer size */
	transmit_start(dev);
	transmit_partial(dev, data, length);
	transmit_end(dev, length);
}

#ifdef ENC28J60_USE_PBUF
/** Like enc_transmit, but read from a pbuf. This is not a trivial wrapper
 * around enc_transmit as the pbuf is not guaranteed to have a contiguous
 * memory region to be transmitted. */
void enc_transmit_pbuf(enc_device_t *dev, struct pbuf *buf)
{
	uint16_t length = buf->tot_len;

	/** @todo check buffer size */
	transmit_start(dev);
	while(1) {
		transmit_partial(dev, buf->payload, buf->len);
		if (buf->len == buf->tot_len)
			break;
		buf = buf->next;
	}
	transmit_end(dev, length);
}
#endif

void receive_start(enc_device_t *dev, uint8_t header[6], uint16_t *length)
{
	enc_RBM(dev, header, dev->next_frame_location, 6);
	*length = header[2] | ((header[3] & 0x7f) << 8);
}

void receive_end(enc_device_t *dev, uint8_t header[6])
{
	dev->next_frame_location = header[0] + (header[1] << 8);

	/* workaround for 80349c.pdf (errata) #14 start.
	 *
	 * originally, this would have been
	 * enc_WCR16(dev, ENC_ERXRDPTL, next_location);
	 * but thus: */
	if (dev->next_frame_location == /* enc_RCR16(dev, ENC_ERXSTL) can be simplified because of errata item #5 */ 0)
		enc_WCR16(dev, ENC_ERXRDPTL, enc_RCR16(dev, ENC_ERXNDL));
	else
		enc_WCR16(dev, ENC_ERXRDPTL, dev->next_frame_location - 1);
	/* workaround end */

	DEBUG("before %d, ", enc_RCR(dev, ENC_EPKTCNT));
	enc_BFS(dev, ENC_ECON2, ENC_ECON2_PKTDEC);
	DEBUG("after %d.\n", enc_RCR(dev, ENC_EPKTCNT));

	DEBUG("read with header (%02x %02x) %02x %02x %02x %02x.\n", header[1], /* swapped due to endianness -- i want to read 1234 */ header[0], header[2], header[3], header[4], header[5]);
}

/** Read a received frame into data; may only be called when one is
 * available. Writes up to maxlength bytes and returns the total length of the
 * frame. (If the return value is > maxlength, parts of the frame were
 * discarded.) */
uint16_t enc_read_received(enc_device_t *dev, uint8_t *data, uint16_t maxlength)
{
	uint8_t header[6];
	uint16_t length;

	receive_start(dev, header, &length);

	if (length > maxlength)
	{
		enc_RBM(dev, data, ENC_READLOCATION_ANY, maxlength);
		DEBUG("discarding some bytes\n");
		/** @todo should that really be accepted at all? */
	} else {
		enc_RBM(dev, data, ENC_READLOCATION_ANY, length);
	}

	receive_end(dev, header);

	return length;
}

#ifdef ENC28J60_USE_PBUF
/** Like enc_read_received, but allocate a pbuf buf. Returns 0 on success, or
 * unspecified non-zero values on errors. */
int enc_read_received_pbuf(enc_device_t *dev, struct pbuf **buf)
{
	uint8_t header[6];
	uint16_t length;

	if (*buf != NULL)
		return 1;

	receive_start(dev, header, &length);
	length -= 4; /* Drop the 4 byte CRC from length */

	/* pbuf processing looks at the total length of pbuf */
	*buf = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
	if (*buf == NULL)
		DEBUG("failed to allocate buf of length %u, discarding", length);
	else
		enc_RBM(dev, (*buf)->payload, ENC_READLOCATION_ANY, length);

	receive_end(dev, header);

	return (*buf == NULL) ? 2 : 0;
}
#endif
