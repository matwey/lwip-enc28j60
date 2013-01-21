#include <netif/mchdrv.h>
#include <lwip/pbuf.h>
#include <netif/etharp.h>
#include "enc28j60.h"

uint8_t mac_addr[6] = {0x02 /* u/l, local */, 0x04, 0xA3, /* microchip's oui as per manual is 00:04:a3 */
	0x11, 0x22, 0x33};

enc_device_t encdevice;

void mchdrv_poll(struct netif *netif) {
	int receivedlength;
	err_t result;
	struct pbuf *buf = NULL;

	uint8_t epktcnt;

	epktcnt = enc_RCR(&encdevice, ENC_EPKTCNT);

	if (epktcnt) {
		enc_read_received_pbuf(&encdevice, &buf);
		log_message("incoming: %d packages, first read into %x\n", epktcnt, (unsigned int)(buf));
		result = netif->input(buf, netif);
		log_message("received with result %d\n", result);
	}
}

static err_t mchdrv_linkoutput(struct netif *netif, struct pbuf *p)
{
	enc_transmit_pbuf(&encdevice, p);
	log_message("sent %d bytes.\n", p->tot_len);
	/* FIXME: evaluate result state */
	return ERR_OK;
}

err_t mchdrv_init(struct netif *netif) {
	int result;

	log_message("Starting mchdrv_init.\n");

	result = enc_setup_basic(&encdevice);
	if (result != 0)
	{
		log_message("Error %d in enc_setup, interface setup aborted.\n", result);
		return ERR_IF;
	}
	result = enc_bist_manual(&encdevice);
	if (result != 0)
	{
		log_message("Error %d in enc_bist_manual, interface setup aborted.\n", result);
		return ERR_IF;
	}
	enc_ethernet_setup(&encdevice, 4*1024, mac_addr);

	netif->state = &encdevice; /* is this how it's supposed to be used? */
	netif->output = etharp_output;
	netif->linkoutput = mchdrv_linkoutput;

	netif->hwaddr_len = 6;
	memcpy(netif->hwaddr, mac_addr, 6);

	netif->flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	log_message("Driver initialized.\n");

	return ERR_OK;
}

/** Temporary helper function: Sometimes, the network interface half eats packages. This will test how to reset from that condition. */
void mchdrv_reset(void) {
	enc_ethernet_setup(&encdevice, 4*1024, mac_addr);
}
