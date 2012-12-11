#include "enc28j60-consts.h"

void enc_setup(void);
uint8_t enc_bist(void);
uint8_t enc_RCR(enc_register_t reg);
uint16_t enc_RCR16(enc_register_t reg);
void enc_WCR(uint8_t reg, uint8_t data);
void enc_WCR16(uint8_t reg, uint16_t data);
void enc_BFS(uint8_t reg, uint8_t data);
void enc_BFC(uint8_t reg, uint8_t data);
void enc_SRC(void);
void enc_wait(void);
uint16_t enc_MII_read(enc_register_t mireg);
void enc_MII_write(uint8_t mireg, uint16_t data);
