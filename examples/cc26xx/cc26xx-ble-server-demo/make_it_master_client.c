
#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"
#include "net/commlatency.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define SLAVE_SERVER_IP		"fe80::9a07:2dff:fe3c:2d00"
#define SLAVE_SERVER_PORT           61617
#define MASTER_CLIENT_PORT           61616

#define PING_TIMEOUT              (CLOCK_SECOND / 4)


#define UDP_LEN_MAX           255
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF    ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])


static struct uip_icmp6_echo_reply_notification icmp_notification;
static uint8_t echo_received;

static struct etimer timer;
static uip_ipaddr_t slave_slave_addr;

static struct uip_udp_conn *master_client_conn;
static char buf[UDP_LEN_MAX];
static char data[513];

/*---------------------------------------------------------------------------*/
PROCESS(ipv6_ble_server_process, "IPv6 over BLE - server process");
AUTOSTART_PROCESSES(&ipv6_ble_server_process);
/*---------------------------------------------------------------------------*/

static void tcpip_handler(void)
{		
	if(uip_newdata()) 
	{
		//printf("First Character is %c",*((char *)uip_appdata));
		if (uip_datalen()==1)    					/* Probe packet is received */
		{
			printf("Probe signal is received\n");					
			strncpy(data, uip_appdata, uip_datalen());
			data[uip_datalen()] = '\0';
			send_probe(master_client_conn, data, strlen(data)); 	/* Resending probe packet */
		}		
		else							/* Regular message is received */					
		{			
			strncpy(buf, uip_appdata, uip_datalen());			
			buf[uip_datalen()] = '\0';
			PRINTF("rec. message: %s\n", buf); 		/* Displaying received message */				
		}		
	  }
}


void icmp_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  printf("echo response received\n");
  echo_received = 1;
}



/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ipv6_ble_server_process, ev, data)
{
	PROCESS_BEGIN();
	PRINTF("CC26XX-IPv6-over-BLE client-master started\n");
	PRINTF("pinging the IPv6-over-BLE server-slave: ");
	
	
	uiplib_ipaddrconv(SLAVE_SERVER_IP, &slave_slave_addr);
	uip_icmp6_echo_reply_callback_add(&icmp_notification, icmp_reply_handler);
	do 
	{
		etimer_set(&timer, PING_TIMEOUT);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		uip_icmp6_send(&slave_slave_addr, ICMP6_ECHO_REQUEST, 0, 20);
	} while(!echo_received);
	
	printf("Echo response is received\n");
	
	master_client_conn = udp_new(&slave_slave_addr, UIP_HTONS(SLAVE_SERVER_PORT), NULL);
	udp_bind(master_client_conn, UIP_HTONS(MASTER_CLIENT_PORT));
	

	printf("Connection is created\n");
	while(1) 
	{
	PROCESS_WAIT_EVENT();
		if(ev == tcpip_event) 					/* If any TCPIP event */
		{
			tcpip_handler();
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
