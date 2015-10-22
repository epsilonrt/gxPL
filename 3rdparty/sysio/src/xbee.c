/**
 * @file xbee.c
 * @brief XBee module interface functions
 *
 * Copyright © 2006-2008 Tymm Twillman <tymm@booyaka.com>
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sysio/serial.h>
#include <sysio/log.h>
#include "xbee_private.h"

/* private functions ======================================================== */

/* -----------------------------------------------------------------------------
 * Génère un numéro de trame différent de zéro
 */
uint8_t
ucXBeeNextFrameId (xXBee *xbee) {
  uint8_t frame_id;

  pthread_mutex_lock (&xbee->mutex);

  xbee->out.frame_id++;
  if (xbee->out.frame_id == 0) {

    xbee->out.frame_id++;
  }
  frame_id = xbee->out.frame_id;

  pthread_mutex_unlock (&xbee->mutex);
  return frame_id;
}

/* -----------------------------------------------------------------------------
 * Calcule le CRC d'un paquet
 */
uint8_t
ucXBeeCrc (const xXBeePkt *pkt) {
  uint8_t *pkt_data = ( (uint8_t *) pkt) + sizeof (xXBeePktHdr);
  uint16_t i;
  uint8_t crc = 0;

  for (i = 0; i < ntohs ( ( (xXBeePktHdr *) pkt)->len); i++) {

    crc += * (pkt_data++);
  }

  return ~crc;
}

#if 0
// -----------------------------------------------------------------------------
uint32_t
ulXBeePktAddress32 (xXBeePkt *pkt, uint8_t ucOffset) {
  static uint32_t s;
  uint8_t *p = pucXBeePktAddrSrc64 (pkt);

  if (p) {

    memcpy (&s, p + ucOffset, 4);
    vSwapBytes ( (uint8_t *) &s, 4);
  }
  else {

    s = XBEE_SH_UNKNOWN;
  }
  return s;
}
// -----------------------------------------------------------------------------
#endif


/* private functions ======================================================== */

/* -----------------------------------------------------------------------------
 * Queue a packet for transmission
 *
 * needs to queue packet to be sent to XBEE module; e.g. copy the packet to a
 * UART buffer.
 *  On error, -1 should be returned and the packet should NOT be freed.
 *  On success, 0 should be returned; if XBEE_ALLOC is set, this function or
 *   someone downstream is responsible for freeing it -- the packet has been
 *   handed off.  This is to minimize copying of data.
 */
int
iXBeeOut (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  int iDataWrite = 0;

  while (len) {

    iDataWrite = write (xbee->fd, &pkt[iDataWrite], len);

    if (iDataWrite < 0) {

      return -1;
    }
    len -= iDataWrite;
  }
  vXBeeFreePkt (xbee, pkt);

  return 0;
}

/* -----------------------------------------------------------------------------
 * Accept data from an XBee module & build into valid XBEE
 *  packets
 */
void
vXBeeIn (xXBee *xbee, const void *buf, uint8_t len) {
  uint8_t *data = (uint8_t *) buf;

  while (len) {
    xXBeePktHdr * hdr;

    switch (xbee->in.bytes_rcvd) {

      case 0: // Attente 0x7E
        while (*data != XBEE_PKT_START) {

          if (--len == 0) {

            return;
          }
          data++;
        }
        
        // 0x7E reçu, on le stocke
        xbee->in.hdr_data[xbee->in.bytes_rcvd++] = *data++;
        if (--len == 0) {

          return;
        }

        /* Fall thru */

      case 1:
        // Poids fort longueur trame
        xbee->in.hdr_data[xbee->in.bytes_rcvd++] = *data++;
        if (--len == 0) {

          return;
        }

        /* Fall thru */

      case 2:
        // Poids faible longueur trame
        xbee->in.hdr_data[xbee->in.bytes_rcvd++] = *data++;

        /* Got enough to get packet length */
        hdr = (xXBeePktHdr *) xbee->in.hdr_data;
        xbee->in.bytes_left = ntohs (hdr->len);

        if (xbee->in.bytes_left > XBEE_MAX_DATA_LEN
            || ( (xbee->in.packet
                  = pvXBeeAllocPkt (xbee, XBEE_RECV, xbee->in.bytes_left + 4)) == NULL)
           ) {

          xbee->in.bytes_left = 0;
          INC_RX_ERROR (xbee);
          continue;
        }

        xbee->in.bytes_left++; /* Extra for crc (alloc_pkt already accounts for it) */

        memcpy (& (xbee->in.packet->hdr), & (xbee->in.hdr_data),
                sizeof (xbee->in.hdr_data));

        if (--len == 0) {

          return;
        }

        /* Fall thru */

      default:
        while (xbee->in.bytes_left--) {

          ( (uint8_t *) xbee->in.packet) [xbee->in.bytes_rcvd++] = *data++;
          if (!--len && xbee->in.bytes_left) {

            return;
          }
        }
    }

    if (ucXBeeCrc (xbee->in.packet)
        != ( (uint8_t *) xbee->in.packet) [xbee->in.bytes_rcvd - 1]) {

      xbee->in.bytes_rcvd = 0;
      INC_RX_CRC_ERROR (xbee);
      continue;
    }

    // Paquet reçu et vérifié, on le passe au callback
    if (iXBeeRecvPktCB (xbee, xbee->in.packet, xbee->in.bytes_rcvd) == -1) {

      vXBeeFreePkt (xbee, xbee->in.packet);
      INC_RX_DROPPED (xbee);
    }

    xbee->in.bytes_rcvd = 0;
  }
}

/* -----------------------------------------------------------------------------
 * Handle an incoming packet
 *
 * the packet will be fully formed and verified
 * for proper construction before being passed off to this function.  This
 * function should dig into the packet & process based on its contents.
 * @return -1 if packet has not been processed by a callback and must be dropped
 */
int
iXBeeRecvPktCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  eXBeeCbType eCallback = XBEE_CB_UNKNOWN;

  int iPktType = ucXBeePktType (pkt);

  switch (iPktType) {

    case XBEE_PKT_TYPE_ATCMD_RESP:
      eCallback = XBEE_CB_AT_LOCAL;
      break;

    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      eCallback = XBEE_CB_AT_REMOTE;
      break;

    case XBEE_PKT_TYPE_MODEM_STATUS:
      eCallback = XBEE_CB_MODEM_STATUS;
      break;

    default:
      break;
  }

  if (eCallback == XBEE_CB_UNKNOWN) {

    switch (xbee->series) {

      case XBEE_SERIES_S1:
        //----------------------------------------------------------------------
        switch (iPktType) {
          case XBEE_PKT_TYPE_TX_STATUS:
            eCallback = XBEE_CB_TX_STATUS;
            break;

          case XBEE_PKT_TYPE_RX16:
          case XBEE_PKT_TYPE_RX64:
            eCallback = XBEE_CB_DATA;
            break;

          case XBEE_PKT_TYPE_RX16_IO:
          case XBEE_PKT_TYPE_RX64_IO:
            eCallback = XBEE_CB_IO;
            break;

          default:
            return -1;
        }
        break;

      case XBEE_SERIES_S2:
      case XBEE_SERIES_S2B:
        //----------------------------------------------------------------------
        switch (iPktType) {
          case XBEE_PKT_TYPE_ZB_TX_STATUS:
            eCallback = XBEE_CB_TX_STATUS;
            break;

          case XBEE_PKT_TYPE_ZB_RX:
            eCallback = XBEE_CB_DATA;
            break;

          case XBEE_PKT_TYPE_ZB_RX_IO:
            eCallback = XBEE_CB_IO;
            break;

          case XBEE_PKT_TYPE_ZB_RX_SENSOR:
            eCallback = XBEE_CB_SENSOR;
            break;

          case XBEE_PKT_TYPE_ZB_NODE_IDENT:
            eCallback = XBEE_CB_NODE_IDENT;
            break;

          default:
            return -1;
        }
        break;

      default:
        return -1;
    }
  }

  if (xbee->in.user_cb[eCallback] != 0) {

    return xbee->in.user_cb[eCallback] (xbee, pkt, len);
  }
  return -1; /* pkt was dropped */
}

/* internal public functions ================================================ */

/* -----------------------------------------------------------------------------
 */
int
iXBeeInit (xXBee *xbee, eXBeeSeries series, int fd) {

  if (xbee) {
    if (bSerialFdIsValid (fd)) {

      memset (xbee, 0, sizeof (xXBee));
      pthread_mutex_init (&xbee->mutex, NULL);
      xbee->fd = fd;
      xbee->series = series;
      return 0;
    }
  }
  return -1;
}

/* -----------------------------------------------------------------------------
 */
void
vXBeeSetCB (xXBee * xbee, eXBeeCbType cb_type, iXBeeRxCB cb) {

  if ( (cb_type >= XBEE_CB_FIRST) && (cb_type <= XBEE_CB_LAST)) {

    xbee->in.user_cb[cb_type] = cb;
  }
}

/* -----------------------------------------------------------------------------
 * Send a command to an XBee module
 */
int
iXBeeSendAt (xXBee * xbee,
             const char cmd[],
             const uint8_t * params,
             uint8_t param_len) {
  xXBeeAtCmdPkt *pkt;
  uint8_t frame_id;
  int ret;


  pkt = (xXBeeAtCmdPkt *) pvXBeeAllocPkt (xbee, XBEE_XMIT, param_len + 8);
  if (pkt == NULL) {

    INC_TX_ERROR (xbee);
    return -ENOMEM;
  }

  XBEE_HDR_INIT (pkt->hdr, param_len + 4);

  pkt->type = XBEE_PKT_TYPE_ATCMD;

  frame_id = ucXBeeNextFrameId (xbee);

  pkt->frame_id = frame_id;

  pkt->command[0] = cmd[0];
  pkt->command[1] = cmd[1];

  if ( (params) && (param_len)) {

    memcpy (pkt->param, params, param_len);
  }
  pkt->param[param_len] = ucXBeeCrc ( (xXBeePkt *) pkt);

  ret = iXBeeOut (xbee, (xXBeePkt *) pkt,
                  sizeof (xXBeeAtCmdPkt) + param_len + 1);

  if (ret >= 0) {

    return frame_id;
  }

  vXBeeFreePkt (xbee, (xXBeePkt *) pkt);

  INC_TX_ERROR (xbee);

  return ret;
}


/* -----------------------------------------------------------------------------
 * Send a command to a remote XBee module
 */
int
iXBeeSendRemoteAt (xXBee * xbee,
                   const char cmd[],
                   const uint8_t params[],
                   uint8_t param_len,
                   const uint8_t addr64[8],
                   const uint8_t addr16[2],
                   uint8_t apply) {
  xXBeeRemoteAtCmdPkt *pkt;
  uint8_t frame_id;
  int ret;


  pkt = (xXBeeRemoteAtCmdPkt *) pvXBeeAllocPkt (xbee, XBEE_XMIT, param_len + 19);
  if (pkt == NULL) {

    INC_TX_ERROR (xbee);
    return -ENOMEM;
  }

  XBEE_HDR_INIT (pkt->hdr, param_len + 15);

  pkt->type = XBEE_PKT_TYPE_REMOTE_ATCMD;

  frame_id = ucXBeeNextFrameId (xbee);
  pkt->frame_id = frame_id;

  memcpy (pkt->dest64, addr64, 8);
  memcpy (pkt->dest16, addr16, 2);

  pkt->apply = apply ? 2 : 0;

  pkt->command[0] = cmd[0];
  pkt->command[1] = cmd[1];

  memcpy (pkt->param, params, param_len);
  pkt->param[param_len] = ucXBeeCrc ( (xXBeePkt *) pkt);

  ret = iXBeeOut (xbee, (xXBeePkt *) pkt,
                  sizeof (xXBeeRemoteAtCmdPkt) + param_len + 1);

  if (ret == 0) {

    return frame_id;
  }

  vXBeeFreePkt (xbee, (xXBeePkt *) pkt);
  INC_TX_ERROR (xbee);
  return ret;
}


/* -----------------------------------------------------------------------------
 * Poll the inputstream to read incoming bytes
 */
int
iXBeePoll (xXBee * xbee, int timeout_ms) {
  int iDataAvailable = iSerialPoll (xbee->fd, timeout_ms);

  if (iDataAvailable > 0) {
    int iDataRead;
    uint8_t ucBuffer[XBEE_MAX_RF_PAYLOAD];

    iDataRead = read (xbee->fd, ucBuffer, MIN (iDataAvailable, XBEE_MAX_RF_PAYLOAD));
    if (iDataRead > 0) {

#ifdef XBEE_DEBUG
      printf ("read %d bytes: [", iDataRead);
      for (int i = 0; i < iDataRead; i++) {
        printf ("%02X", ucBuffer[i]);
      }
      printf ("]\n");
#endif
      vXBeeIn (xbee, ucBuffer, iDataRead);
      return 0;
    }
    return iDataRead;
  }

  return iDataAvailable;
}

// -----------------------------------------------------------------------------
bool
bXBeePktAddressIsEqual (const uint8_t *a1, const uint8_t *a2, uint8_t len) {

  return memcmp (a1, a2, len) == 0;
}

// -----------------------------------------------------------------------------
const uint8_t *
pucXBeeAddr16Unknown (void) {
  static const uint8_t ucAddr16Unknown[] = { 0xFF, 0xFE };

  return ucAddr16Unknown;
}

// -----------------------------------------------------------------------------
const uint8_t *
pucXBeeAddr64Unknown (void) {
  static const uint8_t ucAddr64Unknown[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  return ucAddr64Unknown;
}

// -----------------------------------------------------------------------------
const uint8_t *
pucXBeeAddr64Coordinator (void) {
  static const uint8_t ucAddr64Coordinator[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  return ucAddr64Coordinator;
}

// -----------------------------------------------------------------------------
const uint8_t *
pucXBeeAddr64Broadcast (void) {
  static const uint8_t ucAddr64Broadcast[] = { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF };

  return ucAddr64Broadcast;
}

// -----------------------------------------------------------------------------
int
iXBeePktIsBroadcast (xXBeePkt * pkt) {
  int iOpt = iXBeePktOptions (pkt);

  if (iOpt >= 0) {

    return (iOpt & XBEE_RX_FLAG_ADDR_BCAST) ? true : false;
  }
  return iOpt;
}

// -----------------------------------------------------------------------------
uint8_t *
pucXBeePktAddrSrc64 (xXBeePkt *pkt) {

  switch (ucXBeePktType (pkt)) {

    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      return ( (xXBeeRemoteAtCmdRespPkt *) pkt)->src64;

      // XBEE_SERIES_S1
    case XBEE_PKT_TYPE_RX64:
      return ( (xXBeeRx64Pkt *) pkt)->src;
    case XBEE_PKT_TYPE_RX64_IO:
      return ( (xXBeeRxIo64Pkt *) pkt)->src;

      // XBEE_SERIES_S2
    case XBEE_PKT_TYPE_ZB_RX:
      return ( (xXBeeZbRxPkt *) pkt)->src64;
    case XBEE_PKT_TYPE_ZB_RX_IO:
      return ( (xXBeeZbRxIoPkt *) pkt)->src64;
    case XBEE_PKT_TYPE_ZB_RX_SENSOR:
      return ( (xXBeeZbRxSensorPkt *) pkt)->src64;
    case XBEE_PKT_TYPE_ZB_NODE_IDENT:
      return ( (xXBeeZbNodeIdPkt *) pkt)->src64;

    default:
      break;
  }
  return 0;
}

// -----------------------------------------------------------------------------
uint8_t *
pucXBeePktAddrSrc16 (xXBeePkt *pkt) {

  switch (ucXBeePktType (pkt)) {

    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      return ( (xXBeeRemoteAtCmdRespPkt *) pkt)->src16;

      // XBEE_SERIES_S1
    case XBEE_PKT_TYPE_RX16:
      return ( (xXBeeRx16Pkt *) pkt)->src;
    case XBEE_PKT_TYPE_RX16_IO:
      return ( (xXBeeRxIo16Pkt *) pkt)->src;

      // XBEE_SERIES_S2
    case XBEE_PKT_TYPE_ZB_RX:
      return ( (xXBeeZbRxPkt *) pkt)->src16;
    case XBEE_PKT_TYPE_ZB_RX_IO:
      return ( (xXBeeZbRxIoPkt *) pkt)->src16;
    case XBEE_PKT_TYPE_ZB_RX_SENSOR:
      return ( (xXBeeZbRxSensorPkt *) pkt)->src16;
    case XBEE_PKT_TYPE_ZB_NODE_IDENT:
      return ( (xXBeeZbNodeIdPkt *) pkt)->src16;

    default:
      break;
  }
  return 0;
}

// -----------------------------------------------------------------------------
uint8_t *
pucXBeePktData (xXBeePkt *pkt) {

  switch (ucXBeePktType (pkt)) {

      // XBEE_SERIES_S1
    case XBEE_PKT_TYPE_RX64:
      return ( (xXBeeRx64Pkt *) pkt)->data;
    case XBEE_PKT_TYPE_RX64_IO:
      return (uint8_t *) ( (xXBeeRxIo64Pkt *) pkt)->data;
    case XBEE_PKT_TYPE_RX16:
      return ( (xXBeeRx16Pkt *) pkt)->data;
    case XBEE_PKT_TYPE_RX16_IO:
      return (uint8_t *) ( (xXBeeRxIo16Pkt *) pkt)->data;

      // XBEE_SERIES_S2
    case XBEE_PKT_TYPE_ZB_RX:
      return ( (xXBeeZbRxPkt *) pkt)->data;
    case XBEE_PKT_TYPE_ZB_RX_IO:
      return (uint8_t *) ( (xXBeeZbRxIoPkt *) pkt)->data;

    default:
      break;
  }
  return 0;
}

// -----------------------------------------------------------------------------
int
iXBeePktDataLen (xXBeePkt *pkt) {
  int len = usXBeePktLength (pkt) + 3;

  switch (ucXBeePktType (pkt)) {

      // XBEE_SERIES_S1
    case XBEE_PKT_TYPE_RX64:
      len -= sizeof (xXBeeRx64Pkt);
      break;
    case XBEE_PKT_TYPE_RX64_IO:
      len -= sizeof (xXBeeRxIo64Pkt);
      break;
    case XBEE_PKT_TYPE_RX16:
      len -= sizeof (xXBeeRx16Pkt);
      break;
    case XBEE_PKT_TYPE_RX16_IO:
      len -= sizeof (xXBeeRxIo16Pkt);
      break;

      // XBEE_SERIES_S2
    case XBEE_PKT_TYPE_ZB_RX:
      len -= sizeof (xXBeeZbRxPkt);
      break;
    case XBEE_PKT_TYPE_ZB_RX_IO:
      len -= sizeof (xXBeeZbRxIoPkt);
      break;

    default:
      return -1;
  }

  return  len;
}

// -----------------------------------------------------------------------------
uint8_t
ucXBeePktType (xXBeePkt *pkt) {

  return pkt->type;
}

// -----------------------------------------------------------------------------
uint16_t
usXBeePktLength (xXBeePkt *pkt) {

  return ntohs (pkt->hdr.len);
}

// -----------------------------------------------------------------------------
int
iXBeePktFrameId (xXBeePkt *pkt) {


  switch (ucXBeePktType (pkt)) {

      /* XBEE_PKT_TYPE_ATCMD 0x08: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD:
      return ( (xXBeeAtCmdPkt *) pkt)->frame_id;

      /* XBEE_PKT_TYPE_ATCMD_RESP 0x88: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD_RESP:
      return ( (xXBeeAtCmdRespPkt *) pkt)->frame_id;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD 0x17: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD:
      return ( (xXBeeRemoteAtCmdPkt *) pkt)->frame_id;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD_RESP 0x97: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      return ( (xXBeeRemoteAtCmdRespPkt *) pkt)->frame_id;

      // XBEE_SERIES_S1
      /* XBEE_PKT_TYPE_TX64 0x00: S1 Series */
    case XBEE_PKT_TYPE_TX64:
      return ( (xXBeeTxReq64Pkt *) pkt)->frame_id;

      /* XBEE_PKT_TYPE_TX16 0x01: S1 Series */
    case XBEE_PKT_TYPE_TX16:
      return ( (xXBeeTxReq16Pkt *) pkt)->frame_id;

      /* XBEE_PKT_TYPE_TX_STATUS 0x89: S1 Series */
    case XBEE_PKT_TYPE_TX_STATUS:
      return ( (xXBeeTxStatusPkt *) pkt)->frame_id;

      // XBEE_SERIES_S2
      /* XBEE_PKT_TYPE_ZB_TX_REQ 0x10: S2 Series */
    case XBEE_PKT_TYPE_ZB_TX_REQ:
      return ( (xXBeeZbTxReqPkt *) pkt)->frame_id;

      /* XBEE_PKT_TYPE_ZB_TX_STATUS 0x8B: S2 Series */
    case XBEE_PKT_TYPE_ZB_TX_STATUS:
      return ( (xXBeeZbTxStatusPkt *) pkt)->frame_id;

    default:
      break;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iXBeePktStatus (xXBeePkt *pkt) {

  switch (ucXBeePktType (pkt)) {

      /* XBEE_PKT_TYPE_ATCMD_RESP 0x88: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD_RESP:
      return ( (xXBeeAtCmdRespPkt *) pkt)->status;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD_RESP 0x97: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      return ( (xXBeeRemoteAtCmdRespPkt *) pkt)->status;

      /* XBEE_PKT_TYPE_MODEM_STATUS 0x8A: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_MODEM_STATUS:
      return ( (xXBeeModemStatusPkt *) pkt)->status;

      // XBEE_SERIES_S1
      /* XBEE_PKT_TYPE_TX_STATUS 0x89: S1 Series */
    case XBEE_PKT_TYPE_TX_STATUS:
      return ( (xXBeeTxStatusPkt *) pkt)->status;

      // XBEE_SERIES_S2
      /* XBEE_PKT_TYPE_ZB_TX_STATUS 0x8B: S2 Series */
    case XBEE_PKT_TYPE_ZB_TX_STATUS:
      return ( (xXBeeZbTxStatusPkt *) pkt)->status;

    default:
      break;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iXBeePktDiscovery (xXBeePkt *pkt) {

  if (ucXBeePktType (pkt) == XBEE_PKT_TYPE_ZB_TX_STATUS) {

    // XBEE_SERIES_S2
    return ( (xXBeeZbTxStatusPkt *) pkt)->discovery;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iXBeePktRetry (xXBeePkt *pkt) {

  if (ucXBeePktType (pkt) == XBEE_PKT_TYPE_ZB_TX_STATUS) {

    // XBEE_SERIES_S2
    return ( (xXBeeZbTxStatusPkt *) pkt)->retry;
  }
  return -1;
}

// -----------------------------------------------------------------------------
uint8_t *
iXBeePktDst16 (xXBeePkt *pkt) {

  if (ucXBeePktType (pkt) == XBEE_PKT_TYPE_ZB_TX_STATUS) {

    // XBEE_SERIES_S2
    return ( (xXBeeZbTxStatusPkt *) pkt)->dst16;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// TODO: non testé
char *
pcXBeePktCommand (xXBeePkt *pkt) {

  switch (ucXBeePktType (pkt)) {

      /* XBEE_PKT_TYPE_ATCMD 0x08: S1 & S2 Series -- */
      /* XBEE_PKT_TYPE_QATCMD 0x09: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD:
    case XBEE_PKT_TYPE_QATCMD:
      return (char *) ( (xXBeeAtCmdPkt *) pkt)->command;

      /* XBEE_PKT_TYPE_ATCMD_RESP 0x88: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD_RESP:
      return (char *) ( (xXBeeAtCmdRespPkt *) pkt)->command;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD 0x17: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD:
      return (char *) ( (xXBeeRemoteAtCmdPkt *) pkt)->command;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD_RESP 0x97: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      return (char *) ( (xXBeeRemoteAtCmdRespPkt *) pkt)->command;

    default:
      break;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// TODO: non testé
int
iXBeePktParamLen (xXBeePkt *pkt) {
  int len = usXBeePktLength (pkt) + 3;

  switch (ucXBeePktType (pkt)) {

      /* XBEE_PKT_TYPE_ATCMD 0x08: S1 & S2 Series -- */
      /* XBEE_PKT_TYPE_QATCMD 0x09: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD:
    case XBEE_PKT_TYPE_QATCMD:
      len -= sizeof (xXBeeAtCmdPkt);
      break;

      /* XBEE_PKT_TYPE_ATCMD_RESP 0x88: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD_RESP:
      len -= sizeof (xXBeeAtCmdRespPkt);
      break;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD 0x17: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD:
      len -= sizeof (xXBeeRemoteAtCmdPkt);
      break;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD_RESP 0x97: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      len -= sizeof (xXBeeRemoteAtCmdRespPkt);
      break;

    default:
      return -1;
  }
  return len;
}

// -----------------------------------------------------------------------------
int
iXBeePktParamGetStr (char * pcDest, xXBeePkt *pkt, int iDestSize) {

  int iLen = MIN (iXBeePktParamLen (pkt), iDestSize);
  if (iLen >= 0) {
    strncpy (pcDest, (char *) pucXBeePktParam (pkt), iLen);
    // Ajout du zéro final si il y a de la place
    if (iLen < iDestSize) {
      pcDest[iLen++] = '\0';
    }
  }
  else {

    iLen = -1;
  }
  return iLen;
}

// -----------------------------------------------------------------------------
int
iXBeePktParamGetULong (uint32_t * ulDest, xXBeePkt *pkt, int iOffset) {
  int iLen;
  uint32_t ulNet;

  iLen = iXBeePktParamLen (pkt) - iOffset;

  if (iLen >= sizeof (ulNet)) {

    memcpy (&ulNet, pucXBeePktParam (pkt) + iOffset, sizeof (ulNet));
    *ulDest = ntohl (ulNet);
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iXBeePktParamGetUShort (uint16_t * usDest, xXBeePkt *pkt, int iOffset) {
  int iLen;
  uint16_t usNet;

  iLen = iXBeePktParamLen (pkt) - iOffset;

  if (iLen >= sizeof (usNet)) {

    memcpy (&usNet, pucXBeePktParam (pkt) + iOffset, sizeof (usNet));
    *usDest = ntohs (usNet);
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
uint8_t *
pucXBeePktParam (xXBeePkt *pkt) {

  switch (ucXBeePktType (pkt)) {

      /* XBEE_PKT_TYPE_ATCMD 0x08: S1 & S2 Series -- */
      /* XBEE_PKT_TYPE_QATCMD 0x09: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD:
    case XBEE_PKT_TYPE_QATCMD:
      return ( (xXBeeAtCmdPkt *) pkt)->param;

      /* XBEE_PKT_TYPE_ATCMD_RESP 0x88: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_ATCMD_RESP:
      return ( (xXBeeAtCmdRespPkt *) pkt)->param;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD 0x17: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD:
      return ( (xXBeeRemoteAtCmdPkt *) pkt)->param;

      /* XBEE_PKT_TYPE_REMOTE_ATCMD_RESP 0x97: S1 & S2 Series -- */
    case XBEE_PKT_TYPE_REMOTE_ATCMD_RESP:
      return ( (xXBeeRemoteAtCmdRespPkt *) pkt)->param;

    default:
      break;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// TODO: non testé
int
iXBeePktRadius (xXBeePkt *pkt) {

  /* XBEE_PKT_TYPE_ZB_TX_REQ 0x10: S2 Series */
  if (ucXBeePktType (pkt) == XBEE_PKT_TYPE_ZB_TX_REQ) {

    // XBEE_SERIES_S2
    return ( (xXBeeZbTxReqPkt *) pkt)->radius;
  }
  return -1;
}

// -----------------------------------------------------------------------------
// TODO: non testé
int
iXBeePktApply (xXBeePkt * pkt) {

  /* XBEE_PKT_TYPE_REMOTE_ATCMD 0x17: S1 & S2 Series -- */
  if (ucXBeePktType (pkt) == XBEE_PKT_TYPE_REMOTE_ATCMD) {

    return ( (xXBeeRemoteAtCmdPkt *) pkt)->apply;
  }
  return -1;
}

// -----------------------------------------------------------------------------
// TODO: non testé
int iXBeePktRssi (xXBeePkt *pkt) {

  switch (ucXBeePktType (pkt)) {

      // XBEE_SERIES_S1
      /*  XBEE_PKT_TYPE_RX64 0x80: S1 Series */
    case XBEE_PKT_TYPE_RX64:
      return ( (xXBeeRx64Pkt *) pkt)->rssi;

      /* XBEE_PKT_TYPE_RX16 0x81: S1 Series */
    case XBEE_PKT_TYPE_RX16:
      return ( (xXBeeRx16Pkt *) pkt)->rssi;

      /* XBEE_PKT_TYPE_RX64_IO 0x82: S1 Series */
    case XBEE_PKT_TYPE_RX64_IO:
      return ( (xXBeeRxIo64Pkt *) pkt)->rssi;

      /* XBEE_PKT_TYPE_RX16_IO 0x83: S1 Series */
    case XBEE_PKT_TYPE_RX16_IO:
      return ( (xXBeeRxIo16Pkt *) pkt)->rssi;

    default:
      break;
  }
  return -1;
}

// -----------------------------------------------------------------------------
// TODO: non testé
int
iXBeePktOptions (xXBeePkt * pkt) {

  switch (ucXBeePktType (pkt)) {
      // XBEE_SERIES_S1
      /* XBEE_PKT_TYPE_TX64 0x00: S1 Series */
    case XBEE_PKT_TYPE_TX64:
      return ( (xXBeeTxReq64Pkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_RX64 0x80: S1 Series */
    case XBEE_PKT_TYPE_RX64:
      return ( (xXBeeRx64Pkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_TX16 0x01: S1 Series */
    case XBEE_PKT_TYPE_TX16:
      return ( (xXBeeTxReq16Pkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_RX16 0x81: S1 Series */
    case XBEE_PKT_TYPE_RX16:
      return ( (xXBeeRx16Pkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_RX64_IO 0x82: S1 Series */
    case XBEE_PKT_TYPE_RX64_IO:
      return ( (xXBeeRxIo64Pkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_RX16_IO 0x83: S1 Series */
    case XBEE_PKT_TYPE_RX16_IO:
      return ( (xXBeeRxIo16Pkt *) pkt)->opt;

      // XBEE_SERIES_S2
      /* XBEE_PKT_TYPE_ZB_TX_REQ 0x10: S2 Series */
    case XBEE_PKT_TYPE_ZB_TX_REQ:
      return ( (xXBeeZbTxReqPkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_ZB_RX 0x90: S2 Series */
    case XBEE_PKT_TYPE_ZB_RX:
      return ( (xXBeeZbRxPkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_ZB_RX_IO 0x92: S2 Series */
    case XBEE_PKT_TYPE_ZB_RX_IO:
      return ( (xXBeeZbRxIoPkt *) pkt)->opt;
      /* XBEE_PKT_TYPE_ZB_RX_SENSOR 0x94: S2 Series */
    case XBEE_PKT_TYPE_ZB_RX_SENSOR:
      return ( (xXBeeZbRxSensorPkt *) pkt)->opt;
    case XBEE_PKT_TYPE_ZB_NODE_IDENT:
      return ( (xXBeeZbNodeIdPkt *) pkt)->opt;
  }
  return -1;
}

#if 1
// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO -
// -----------------------------------------------------------------------------
static inline uint16_t sample_len_from_mask (uint16_t mask) {
  uint8_t len = 0;

  // Digital; all fit in 1 sample
  if (mask & 0x1ff) {
    len++;
  }

  // Analog lines 0-4
  if (len & 0x0200) {
    len++;
  }
  if (len & 0x0400) {
    len++;
  }
  if (len & 0x0800) {
    len++;
  }
  if (len & 0x1000) {
    len++;
  }
  if (len & 0x2000) {
    len++;
  }

  return len;
}

/* -----------------------------------------------------------------------------
 * Note: no checking for case where packet is corrupt (says has more samples
 * than really has) if request for higher sample than really contains.
 */
int
iXBeePktDigital (xXBeePkt * pkt, int array[9], unsigned int index) {
  uint16_t mask = 0;
  uint16_t pin_vals = 0;
  uint8_t i;
  uint8_t samp_len;
  uint8_t npins = 0;

  for (i = 0; i < 9; i++) {

    array[i] = -1;
  }

  if (pkt->type == XBEE_PKT_TYPE_RX16_IO) {
    xXBeeRxIo16Pkt *io16pkt = (xXBeeRxIo16Pkt *) pkt;
    mask = ntohs (io16pkt->ch_mask);
    samp_len = sample_len_from_mask (mask);

    if ( (mask & 0x1ff) != 0 && (index < io16pkt->num_samples)) {
      pin_vals = ntohs (io16pkt->data[index * samp_len]);
    }
  }
  else if (pkt->type == XBEE_PKT_TYPE_RX64_IO) {
    xXBeeRxIo64Pkt *io64pkt = (xXBeeRxIo64Pkt *) pkt;
    mask = ntohs (io64pkt->ch_mask);
    samp_len = sample_len_from_mask (mask);

    if ( (mask & 0x1ff) != 0 && (index < io64pkt->num_samples)) {
      pin_vals = ntohs (io64pkt->data[index * samp_len]);
    }
  }
  else {
    // Unusable packet type
    return -1;
  }

  for (i = 0; i < 9; i++) {
    uint16_t b = (1 << i);
    if (mask & b) {
      if (pin_vals & b) {
        array[i] = 1;
      }
      else {
        array[i] = 0;
      }
      npins++;
    }
  }

  return npins;
}


/* -----------------------------------------------------------------------------
 * Note: no checking for case where packet is corrupt (says has more samples
 * than really has) if request for higher sample than really contains.
 */
int
iXBeePktAnalog (xXBeePkt * pkt, int array[6], unsigned int index) {
  uint16_t mask = 0;
  uint8_t pos = 0;
  uint8_t npins = 0;
  uint8_t i;
  uint8_t samp_len;


  for (i = 0; i < 6; i++) {
    array[i] = -1;
  }

  if (pkt->type == XBEE_PKT_TYPE_RX16_IO) {
    xXBeeRxIo16Pkt *io16pkt = (xXBeeRxIo16Pkt *) pkt;
    mask = ntohs (io16pkt->ch_mask);
    samp_len = sample_len_from_mask (mask);

    if (mask & 0x1ff) {
      // Skip digital readings
      pos++;
    }

    for (i = 0; i < 6; i++) {
      if (mask & (0x200 << i)) {
        array[i] = ntohs (io16pkt->data[index * samp_len + pos]);
        pos++;
        npins++;
      }
    }
  }
  else if (pkt->type == XBEE_PKT_TYPE_RX64_IO) {
    xXBeeRxIo64Pkt *io64pkt = (xXBeeRxIo64Pkt *) pkt;
    mask = ntohs (io64pkt->ch_mask);
    samp_len = sample_len_from_mask (mask);

    if (mask & 0x1ff) {
      // Skip digital readings
      pos++;
    }

    for (i = 0; i < 6; i++) {
      if (mask & (0x200 << i)) {
        array[i] = ntohs (io64pkt->data[index * samp_len + pos]);
        pos++;
        npins++;
      }
    }
  }
  else {
    return -1;
  }

  return npins;
}


/* -----------------------------------------------------------------------------
 */
int
iXBeePktSamples (xXBeePkt * pkt) {

  if (pkt->type == XBEE_PKT_TYPE_RX16_IO) {
    return ( (xXBeeRxIo16Pkt *) pkt)->num_samples;
  }
  else if (pkt->type == XBEE_PKT_TYPE_RX64_IO) {
    return ( (xXBeeRxIo16Pkt *) pkt)->num_samples;
  }

  return -1;
}

// -----------------------------------------------------------------------------
// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO -
#endif

/* default public functions ================================================= */

/* -----------------------------------------------------------------------------
 * @brief Alloue la mémoire pour un paquet
 *
 * Cette fonction est utilisée par la bibliothèque pour allouer de la mémoire. \n
 * Le système d'allocation mémoire peut être adapté en fonction des besoins. \n
 * Par défaut, AvrIO utilise malloc() pour l'allocation et ne tiens pas compte de
 * direction. Si l'utilisateur ne souhaite pas utiliser ce mécanisme, il devra
 * réimplémenter cette fonction ainsi que \ref vXBeeFreePkt(). \n
 * Il pourra alors utiliser le paramètre direction pour différencier le
 * mécansime d'allocation.
 *
 * @param xbee pointeur sur le contexte
 * @param direction permet de diférencier le mécanisme d'allocation (inutilisé par défaut)
 * @param len taille en octet du buffer demandé
 * @return pointeur sur le buffer alloué
 */
void *
pvXBeeAllocPkt (xXBee * xbee, uint8_t direction, uint8_t len) {

  return malloc (len);
}

/* -----------------------------------------------------------------------------
 * @brief Libère un paquet alloué avec pvXBeeAllocPkt()
 *
 * Par défaut, AvrIO utilise free(). Si l'utilisateur ne souhaite pas utiliser
 * ce mécanisme, il devra réimplémenter cette fonction ainsi
 * que \ref pvXBeeAllocPkt().
 */
void
vXBeeFreePkt (xXBee * xbee, xXBeePkt * pkt) {

  free (pkt);
}

/* ========================================================================== */
