/*
 * gxpl-test-core.c
 * @brief gxPL Core test
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <gxPL.h>
#include <sysio/delay.h>
#include "version-git.h"

/* constants ================================================================ */
#define HBEAT_INTERVAL 5

static const gxPLId source = {
  .vendor = "xpl",
  .device = "xplhal",
  .instance = "myhouse"
};

/* macros =================================================================== */
#define test(t) do { \
    if (!(t)) { \
      fprintf (stderr, "line %d in %s: test %d failed !\n",  __LINE__, \
               __FUNCTION__, test_count); \
      exit (EXIT_FAILURE); \
    } \
  } while (0)

/* private variables ======================================================== */
static gxPL * gxpl;
static gxPLMessage * message;
static bool hasHub;
static int test_count;
static int msg_count;

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;
static void prvMessageHandler (gxPL * gxpl, gxPLMessage * msg, void * p);
static void prvHeartbeatMessageNew (void);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0;
  xVector * iolist;
  gxPLSetting * setting;
  char hello[] = ".";

  // Gets the available io layer list
  test_count++;
  iolist = gxPLIoLayerList();
  test (iolist);
  printf ("%d iolayers found on this system:\nName\n", iVectorSize (iolist));
  for (int i = 0; i < iVectorSize (iolist); i++) {
    printf ("%s\n", (char *) pvVectorGet (iolist, i));
  }

  // retrieved the requested configuration from the command line
  test_count++;
  setting = gxPLSettingNewFromCommandArgs (argc, argv, gxPLConnectViaHub);
  test (setting);

  // verify that the requested io layer is available
  test_count++;
  ret = iVectorFindFirstIndex (iolist, setting->iolayer);
  test (ret >= 0);

  // opens the xPL network
  test_count++;
  vVectorDestroy (iolist);
  gxpl = gxPLOpen (setting);
  test (gxpl);

  // adds a message listener
  test_count++;
  ret = gxPLMessageListenerAdd (gxpl, prvMessageHandler, hello);
  prvHeartbeatMessageNew();

  // View network information
  printf ("Starting test service on %s...\n", gxPLIoInterfaceGet (gxpl));
  printf ("  listen on  %s:%d\n", gxPLIoLocalAddrGet (gxpl),
          gxPLIoInfoGet (gxpl)->port);
  printf ("  broadcast on  %s\n", gxPLIoBcastAddrGet (gxpl));
  printf ("Press Ctrl+C to abort ...\n");

  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);
  signal (SIGALRM, prvSignalHandler);

  // start the countdown for sending message
  alarm (10);

  for (;;) {

    // Main loop
    ret = gxPLPoll (gxpl, 100);
    test (ret == 0);
    fflush (stdout);
  }

  return 0;
}


/* private functions ======================================================== */
// -----------------------------------------------------------------------------
// signal handler
static void
prvSignalHandler (int sig) {
  int ret;

  switch (sig) {

    case SIGALRM:
      msg_count++;
      printf ("\n\n******** Sending message[%d] ********\n", msg_count);
      ret = gxPLMessageSend (gxpl, message);
      test (ret > 0);
      if (hasHub) {

        alarm (HBEAT_INTERVAL * 60 - 10);
      }
      else {

        alarm (10);
      }
      break;

    case SIGTERM:
    case SIGINT:
      // Delete the message
      gxPLMessageDelete (message);

      test_count++;
      ret = gxPLClose (gxpl);
      test (ret == 0);

      printf ("\neverything was closed.\nHave a nice day !\n");
      exit (EXIT_SUCCESS);
      break;
  }

}

// -----------------------------------------------------------------------------
// message handler
static void
prvMessageHandler (gxPL * gxpl, gxPLMessage * msg, void * p) {

  // See if we need to check the message for hub detection
  if ( (hasHub == false) && (gxPLMessageIsHubEcho (gxpl, msg, NULL) == true)) {

    hasHub = true;
    printf ("\n******** Hub detected and confirmed existing ********\n");
  }
}

// -----------------------------------------------------------------------------
// Create Heartbeat message
static void
prvHeartbeatMessageNew (void) {
  int ret;

  test_count++;
  message = gxPLMessageNew (gxPLMessageStatus);
  test (message);

  test_count++;
  ret = gxPLMessageSourceIdSet (message, &source);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageBroadcastSet (message, true);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSchemaClassSet (message, "hbeat");
  test (ret == 0);

  if (gxPLIoInfoGet (gxpl)->family & gxPLNetFamilyInet)  {

    ret = gxPLMessageSchemaTypeSet (message, "app");
  }
  else {

    ret = gxPLMessageSchemaTypeSet (message, "basic");
  }

  test_count++;
  ret = gxPLMessagePairAddFormat (message, "interval", "%d", HBEAT_INTERVAL);
  test (ret == 0);

  if (gxPLIoInfoGet (gxpl)->family & gxPLNetFamilyInet)  {

    test_count++;
    ret = gxPLMessagePairAddFormat (message, "port", "%d", gxPLIoInfoGet (gxpl)->port);
    test (ret == 0);

    test_count++;
    ret = gxPLMessagePairAdd (message, "remote-ip", gxPLIoLocalAddrGet (gxpl));
    test (ret == 0);
  }

  test_count++;
  ret = gxPLMessagePairAdd (message, "version", VERSION_SHORT);
  test (ret == 0);
}

/* ========================================================================== */
