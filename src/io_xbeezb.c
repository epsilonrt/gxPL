/**
 * @file
 * xPL Hardware Layer, XBee Modules Series 2 (Zigbee), API Mode (AP=1)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define GXPL_IO_INTERNALS
#include "io_p.h"

#if defined(__unix__)
#include <sysio/xbee.h>
#define PROGMEM
#define memcpy_P(dst,src,size) memcpy(dst,src,size)
#define strcpy_P(dst,src) strcpy(dst,src)

#elif defined(__AVR__)
#include <avrio/xbee.h>

#else
#error This target platform is not supported.
#endif

/* constants ================================================================ */
#define DEFAUL
#define IO_NAME "xbeezb"

/* types ==================================================================== */

/* structures =============================================================== */
typedef struct xbeezb_data {
  xXBee xbee;
  xXBeePkt * rxpkt;
  xXBeePkt * atpkt;
  int bytes_read;
  uint8_t local_addr[8];
  int max_payload;

  volatile int fid; // frame id
} xbeezb_data;

/* macros =================================================================== */
#define dp ((xbeezb_data *)io->pdata)

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// gxPLNetFamilyZigbee16: xx:xx
// gxPLNetFamilyZigbee64: xx:xx:xx:xx:xx:xx:xx:xx
static const char *
prvZbAddrToString (uint8_t * zbaddr, uint8_t zbaddr_size) {
  static char buffer[8 * 3];
  
  buffer[0] = '\0';
  
  if (zbaddr_size == 8) {

    sprintf (buffer, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
             zbaddr[0], zbaddr[1], zbaddr[2], zbaddr[3],
             zbaddr[4], zbaddr[5], zbaddr[6], zbaddr[7]);
  }
  else if (zbaddr_size == 2) {

    sprintf (buffer, "%02x:%02x", zbaddr[0], zbaddr[1]);
  }
  return buffer;
}

// -----------------------------------------------------------------------------
// gxPLNetFamilyZigbee16: xx:xx
// gxPLNetFamilyZigbee64: xx:xx:xx:xx:xx:xx:xx:xx
static int
prvZbAddrFromString (gxPLIoAddr * zbaddr, const char * str) {
  int n;
  char * endptr;

  zbaddr->addrlen = 0;
  do {

    n = strtol (str, &endptr, 16);
    if ( (*endptr != ':') || (*endptr != '\0')) {

      zbaddr->addr[zbaddr->addrlen++] = n;
    }
    str = endptr + 1;
  }
  while ( (*endptr != '\0') && (zbaddr->addrlen < sizeof (zbaddr->addr)));

  if (zbaddr->addrlen == 2) {

    zbaddr->family = gxPLNetFamilyZigbee16;
  }
  else if (zbaddr->addrlen == 8) {

    zbaddr->family = gxPLNetFamilyZigbee64;
  }
  else {

    zbaddr->family = gxPLNetFamilyUnknown;
    return -1;
  }
  return 0;
}

// -----------------------------------------------------------------------------
static void
prvSetDefaultIos (gxPLIo * io) {
  const xSerialIos default_ios PROGMEM = {
    .baud = DEFAULT_XBEE_BAUDRATE,
    .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE,
    .sbits = SERIAL_STOPBIT_ONE,
    .flow = DEFAULT_XBEE_FLOW,
    .flag = 0
  };
  memcpy_P (&io->setting->xbee.ios, &default_ios, sizeof (xSerialIos));
}

// -----------------------------------------------------------------------------
static void
prvSetDefaultIface (gxPLIo * io) {
  const char default_iface[] PROGMEM = DEFAULT_XBEE_PORT;

  strcpy_P (io->setting->iface, default_iface);
}

// -----------------------------------------------------------------------------
static int
prvZbDataCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  gxPLIo * io = (gxPLIo *) xbee->user_context;

  if ( (iXBeePktDataLen (pkt) > 0) && (dp->rxpkt == NULL)) {

    dp->rxpkt = pkt;
    return 0;
  }

  // Flush null length and overflow packets
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvZbLocalAtCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  gxPLIo * io = (gxPLIo *) xbee->user_context;

  if ( (iXBeePktParamLen (pkt) > 0) && (dp->atpkt == NULL)) {

    dp->atpkt = pkt;
    return 0;
  }

  // Flush null length and overflow packets
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
// Gestionnaire de status de fin de transmission
static int
prvZbTxStatusCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  gxPLIo * io = (gxPLIo *) xbee->user_context;

  if (iXBeePktFrameId (pkt) == dp->fid) {
    int status = iXBeePktStatus (pkt);

    if (status) {

      vLog (LOG_ERR, "Tx%d Err. %d\n", dp->fid, status);
    }
    dp->fid = 0;
  }
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvZbNodeIdCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {

  vLog (LOG_INFO, "%s joined zigbee network",
        prvZbAddrToString (pucXBeePktAddrRemote64 (pkt), 8));
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvIoPoll (gxPLIo * io, int * available_data, int timeout_ms) {
  int ret = 0;

  while ( (timeout_ms > 0) && (dp->rxpkt == NULL) && (ret == 0)) {

    // loop until AT response was received (or timeout or error)
    ret = iXBeePoll (&dp->xbee, 10);
    timeout_ms -= 10;
  }

  if (ret == 0) {

    if (dp->rxpkt) {

      *available_data = iXBeePktDataLen (dp->rxpkt);
    }
    else {

      *available_data = 0;
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
static int
prvSendLocalAt (gxPLIo * io,
                const char cmd[],
                const uint8_t *params,
                uint8_t param_len,
                int timeout_ms) {
  int ret;

  // Clear previous AT response
  vXBeeFreePkt (&dp->xbee, dp->atpkt);
  dp->atpkt = NULL;

  int frame_id = iXBeeSendAt (&dp->xbee, cmd, params, param_len);

  do {

    // loop until AT response was received (or timeout)
    ret = iXBeePoll (&dp->xbee, 10);
    timeout_ms -= 10;
  }
  while ( (timeout_ms > 0) && (dp->atpkt == NULL) && (ret == 0));

  if (ret == 0) {

    if (dp->atpkt) {

      if (iXBeePktFrameId (dp->atpkt) == frame_id) {

        ret = iXBeePktStatus (dp->atpkt);
        if (ret == XBEE_PKT_STATUS_OK) {
          char * rcmd = pcXBeePktCommand (dp->atpkt);

          if (strncmp (cmd, rcmd, 2) == 0) {

            // the response matches the request
            return 0;
          }
          // bad response, the frame id being good this should not happen !
          errno = EBADMSG;
        }
        else {

          // command status error
          vLog (LOG_ERR, "AT command %2s failed with 0x%02X status\n", cmd, ret);
          errno = EIO;
        }
      }
      else {

        // bad frame id
        errno = EBADMSG;
      }
    }
    else {

      // timeout, no response
      errno = ETIMEDOUT;
    }
  }
  return -1;
}

/* private API functions ==================================================== */
// -----------------------------------------------------------------------------
static int
gxPLXBeeZbClose (gxPLIo * io) {

  if (io->pdata) {

    vSerialClose (dp->xbee.fd);
    vXBeeFreePkt (&dp->xbee, dp->atpkt);
    dp->atpkt = NULL;
    vXBeeFreePkt (&dp->xbee, dp->rxpkt);
    dp->rxpkt = NULL;
    free (io->pdata);
    io->pdata = NULL;
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLXBeeZbOpen (gxPLIo * io) {

  if (io->pdata == NULL) {
    int fd;
    int ret;
    uint16_t word;

    if (io->setting->iosflag == 0) {

      prvSetDefaultIos (io);
    }

    if (strlen (io->setting->iface) == 0) {

      prvSetDefaultIface (io);
    }

    if ( (fd = iSerialOpen (io->setting->iface, &io->setting->xbee.ios)) < 0) {

      vLog (LOG_ERR, "Unable to open serial port: %s (%d)",
            strerror (errno), errno);
      return -1;
    }

    io->pdata = calloc (1, sizeof (xbeezb_data));
    assert (io->pdata);

    ret = iXBeeInit (&dp->xbee, XBEE_SERIES_S2, fd);
    if (ret != 0) {

      gxPLXBeeZbClose (io);
      vLog (LOG_ERR, "Unable to init XBee module, return %d", ret);
      return -1;
    }

    dp->xbee.user_context = io;
    vXBeeSetCB (&dp->xbee, XBEE_CB_AT_LOCAL, prvZbLocalAtCB);

    // Gets and checks firmware version
    ret = prvSendLocalAt (io, XBEE_CMD_VERS_FIRMWARE, NULL, 0, 1000);
    if (ret != 0) {

      vLog (LOG_ERR, "Unable to read XBee module, return %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }

    if (iXBeePktParamLen (dp->atpkt) > 0) {
      uint8_t fwid;

      fwid = *pucXBeePktParam (dp->atpkt);
      if ( ( (fwid & 0xF0) != 0x20) || ( (fwid & 1) == 0)) {

        vLog (LOG_ERR, "Bad XBee module or firmware version: 0x%02Xxx", fwid);
        gxPLXBeeZbClose (io);
        return -1;
      }
      if (fwid == 0x21) {

        io->setting->xbee.coordinator = 1;
      }
    }

    // Gets 64-bit local address
    ret = prvSendLocalAt (io, XBEE_CMD_SER_HI, NULL, 0, 1000);
    if (ret != 0) {

      vLog (LOG_ERR, "Unable to read XBee module, return %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }

    memcpy (&dp->local_addr[0], pucXBeePktParam (dp->atpkt), 4);

    ret = prvSendLocalAt (io, XBEE_CMD_SER_LO, NULL, 0, 1000);
    if (ret != 0) {

      vLog (LOG_ERR, "Unable to read XBee module, return %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }
    memcpy (&dp->local_addr[4], pucXBeePktParam (dp->atpkt), 4);

    // Gets PAN ID
    ret = prvSendLocalAt (io, XBEE_CMD_PAN_ID, NULL, 0, 1000);
    if (ret != 0) {

      vLog (LOG_ERR, "Unable to read XBee module, return %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }
    else {
      uint8_t panid[8];
      uint8_t spanid[8];

      * (uint64_t *) spanid = htonll (io->setting->xbee.panid);

      memcpy (panid, pucXBeePktParam (dp->atpkt), 8);

      if (memcmp (panid, spanid, 8) != 0) {

        // current and setting PAN ID differs: change to new PAN ID
        ret = prvSendLocalAt (io, XBEE_CMD_PAN_ID, spanid, 8, 1000);
        if (ret != 0) {

          vLog (LOG_ERR, "Unable to read XBee module, return %d", ret);
          gxPLXBeeZbClose (io);
          return -1;
        }

        ret = prvSendLocalAt (io, XBEE_CMD_WRITE_PARAMS, NULL, 0, 2000);
        if (ret != 0) {

          vLog (LOG_ERR, "Unable to read XBee module, return %d", ret);
          gxPLXBeeZbClose (io);
          return -1;
        }
      }
    }
    
    // Gets Maximum RF payload bytes (NP)
    ret = prvSendLocalAt (io, XBEE_CMD_MAX_PAYLOAD, NULL, 0, 1000);
    if (ret != 0) {

      vLog (LOG_ERR, "Unable to read XBee module, return %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }

    if (iXBeePktParamGetUShort (&word, dp->atpkt, 0) == 0) {
      
      dp->max_payload = word;
      PDEBUG("Maximum RF payload %d bytes", dp->max_payload);
    }
    
    vXBeeSetCB (&dp->xbee, XBEE_CB_DATA, prvZbDataCB);
    vXBeeSetCB (&dp->xbee, XBEE_CB_TX_STATUS, prvZbTxStatusCB);
    if (io->setting->xbee.coordinator) {

      vXBeeSetCB (&dp->xbee, XBEE_CB_NODE_IDENT, prvZbNodeIdCB);
    }
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLXBeeZbRecv (gxPLIo * io, void * buffer, int count, gxPLIoAddr * source) {

  if (dp->rxpkt) {
    uint8_t * data = pucXBeePktData (dp->rxpkt);

    int bytes_to_read = MIN (iXBeePktDataLen (dp->rxpkt), count);
    if (source)  {

      // Get and copy the source address
      source->family = gxPLNetFamilyZigbee64;
      source->addrlen = 8;
      source->port = -1;
      source->flag = 0;
      memcpy (source->addr, pucXBeePktAddrSrc64 (dp->rxpkt), source->addrlen);
    }

    memcpy (buffer, &data[dp->bytes_read], bytes_to_read);
    dp->bytes_read += bytes_to_read;

    if (dp->bytes_read >= iXBeePktDataLen (dp->rxpkt)) {

      vXBeeFreePkt (&dp->xbee, dp->rxpkt);
      dp->rxpkt = NULL;
      dp->bytes_read = 0;
    }
    return bytes_to_read;
  }
  return 0;
}

// -----------------------------------------------------------------------------
static int
gxPLXBeeZbSend (gxPLIo * io, const void * buffer, int count,
                const gxPLIoAddr * target) {
  const uint8_t * dst64;
  const uint8_t * dst16;

  if (target) {

    if (target->isbroadcast == 0) {
      if (target->family == gxPLNetFamilyZigbee16) {


        dst16 = (const uint8_t *) target->addr;
        dst64 = pucXBeeAddr64Unknown();
      }
      else {

        dst16 = pucXBeeAddr16Unknown();
        dst64 = (const uint8_t *) target->addr;
      }
    }
    else {

      dst16 = pucXBeeAddr16Unknown();
      dst64 = pucXBeeAddr64Broadcast();
    }
  }
  else {

    if (io->setting->xbee.coordinator) {

      dst64 = pucXBeeAddr64Broadcast();
    }
    else {

      dst64 = pucXBeeAddr64Coordinator();
    }
    dst16 = pucXBeeAddr16Unknown();
  }

  // Try to send the message
  dp->fid = iXBeeZbSend (&dp->xbee, buffer, count, dst64, dst16, 0, 0);

  if (dp->fid < 0) {

    vLog (LOG_ERR, "Unable to deliver the message, error: %d", dp->fid);
    dp->fid = 0;
    return -1;
  }
  PDEBUG ("Send ZigBee frame #%d (%d bytes)", dp->fid, count);

  return dp->fid;
}

// -----------------------------------------------------------------------------
static int
gxPLXBeeZbCtl (gxPLIo * io, int c, va_list ap) {
  int ret = 0;

  switch (c) {

      // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncPoll, int * available_bytes, int timeout_ms)
    case gxPLIoFuncPoll: {
      int * available_bytes = va_arg (ap, int*);
      int timeout_ms = va_arg (ap, int);
      ret = prvIoPoll (io, available_bytes, timeout_ms);
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetBcastAddr, gxPLIoAddr * bcast_addr)
    case gxPLIoFuncGetBcastAddr: {
      gxPLIoAddr * bcast_addr = va_arg (ap, gxPLIoAddr*);
      bcast_addr->addrlen = 8;
      bcast_addr->family = gxPLNetFamilyZigbee64;
      bcast_addr->flag = 0;
      bcast_addr->port = -1;
      memcpy (bcast_addr->addr, pucXBeeAddr64Broadcast(), 8);
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetNetInfo, gxPLIoAddr * local_addr)
    case gxPLIoFuncGetNetInfo: {
      gxPLIoAddr * local_addr = va_arg (ap, gxPLIoAddr*);
      local_addr->addrlen = 8;
      local_addr->family = gxPLNetFamilyZigbee64;
      local_addr->flag = 0;
      local_addr->port = -1;
      memcpy (local_addr->addr, dp->local_addr, 8);
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncNetAddrToString, gxPLIoAddr * net_addr, char ** str_addr)
    case gxPLIoFuncNetAddrToString: {
      gxPLIoAddr * addr = va_arg (ap, gxPLIoAddr*);

      if ( (addr->family == gxPLNetFamilyZigbee16) ||
           (addr->family == gxPLNetFamilyZigbee64)) {
        const char ** str_addr = va_arg (ap, char**);

        *str_addr = prvZbAddrToString (addr->addr, addr->addrlen);
      }
      else {

        errno = EINVAL;
        ret = -1;
      }
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncNetAddrFromString, gxPLIoAddr * net_addr, const char * str_addr)
    case gxPLIoFuncNetAddrFromString: {
      gxPLIoAddr * net_addr = va_arg (ap, gxPLIoAddr*);
      const char * str_addr = va_arg (ap, char*);

      ret = prvZbAddrFromString (net_addr, str_addr);
    }
    break;

    default:
      errno = EINVAL;
      ret = -1;
      break;
  }

  return ret;
}

/* private variables ======================================================== */
static gxPLIoOps
ops = {
  .open  = gxPLXBeeZbOpen,
  .recv  = gxPLXBeeZbRecv,
  .send  = gxPLXBeeZbSend,
  .close = gxPLXBeeZbClose,
  .ctl   = gxPLXBeeZbCtl
};

/* public functions ========================================================= */

// -----------------------------------------------------------------------------
int __gxplio_init
gxPLXBeeZbInit (void) {

  return gxPLIoRegister (IO_NAME, &ops);
}

// -----------------------------------------------------------------------------
int __gxplio_exit
gxPLXBeeZbExit (void) {

  return gxPLIoUnregister (IO_NAME);
}

/* ========================================================================== */
