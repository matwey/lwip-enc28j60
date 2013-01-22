#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
#include <lwip/init.h>
#include <lwip/stats.h>
#include <netif/etharp.h>

#include <netif/mchdrv.h>

#include <board.h>

struct ip_addr mch_myip_addr = {0x0200a8c0UL}; /* 192.168.0.2 */
struct ip_addr gw_addr = {0x0100a8c0UL}, netmask = {0x000000ffUL}; /* 192.168.0.1 */

static struct netif mchdrv_netif;

void mch_net_init(void)
{
    // Initialize LWIP
    lwip_init();

    // Add our netif to LWIP (netif_add calls our driver initialization function)
    if (netif_add(&mchdrv_netif, &mch_myip_addr, &netmask, &gw_addr, NULL,
                mchdrv_init, ethernet_input) == NULL) {
        LWIP_ASSERT("mch_net_init: netif_add (mchdrv_init) failed\n", 0);
    }

    netif_set_default(&mchdrv_netif);
    netif_set_up(&mchdrv_netif);
}

void mch_net_poll(void)
{
    mchdrv_poll(&mchdrv_netif);
}

int main(void)
{
    board_setup();

    mch_net_init();

    testapp_init();

    while (1) {
        mch_net_poll();
    }
}
