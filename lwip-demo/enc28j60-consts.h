typedef enum {
	/* the top 3 bits are used to indicate the bank. lowest bit means
	 * explicit bank, 110 means "present in all banks", 100 mean "don't
	 * know". thus, right-shifting by 6 gives bank number. */
	ENC_BANKMASK = 0xe0,

	ENC_BANKALL = 0xc0,
	ENC_BANK0 = 0x20,
	ENC_BANK1 = 0x60,
	ENC_BANK2 = 0xa0,
	ENC_BANK3 = 0xe0,
	/* initial value used when optimizing away bank changes */
	ENC_BANK_INDETERMINATE = 0x80,

	ENC_REGISTERMASK = 0x1f,

	/* actual registers start here */

	ENC_EIE = 0x1b | ENC_BANKALL,
	ENC_EIR = 0x1c | ENC_BANKALL,
	ENC_ESTAT = 0x1d | ENC_BANKALL,
	ENC_ECON2 = 0x1f | ENC_BANKALL,
	ENC_ECON1 = 0x1f | ENC_BANKALL,

	ENC_MACON2 = 0x01 | ENC_BANK2,
	ENC_MICMD = 0x12 | ENC_BANK2,
	ENC_MIREGADR = 0x14 | ENC_BANK2,
	ENC_MIWRL = 0x16 | ENC_BANK2,
	ENC_MIWRH = 0x17 | ENC_BANK2,
	ENC_MIRDL = 0x18 | ENC_BANK2,
	ENC_MIRDH = 0x19 | ENC_BANK2,

	ENC_MISTAT = 0x0a | ENC_BANK3,
} enc_register_t;

/* regular register flags */
#define ENC_ESTAT_CLKRDY 0x01
#define ENC_MACON2_MARST 0x80
#define ENC_MICMD_MIIRD 1
#define ENC_MISTAT_BUSY 1

/* mii registers */
typedef enum {
	ENC_PHCON1 = 0x00,
	ENC_PHSTAT1 = 0x01,
	ENC_PHID1 = 0x02,
	ENC_PHID2 = 0x03,
	ENC_PHCON2 = 0x10,
	ENC_PHSTAT2 = 0x11,
	ENC_PHIE = 0x12,
	ENC_PHIR = 0x13,
	ENC_PHLCON = 0x14,
} enc_phreg_t;

#define ENC_LCFG_ON 0x8
#define ENC_LCFG_OFF 0x9
#define ENC_LCFG_BLINKFAST 0xa
#define ENC_LCFG_BLINKSLOW 0xb

#define ENC_LACFG_OFFSET 8
#define ENC_LBCFG_OFFSET 4
