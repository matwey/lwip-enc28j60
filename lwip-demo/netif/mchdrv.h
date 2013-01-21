#ifndef NETIF_MCHDRV_H
#define NETIF_MCHDRV_H

#include <lwip/netif.h>
#include <lwip/err.h>

err_t mchdrv_init(struct netif *netif);
void mchdrv_poll(struct netif *netif);
void mchdrv_reset(void);

#endif
