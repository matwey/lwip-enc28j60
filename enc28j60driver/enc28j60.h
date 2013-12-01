/**
 *
 * @addtogroup enc28j60 ENC28J60 driver
 *
 * Driver for the MicroChip ENC28J60 Ethernet module.
 *
 * This software requires an implementation of the `enchw_*` interface
 * specified by `enchw.h` (which describes a very basic SPI interface), and
 * provides the various command types as well as read and write access to the
 * ENC28J60 memory.
 *
 * Optionally, support for lwIP's pbuf memory allocation can be compiled in by
 * defining `ENC28J60_USE_PBUF`.
 *
 * @{
 */

#include "enc28j60-consts.h"
#ifdef ENC28J60_USE_PBUF
#include <lwip/pbuf.h>
#endif

/** Container that stores locally cached information about the ENC28J60 device
 * (eg. last used register) to optimize access. */

typedef struct {
	/** The chip's active register page ENC_ECON1[0:1] */
	enc_register_t last_used_register;
	/** Configured receiver buffer size; cached value of of ERXND[H:L] */
	uint16_t rxbufsize;
	/** Read pointer; cached value for ERDPT */
	uint16_t rdpt;

	/** Where to start reading the next received frame */
	uint16_t next_frame_location;

	/** Pointer for the hardware implementation to access device
	 * information */
	void *hwdev;
} enc_device_t;

int enc_setup_basic(enc_device_t *dev);
uint8_t enc_bist(enc_device_t *dev);
uint8_t enc_bist_manual(enc_device_t *dev);
uint8_t enc_RCR(enc_device_t *dev, enc_register_t reg);
uint16_t enc_RCR16(enc_device_t *dev, enc_register_t reg);
void enc_WCR(enc_device_t *dev, uint8_t reg, uint8_t data);
void enc_WCR16(enc_device_t *dev, uint8_t reg, uint16_t data);
void enc_BFS(enc_device_t *dev, uint8_t reg, uint8_t data);
void enc_BFC(enc_device_t *dev, uint8_t reg, uint8_t data);
void enc_SRC(enc_device_t *dev);
void enc_RBM(enc_device_t *dev, uint8_t *dest, uint16_t start, uint16_t length);
void enc_WBM(enc_device_t *dev, uint8_t *src, uint16_t start, uint16_t length);
int enc_wait(enc_device_t *dev);
uint16_t enc_MII_read(enc_device_t *dev, enc_register_t mireg);
void enc_MII_write(enc_device_t *dev, uint8_t mireg, uint16_t data);
void enc_LED_set(enc_device_t *dev, enc_lcfg_t ledconfig, enc_led_t led);

void enc_ethernet_setup(enc_device_t *dev, uint16_t rxbufsize, uint8_t mac[6]);
void enc_transmit(enc_device_t *dev, uint8_t *data, uint16_t length);
uint16_t enc_read_received(enc_device_t *dev, uint8_t *data, uint16_t maxlength);

#ifdef ENC28J60_USE_PBUF
int enc_read_received_pbuf(enc_device_t *dev, struct pbuf **buf);
void enc_transmit_pbuf(enc_device_t *dev, struct pbuf *buf);
#endif

/** @} */
