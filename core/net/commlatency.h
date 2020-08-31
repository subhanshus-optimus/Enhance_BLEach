#ifndef COMMLATENCY_H
#define COMMLATENCY_H
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "net/ip/uip.h"

/*Global variable */

#define maxRoundTripTypeA 0.375
#define maxRoundTripTypeB 0.200
#define maxRoundTripTypeC 0.100 


int connectionParameterUpdatedIndicator;
float roundTrip;

uint16_t current_conn_interval;
uint16_t current_slave_latency;

char trafficTypePrevious[6];
char trafficTypeCurrent[6];


float avg_round_trip_delay;
int roundTrip_count;




int trafficChangeIndicator;

void send_message(struct uip_udp_conn *c, const void *data, int len);

void send_probe(struct uip_udp_conn *c, const void *data, int len);

void adaptation(void);

/* Function for generating random traffic type**/
void randomTrafficGenerator(char *ptr);

//char* newTrafficGenerator(void);

static char *c;


//void setRoundtrip(void);

//float getRoundtrip(void);

#endif /* COMMLATENCY_H */

