/**
 * @file
 * xPL Hardware Layer, XBee Modules Series 2 (Zigbee), API Mode (AP=1)
 *                    (avr 8-bits source code)
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#if defined(__AVR__)
#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avrio/xbee.h>
#include <avrio/task.h>
#include <avrio/mutex.h>
#include <gxPL/util.h>

#define GXPL_IO_INTERNALS
#include "io_p.h"

/* constants ================================================================ */
#define IO_NAME "xbeezb"

/* structures =============================================================== */
typedef struct xbeezb_data {
  xXBee * xbee;
  xXBeePkt * rxpkt;
  xXBeePkt * atpkt;
  int bytes_read;
  uint8_t local_addr[8];
  int max_payload;

  volatile int fid; // frame id
  int modem_status;

  xTaskHandle task;
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

    sprintf_P (buffer, PSTR ("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x"),
               zbaddr[0], zbaddr[1], zbaddr[2], zbaddr[3],
               zbaddr[4], zbaddr[5], zbaddr[6], zbaddr[7]);
  }
  else if (zbaddr_size == 2) {

    sprintf_P (buffer, PSTR ("%02x:%02x"), zbaddr[0], zbaddr[1]);
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
  static const xSerialIos default_ios  = {
    .baud = GXPL_DEFAULT_BAUDRATE,
    .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE,
    .sbits = SERIAL_STOPBIT_ONE,
    .flow = GXPL_DEFAULT_FLOW,
    .eol = SERIAL_BINARY
  };
  memcpy (&io->setting->xbee.ios, &default_ios, sizeof (xSerialIos));
}

// -----------------------------------------------------------------------------
static void
prvSetDefaultIface (gxPLIo * io) {
  const char default_iface[]  = DEFAULT_XBEE_PORT;

  strcpy (io->setting->iface, default_iface);
}

// -----------------------------------------------------------------------------
static int
prvZbDataCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  gxPLIo * io = (gxPLIo *) pvXBeeGetUserContext (xbee);

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
  gxPLIo * io = (gxPLIo *) pvXBeeGetUserContext (xbee);

  if ( (iXBeePktParamLen (pkt) >= 0) && (dp->atpkt == NULL)) {

    dp->atpkt = pkt;
    return 0;
  }

  // Flush null length and overflow packets
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvZbModemStatusCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  gxPLIo * io = (gxPLIo *) pvXBeeGetUserContext (xbee);

  dp->modem_status = iXBeePktStatus (pkt);
#ifndef __AVR__
  PDEBUG ("%s (0x%02X)",
          sXBeeModemStatusToString (dp->modem_status), dp->modem_status);
#else
  PDEBUG ("0x%02X", dp->modem_status);
#endif

  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
// Gestionnaire de status de fin de transmission
static int
prvZbTxStatusCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  gxPLIo * io = (gxPLIo *) pvXBeeGetUserContext (xbee);
  int fid = iXBeePktFrameId (pkt);

  if ( (fid > 0) && (fid == dp->fid)) {
    int status = iXBeePktStatus (pkt);

    if (status != 0) {
#ifndef __AVR__
      PWARNING ("frame #%d: %s (0x%02X)", fid,
                sXBeeTransmitStatusToString (status), status);
#else
      PWARNING ("frame #%d: 0x%02X", fid, status);
#endif
    }
    dp->fid = 0;
  }
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvZbNodeIdCB (xXBee * xbee, xXBeePkt * pkt, uint8_t len) {

  PINFO ("%s joined zigbee network",
         prvZbAddrToString (pucXBeePktAddrRemote64 (pkt), 8));
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvWaitStatusModem (gxPLIo * io, int status, unsigned int timeout_ms) {
  int ret;
  bool xStarted;

  // start timeout timer
  ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {

    vTaskSetInterval (dp->task, xTaskConvertTicks (timeout_ms));
    vTaskStart (dp->task);
  }

  do  {

    // loop until modem was joined (or timeout or error)
    ret = iXBeePoll (dp->xbee, 0);
    xStarted = xTaskIsStarted (dp->task);
  }
  while ( (dp->modem_status != status) && (ret == 0) && xStarted);

  if (ret != 0) {

    PERROR ("Modem status error %d", ret);
  }
  else if (dp->modem_status != status) {

    PWARNING ("Modem status timeout");
    ret = -2;
  }
  return ret;
}

// -----------------------------------------------------------------------------
static int
prvWaitPacket (gxPLIo * io, xXBeePkt ** pkt, int timeout_ms) {
  int ret = 0;

  if (*pkt == NULL) {
    bool xStarted;

    if (timeout_ms > 0) {

      ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        vTaskSetInterval (dp->task, xTaskConvertTicks (timeout_ms));
        vTaskStart (dp->task);
      }
    }
    do  {

      // loop until AT response was received (or timeout or error)
      ret = iXBeePoll (dp->xbee, 0);
      xStarted = xTaskIsStarted (dp->task);
    }
    while ( (*pkt == NULL) && (ret == 0) && xStarted);

    if (ret != 0) {

      PERROR ("iXBeePoll() return %d",  ret);
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
static int
prvIoPoll (gxPLIo * io, int * available_data, int timeout_ms) {
  int ret = 0;

  ret = prvWaitPacket (io, &dp->rxpkt, timeout_ms);

  if (ret == 0) {

    if (dp->rxpkt) {

      *available_data = iXBeePktDataLen (dp->rxpkt) - dp->bytes_read;
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
                const uint8_t * params,
                uint8_t param_len,
                unsigned int timeout_ms) {
  int ret;

  // Clear previous AT response
  vXBeeFreePkt (dp->xbee, dp->atpkt);
  dp->atpkt = NULL;

  int frame_id = iXBeeSendAt (dp->xbee, cmd, params, param_len);

  ret = prvWaitPacket (io, &dp->atpkt, timeout_ms);

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
          PERROR ("AT%2s failed 0x%02X\n", cmd, ret);
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
    int ret;

    ret = iXBeeClose (dp->xbee);
    vXBeeFreePkt (dp->xbee, dp->atpkt);
    dp->atpkt = NULL;
    vXBeeFreePkt (dp->xbee, dp->rxpkt);
    dp->rxpkt = NULL;
    free (dp->xbee);
    free (io->pdata);
    io->pdata = NULL;
    return ret;
  }
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLXBeeZbOpen (gxPLIo * io) {

  if (io->pdata == NULL) {
    xXBee * xbee;
    int ret;
    uint16_t word;
    bool resetdone = false;
    xDPin * reset = NULL;

    if (io->setting->iosflag == 0) {

      prvSetDefaultIos (io);
    }

    if (strlen (io->setting->iface) == 0) {

      prvSetDefaultIface (io);
    }

#if defined(CONFIG_XBEE_RESET_PORT) && defined(CONFIG_XBEE_RESET_PIN)
    reset = &io->setting->xbee.reset_pin;
    reset->port = &CONFIG_XBEE_RESET_PORT;
    reset->pin = CONFIG_XBEE_RESET_PIN;
    io->setting->xbee.reset_hw = 1;
#else
    io->setting->xbee.reset_hw = 0;
#endif

    xbee = xXBeeNew (XBEE_SERIES_S2, reset);
    assert (xbee);

    io->pdata = calloc (1, sizeof (xbeezb_data));
    assert (io->pdata);

    vXBeeSetUserContext (xbee, io);
    vXBeeSetCB (xbee, XBEE_CB_AT_LOCAL, prvZbLocalAtCB);
    vXBeeSetCB (xbee, XBEE_CB_MODEM_STATUS, prvZbModemStatusCB);

    dp->xbee = xbee;
    dp->task = xTaskCreate (0, NULL);
    assert (dp->task != AVRIO_KERNEL_ERROR);

    if (iXBeeOpen (xbee, io->setting->iface, &io->setting->xbee.ios) != 0) {

      PERROR ("XBee open %d", errno);
      free (io->pdata);
      io->pdata = NULL;
      return -1;
    }

#ifdef GXPL_XBEEZB_HAS_HWRESET
    if (io->setting->xbee.reset_hw)  {

      PINFO ("Please wait while hardware reset...");
      if (prvWaitStatusModem (io, XBEE_PKT_MODEM_HARDWARE_RESET, 5000) != 0) {

        gxPLXBeeZbClose (io);
        return -1;
      }
      else {
        resetdone = true;
        delay_ms (500);
      }
    }
#endif

    // Reads the firmware version
    ret = prvSendLocalAt (io, XBEE_CMD_VERS_FIRMWARE, NULL, 0, 1000);
    if (ret != 0) {

      PERROR ("XBee read %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }

    // Checks firmware version for Series 2 (Zigbee)
    if (iXBeePktParamLen (dp->atpkt) > 0) {
      uint8_t fwid;

      fwid = *pucXBeePktParam (dp->atpkt);
      if ( ( (fwid & 0xF0) != 0x20) || ( (fwid & 1) == 0)) {

        PERROR ("Bad XBee: 0x%02Xxx", fwid);
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

      PERROR ("XBee read %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }

    memcpy (&dp->local_addr[0], pucXBeePktParam (dp->atpkt), 4);

    ret = prvSendLocalAt (io, XBEE_CMD_SER_LO, NULL, 0, 1000);
    if (ret != 0) {

      PERROR ("XBee read %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }
    memcpy (&dp->local_addr[4], pucXBeePktParam (dp->atpkt), 4);

    if (io->setting->xbee.new_panid) {

      // New PAN ID
      ret = prvSendLocalAt (io, XBEE_CMD_PAN_ID, NULL, 0, 1000);
      if (ret != 0) {

        PERROR ("XBee read %d", ret);
        gxPLXBeeZbClose (io);
        return -1;
      }
      else {
        uint64_t panid;

        ret = iXBeePktParamGetULongLong (&panid, dp->atpkt, 0);

        if ( (ret == 0) && (panid != io->setting->xbee.panid)) {
          uint64_t new_panid = htonll (io->setting->xbee.panid);

          // current and setting PAN ID differs: change to new PAN ID
          PINFO ("Write PAN ID 0x%lu", io->setting->xbee.panid);
          ret = prvSendLocalAt (io, XBEE_CMD_PAN_ID, (uint8_t *) &new_panid, 8, 1000);
          if (ret != 0) {

            PERROR ("XBee read %d", ret);
            gxPLXBeeZbClose (io);
            return -1;
          }

          ret = prvSendLocalAt (io, XBEE_CMD_WRITE_PARAMS, NULL, 0, 2000);
          if (ret != 0) {

            PERROR ("XBee read %d", ret);
            gxPLXBeeZbClose (io);
            return -1;
          }
        }
      }
    }

    if ((io->setting->xbee.reset_sw) && !(io->setting->xbee.reset_hw)) {

      ret = prvSendLocalAt (io, XBEE_CMD_RESET_SOFT, NULL, 0, 1000);
      if (ret != 0) {

        PERROR ("XBee SW Reset %d", ret);
        gxPLXBeeZbClose (io);
        return -1;
      }

      PINFO ("Please wait while software reset...");
      if (prvWaitStatusModem (io, XBEE_PKT_MODEM_WATCHDOG_TIMER_RESET, 5000) != 0) {

        gxPLXBeeZbClose (io);
        return -1;
      }
      else {

        resetdone = true;
      }
    }

    if ( (resetdone) && (io->setting->xbee.coordinator == 0)) {

      PINFO ("Please wait while joining Zigbee network...");
      if (prvWaitStatusModem (io, XBEE_PKT_MODEM_JOINED_NETWORK, 20000) != 0) {

        gxPLXBeeZbClose (io);
        return -1;
      }
    }

    // Gets Association status
    ret = prvSendLocalAt (io, XBEE_CMD_ASSOC_STATUS, NULL, 0, 1000);
    if (ret != 0) {

      PERROR ("XBee read %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }
    else {
      uint8_t status;

      if (iXBeePktParamGetUByte (&status, dp->atpkt, 0) == 0) {

        if (status != 0) {

#ifndef __AVR__
          PERROR ("Association failed %s (0x%02X)",
                  sXBeeAssociationStatusToString (status), status);
#else
          PERROR ("Association failed 0x%02X", status);
#endif
          gxPLXBeeZbClose (io);
          return -1;
        }
        else {

          PDEBUG ("Association successfull");
        }
      }
    }

    vXBeeSetCB (dp->xbee, XBEE_CB_DATA, prvZbDataCB);
    vXBeeSetCB (dp->xbee, XBEE_CB_TX_STATUS, prvZbTxStatusCB);
    if (io->setting->xbee.coordinator) {

      vXBeeSetCB (dp->xbee, XBEE_CB_NODE_IDENT, prvZbNodeIdCB);
    }

    // Gets Operating PAN ID
    ret = prvSendLocalAt (io, XBEE_CMD_OPERATING_PAN_ID, NULL, 0, 1000);
    if (ret != 0) {

      PERROR ("XBee read %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }
    else {
      uint64_t panid;

      ret = iXBeePktParamGetULongLong (&panid, dp->atpkt, 0);
      if (ret == 0) {

        PINFO ("Operating PAN ID: %lu", panid);
      }
    }

    // Gets Maximum RF payload bytes (NP)
    ret = prvSendLocalAt (io, XBEE_CMD_MAX_PAYLOAD, NULL, 0, 1000);
    if (ret != 0) {

      PERROR ("XBee read %d", ret);
      gxPLXBeeZbClose (io);
      return -1;
    }

    if (iXBeePktParamGetUShort (&word, dp->atpkt, 0) == 0) {

      dp->max_payload = word;
      PDEBUG ("RF payload: %d bytes max.", dp->max_payload);
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

    int bytes_read = MIN (iXBeePktDataLen (dp->rxpkt), count);
    if (source)  {

      // Get and copy the source address
      source->family = gxPLNetFamilyZigbee64;
      source->addrlen = 8;
      source->port = -1;
      source->flag = 0;
      memcpy (source->addr, pucXBeePktAddrSrc64 (dp->rxpkt), source->addrlen);
    }

    memcpy (buffer, &data[dp->bytes_read], bytes_read);
    dp->bytes_read += bytes_read;

    if (dp->bytes_read >= iXBeePktDataLen (dp->rxpkt)) {
      // the received packet has been read, you can delete it !
      vXBeeFreePkt (dp->xbee, dp->rxpkt);
      dp->rxpkt = NULL;
      dp->bytes_read = 0;
    }
    return bytes_read;
  }
  return 0;
}

// -----------------------------------------------------------------------------
static int
gxPLXBeeZbSend (gxPLIo * io, const void * buffer, int count,
                const gxPLIoAddr * target) {
  const uint8_t * dst64 = NULL;
  const uint8_t * dst16 = NULL;
  int fid = -1;

  if (target) {

    // target provided
    if (target->isbroadcast == 0) {

      // target is not broadcast
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

      // target is broadcast
      dst16 = pucXBeeAddr16Unknown();
      dst64 = pucXBeeAddr64Broadcast();
    }
  }
  else {

    // target not provided, broadcast if coordinator else deliver to coordinator
    if (io->setting->xbee.coordinator) {

      dst64 = pucXBeeAddr64Broadcast();
    }
    else {

      dst64 = pucXBeeAddr64Coordinator();
    }
    dst16 = pucXBeeAddr16Unknown();
  }

  fid = iXBeeZbSend (dp->xbee, buffer, count, dst64, dst16, 0, 0);

  if (fid < 0) {

    PERROR ("XBee deliver %d", fid);
  }
  else {

    dp->fid = fid;
    PDEBUG ("Send frame #%d", fid);
  }

  return fid;
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
        const char ** str_addr = (const char **) va_arg (ap, char**);

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
#include <avrio/led.h>

// -----------------------------------------------------------------------------
void __gxplio_init
gxPLXBeeZbInit (void) {

  (void) gxPLIoRegister (IO_NAME, &ops);
}

// -----------------------------------------------------------------------------
void __gxplio_exit
gxPLXBeeZbExit (void) {

  (void) gxPLIoUnregister (IO_NAME);
}

/* ========================================================================== */
#endif /* __AVR__ defined */
