
#include "ble-hal.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include <string.h>
#include "lib/list.h"
#include "sys/ctimer.h"
#include "sys/rtimer.h"
#include "net/commlatency.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7])
#else
#define PRINTF(...)
#define PRINTADDR(addr)
#endif
/*---------------------------------------------------------------------------*/
#if !UIP_CONF_ROUTER
#define ADAPTATION_INTERVAL         (4 * CLOCK_SECOND)
#define ADAPTATION_START_TIMEOUT      (30 * CLOCK_SECOND)
#define UTILIZATION_WEIGHT          (0.5f)
#define UTILIZATION_UPPER_THRESHOLD     75
#define UTILIZATION_LOWER_THRESHOLD     25
#define CONN_INTERVAL_MAX          800
#define CONN_INTERVAL_MIN           50

static uint16_t avg_utilization = 0;
static uint16_t tx_requests;
static uint8_t change_pending = 0;
static uint8_t conn_established;
static struct ctimer adaptation_timer;
/*---------------------------------------------------------------------------*/

/**********************************************************************************/


static void
parameter_adaptation_both(void *ptr)
{
	ctimer_set(&adaptation_timer, ADAPTATION_INTERVAL, parameter_adaptation_both, NULL);
}

#endif




/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
//printf("BLE_ADAPT: Send_packet method\n");
  int ret;
#if !UIP_CONF_ROUTER
  tx_requests++;
#endif
  if(NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen()) == RADIO_TX_OK) {
    ret = MAC_TX_OK;
  } else {
    ret = MAC_TX_ERR;
  }
  mac_call_sent_callback(sent, ptr, ret, 1);
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
#if !UIP_CONF_ROUTER


  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME_BLE_CONNECTION_UPDATED) {
    PRINTF("ble-adapt-par: parameter successfully updated\n");
    change_pending = 0;
  } else if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME_BLE_RX_EVENT) {
    if(conn_established == 0) {
      PRINTF("ble-adapt-par: start_adaptation\n");
      conn_established = 1;
      ctimer_set(&adaptation_timer, ADAPTATION_START_TIMEOUT,
                 parameter_adaptation_both, NULL);
    }
  }
#endif
  NETSTACK_MAC.input();
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return NETSTACK_RADIO.on();
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  if(keep_radio_on) {
    return NETSTACK_RADIO.on();
  } else {
    return NETSTACK_RADIO.off();
  }
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  on();
}

void test(void)

{
printf("Test method\n");
}

/*---------------------------------------------------------------------------*/
const struct rdc_driver ble_adapt_par_driver = {
  "ble-adapt-par",
  init,
  send_packet,
  NULL,
  packet_input,
  on,
  off,
  channel_check_interval,
};
