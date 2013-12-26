#include <stdint.h>
#include <asf.h>

typedef struct {
	struct spi_module* pmaster;  // pointer to master SPI, can be shared with other SPI slaves
	// FIXME: we need to lock the pmaster
	struct spi_slave_inst slave; // slave ENC28J60 SPI device, 
} enchw_device_t;

void enchw_setup(enchw_device_t *dev);
void enchw_select(enchw_device_t *dev);
void enchw_unselect(enchw_device_t *dev);
uint8_t enchw_exchangebyte(enchw_device_t *dev, uint8_t byte);
