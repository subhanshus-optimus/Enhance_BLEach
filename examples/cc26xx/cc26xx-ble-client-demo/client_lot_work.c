
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/commlatency.h"
#include "aon_batmon.h"


#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"
#include "net/ip/uiplib.h"
#include "net/ipv6/uip-icmp6.h"

#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
//#define SERVER_IP               "::"

#define SERVER_IP             "fe80::9a07:2dff:fe3c:8d01" //"::"

#define CLIENT_PORT           61617
#define SERVER_PORT           61616

#define PING_TIMEOUT              (CLOCK_SECOND / 4)
#define CLIENT_SEND_INTERVAL      (CLOCK_SECOND * 2.5)
#define PROBE_SIGNAL_INTERVAL 	  (CLOCK_SECOND * 27)

#define UDP_LEN_MAX           255
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t server_addr;
static struct uip_icmp6_echo_reply_notification icmp_notification;
static uint8_t echo_received;
static struct uip_udp_conn *conn;

static struct etimer timer,probe_timer,clientTimer;
static struct timer probeTimer;
static char buf[UDP_LEN_MAX];
static uint16_t packet_counter;
static uint16_t probe_packet_counter;
static uint16_t actualSent_packet_counter;
static uint16_t temperature;
static int flag;

static char buf[UDP_LEN_MAX];
static char probeBuf[512];
//static char newBuf[512];
static char receivedPacketCount[]="1111";
static char bufNew[5];

clock_time_t start,probe_start;

static char *found, *found2;

/*---------------------------------------------------------------------------*/
PROCESS(createConnection, "Creating Connection");
PROCESS(sendingMessage, "Sending Message");
PROCESS(calculateLatency, "Calculate latency");
AUTOSTART_PROCESSES(&createConnection);
/*---------------------------------------------------------------------------*/
void icmp_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  printf("echo response received\n");
  echo_received = 1;
}
char* newTrafficGenerator(void)
{

	int i;
	int count =0;
	static char currentTrafficType[6];
	i = random_rand();
	if (i>0 && i<32767)
	{
		strncpy(currentTrafficType,"Type A",sizeof(currentTrafficType));
	}
	else if(i>32767 && i<55705)
	{
		strncpy(currentTrafficType,"Type B",sizeof(currentTrafficType));
	}
	else
	{
		strncpy(currentTrafficType,"Type C",sizeof(currentTrafficType));
	}
	
	return currentTrafficType;
}




/*---------------------------------------------------------------------------*/

static void tcpip_handler(void)
{
	memcpy(receivedPacketCount,uip_appdata,4);
	sprintf(bufNew, "%04d", actualSent_packet_counter);	
	bufNew[5]='\0';
	printf("Received packet count is: %s and sent packet count is: %s\n",receivedPacketCount,bufNew);
	if (strcmp(receivedPacketCount,bufNew)==0)		/* If sent packet count and received packet count same, calcluate latency */
	{
		float b1 = (float)((int) (clock_time()-probe_start))*1.000;
		float b2 = (float) ((int)CLOCK_SECOND)*1.000;
		float f = b1 / b2;
		printf("Round trip delay is: %ld.%03u seconds for packet count:- %s\n",(long)f
		,(unsigned)((f-floor(f))*1000),receivedPacketCount);
		roundTrip = f;
	}
	else
	{
		printf ("\nProbe packet is dropped");
	}
}
/*---------------------------------------------------------------------------*/
static void timeout_handler(void)
{
	char *c;
	if(AONBatMonNewBatteryMeasureReady()==1)
	{
		temperature = AONBatMonTemperatureGetDegC();
	}
	
	c = newTrafficGenerator();	
	//sprintf(buf, "Current temperature is  %02u and current packet count is: %04u!", temperature,packet_counter);g m
	send_message(conn, c, strlen(c));
	printf("A new message is sent\n");
	packet_counter++;
}


static void probe_signal(void)
{	
	sprintf(probeBuf, "%04u", probe_packet_counter);
	actualSent_packet_counter =probe_packet_counter;
	probe_start = clock_time();
	send_message(conn, probeBuf, strlen(probeBuf));
	printf("Probe signal is sent having packet count:- %d\n",probe_packet_counter);
	printf("Current connLatency is %d and slaveLatency is %d\n",current_conn_interval,current_slave_latency);
	probe_packet_counter++;
}

/*---------------------------------------------------------------------------*/
/******************* Main Process ********************************/

PROCESS_THREAD(createConnection, ev, data)
{
	PROCESS_BEGIN();
	start = clock_time();
	printf("Creating connection process started*****\n");
	uiplib_ipaddrconv(SERVER_IP, &server_addr);
	uip_icmp6_echo_reply_callback_add(&icmp_notification, icmp_reply_handler);
	PRINTF("pinging the IPv6-over-BLE server: ");
	PRINT6ADDR(&server_addr);
	PRINTF("\n");
	timer_set(&probeTimer, PING_TIMEOUT);
	do 
	{
		
		etimer_set(&timer, PING_TIMEOUT);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		uip_icmp6_send(&server_addr, ICMP6_ECHO_REQUEST, 0, 20);
		
		/*if(timer_expired(&probeTimer)) 
		{
     			uip_icmp6_send(&server_addr, ICMP6_ECHO_REQUEST, 0, 20);
			timer_restart(&probeTimer);
   		}*/		
	} while(!echo_received);
	float b1 = (float)((int) (clock_time()-start))*1.000;
	float b2 = (float) ((int)CLOCK_SECOND)*1.000;
	float f = b1 / b2;
	printf("\nTime taken in receiving ping response is: %ld.%03u seconds\n",(long)f
	,(unsigned)((f-floor(f))*1000));
	printf("Creating UDP connection\n");
	conn = udp_new(&server_addr, UIP_HTONS(SERVER_PORT), NULL);
	udp_bind(conn, UIP_HTONS(CLIENT_PORT));
	PRINTF("Creating connection process completed********\n");
	etimer_set(&timer, CLIENT_SEND_INTERVAL);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	timeout_handler();
	etimer_set(&timer, CLIENT_SEND_INTERVAL);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	printf("Test message is sent from original process\n");	
	process_start(&sendingMessage,NULL);	
	PROCESS_END();
}




PROCESS_THREAD(sendingMessage, ev, data)
{
	PROCESS_BEGIN();
	process_start(&calculateLatency,NULL);
	printf("In sendingMessage process\n");	
	etimer_set(&clientTimer, CLIENT_SEND_INTERVAL);
	while(packet_counter< 1204)
	{		
		if((ev == PROCESS_EVENT_TIMER) && (data == &clientTimer)) 
		{
			timeout_handler();
			etimer_set(&clientTimer, CLIENT_SEND_INTERVAL);
		}
		
		process_poll(&calculateLatency);
		PROCESS_YIELD();
	}
	PROCESS_END();
}


PROCESS_THREAD(calculateLatency, ev, data)
{
	PROCESS_BEGIN();
	printf("In calulateDelay process\n");
	etimer_set(&probe_timer, PROBE_SIGNAL_INTERVAL); 
	while(1)
	{
		if((ev == PROCESS_EVENT_TIMER) && (data == &probe_timer)) 
		{
			probe_signal();
			etimer_set(&probe_timer, PROBE_SIGNAL_INTERVAL);			
		}
		else if(ev == tcpip_event) 
		{
			printf("TCPIP event detected\n");
			tcpip_handler();			
		}
		process_poll(&sendingMessage); 
		PROCESS_YIELD();  
	}	
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
