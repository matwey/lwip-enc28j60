#include <lwip/raw.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>

#include <board.h>

/** Demo application that will listen to UDP port 1234, respond to everything
 * sent there, and flash a LED when accessed. */

static struct udp_pcb *my_pcb;

static const char message_text[] = "Received a package, flashed LED.\n";

static void
recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, uint16_t port)
{
	struct pbuf *response;
	struct pbuf *message;

	led1_on();

	response = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RAM);
	message = pbuf_alloc(PBUF_RAW, sizeof(message_text), PBUF_ROM);
	message->payload = message_text;
	pbuf_cat(response, message);

	udp_sendto(my_pcb, response, addr, port);

	pbuf_free(response);

	pbuf_free(p);

	led1_off();
}

void testapp_init(void)
{
	my_pcb = udp_new();
	LWIP_ASSERT("my_pcb != NULL", my_pcb != NULL);

	udp_recv(my_pcb, recv, NULL);
	udp_bind(my_pcb, IP_ADDR_ANY, 1234);
}
