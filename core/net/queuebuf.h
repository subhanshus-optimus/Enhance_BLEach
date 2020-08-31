

#ifndef QUEUEBUF_H_
#define QUEUEBUF_H_

#include "net/packetbuf.h"

/* QUEUEBUF_NUM is the total number of queuebuf */
#ifdef QUEUEBUF_CONF_NUM
#define QUEUEBUF_NUM QUEUEBUF_CONF_NUM
#else
#define QUEUEBUF_NUM 8
#endif

/* QUEUEBUFRAM_NUM is the number of queuebufs stored in RAM.
   If QUEUEBUFRAM_CONF_NUM is set lower than QUEUEBUF_NUM,
   swapping is enabled and queuebufs are stored either in RAM of CFS.
   If QUEUEBUFRAM_CONF_NUM is unset or >= to QUEUEBUF_NUM, all
   queuebufs are in RAM and swapping is disabled. */
#ifdef QUEUEBUFRAM_CONF_NUM
  #if QUEUEBUFRAM_CONF_NUM>QUEUEBUF_NUM
    #error "QUEUEBUFRAM_CONF_NUM cannot be greater than QUEUEBUF_NUM"
  #else
    #define QUEUEBUFRAM_NUM QUEUEBUFRAM_CONF_NUM
    #define WITH_SWAP (QUEUEBUFRAM_NUM < QUEUEBUF_NUM)
  #endif
#else /* QUEUEBUFRAM_CONF_NUM */
  #define QUEUEBUFRAM_NUM QUEUEBUF_NUM
  #define WITH_SWAP 0
#endif /* QUEUEBUFRAM_CONF_NUM */

#ifdef QUEUEBUF_CONF_DEBUG
#define QUEUEBUF_DEBUG QUEUEBUF_CONF_DEBUG
#else /* QUEUEBUF_CONF_DEBUG */
#define QUEUEBUF_DEBUG 0
#endif /* QUEUEBUF_CONF_DEBUG */

struct queuebuf;

void queuebuf_init(void);

#if QUEUEBUF_DEBUG
struct queuebuf *queuebuf_new_from_packetbuf_debug(const char *file, int line);
#define queuebuf_new_from_packetbuf() queuebuf_new_from_packetbuf_debug(__FILE__, __LINE__)
#else /* QUEUEBUF_DEBUG */
struct queuebuf *queuebuf_new_from_packetbuf(void);
#endif /* QUEUEBUF_DEBUG */
void queuebuf_update_attr_from_packetbuf(struct queuebuf *b);
void queuebuf_update_from_packetbuf(struct queuebuf *b);

void queuebuf_to_packetbuf(struct queuebuf *b);
void queuebuf_free(struct queuebuf *b);

void *queuebuf_dataptr(struct queuebuf *b);
int queuebuf_datalen(struct queuebuf *b);

linkaddr_t *queuebuf_addr(struct queuebuf *b, uint8_t type);
packetbuf_attr_t queuebuf_attr(struct queuebuf *b, uint8_t type);

void queuebuf_debug_print(void);

int queuebuf_numfree(void);

#endif /* __QUEUEBUF_H__ */

/** @} */
/** @} */
