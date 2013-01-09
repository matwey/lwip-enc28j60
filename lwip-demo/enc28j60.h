#include "enc28j60-consts.h"

typedef struct {
	uint16_t rxbufsize;
} enc_operation_t;

void enc_setup(void);
uint8_t enc_bist(void);
uint8_t enc_bist_manual(void);
uint8_t enc_RCR(enc_register_t reg);
uint16_t enc_RCR16(enc_register_t reg);
void enc_WCR(uint8_t reg, uint8_t data);
void enc_WCR16(uint8_t reg, uint16_t data);
void enc_BFS(uint8_t reg, uint8_t data);
void enc_BFC(uint8_t reg, uint8_t data);
void enc_SRC(void);
void enc_RBM(uint8_t *dest, uint16_t start, uint16_t length);
void enc_WBM(uint8_t *src, uint16_t start, uint16_t length);
void enc_wait(void);
uint16_t enc_MII_read(enc_register_t mireg);
void enc_MII_write(uint8_t mireg, uint16_t data);
void enc_LED_set(enc_lcfg_t ledconfig, enc_led_t led);

void enc_operation_setup(enc_operation_t *op, uint16_t rxbufsize, uint8_t mac[6]);
void enc_transmit(enc_operation_t *op, uint8_t *data, uint16_t length);
uint16_t enc_read_received(enc_operation_t *op, uint8_t *data, uint16_t maxlength);

#ifdef ENC28J60_USE_PBUF
#include <lwip/pbuf.h>

void enc_read_received_pbuf(enc_operation_t *op, struct pbuf **buf);
#endif
