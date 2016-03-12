/**
 * @file
 * gxPL Core test
 *
 * Copyright 2015-2016 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <gxPL.h>
#include "version-git.h"
#include "config.h"

/* constants ================================================================ */
#define HBEAT_INTERVAL    5
#define POLL_RATE         1000

#ifdef __AVR__
/* constants ================================================================ */
#define AVR_IOLAYER_NAME      "xbeezb"
#define AVR_IOLAYER_PORT      "tty1"

#define AVR_XBEE_RESET_PORT   PORTB
#define AVR_XBEE_RESET_PIN    7

#define AVR_TERMINAL_PORT         "tty0"
#define AVR_TERMINAL_BAUDRATE     500000
#define AVR_TERMINAL_FLOW         SERIAL_FLOW_NONE

/* private variables ======================================================== */
static xDPin xResetPin = { .port = &AVR_XBEE_RESET_PORT, .pin = AVR_XBEE_RESET_PIN };

#endif

#define UTEST_COUNTER test_count
#include <gxPL/utest.h>

/* private variables ======================================================== */
static const gxPLId source = {
  .vendor = "epsirt",
  .device = "test",
  .instance = "core"
};

/* private variables ======================================================== */
static gxPLApplication * app;
static gxPLMessage * message;
static bool hasHub;

/* private functions ======================================================== */
static void prvMessageHandler (gxPLApplication * app, gxPLMessage * msg, void * p);
static void prvHeartbeatMessageNew (void);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  static volatile int ret;
  const char * str;
#ifndef NLOG
  long msg_count = 0;
#endif
  long timeout;
  const gxPLIoAddr * info;
  xVector * iolist;

  gxPLSetting * setting;
  char hello[] = ".";

  vLogSetMask (LOG_UPTO (LOG_DEBUG));
  gxPLStdIoOpen();
  gxPLPrintf ("\ngxPLCore test (%s)\n", GXPL_TARGET_STR);
  UTEST_PMEM_BEFORE();
  gxPLPrintf ("Press any key to proceed...\n");
  gxPLWait();

  // TEST 1
  // Gets the available io layer list
  UTEST_NEW ("gxPLIoLayerList() > ");
  iolist = gxPLIoLayerList();
  assert (iolist);
  ret = iVectorSize (iolist);
  assert (ret > 0);
  UTEST_SUCCESS();

  // TEST 2
  // Prints all io layer available
  gxPLPrintf ("%d io layers found:\n", ret);
  for (int i = 0; i < ret; i++) {
    const char * name = pvVectorGet (iolist, i);
    assert (name);
    gxPLPrintf ("\tlayer[%d]: %s\n", i, name);
  }

  // TEST 3
  // retrieved the requested configuration
#ifndef __AVR__
  UTEST_NEW ("create new default setting from command line args > ");
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  assert (setting);
#else
  UTEST_NEW ("create new default setting for %s layer on %s > ",
             AVR_IOLAYER_NAME, AVR_IOLAYER_PORT);
  setting = gxPLSettingNew (AVR_IOLAYER_PORT, AVR_IOLAYER_NAME, gxPLConnectViaHub);
  assert (setting);
  setting->xbee.reset = &xResetPin;
  setting->log = LOG_DEBUG;
#endif
  UTEST_SUCCESS();

  // TEST 4
  // verify that the requested io layer is available
  UTEST_NEW ("check if %s layer is available ? ", setting->iolayer);
  ret = iVectorFindFirstIndex (iolist, setting->iolayer);
  assert (ret >= 0);
  UTEST_SUCCESS();
  vVectorDestroy (iolist);

  // TEST 5
  // opens the xPL network
  UTEST_NEW ("open xPL network...\n");
  app = gxPLAppOpen (setting);
  assert (app);
  UTEST_SUCCESS();

  // TEST 6
  // View network information
  UTEST_NEW ("read local network interface > ");
  str = gxPLIoInterfaceGet (app);
  assert (str);
  UTEST_SUCCESS();
  gxPLPrintf ("\tinterface:\t%s\n", str);

  // TEST 7
  UTEST_NEW ("read local network address > ");
  str = gxPLIoLocalAddrGet (app);
  assert (str);
  UTEST_SUCCESS();

  // TEST 8
  UTEST_NEW ("read local network infos > ");
  info = gxPLIoInfoGet (app);
  assert (info);
  assert (info->addrlen > 0);
  UTEST_SUCCESS();

  gxPLPrintf ("\tlocal address:\t%s", str);
  if (info->port >= 0) {

    gxPLPrintf (":%d\n", info->port);
  }
  else {

    gxPLPutchar ('\n');
  }

  // TEST 9
  UTEST_NEW ("read broadcast network address > ");
  str = gxPLIoBcastAddrGet (app);
  assert (str);
  UTEST_SUCCESS();
  gxPLPrintf ("\tbcast address:\t%s\n", str);

  // TEST 10
  // adds a messages listener
  UTEST_NEW ("add a messages listener >");
  ret = gxPLMessageListenerAdd (app, prvMessageHandler, hello);
  assert (ret == 0);

  // TEST 11
  UTEST_NEW ("create heartbeat message > ");
  prvHeartbeatMessageNew();
  UTEST_SUCCESS();

  gxPLPrintf ("\nPress any key to start xPL application loop...\n");
  gxPLWait();

  // TEST 12
  UTEST_NEW ("Starting xPL application loop...\n");
  timeout = 0;
  while (hasHub == 0) {

    // Main loop
    ret = gxPLAppPoll (app, POLL_RATE);
    assert (ret == 0);

    if ( (timeout % 1000) == 0) {
      // print a dot each second
      gxPLPutchar ('.');
      gxPLFflush (stdout);
    }

    if ( (timeout -= POLL_RATE) <= 0) {

      // Send heartbeat message each 10 seconds
      gxPLPrintf ("\n\n*** Send heartbeat[%ld] ***\n", ++msg_count);
      ret = gxPLAppBroadcastMessage (app, message);
      assert (ret > 0);
      timeout = 10000;
    }
  }
  UTEST_SUCCESS();

  // TEST 13
  if (hasHub) {

    UTEST_NEW ("\n*** send heartbeat end ***\n");
    ret = gxPLMessageSchemaTypeSet (message, "end");
    assert (ret == 0);
    ret = gxPLAppBroadcastMessage (app, message);
    assert (ret > 0);
    UTEST_SUCCESS();
  }

  // TEST 14
  gxPLPrintf ("\nPress any key to close...\n");
  gxPLWait();

  UTEST_NEW ("close xPL network...\n");
  ret = gxPLAppClose (app);
  assert (ret == 0);
  UTEST_SUCCESS();

  gxPLMessageDelete (message);

  gxPLPrintf ("\n******************************************\n");
  gxPLPrintf ("**** All tests (%d) were successful ! ****\n", test_count);
  gxPLPrintf ("******************************************\n");
  UTEST_PMEM_AFTER();
  gxPLFflush (stdout);
  gxPLStop();
  return 0;
}

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// message handler
static void
prvMessageHandler (gxPLApplication * app, gxPLMessage * msg, void * p) {

  PINFO ("Received from %s-%s.%s of type %d for %s.%s\n",
         gxPLMessageSourceVendorIdGet (msg),
         gxPLMessageSourceDeviceIdGet (msg),
         gxPLMessageSourceInstanceIdGet (msg),
         gxPLMessageTypeGet (msg),
         gxPLMessageSchemaClassGet (msg),
         gxPLMessageSchemaTypeGet (msg));

  // See if we need to check the message for hub detection
  if ( (hasHub == false) && (gxPLAppIsHubEchoMessage (app, msg, NULL) == true)) {

    hasHub = true;
    PNOTICE ("\n*** Hub confirmed ***\n");
  }
}

// -----------------------------------------------------------------------------
// Create Heartbeat message
static void
prvHeartbeatMessageNew (void) {
  static volatile int ret;

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
  assert (ret == 0);

  ret = gxPLMessagePairAddFormat (message, "interval", "%d", HBEAT_INTERVAL);
  assert (ret == 0);

  if (gxPLIoInfoGet (app)->family & gxPLNetFamilyInet)  {

    ret = gxPLMessagePairAddFormat (message, "port", "%d", gxPLIoInfoGet (app)->port);

    ret = gxPLMessagePairAdd (message, "remote-ip", gxPLIoLocalAddrGet (app));
  }
  assert (ret == 0);

  ret = gxPLMessagePairAdd (message, "version", VERSION_SHORT);
  assert (ret == 0);

#if CONFIG_HBEAT_BASIC_EXTENSION
  if (gxPLIoInfoGet (app)->family & gxPLNetFamilyZigbee)  {
    ret = gxPLMessagePairAdd (message, "remote-addr", gxPLIoLocalAddrGet (app));
  }
#endif

#ifndef NLOG
  char * mstr = gxPLMessageToString (message);
  assert (mstr);

  gxPLPrintf ("%s", mstr);
  gxPLFflush (stdout);
  free (mstr);
#endif
}

/* ========================================================================== */
