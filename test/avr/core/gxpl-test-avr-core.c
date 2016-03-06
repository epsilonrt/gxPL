/**
 * @file
 * gxPL Core test
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define TERM_PORT         "tty0"
#define TERM_BAUDRATE     500000
#define TERM_FLOW         SERIAL_FLOW_RTSCTS

#define XBEE_RESET_PORT   PORTB
#define XBEE_RESET_PIN    7

#define IOLAYER_NAME      "xbeezb"
#define IOLAYER_PORT      "tty1"
#define IOLAYER_BAUDRATE  38400

#define HBEAT_INTERVAL    5
#define POLL_RATE         100

/* private variables ======================================================== */
static gxPLApplication * app;
static gxPLMessage * message;
static bool hasHub;
static int test_count;
static int msg_count;
static unsigned long timeout;
static volatile int ret;
static volatile const char * str;

static const char iolayer[] = IOLAYER_NAME;
static const char ioport[] = IOLAYER_PORT;

static const gxPLId source = {
  .vendor = "epsirt",
  .device = "test",
  .instance = "core"
};

/* private functions ======================================================== */
static void prvMessageHandler (gxPLApplication * app, gxPLMessage * msg, void * p);
static void prvHeartbeatMessageNew (void);

#ifndef __AVR__
static const char success[] = "Success\n";
#define iTermInit() (0)
#define PSUCCESS() printf(success)
#define PRINTF(fmt,...) printf(fmt,##__VA_ARGS__)
#define FFLUSH fflush
#else
static xDPin xResetPin = { .port = &XBEE_RESET_PORT, .pin = XBEE_RESET_PIN };
static const char success[] PROGMEM = "Success\n";
static int iTermInit (void);
#define PSUCCESS() printf_P (success)
#define PRINTF(fmt,...) printf_P(PSTR(fmt),##__VA_ARGS__)
#define FFLUSH iFileFlush
INLINE void alarm (unsigned long nbsec) {
  timeout = ( (nbsec) * 1000) / POLL_RATE;
}
#endif

/* main ===================================================================== */
int
main (int argc, char **argv) {
  static volatile const gxPLIoAddr * info;
  xVector * iolist;

  gxPLSetting * setting;
  char hello[] = ".";

  iTermInit();
  vLogSetMask (LOG_UPTO (LOG_DEBUG));
  PRINTF ("\ngxPLCore test for AVR\n"
          "gxPL Lib Version: %s\n"
          "Libc Version: %s\n"
          "Press any key to proceed...\n",
          gxPLVersion(), __AVR_LIBC_VERSION_STRING__);
  getchar();

  // Gets the available io layer list
  PRINTF ("\nTest %d: gxPLIoLayerList() > ", ++test_count);
  iolist = gxPLIoLayerList();
  assert (iolist);
  ret = iVectorSize (iolist);
  assert (ret > 0);
  PSUCCESS();

  // Prints all io layer available
  PRINTF ("%d io layers found:\n", ret);
  for (int i = 0; i < ret; i++) {
    const char * name = pvVectorGet (iolist, i);
    assert (name);
    PRINTF ("\tlayer[%d]: %s\n", i, name);
  }

  // verify that the requested io layer is available
  PRINTF ("\nTest %d: check if %s layer is available ? ",
          ++test_count, iolayer);
  ret = iVectorFindFirstIndex (iolist, iolayer);
  assert (ret >= 0);
  PSUCCESS();
  vVectorDestroy (iolist);

  // retrieved the requested configuration
  PRINTF ("\nTest %d: create new default setting for %s layer on %s > ",
          ++test_count, iolayer, ioport);
  setting = gxPLSettingNew (ioport, iolayer, gxPLConnectViaHub);
  assert (setting);
  setting->debug = 1;
  setting->xbee.reset = &xResetPin;
  PSUCCESS();

  // opens the xPL network
  PRINTF ("\nTest %d: open xPL network...\n", ++test_count);
  app = gxPLAppOpen (setting);
  assert (app);
  PSUCCESS();

  // View network information
  PRINTF ("\nTest %d: retrieve network informations:\n", ++test_count);

  str = gxPLIoInterfaceGet (app);
  assert (str);
  PRINTF ("\tinterface:\t%s\n", str);

  str = gxPLIoLocalAddrGet (app);
  assert (str);
  PRINTF ("\tlocal address:\t%s", str);

  info = gxPLIoInfoGet (app);
  assert (info);

  if (info->port >= 0) {

    PRINTF (":%d\n", info->port);
  }
  else {

    putchar ('\n');
  }

  str = gxPLIoBcastAddrGet (app);
  assert (str);
  PRINTF ("\tbcast address:\t%s\n", str);
  PSUCCESS();

  // adds a messages listener
  PRINTF ("\nTest %d: add a messages listener >", ++test_count);
  ret = gxPLMessageListenerAdd (app, prvMessageHandler, hello);
  assert (ret == 0);
  PSUCCESS();

  prvHeartbeatMessageNew();

  PRINTF ("\nTest %d: Starting xPL application loop...\n", ++test_count);
  alarm (10);

  for (;;) {

    // Main loop
    ret = gxPLAppPoll (app, POLL_RATE);
    assert (ret == 0);

    if (--timeout == 0) {
      msg_count++;
      PRINTF ("\n\n*** Transmitt message[%d] ***\n", msg_count);
      ret = gxPLAppBroadcastMessage (app, message);
      assert (ret > 0);
      if (hasHub) {

        alarm (HBEAT_INTERVAL * 60 - 10);
      }
      else {

        alarm (10);
      }
    }

  }
  return 0;
}

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// message handler
static void
prvMessageHandler (gxPLApplication * app, gxPLMessage * msg, void * p) {

  // See if we need to check the message for hub detection
  if ( (hasHub == false) && (gxPLAppIsHubEchoMessage (app, msg, NULL) == true)) {

    hasHub = true;
    PRINTF ("\n*** Hub detected ***\n");
  }
}

// -----------------------------------------------------------------------------
// Create Heartbeat message
static void
prvHeartbeatMessageNew (void) {
  static volatile int ret;

  PRINTF ("\nTest %d: create heartbeat message... ", ++test_count);

  message = gxPLMessageNew (gxPLMessageStatus);
  assert (message);

  ret = gxPLMessageSourceIdSet (message, &source);
  assert (ret == 0);

  ret = gxPLMessageBroadcastSet (message, true);
  assert (ret == 0);

  ret = gxPLMessageSchemaClassSet (message, "hbeat");
  assert (ret == 0);

  if (gxPLIoInfoGet (app)->family & gxPLNetFamilyInet)  {

    ret = gxPLMessageSchemaTypeSet (message, "app");
  }
  else {

    ret = gxPLMessageSchemaTypeSet (message, "basic");
  }

  ret = gxPLMessagePairAddFormat (message, "interval", "%d", HBEAT_INTERVAL);
  assert (ret == 0);

  if (gxPLIoInfoGet (app)->family & gxPLNetFamilyInet)  {

    ret = gxPLMessagePairAddFormat (message, "port", "%d", gxPLIoInfoGet (app)->port);
    assert (ret == 0);

    ret = gxPLMessagePairAdd (message, "remote-ip", gxPLIoLocalAddrGet (app));
    assert (ret == 0);
  }

  ret = gxPLMessagePairAdd (message, "version", VERSION_SHORT);
  assert (ret == 0);

  char * mstr = gxPLMessageToString (message);
  assert (mstr);
  PSUCCESS();

  PRINTF ("%s", mstr);
  FFLUSH (stdout);
  free (mstr);
}

#ifdef __AVR__
// -----------------------------------------------------------------------------
static int
iTermInit (void) {
  xSerialIos term_setting = {
    .baud = TERM_BAUDRATE, .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE, .sbits = SERIAL_STOPBIT_ONE, 
    .flow = TERM_FLOW, .eol = SERIAL_CRLF
  };

  FILE * tc = xFileOpen (TERM_PORT, O_RDWR, &term_setting);
  if (!tc) {
    return -1;
  }
  stdout = tc;
  stderr = tc;
  stdin = tc;
  sei();
  return 0;
}
#endif

/* ========================================================================== */
