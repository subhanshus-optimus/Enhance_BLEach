
#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"
#include "net/commlatency.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define CLIENT_IP		"fe80::9a07:2dff:fe3c:2d00"
#define CLIENT_PORT           61617
#define SERVER_PORT           61616

#define UDP_LEN_MAX           255
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF    ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static uip_ipaddr_t client_addr;

static struct uip_udp_conn *server_conn;
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
		if (uip_datalen()==4)    					/* Probe packet is received */
		{
			printf("Probe signal is received\n");					
			strncpy(data, uip_appdata, uip_datalen());
			data[uip_datalen()] = '\0';
			send_probe(server_conn, data, strlen(data)); 	/* Resending probe packet */
		}		
		else							/* Regular message is received */					
		{			
			strncpy(buf, uip_appdata, uip_datalen());			
			buf[uip_datalen()] = '\0';
			PRINTF("rec. message: %s\n", buf); 		/* Displaying received message */				
		}		
	  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ipv6_ble_server_process, ev, data)
{
	PROCESS_BEGIN();
	PRINTF("CC26XX-IPv6-over-BLE server started\n");
	uiplib_ipaddrconv(CLIENT_IP, &client_addr);	
	server_conn = udp_new(&client_addr, UIP_HTONS(CLIENT_PORT), NULL);
	udp_bind(server_conn, UIP_HTONS(SERVER_PORT));
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
