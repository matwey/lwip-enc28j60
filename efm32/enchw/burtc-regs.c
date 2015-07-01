#include "burtc-regs.h"

#include <stddef.h>

#include <em_burtc.h>

#define REG(index) BURTC->RET[index].REG

static uint32_t regs_checksum(uint8_t n, uint32_t *data)
{
	uint32_t result = 0;
	while (n--) result ^= *(data++);
	return ~result; /* negate to avoid empty memory being valid */
}

static bool regs_valid(uint8_t index, uint8_t n)
{
	return regs_checksum(n, &(REG(index))) == REG(index + n);
}

void burtc_regs_store(uint8_t index, uint8_t n, uint32_t *value)
{
	uint32_t *writecursor;
	uint32_t *destroyme;
	if (regs_valid(index, n)) {
		writecursor = &(REG(index + n + 1));
		destroyme = &(REG(index + n));
	} else {
		writecursor = &(REG(index));
		destroyme = NULL;
	}
	uint32_t checksum = regs_checksum(n, value);
	while (n--) *(writecursor++) = *(value++);
	*writecursor = checksum;
	if (destroyme != NULL)
		*destroyme += 1;
}

bool burtc_regs_retrieve(uint8_t index, uint8_t n, uint32_t *value)
{
	uint32_t *readcursor = 0;
	if (regs_valid(index, n)) {
		readcursor = &(REG(index));
	} else if (regs_valid(index + n + 1, n)) {
		readcursor = &(REG(index + n + 1));
	}
	if (readcursor == 0) return false;
	while (n--) *(value++) = *(readcursor++);
	return true;
}
