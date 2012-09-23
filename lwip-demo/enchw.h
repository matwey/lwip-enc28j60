#include <stdint.h>

void enchw_setup(void);

/* everything after here should rather be in an own enc module */

/* all pages */
#define ENC_ESTAT 0x1d
#define ENC_ESTAT_CLKRDY 0x01
#define ENC_ECON1 0x1f

/* page 2 */
#define ENC_MACON2 0x01
#define ENC_MACON2_MARST 0x80

#define ENC_MICMD 0x12
#define ENC_MICMD_MIIRD 1
#define ENC_MIREGADR 0x14
#define ENC_MIWRL 0x16
#define ENC_MIWRH 0x17
#define ENC_MIRDL 0x18
#define ENC_MIRDH 0x19

/* page 3 */
#define ENC_MISTAT 0x0a
#define ENC_MISTAT_BUSY 1

/* mii registers */
#define ENC_PHLCON 0x14
#define ENC_PHCON1 0x00
#define ENC_PHCON2 0x01
#define ENC_PHID1 0x02
#define ENC_PHID2 0x03

#define ENC_LCFG_ON 0x8
#define ENC_LCFG_OFF 0x9
#define ENC_LCFG_BLINKFAST 0xa
#define ENC_LCFG_BLINKSLOW 0xb

#define ENC_LACFG_OFFSET 8
#define ENC_LBCFG_OFFSET 4

uint8_t enc_RCR(uint8_t reg);
void enc_WCR(uint8_t reg, uint8_t data);
void enc_BFS(uint8_t reg, uint8_t data);
void enc_BFC(uint8_t reg, uint8_t data);
void enc_SC(void);
void enc_select_page(uint8_t page);
void enc_wait(void);
uint16_t enc_MII_read(uint8_t mireg);
void enc_MII_write(uint8_t mireg, uint16_t data);
