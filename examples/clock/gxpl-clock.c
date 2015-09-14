/* xPL_Clock.c -- Simple xPL clock service that sends a time update out once a minute */
/* Copyright (c) 2004, Gerald R. Duprey Jr. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <gxPL.h>

/* private variables ======================================================== */
static gxPL * xpl;

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static void
vShutdownHandler (int sig) {
  int ret;

  ret = gxPLClose(xpl);
  assert (ret == 0);

  printf("\neverything was closed.\nHave a nice day !\n");
  exit(EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  int ret;
  
  xpl = gxPLOpenWithArgs (argc, argv, xPLConnectViaHub);
  assert (xpl);

  /* Install signal traps for proper shutdown */
  signal (SIGTERM, vShutdownHandler);
  signal (SIGINT, vShutdownHandler);
  
  for (;;) {
    // Main loop
    ret = gxPLPoll(xpl, 100);
    assert (ret == 0);
    putchar('.'); fflush(stdout);
  }

  return 0;
}
/* ========================================================================== */


#if 0
/* constants ================================================================ */
#define CLOCK_VERSION VERSION_SHORT

/* private variables ======================================================== */
static time_t lastTimeSent = 0;
static xPL_Service * clockService = NULL;
static gxPLMessage * clockTickMessage = NULL;

/* internal public functions ================================================ */

/* -------------------------------------------------------------------------- */
void
clockMessageHandler (xPL_Service * theService, gxPLMessage * theMessage,
                     xPL_Object * userValue) {
  fprintf (stderr,
           "Received a Clock Message from %s-%s.%s of type %d for %s.%s\n",
           xPL_getSourceVendor (theMessage),
           xPL_getSourceDeviceID (theMessage),
           xPL_getSourceInstanceID (theMessage),
           xPL_getMessageType (theMessage),
           xPL_getSchemaClass (theMessage),
           xPL_getSchemaType (theMessage));
}

/* -------------------------------------------------------------------------- */
void
shutdownHandler (int onSignal) {

  xPL_setServiceEnabled (clockService, FALSE);
  xPL_releaseService (clockService);
  gxPLClose();
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
  xPL_setMessageNamedValue (clockTickMessage, "time", theDateTime);

  /* Broadcast the message */
  xPL_sendMessage (clockTickMessage);

  /* And reset when we last sent the clock update */
  lastTimeSent = rightNow;
}

int
main (int argc, char * argv[]) {
  
  /* Parse command line parms */
  if (!xPL_parseCommonArgs (&argc, argv, FALSE)) {
    exit (1);
  }

  /* Start xPL up */
  if (!gxPLOpen (gxPLGetConnectionType())) {
    
    fprintf (stderr, "Unable to start xPL");
    exit (1);
  }

  /* Initialze clock service */
  /* Create  a service for us */
  clockService = xPL_createService ("cdp1802", "clock", "default");
  xPL_setServiceVersion (clockService, CLOCK_VERSION);

  /* Add a responder for time setting */
  xPL_addServiceListener (clockService, clockMessageHandler, xPLMessageAny, "clock", NULL, NULL);

  /* Create a message to send */
  clockTickMessage = xPL_createBroadcastMessage (clockService, xPLMessageStatus);
  xPL_setSchema (clockTickMessage, "clock", "update");

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

  return 0;
}

#endif
