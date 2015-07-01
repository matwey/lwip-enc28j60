/**
 * @addtogroup burtc-regs BURTC retention register access tools
 * @{
 *
 * This module provides an interface for storing data in the BURTC retention
 * registers of EFM32 devices in a way that data (even more than one register)
 * can be stored atomically. After returning from backup power domain operation
 * during a write access, eithe the original value is read, or the new one.
 *
 * At powerup, memory is typically invalid (unless the unspecified reset state
 * happens to match the 32bit checksum).
 *
 * Data is accessed as groups of 32bit registers, which are described by index
 * (number of first register) and length (number of registers). It is stored
 * double buffered with a one register sized checksum for each buffer, and a
 * slice at index I with length L occupies the register from I to I + (L + 1) *
 * 2 - 1 inclusive. As that memory is not managed by the linker, it is up to
 * the user to take care to coordinate use of the indices.
 *
 * Note that for access to the registers, the CORELE clock domain has to be
 * enabled, and its reset condition BURSTEN has to be removed.
 *
 */

#include <stdint.h>
#include <stdbool.h>

/** Copy @p length words from @p value into the retention registers @p index
 * (consuming memory up to excluding index + 2 * (length + 1). */
void burtc_regs_store(uint8_t index, uint8_t length, uint32_t *value);

/** Read @p length words from retention registers @p index into the prepared
 * buffer at @p value.
 *
 * Returns true if the data is valid, otherwise false (and does not change data
 * in @p value) */
bool burtc_regs_retrieve(uint8_t index, uint8_t length, uint32_t *value);

/** @} */
