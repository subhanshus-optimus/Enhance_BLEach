
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
//#define MASTER_CLIENT_IP               "::"

#define MASTER_CLIENT_IP             "fe80::9a07:2dff:fe3c:8d01" //"::"

#define SLAVE_SERVER_PORT           61617
#define MASTER_CLIENT_PORT           61616

#define PING_TIMEOUT              (CLOCK_SECOND / 4)
#define DELAY      (CLOCK_SECOND * 10)
#define MESSAGE_SENT_INTERVAL 	  (CLOCK_SECOND * 10)

#define UDP_LEN_MAX           255
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t master_client_addr;
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
static char receivedPacketCount[512];
static char bufNew[5];

clock_time_t start,probe_start;


/*---------------------------------------------------------------------------*/
PROCESS(ipv6_ble_client_process, "IPv6 over BLE - slave_server process");
AUTOSTART_PROCESSES(&ipv6_ble_client_process);
/*---------------------------------------------------------------------------*/

int i =0,probeCounter=0;
int count=0;


//char *traffictype[11]={"Type A","Type C","Type A","Type C","Type C","Type C","Type C","Type C","Type B","Type B","Type A"};

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

static void send_datamessage(void)
{
	char *data_message;	
	data_message = newTrafficGenerator();
	send_message(conn, data_message, strlen(data_message));
	printf("A new message is sent of message type %s\n",c);
	
}


static void send_probe_signal(void)
{	
	if(probeCounter<9)
	{
		probeCounter++;
	}
	else
	{
		probeCounter=0;
	}
	sprintf(probeBuf, "%d", probeCounter);
	actualSent_packet_counter =probeCounter;
	probe_start = clock_time();
	send_probe(conn, probeBuf, strlen(probeBuf));
	printf("Probe signal is sent having packet count:- %d\n",probeCounter);
	//printf("Current connLatency is %d and slaveLatency is %d\n",current_conn_interval,current_slave_latency);

}



static void tcpip_handler(void)
{
	memcpy(receivedPacketCount,uip_appdata,1);
	sprintf(bufNew, "%d", actualSent_packet_counter);	

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


PROCESS_THREAD(ipv6_ble_client_process, ev, data)
{
	PROCESS_BEGIN();
	
	
	PRINTF("CC26XX-IPv6-over-BLE client started\n");
	uiplib_ipaddrconv(MASTER_CLIENT_IP, &master_client_addr);
	
	conn = udp_new(&master_client_addr, UIP_HTONS(MASTER_CLIENT_PORT), NULL);
	udp_bind(conn, UIP_HTONS(SLAVE_SERVER_PORT));
	
	strncpy(trafficTypePrevious,"Type A",sizeof(trafficTypePrevious));
	strncpy(trafficTypeCurrent,"Type A",sizeof(trafficTypeCurrent));
	etimer_set(&timer, DELAY);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

	etimer_set(&timer, MESSAGE_SENT_INTERVAL);
	
	while(1)
	{	
		PROCESS_YIELD();
		if((ev == PROCESS_EVENT_TIMER) && (data == &timer)) 
		{
			if(count<3)
			{
				send_probe_signal();
				count++;
				etimer_set(&timer, MESSAGE_SENT_INTERVAL);
			}
			else
			{
				send_datamessage();
				count=0;
				etimer_set(&timer, MESSAGE_SENT_INTERVAL);
			}
		}
		else if(ev == tcpip_event) 
		{
			//printf("TCPIP event detected\n");
			tcpip_handler();			
		}
	}
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
