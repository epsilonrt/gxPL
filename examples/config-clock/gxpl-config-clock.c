/**
 * @file gxpl-config-clock.c
 * Simple configurable xPL clock service that sends a time update periodically 
 *
 * Copyright 2005 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define CLOCK_VERSION VERSION_SHORT

#define DEFAULT_TICK_RATE 60
#define TICK_RATE_CFG_NAME "tickrate"

/* private variables ======================================================== */
static time_t lastTimeSent = 0;
static int tickRate = 0;  /* Second between ticks */
static gxPLService * clockService = NULL;
static gxPLMessage * clockTickMessage = NULL;

static char numBuffer[10];

/* private functions ======================================================== */

/* --------------------------------------------------------------------------
 * Quickly to convert an integer to string */
static char * intToStr (int value) {

  sprintf (numBuffer, "%d", value);
  return numBuffer;
}

/* --------------------------------------------------------------------------
 * It's best to put the logic for reading the service configuration
 * and parsing it into your code in a seperate function so it can
 * be used by your configChangedHandler and your startup code that
 * will want to parse the same data after a config file is loaded */
static void
parseConfig (gxPLService * service) {
  /* Get the tickrate */
  char * newRate = xPL_getServiceConfigValue (service, TICK_RATE_CFG_NAME);
  int newTickRate;
  char * endChar;

  /* Handle bad configurable (override it) */
  if ( (newRate == NULL) || (strlen (newRate) == 0)) {
    xPL_setServiceConfigValue (service, TICK_RATE_CFG_NAME, intToStr (tickRate));
    return;
  }

  /* Convert text to a number */
  newTickRate = strtol (newRate, &endChar, 10);
  if (*endChar != '\0') {
    /* Bad value -- override it */
    xPL_setServiceConfigValue (service, TICK_RATE_CFG_NAME, intToStr (tickRate));
    return;
  }

  /* Install new tick rate */
  tickRate = newTickRate;
}

/* --------------------------------------------------------------------------
 * Handle a change to the clock service configuration */
static void
configChangedHandler (gxPLService * service, void * userData) {
  
  /* Read config items for service and install */
  parseConfig (service);
}

/* ----------------------------------------------------------------------------- 
 * When the user hits ^C, logically shutdown (including telling 
 * the network the service is ending) */                         
static void
shutdownHandler (int onSignal) {
  xPL_setServiceEnabled (clockService, FALSE);
  xPL_releaseService (clockService);
  gxPLClose();
  exit (0);
}

/* ----------------------------------------------------------------------------- 
 * Build up a message with the current time in it and send it along.  An 
 * important detail is that we use xPL_sendServiceMessage() to send the  
 * message (vs gxPLSendMessage) because this is a configurable service   
 * and since we created the message early on, it could have the wrong    
 * source identifiers after a reconfig. */                                 
static void
sendClockTick (void) {
  time_t rightNow = time (NULL);
  struct tm * decodedTime;
  char theDateTime[24];

  /* Skip unless a minute has passed (or this is our first time */
  if ( (lastTimeSent != 0) && ( (rightNow - lastTimeSent) < tickRate)) {
    return;
  }

  /* Format the date/time */
  decodedTime = localtime (&rightNow);
  strftime (theDateTime, 24, "%Y%m%d%H%M%S", decodedTime);

  /* Install the value and send the message */
  gxPLMessagePairValueSet (clockTickMessage, "time", theDateTime);

  /* Broadcast the message */
  xPL_sendServiceMessage (clockService, clockTickMessage);

  /* And reset when we last sent the clock update */
  lastTimeSent = rightNow;
}

/* main ===================================================================== */
int
main (int argc, char * argv[]) {

  /* Parse command line parms */
  if (!xPL_parseCommonArgs (&argc, argv, FALSE)) {

    exit (1);
  }

  /* Start xPL up */
  if (!gxPLNewConfig (gxPLGetConnectionType())) {

    fprintf (stderr, "Unable to start xPL");
    exit (1);
  }

  /* Initialze clock service */

  /* Create a configurable service and ser our applications version */
  clockService = xPL_createConfigurableService ("cdp1802", "clock", "clock.xpl");
  xPL_setServiceVersion (clockService, CLOCK_VERSION);

  /* If the configuration was not reloaded, then this is our first time and   */
  /* we need to define what the configurables are and what the default values */
  /* should be.                                                               */
  if (!gxPLIsServiceConfigured (clockService)) {

    /* Define a configurable item and give it a default */
    xPL_addServiceConfigurable (clockService, TICK_RATE_CFG_NAME, gxPLConfigReconf, 1);
    xPL_setServiceConfigValue (clockService, TICK_RATE_CFG_NAME, intToStr (DEFAULT_TICK_RATE));
  }

  /* Parse the service configurables into a form this program */
  /* can use (whether we read a config or not)                */
  parseConfig (clockService);

  /* Add a service change listener we'll use to pick up a new tick rate */
  xPL_addServiceConfigChangedListener (clockService, configChangedHandler, NULL);

  /* Create a message to send.  We don't have to do it here -- you can */
  /* create a message anytime and release it later.  But since we know */
  /* we're going to use this over and over, create one for our life    */
  clockTickMessage = gxPLMessageNewBroadcast (clockService, gxPLMessageStatus);
  gxPLMessageSchemaSetAll (clockTickMessage, "clock", "update");

  /* Install signal traps for proper shutdown */
  signal (SIGTERM, shutdownHandler);
  signal (SIGINT, shutdownHandler);

  /* Enable the service */
  xPL_setServiceEnabled (clockService, TRUE);

  /** Main Loop of Clock Action **/

  for (;;) {
    /* Let XPL run for a while, returning after it hasn't seen any */
    /* activity in 100ms or so                                     */
    xPL_processMessages (100);

    /* Process clock tick update checking */
    sendClockTick();
  }
}
/* ========================================================================== */
