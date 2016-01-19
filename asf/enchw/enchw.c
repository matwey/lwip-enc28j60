#include "enchw.h"

#include <asf.h>

void enchw_setup(enchw_device_t *dev)
{
	while(spi_lock(dev->pmaster) != STATUS_OK);
	spi_enable(dev->pmaster);
	spi_unlock(dev->pmaster);
}

/***
 * We rely on enc28j60.c behaviour
 * that it always calls our functions in the following order:
 *
 *   1. enchw_select
 *   2. enchw_exchangebyte
 *   3. enchw_unselect
 *
 * So we do spi_lock at enchw_select, and do spi_unlock at enchw_unselect,
 * dev->pmaster in enchw_exchangebyte is locked.
 */

void enchw_select(enchw_device_t *dev)
{
	while(spi_lock(dev->pmaster) != STATUS_OK);
	spi_select_slave(dev->pmaster, &(dev->slave), true);
}

void enchw_unselect(enchw_device_t *dev)
{
	spi_select_slave(dev->pmaster, &(dev->slave), false);
	spi_unlock(dev->pmaster);
}

uint8_t enchw_exchangebyte(enchw_device_t *dev, uint8_t byte)
{
	// due to 9-bit support we have to allocate uint16_t here
	uint16_t rx;
	spi_transceive_wait(dev->pmaster, byte, &rx);
	return rx;
}

