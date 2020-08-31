
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
#define CLIENT_SEND_INTERVAL      (CLOCK_SECOND * 10)
#define PROBE_SIGNAL_INTERVAL 	  (CLOCK_SECOND * 11)

#define UDP_LEN_MAX           255
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t server_addr;
static struct uip_icmp6_echo_reply_notification icmp_notification;
static uint8_t echo_received;
static struct uip_udp_conn *conn;

static struct etimer timer,probe_timer;
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

int traffic_type_count =0;

static char *found, *found2;
char traffictype[11][10]={"Type A","Type C","Type A","Type C","Type C","Type C","Type C","Type C","Type B","Type B","Type A"};

/*---------------------------------------------------------------------------*/
PROCESS(ipv6_ble_client_process, "IPv6 over BLE - client process");
AUTOSTART_PROCESSES(&ipv6_ble_client_process);
/*---------------------------------------------------------------------------*/
void icmp_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  printf("echo response received\n");
  echo_received = 1;
}
/*---------------------------------------------------------------------------*/
static void print_local_addresses(void) 
{ int i; uint8_t state; PRINTF("Message from client, ipv6 address is: "); for(i = 0; i < UIP_DS6_ADDR_NB; i++) { state = uip_ds6_if.addr_list[i].state; if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) { PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr); PRINTF("\n"); } } }
/*---------------------------------------------------------------------------*/

char* newTrafficGenerator(void)
{

	int i;
	int count =0;
	static char currTrafficType[6];
	i = random_rand();
	if (i>0 && i<(int)(0.6*RANDOM_RAND_MAX))
	{
		strncpy(currTrafficType,"Type C",sizeof(currTrafficType));
	}
	else if(i>(int)(0.6*RANDOM_RAND_MAX) && i<(int)(0.85*RANDOM_RAND_MAX))
	{
		strncpy(currTrafficType,"Type B",sizeof(currTrafficType));
	}
	else
	{
		strncpy(currTrafficType,"Type A",sizeof(currTrafficType));
	}
	
	return currTrafficType;
}

char* returnAddress(int num)
{
	return *(traffictype+num);
}




/*---------------------------------------------------------------------------*/

static void tcpip_handler(void)
{
	memcpy(receivedPacketCount,uip_appdata,4);
	sprintf(bufNew, "%04d", actualSent_packet_counter);	
	bufNew[5]='\0';
	//printf("Received packet count is: %s and sent packet count is: %s\n",receivedPacketCount,bufNew);
	if (strcmp(receivedPacketCount,bufNew)==0)		/* If sent packet count and received packet count same, calcluate latency */
	{
		float b1 = (float)((int) (clock_time()-probe_start))*1.000;
		float b2 = (float) ((int)CLOCK_SECOND)*1.000;
		float f = b1 / b2;
		printf("Round trip delay is: %ld.%03u seconds for packet count:- %s\n",(long)f
		,(unsigned)((f-floor(f))*1000),receivedPacketCount);
		roundTrip = f;
		avg_round_trip_delay = (avg_round_trip_delay * roundTrip_count + roundTrip)/(roundTrip_count+1);
		roundTrip_count = roundTrip_count + 1;
	}	
	else
	{
		printf ("\nProbe packet is dropped");
	}
}
/*---------------------------------------------------------------------------*/
static void timeout_handler(int num)
{
	char *c;
	if(AONBatMonNewBatteryMeasureReady()==1)
	{
		temperature = AONBatMonTemperatureGetDegC();
	}
	
	c = newTrafficGenerator();
	//sprintf(buf, "Current temperature is  %02u and current packet count is: %04u!", temperature,packet_counter);g m
	send_message(conn, c, strlen(c));
	printf("A new message is sent of message type %s\n",c);
	//packet_counter++;
	//traffic_type_count++;
}




static void probe_signal(void)
{	
	sprintf(probeBuf, "%04u", probe_packet_counter);
	actualSent_packet_counter =probe_packet_counter;
	probe_start = clock_time();
	send_probe(conn, probeBuf, strlen(probeBuf));
	printf("Probe signal is sent having packet count:- %d\n",probe_packet_counter);
	//printf("Current connLatency is %d and slaveLatency is %d\n",current_conn_interval,current_slave_latency);
	probe_packet_counter++;
}

/*---------------------------------------------------------------------------*/
/******************* Main Process ********************************/


PROCESS_THREAD(ipv6_ble_client_process, ev, data)
{
	PROCESS_BEGIN();
	
	strncpy(trafficTypePrevious,"Type A",sizeof(trafficTypePrevious));
	strncpy(trafficTypeCurrent,"Type A",sizeof(trafficTypeCurrent));
	avg_round_trip_delay = 0.0;
	roundTrip_count = 0;			//Setting round trip count is 1
	
	start = clock_time();
	PRINTF("CC26XX-IPv6-over-BLE client started\n");
	uiplib_ipaddrconv(SERVER_IP, &server_addr);
	uip_icmp6_echo_reply_callback_add(&icmp_notification, icmp_reply_handler);
	PRINTF("pinging the IPv6-over-BLE server: ");
	PRINT6ADDR(&server_addr);
	PRINTF("\n");
	do 
	{
		etimer_set(&timer, PING_TIMEOUT);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		uip_icmp6_send(&server_addr, ICMP6_ECHO_REQUEST, 0, 20);
	} while(!echo_received);
	float b1 = (float)((int) (clock_time()-start))*1.000;
	float b2 = (float) ((int)CLOCK_SECOND)*1.000;
	float f = b1 / b2;
	printf("\nTime taken in receiving ping response is: %ld.%03u seconds\n",(long)f
	,(unsigned)((f-floor(f))*1000));
	printf("Creating UDP connection\n");
	conn = udp_new(&server_addr, UIP_HTONS(SERVER_PORT), NULL);
	udp_bind(conn, UIP_HTONS(CLIENT_PORT));
	etimer_set(&probe_timer, PROBE_SIGNAL_INTERVAL);
	etimer_set(&timer, CLIENT_SEND_INTERVAL);
	/*while(traffic_type_count< 11)
	{
		
		PROCESS_YIELD();
		if((ev == PROCESS_EVENT_TIMER) && (data == &timer)) 
		{
			timeout_handler(traffic_type_count);
			etimer_set(&timer, CLIENT_SEND_INTERVAL);
		} 
		else if((ev == PROCESS_EVENT_TIMER) && (data == &probe_timer)) 
		{
			probe_signal();
			etimer_set(&probe_timer, PROBE_SIGNAL_INTERVAL);			
		}
		else if(ev == tcpip_event) 
		{
			//printf("TCPIP event detected\n");
			tcpip_handler();			
		}
		
	}*/

	/*while(1)
	{
		
		PROCESS_YIELD();
		if((ev == PROCESS_EVENT_TIMER) && (data == &timer)) 
		{
			timeout_handler(traffic_type_count);
			etimer_set(&timer, CLIENT_SEND_INTERVAL);
		} 
		else if((ev == PROCESS_EVENT_TIMER) && (data == &probe_timer)) 
		{
			probe_signal();
			etimer_set(&probe_timer, PROBE_SIGNAL_INTERVAL);			
		}
		else if(ev == tcpip_event) 
		{
			//printf("TCPIP event detected\n");
			tcpip_handler();			
		}
		
	}
	*/
	etimer_set(&timer, CLIENT_SEND_INTERVAL);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	printf("***********************Sending data message *************************\n");
	
	char msg[] = "This is a long string and will be repeating again and again";
	send_message(conn, msg, strlen(msg));
	

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
