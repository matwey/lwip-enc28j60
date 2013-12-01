#include <lwip/inet.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
#include <lwip/init.h>
#include <lwip/stats.h>
#include <lwip/timers.h>
#include <netif/etharp.h>

#include <netif/mchdrv.h>

#include <rtc.h>
#include <board.h>
#include <enc28j60.h>

#include <testapp.h>

struct ip_addr mch_myip_addr = {0x0200a8c0UL}; /* 192.168.0.2 */
struct ip_addr gw_addr = {0x0100a8c0UL}, netmask = {0x000000ffUL}; /* 192.168.0.1 */

static struct netif mchdrv_netif;

static enc_device_t mchdrv_hw;

void mch_net_init(void)
{
    // Initialize LWIP
    lwip_init();

    mchdrv_netif.hwaddr_len = 6;
    /* demo mac address */
    mchdrv_netif.hwaddr[0] = 0;
    mchdrv_netif.hwaddr[1] = 1;
    mchdrv_netif.hwaddr[2] = 2;
    mchdrv_netif.hwaddr[3] = 3;
    mchdrv_netif.hwaddr[4] = 4;
    mchdrv_netif.hwaddr[5] = 5;

    // Add our netif to LWIP (netif_add calls our driver initialization function)
    if (netif_add(&mchdrv_netif, &mch_myip_addr, &netmask, &gw_addr, &mchdrv_hw,
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

uint32_t sys_now(void)
{
	/* assuming the 512Hz implemenetation of the provided rtc, and that 2.4% error are ok for lwip's sys_now */
	return rtc_get32() * 2;
}

int main(void)
{
    rtc_setup();

    board_setup();

    mch_net_init();

    testapp_init();

    while (1) {
        mch_net_poll();
        sys_check_timeouts();
    }
}
