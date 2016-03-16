/**
 * @file
 * Simple configurable xPL device device that sends a time update periodically
 * The available options are:
 *    -i / --interface xxx : interface or device used to access the network
 *    -n / --net       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 *    -b / --baudrate      : serial baudrate (if iolayer use serial port)
 *
 * Copyright 2005 (c), Gerald R Duprey Jr
 * Copyright (c) 2015-2016, Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <gxPL.h>
#include "version-git.h"
#include <gxPL/stdio.h>

/* constants ================================================================ */
#define CLOCK_VERSION         VERSION_SHORT
#define DEFAULT_TICK_RATE     60
#define TICK_RATE_CFG_NAME    "tickrate"
#define STARTED_CFG_NAME      "started"
#define REPORT_OWN_MESSAGE    true
#define DEFAULT_CONFIG_FILE   "gxpl-clock.xpl"
#define POLL_RATE_MS          1000
#define GXPL_DMEM_DEBUG 1

/* private variables ======================================================== */
static gxPLApplication * app;
static gxPLDevice * device;
static gxPLMessage * message;

/* configurable items ======================================================= */
static int tick_rate;  // Second between ticks
static bool started;   // false to stop sending clock messages

/* private functions ======================================================== */
static void prvSendTick (void);
static void prvMessageListener (gxPLDevice * device, gxPLMessage * msg, void * udata) ;
static const char * prvIntToStr (int value);
static void prvSetConfig (gxPLDevice * device);
static void prvConfigChanged (gxPLDevice * device, void * udata);
static void prvCloseAll (void);
static void prvSignalHandler (int sig) ;

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  int ret;
  gxPLSetting * setting;

  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  assert (setting);

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    PERROR ("Unable to start xPL");
    gxPLExit (EXIT_FAILURE);
  }

  // Create a configurable device and set our application version
  device = gxPLAppAddConfigurableDevice (app, "epsirt", "clock",
                                         gxPLConfigPath (DEFAULT_CONFIG_FILE));
  assert (device);

  ret = gxPLDeviceVersionSet (device, CLOCK_VERSION);
  assert (ret == 0);

  ret = gxPLDeviceReportOwnMessagesSet (device, REPORT_OWN_MESSAGE);
  assert (ret == 0);

  // If the configuration was not reloaded, then this is our first time and
  // we need to define what the configurables are and what the default values
  // should be.
  if (gxPLDeviceIsConfigured (device) == false) {

    // Define a configurable item and give it a default
    ret = gxPLDeviceConfigItemAdd (device, TICK_RATE_CFG_NAME, gxPLConfigReconf, 1);
    assert (ret == 0);
    ret = gxPLDeviceConfigValueSet (device, TICK_RATE_CFG_NAME, prvIntToStr (DEFAULT_TICK_RATE));
    assert (ret == 0);
    ret = gxPLDeviceConfigItemAdd (device, STARTED_CFG_NAME, gxPLConfigReconf, 1);
    assert (ret == 0);
    ret = gxPLDeviceConfigValueSet (device, STARTED_CFG_NAME, "on");
    assert (ret == 0);
  }

  // Parse the device configurables into a form this program
  // can use (whether we read a setting or not)
  prvSetConfig (device);

  // Add a responder for time setting
  ret = gxPLDeviceListenerAdd (device, prvMessageListener, gxPLMessageAny,
                               NULL, NULL, NULL);
  assert (ret == 0);

  // Add a device change listener we'll use to pick up a new tick rate
  ret = gxPLDeviceConfigListenerAdd (device, prvConfigChanged, NULL);
  assert (ret == 0);

  // Create a message to send
  message = gxPLDeviceMessageNew (device, gxPLMessageStatus);
  assert (message);
  // Setting up the message
  ret = gxPLMessageBroadcastSet (message, true);
  assert (ret == 0);
  ret = gxPLMessageSchemaSet (message, "clock", "update");
  assert (ret == 0);

  gxPLPrintf ("Starting xPL Clock\n");
  gxPLPrintf ("Press Ctrl+C to abort ...\n");

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);

  // Enable the service
  ret = gxPLDeviceEnable (device, true);
  assert (ret == 0);

  for (;;) {

    ret = gxPLAppPoll (app, POLL_RATE_MS);
    assert (ret == 0);

    // Process clock tick update checking
    prvSendTick();

    if (gxPLIsInterrupted()) {

      prvCloseAll();
      gxPLExit (EXIT_SUCCESS);
    }
  }
  return 0;
}

/* private functions ======================================================== */

// --------------------------------------------------------------------------
//  Quickly to convert an integer to string
static const char *
prvIntToStr (int value) {
  static char numBuffer[10];

  sprintf (numBuffer, "%d", value);
  return numBuffer;
}

// --------------------------------------------------------------------------
//  It's best to put the logic for reading the device configuration
//  and parsing it into your code in a seperate function so it can
//  be used by your prvConfigChanged and your startup code that
//  will want to parse the same data after a setting file is loaded
static void
prvSetConfig (gxPLDevice * device) {

  // Get the tickrate
  const char * str_rate = gxPLDeviceConfigValueGet (device, TICK_RATE_CFG_NAME);
  const char * str_started = gxPLDeviceConfigValueGet (device, STARTED_CFG_NAME);
  int new_rate;
  char * endptr;

  // Handle bad configurable (override it)
  if ( (str_rate == NULL) || (strlen (str_rate) == 0)) {
    gxPLDeviceConfigValueSet (device, TICK_RATE_CFG_NAME, prvIntToStr (tick_rate));
    return;
  }

  // Convert text to a number
  new_rate = strtol (str_rate, &endptr, 10);

  if (*endptr != '\0') {
    // Bad value -- override it
    gxPLDeviceConfigValueSet (device, TICK_RATE_CFG_NAME, prvIntToStr (tick_rate));
    return;
  }

  if (strcmp (str_started, "on") == 0) {
    started = true;
  }
  else if (strcmp (str_started, "off") == 0) {
    started = false;
  }

  // Install new tick rate
  tick_rate = new_rate;
}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration
static void
prvConfigChanged (gxPLDevice * device, void * udata) {

  // Read setting items for device and install
  prvSetConfig (device);
}

// -----------------------------------------------------------------------------
// message handler
static void
prvMessageListener (gxPLDevice * device, gxPLMessage * msg, void * udata) {

  PINFO ("received %s msg from %s-%s.%s with %s.%s schema\n",
         gxPLMessageTypeToString (gxPLMessageTypeGet (msg)),
         gxPLMessageSourceVendorIdGet (msg),
         gxPLMessageSourceDeviceIdGet (msg),
         gxPLMessageSourceInstanceIdGet (msg),
         gxPLMessageSchemaClassGet (msg),
         gxPLMessageSchemaTypeGet (msg));
}

// -----------------------------------------------------------------------------
static void
prvSendTick (void) {

  if ( (tick_rate > 0) && (started) && gxPLDeviceIsHubConfirmed (device))  {
    static unsigned long last;
    unsigned long now = gxPLTime();

    // Skip unless the delay has passed
    if ( (last == 0) || ( (now - last) >= tick_rate)) {
      char * strtime = gxPLDateTimeStr (now, NULL);

      // Install the value and send the message
      gxPLMessagePairSet (message, "time", strtime);

#ifdef GXPL_DMEM_DEBUG
    int dmem = gxPLDynamicMemoryUsed();
    gxPLMessagePairSet (message, "dmem", prvIntToStr(dmem));
#endif

      // Broadcast the message
      gxPLDeviceMessageSend (device, message);

      // And reset when we last sent the clock update
      last = now;
    }
  }
}

// -----------------------------------------------------------------------------
static void
prvCloseAll (void) {
  int ret;

  // Sends heartbeat end messages to all devices
  ret = gxPLAppDisableAllDevices (app);
  assert (ret == 0);

  gxPLPrintf ("\nPress any key to close...\n");
  gxPLWait();
  ret = gxPLAppClose (app);
  assert (ret == 0);

  gxPLMessageDelete (message);

  gxPLPrintf ("\neverything was closed.\nHave a nice day !\n");
}

// -----------------------------------------------------------------------------
// unix signal handler
static void
prvSignalHandler (int sig) {

  prvCloseAll();
  gxPLExit (EXIT_SUCCESS);
}

/* ========================================================================== */
