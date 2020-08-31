#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip-debug.h"
#include "net/ip/uiplib.h"
#include "net/ipv6/uip-icmp6.h"
#include <string.h>
#include <stdio.h>
#include "net/commlatency.h"

#define SERVER_IP             "fe80::9a07:2dff:fe3c:8d01" //"::"
#define CLIENT_PORT           61617
#define SERVER_PORT           61616
#define PING_TIMEOUT              (CLOCK_SECOND / 4)
#define CLIENT_SEND_INTERVAL      (CLOCK_SECOND * 2.5)
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t server_addr;
static struct uip_icmp6_echo_reply_notification icmp_notification;
static uint8_t echo_received;
//static struct uip_udp_conn *conn;
static struct etimer timer;

static int i;
/*---------------------------------------------------------------------------*/
PROCESS(createConnection, "Creating Connection");
PROCESS(sendingMessage, "Sending Message");
AUTOSTART_PROCESSES(&createConnection);
/*---------------------------------------------------------------------------*/
void icmp_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  printf("echo response received\n");
  echo_received = 1;
}
void timeout_handler(void)
{
	char c[] = "TEST MESSAGE";
	send_message(conn, c, strlen(c));
	printf("A new message is sent\n");
}
PROCESS_THREAD(createConnection, ev, data)
{
	PROCESS_BEGIN();
	uiplib_ipaddrconv(SERVER_IP, &server_addr);
	uip_icmp6_echo_reply_callback_add(&icmp_notification, icmp_reply_handler);
	do 
	{
		etimer_set(&timer, PING_TIMEOUT);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		uip_icmp6_send(&server_addr, ICMP6_ECHO_REQUEST, 0, 20);		
	} while(!echo_received);
	conn = udp_new(&server_addr, UIP_HTONS(SERVER_PORT), NULL);
	udp_bind(conn, UIP_HTONS(CLIENT_PORT));
	/*for (i=0;i<10;i++)
	{
		etimer_set(&timer, CLIENT_SEND_INTERVAL);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		printf("Loop count is %d\n",i);
		timeout_handler();
	}*/
	printf(" local/remote port %u/%u\n",UIP_HTONS(conn->lport), UIP_HTONS(conn->rport));		
	process_start(&sendingMessage,NULL);	
	PROCESS_END();
}
PROCESS_THREAD(sendingMessage, ev, data)
{
	PROCESS_BEGIN();
	printf("In second process\n");
	printf(" local/remote port %u/%u\n",UIP_HTONS(conn->lport), UIP_HTONS(conn->rport));
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	timeout_handler();
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
