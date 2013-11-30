#include <netif/mchdrv.h>
#include <lwip/pbuf.h>
#include <netif/etharp.h>
#include "enc28j60.h"

void mchdrv_poll(struct netif *netif) {
	err_t result;
	struct pbuf *buf = NULL;

	uint8_t epktcnt;
	enc_device_t *encdevice = (enc_device_t*)netif->state;

	epktcnt = enc_RCR(encdevice, ENC_EPKTCNT);

	if (epktcnt) {
		if (enc_read_received_pbuf(encdevice, &buf) == 0)
		{
			LWIP_DEBUGF(NETIF_DEBUG, ("incoming: %d packages, first read into %x\n", epktcnt, (unsigned int)(buf)));
			result = netif->input(buf, netif);
			LWIP_DEBUGF(NETIF_DEBUG, ("received with result %d\n", result));
		} else {
			/* FIXME: error reporting */
			LWIP_DEBUGF(NETIF_DEBUG, ("didn't receive.\n"));
		}
	}
}

static err_t mchdrv_linkoutput(struct netif *netif, struct pbuf *p)
{
	enc_device_t *encdevice = (enc_device_t*)netif->state;
	enc_transmit_pbuf(encdevice, p);
	LWIP_DEBUGF(NETIF_DEBUG, ("sent %d bytes.\n", p->tot_len));
	/* FIXME: evaluate result state */
	return ERR_OK;
}

err_t mchdrv_init(struct netif *netif) {
	int result;
	enc_device_t *encdevice = (enc_device_t*)netif->state;

	LWIP_DEBUGF(NETIF_DEBUG, ("Starting mchdrv_init.\n"));

	result = enc_setup_basic(encdevice);
	if (result != 0)
	{
		LWIP_DEBUGF(NETIF_DEBUG, ("Error %d in enc_setup, interface setup aborted.\n", result));
		return ERR_IF;
	}
	result = enc_bist_manual(encdevice);
	if (result != 0)
	{
		LWIP_DEBUGF(NETIF_DEBUG, ("Error %d in enc_bist_manual, interface setup aborted.\n", result));
		return ERR_IF;
	}
	enc_ethernet_setup(encdevice, 4*1024, netif->hwaddr);

	netif->output = etharp_output;
	netif->linkoutput = mchdrv_linkoutput;

	netif->mtu = 1500; /** FIXME check with documentation when jumboframes can be ok */

	netif->flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	LWIP_DEBUGF(NETIF_DEBUG, ("Driver initialized.\n"));

	return ERR_OK;
}
