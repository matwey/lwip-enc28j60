#include <netif/mchdrv.h>
#include <lwip/pbuf.h>
#include "enc28j60.h"

enc_operation_t encop;

err_t mchdrv_init(struct netif *netif) {
	uint8_t mac_addr[6] = {0x02 /* u/l, local */, 0x04, 0xA3, /* microchip's oui as per manual is 00:04:a3 */
		0x11, 0x22, 0x33};

	enc_setup();
	enc_bist_manual();
	enc_operation_setup(&encop, 4*1024, mac_addr);
	return 0;
}
void mchdrv_poll(struct netif *netif) {
	struct pbuf *buffer;
	int receivedlength;
	err_t result;

	if (enc_RCR(ENC_EPKTCNT)) {
		buffer = pbuf_alloc(PBUF_LINK, 100 /* i'll have to change the lower layer to fix that*/, PBUF_RAM);
		if (buffer == NULL)
		{
			log_message("allocation failed\n");
			return;
		}
		receivedlength = enc_read_received(&encop, buffer->payload, 100);
		result = netif->input(buffer, netif);
		log_message("received %d, result %d\n", receivedlength, result);
	}
}
