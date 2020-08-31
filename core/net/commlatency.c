#include "net/commlatency.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "net/ip/uip-udp-packet.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "ble-hal.h"

uint16_t conn_interval;
uint16_t slave_latency;



int connPara_setting_A[]= {50,0};
int connPara_setting_B[]= {100,0};
int connPara_setting_C[]= {250,0};

void send_message(struct uip_udp_conn *c, const void *data, int len)

{	
	printf("Average round trip delay is %ld.%03u\n",(long)avg_round_trip_delay,(unsigned)((avg_round_trip_delay-floor(avg_round_trip_delay))*1000));
	NETSTACK_RADIO.get_value(RADIO_PARAM_BLE_CONN_INTERVAL, (radio_value_t *)&conn_interval);
	NETSTACK_RADIO.get_value(RADIO_PARAM_BLE_CONN_LATENCY, (radio_value_t *)&slave_latency);
	current_conn_interval = conn_interval;
	current_slave_latency = slave_latency;
	strcpy(trafficTypeCurrent,data+strlen(data)-6);
	adaptation();

	uip_udp_packet_send(c, data, len);

	strcpy(trafficTypePrevious,data+strlen(data)-6);
}

void send_probe(struct uip_udp_conn *c, const void *data, int len)

{	

	NETSTACK_RADIO.get_value(RADIO_PARAM_BLE_CONN_INTERVAL, (radio_value_t *)&conn_interval);
	NETSTACK_RADIO.get_value(RADIO_PARAM_BLE_CONN_LATENCY, (radio_value_t *)&slave_latency);
	current_conn_interval = conn_interval;
	current_slave_latency = slave_latency;
	//adaptation();

	uip_udp_packet_send(c, data, len);
}


void adaptation(void)

{
	if(strcmp(trafficTypeCurrent,trafficTypePrevious)!=0)

	{	
		
		printf("Traffic type is changed and current type is: %s and previous is: %s\n",trafficTypeCurrent,trafficTypePrevious);
		if (strcmp(trafficTypeCurrent,"Type A")==0)
		{
			
			updateConnectionParameters(connPara_setting_A);
		}
		else if (strcmp(trafficTypeCurrent,"Type B")==0)
		{

			updateConnectionParameters(connPara_setting_B);
		}
		else if (strcmp(trafficTypeCurrent,"Type C")==0)
		{
			updateConnectionParameters(connPara_setting_C);
		}
		
	}
	
}







void updateConnectionParameters(int connectionParameterSetting[])

{
	uint16_t conn_interval_new;
	uint16_t slave_latency_new;
	uint16_t check_updated_conn_interval_new;
	uint16_t check_updated_slave_latency_new;	
	//printf("Received conn_int is %d and slave latency is: %d\n",connectionParameterSetting[0],connectionParameterSetting[1]);	

	conn_interval_new = connectionParameterSetting[0];
	slave_latency_new = connectionParameterSetting[1];
	
	NETSTACK_RADIO.set_value(RADIO_PARAM_BLE_CONN_INTERVAL, conn_interval_new);
	NETSTACK_RADIO.set_value(RADIO_PARAM_BLE_CONN_LATENCY, slave_latency_new);
	NETSTACK_RADIO.set_value(RADIO_PARAM_BLE_CONN_UPDATE, 1);
	avg_round_trip_delay = 0.0;
	roundTrip_count = 0;
	
	NETSTACK_RADIO.get_value(RADIO_PARAM_BLE_CONN_INTERVAL, check_updated_conn_interval_new);
	NETSTACK_RADIO.get_value(RADIO_PARAM_BLE_CONN_LATENCY, check_updated_slave_latency_new);

	printf("Changing conn_interval and slave latency from %d and %d  to %d and %d\n",
			current_conn_interval, current_slave_latency,check_updated_conn_interval_new,check_updated_slave_latency_new);

}




void randomTrafficGenerator(char *ptr)
{
	int i;
	int count =0;
	char type[6];
	i = random_rand();
	if (i>0 && i<32767)
	{
		strncpy(type,"Type A",sizeof(type));
	}
	else if(i>32767 && i<55705)
	{
		strncpy(type,"Type B",sizeof(type));
	}
	else
	{
		strncpy(type,"Type C",sizeof(type));
	}
	for (count =0;count<10;count++)
	{
		*(ptr+count) = type[count];
	}
}

/*
char* newTrafficGenerator(void)
{
	random_init(50);
	int i;
	int count =0;
	static char currentTrafficType[6];
	i = random_rand();
	printf("Current random no is: %d\n",i);
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
*/




/** @} */
