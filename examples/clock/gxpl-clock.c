/**
 * @brief  Simple xPL clock device that sends a time update out
 *
 * Copyright (c) 2004, Gerald R. Duprey Jr.
 * Copyright 2015-2016 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdlib.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define CLOCK_VERSION         VERSION_SHORT
#define CLOCK_UPDATE_INTERVAL 30
#define REPORT_OWN_MESSAGE    true
#define POLL_RATE_MS          1000

#ifdef __AVR__
// -----------------------------------AVR---------------------------------------
/* constants ================================================================ */
/* =============================================================================
 * AVR has no command line or terminal, constants below allow to setup the 
 * io layer and the serial port terminal, if NLOG is defined in the makefile, 
 * no terminal is used.
 * ============================================================================= 
 */
#define AVR_IOLAYER_NAME      "xbeezb"
#define AVR_IOLAYER_PORT      "tty1"

//#define AVR_XBEE_RESET_PORT   PORTB
//#define AVR_XBEE_RESET_PIN    7

#define AVR_TERMINAL_PORT     "tty0"
#define AVR_TERMINAL_BAUDRATE 500000
#define AVR_TERMINAL_FLOW     SERIAL_FLOW_NONE

#define AVR_INTERRUPT_BUTTON  BUTTON_BUTTON1

/* private variables ======================================================== */
#if defined(AVR_XBEE_RESET_PORT) && defined(AVR_XBEE_RESET_PIN)
static xDPin xResetPin = {
  .port = &AVR_XBEE_RESET_PORT,
  .pin = AVR_XBEE_RESET_PIN
};
#endif

#else /* __AVR__ not defined */
// ----------------------------------UNIX---------------------------------------
#include <signal.h>
#include <unistd.h>

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;

// -----------------------------------------------------------------------------
#endif /* __AVR__ not defined */
#include <gxPL/stdio.h>

/* private variables ======================================================== */
static gxPLApplication * app;
static gxPLDevice * device;
static gxPLMessage * message;

/* private functions ======================================================== */
static void prvSendTick (void);
static void prvMessageListener (gxPLDevice * device, gxPLMessage * msg, void * udata) ;
static void prvCloseAll (void);

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  int ret;
  gxPLSetting * setting;

#ifdef __AVR__
  gxPLStdIoOpen();
  setting = gxPLSettingNew (AVR_IOLAYER_PORT, AVR_IOLAYER_NAME, gxPLConnectViaHub);
  assert (setting);
#if defined(AVR_XBEE_RESET_PORT) && defined(AVR_XBEE_RESET_PIN)
  setting->xbee.reset = &xResetPin;
#endif
  setting->log = LOG_DEBUG;
  gxPLPrintf ("Press any key to start...\n");
  gxPLWait();
#else
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  assert (setting);
#endif

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    PERROR ("Unable to start xPL");
    gxPLExit (EXIT_FAILURE);
  }

  // Initialize clock device
  // Create  a device for us
  device = gxPLAppAddDevice (app, "epsirt", "clock", NULL);
  assert (device);

  ret = gxPLDeviceVersionSet (device, CLOCK_VERSION);
  assert (ret == 0);

  ret = gxPLDeviceReportOwnMessagesSet (device, REPORT_OWN_MESSAGE);
  assert (ret == 0);

  // Add a responder for time setting
  ret = gxPLDeviceListenerAdd (device, prvMessageListener, gxPLMessageAny,
                               NULL, NULL, NULL);
  assert (ret == 0);

  // Create a message to send
  message = gxPLDeviceMessageNew (device, gxPLMessageStatus);
  assert (message);
  // Setting up the message
  ret = gxPLMessageBroadcastSet (message, true);
  assert (ret == 0);
  ret = gxPLMessageSchemaSet (message, "clock", "update");
  assert (ret == 0);

#ifdef __AVR__
  gxPLPrintf ("Press Button to abort ...\n");
#else
  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);
  gxPLPrintf ("Press Ctrl+C to abort ...\n");
#endif

  // Enable the device
  ret = gxPLDeviceEnable (device, true);
  assert (ret == 0);

  // Main Loop of Clock Action
  for (;;) {

    // Let XPL run for a while, returning after it hasn't seen any
    // activity in 100ms or so
    ret = gxPLAppPoll (app, POLL_RATE_MS);
    assert(ret == 0);

    // Process clock tick update checking
    prvSendTick();
    
    if (gxPLIsInterrupted()) {

      prvCloseAll();
      gxPLExit (EXIT_SUCCESS);
    }
  }
}

// -----------------------------------------------------------------------------
static void
prvSendTick (void) {

  if (gxPLDeviceIsHubConfirmed (device) == true) {
    static unsigned long last;
    unsigned long now = gxPLTime();

    // Skip unless the delay has passed
    if ( (last == 0) || ( (now - last) >= CLOCK_UPDATE_INTERVAL)) {
      char * strtime = gxPLDateTimeStr (now, NULL);

      // Install the value and send the message
      gxPLMessagePairSet (message, "time", strtime);

      // Broadcast the message
      gxPLDeviceMessageSend (device, message);

      // And reset when we last sent the clock update
      last = now;
    }
  }
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
prvCloseAll (void) {
  int ret;

  // Sends heartbeat end messages to all devices
  ret = gxPLAppDisableAllDevices (app);
  assert (ret == 0);

#ifdef __AVR__
  gxPLPrintf ("\nPress any key to close...\n");
  gxPLWait();
#endif
  ret = gxPLAppClose (app);
  assert (ret == 0);

  gxPLMessageDelete (message);

  gxPLPrintf ("\neverything was closed.\nHave a nice day !\n");
}

#ifndef __AVR__
// -----------------------------------------------------------------------------
// unix signal handler
static void
prvSignalHandler (int sig) {

  prvCloseAll();
  gxPLExit (EXIT_SUCCESS);
}
#endif

/* ========================================================================== */
