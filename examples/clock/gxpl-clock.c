/* gxPLClock.c -- Simple xPL clock service that sends a time update out once a minute */
/* Copyright (c) 2004, Gerald R. Duprey Jr. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define CLOCK_VERSION VERSION_SHORT

/* private variables ======================================================== */
static time_t lastTimeSent = 0;
static gxPLDevice * clockService = NULL;
static gxPLMessage * clockTickMessage = NULL;

/* internal public functions ================================================ */

/* -------------------------------------------------------------------------- */
void
clockMessageHandler (gxPLDevice * theService, gxPLMessage * theMessage,
                     void * userValue) {
  fprintf (stderr,
           "Received a Clock Message from %s-%s.%s of type %d for %s.%s\n",
           gxPLgetSourceVendor (theMessage),
           gxPLgetSourceDeviceID (theMessage),
           gxPLgetSourceInstanceID (theMessage),
           gxPLgetMessageType (theMessage),
           gxPLgetSchemaClass (theMessage),
           gxPLgetSchemaType (theMessage));
}

/* -------------------------------------------------------------------------- */
void
shutdownHandler (int onSignal) {

  gxPLDeviceEnabledSet (clockService, FALSE);
  gxPLDelete (clockService);
  gxPLshutdown();
  exit (0);
}

/* private functions ======================================================== */

/* -------------------------------------------------------------------------- */
static void
sendClockTick(void) {
  time_t rightNow = time (NULL);
  struct tm * decodedTime;
  char theDateTime[24];

  /* Skip unless a minute has passed (or this is our first time */
  /*if ((lastTimeSent != 0) && ((rightNow - lastTimeSent) < 60)) return; */
  if ( (lastTimeSent != 0) && ( (rightNow - lastTimeSent) < 1)) {

    return;
  }

  /* Format the date/time */
  decodedTime = localtime (&rightNow);
  strftime (theDateTime, 24, "%Y%m%d%H%M%S", decodedTime);

  /* Install the value and send the message */
  gxPLsetMessageNamedValue (clockTickMessage, "time", theDateTime);

  /* Broadcast the message */
  gxPLsendMessage (clockTickMessage);

  /* And reset when we last sent the clock update */
  lastTimeSent = rightNow;
}

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  /* Parse command line parms */
  if (!gxPLparseCommonArgs (&argc, argv, FALSE)) {
    exit (1);
  }

  /* Start xPL up */
  if (!gxPLinitialize (gxPLgetParsedConnectionType())) {
    
    fprintf (stderr, "Unable to start xPL");
    exit (1);
  }

  /* Initialze clock service */

  /* Create  a service for us */
  clockService = gxPLDeviceNew ("cdp1802", "clock", "default");
  gxPLDeviceVersionSet (clockService, CLOCK_VERSION);

  /* Add a responder for time setting */
  gxPLDeviceListenerAdd (clockService, clockMessageHandler, gxPLMESSAGE_ANY, "clock", NULL, NULL);

  /* Create a message to send */
  clockTickMessage = gxPLcreateBroadcastMessage (clockService, gxPLMESSAGE_STATUS);
  gxPLsetSchema (clockTickMessage, "clock", "update");

  /* Install signal traps for proper shutdown */
  signal (SIGTERM, shutdownHandler);
  signal (SIGINT, shutdownHandler);

  /* Enable the service */
  gxPLDeviceEnabledSet (clockService, TRUE);

  /** Main Loop of Clock Action **/

  for (;;) {
    /* Let XPL run for a while, returning after it hasn't seen any */
    /* activity in 100ms or so                                     */
    gxPLprocessMessages (100);

    /* Process clock tick update checking */
    sendClockTick();
  }
}
/* ========================================================================== */
