/**
 * @file
 * gxPLDevice test
 *
 * Copyright 2015-2016 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define HBEAT_INTERVAL    5
#define POLL_RATE_MS      1000
#define MSG_INTERVAL      30
#define DEVICES_MAX       2

#ifdef __AVR__
// -----------------------------------AVR---------------------------------------
/* constants ================================================================ */
#define AVR_IOLAYER_NAME      "xbeezb"
#define AVR_IOLAYER_PORT      "tty1"


#define AVR_TERMINAL_PORT         "tty0"
#define AVR_TERMINAL_BAUDRATE     500000
#define AVR_TERMINAL_FLOW         SERIAL_FLOW_NONE

#define AVR_INTERRUPT_BUTTON      BUTTON_BUTTON1


#else /* __AVR__ not defined */
// ----------------------------------UNIX---------------------------------------
#include <signal.h>
#include <unistd.h>

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;

#endif /* __AVR__ not defined */

#define UTEST_COUNTER test_count
#include <gxPL/utest.h>

/* private variables ======================================================== */
static gxPLApplication * app;
static gxPLDevice * device[DEVICES_MAX];
static gxPLMessage * message[DEVICES_MAX];
static  char * udata[] = {"udata1", "udata2"};
static unsigned long counter[DEVICES_MAX];
static int nof_device;

/* private functions ======================================================== */
static void prvDeviceHandler (gxPLDevice * device, gxPLMessage * msg, void * p);
static void prvCloseAll (void);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0, d;
  gxPLSetting * setting;
  char uid[GXPL_INSTANCEID_MAX + 1];
  const char * str;
  const gxPLIoAddr * info;
  long timeout;

  vLogSetMask (LOG_UPTO (LOG_DEBUG));
  gxPLStdIoOpen();
  gxPLPrintf ("\ngxPLDevice test (%s)\n", GXPL_TARGET_STR);
  UTEST_PMEM_BEFORE();
  gxPLPrintf ("Press any key to proceed...\n");
  gxPLWait();

  // TEST 1
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
  setting->log = LOG_DEBUG;
#endif
#ifndef GXPL_XBEEZB_HAS_HWRESET
  setting->xbee.reset_sw = 1;
#endif
  UTEST_SUCCESS();

  // TEST 2
  // opens the xPL network
  UTEST_NEW ("Please wait while opening xPL network...\n");
  app = gxPLAppOpen (setting);
  assert (app);
  UTEST_SUCCESS();

  // TEST 3
  // View network information
  UTEST_NEW ("read local network interface > ");
  str = gxPLIoInterfaceGet (app);
  assert (str);
  UTEST_SUCCESS();
  gxPLPrintf ("\tinterface:\t%s\n", str);

  // TEST 4
  UTEST_NEW ("read local network address > ");
  str = gxPLIoLocalAddrGet (app);
  assert (str);
  UTEST_SUCCESS();

  // TEST 5
  UTEST_NEW ("read local network infos > ");
  info = gxPLIoInfoGet (app);
  assert (info);
  assert (info->addrlen > 0);
  UTEST_SUCCESS();

  gxPLPrintf ("\tlocal address:\t%s", str);
  if (info->port >= 0) {
    /*
     * port is positive or zero, ie that the physical layer has the capability
     * to run multiple devices, so we will test it !
     */
    nof_device = DEVICES_MAX;
    gxPLPrintf (":%d\n", info->port);
  }
  else {
    /*
     * no port, only a single device is possible !
     */
    nof_device = 1;
    gxPLPutchar ('\n');
  }

  // TEST 6
  UTEST_NEW ("read broadcast network address > ");
  str = gxPLIoBcastAddrGet (app);
  assert (str);
  UTEST_SUCCESS();
  gxPLPrintf ("\tbcast address:\t%s\n", str);

  // TEST 7
  // gxPLGenerateUniqueId() Test
  UTEST_NEW ("generates a fairly unique identifier > ");
  for (int i = 0; i < 32; i++) {

    ret = gxPLGenerateUniqueId (app, uid, GXPL_INSTANCEID_MAX);
    assert (ret == GXPL_INSTANCEID_MAX);
    gxPLPrintf ("Unique id: %s\n", uid);
    gxPLTimeDelayMs (1);
  }
  UTEST_SUCCESS();

  for (d = 0; d < nof_device; d++) {

    // TEST 8-12
    UTEST_NEW ("adds a new device #%d on the network > ", d + 1);
    device[d] = gxPLAppAddDevice (app, "epsirt", "test" GXPL_TARGET_STR, NULL);
    assert (device[d]);
    assert (gxPLAppDeviceCount (app) == (d + 1));
    assert (gxPLAppDeviceAt (app, d) == device[d]);
    UTEST_SUCCESS();

    // TEST 9-13
    UTEST_NEW ("adds a message listener to device #%d > ", d + 1);
    ret = gxPLDeviceListenerAdd (device[d], prvDeviceHandler, gxPLMessageAny,
                                 NULL, NULL, udata[d]);
    assert (ret == 0);
    UTEST_SUCCESS();

    // TEST 10-14
    // Create a message to send
    UTEST_NEW ("create message to send for device #%d > ", d + 1);
    message[d] = gxPLDeviceMessageNew (device[d], gxPLMessageStatus);
    assert (message[d]);
    // Setting up the message
    ret = gxPLMessageBroadcastSet (message[d], true);
    assert (ret == 0);
    ret = gxPLMessageSchemaSet (message[d], "sensor", "basic");
    assert (ret == 0);
    ret = gxPLMessagePairAddFormat (message[d], "device", "sensor%d", d + 1);
    assert (ret == 0);
    ret = gxPLMessagePairAdd (message[d], "type", "count");
    assert (ret == 0);
    UTEST_SUCCESS();

    // TEST 11-15
    UTEST_NEW ("set version of device #%d: %s > ", d + 1, VERSION_SHORT);
    ret = gxPLDeviceVersionSet (device[d], VERSION_SHORT);
    assert (ret == 0);
    UTEST_SUCCESS();
  }

  gxPLPrintf ("\nPress any key to start xPL application loop...\n");
  gxPLWait();

  // TEST 16
  UTEST_NEW ("\n\nStarting xPL loop...\n");
  gxPLPrintf ("Press Ctrl+C to abort ...\n");
#ifndef __AVR__
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);
#endif

  timeout = 0;
  d = 0;

  for (d = 0; d < nof_device; d++) {
    // TEST 17-18
    UTEST_NEW ("enable the device #%d > ", d + 1);
    ret = gxPLDeviceEnable (device[d], true);
    assert (ret == 0);
    UTEST_SUCCESS();
  }

  for (;;) {

    // Main loop
    ret = gxPLAppPoll (app, POLL_RATE_MS);
    assert (ret == 0);

    if (gxPLIsInterrupted()) {

      prvCloseAll();
      gxPLExit (EXIT_SUCCESS);
    }

    // Process clock tick update checking
    if ( (timeout % 1000) == 0) {

      // print a dot each second
      gxPLPutchar ('.');
      gxPLFflush (stdout);
    }

    if ( (timeout -= POLL_RATE_MS) <= 0) {

      d %= nof_device;
      if (gxPLDeviceIsHubConfirmed (device[d]) == true) {

        ++counter[d];

        // Send heartbeat message each 10 seconds
        gxPLPrintf ("\n\n*** send sensor.basic msg for device #%d current=%lu ***\n",
                    d + 1, counter[d]);

        // Write the value and send the message
        ret = gxPLMessagePairSetFormat (message[d], "current", "%d", counter[d]);
        assert (ret == 0);

        // Broadcast the message
        ret = gxPLDeviceMessageSend (device[d], message[d]);
        assert (ret > 0);
      }
      timeout = MSG_INTERVAL * 1000UL;
      d++;
    }
  }

  return 0;
}

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// message handler
static void
prvDeviceHandler (gxPLDevice * device, gxPLMessage * msg, void * p) {
  char * ud = (char *) p;
  int d = gxPLAppDeviceIndex (gxPLDeviceParentGet (device), device);

  // Check if userdata provided is correct
  int ret = strcmp (ud, udata[d]);
  assert (ret == 0);

  gxPLPrintf ("\n+++ dev[%d]: received %s message +++\n", d + 1,
              gxPLMessageTypeToString (gxPLMessageTypeGet (msg)));
}

// -----------------------------------------------------------------------------
static void
prvCloseAll (void) {
  int ret;

  UTEST_SUCCESS();

  UTEST_NEW ("Disable all devices > ");
  // Sends heartbeat end messages to all devices
  ret = gxPLAppDisableAllDevices (app);
  assert (ret == 0);
  UTEST_SUCCESS();

  gxPLPrintf ("\nPress any key to close...\n");
  gxPLWait();

  UTEST_NEW ("close the xPL network > ");
  ret = gxPLAppClose (app);
  assert (ret == 0);

  gxPLPrintf ("\n******************************************\n");
  gxPLPrintf ("**** All tests (%d) were successful ! ****\n", test_count);
  gxPLPrintf ("******************************************\n");
  UTEST_PMEM_AFTER();
  gxPLFflush (stdout);
}

#ifndef __AVR__
// -----------------------------------------------------------------------------
// unix signal handler
static void
prvSignalHandler (int sig) {

  prvCloseAll();
  exit (EXIT_SUCCESS);
}
#endif

/* ========================================================================== */
