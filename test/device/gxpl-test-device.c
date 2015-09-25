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
static gxPLDevice * device;
static int test_count;

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;
static int prvMessageHandler (gxPLDevice * device, const gxPLMessage * msg, void * p);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0;
  gxPLConfig * config;
  char hello[] = ".";
  char str[GXPL_INSTANCEID_MAX + 1];

  // retrieved the requested configuration from the command line
  test_count++;
  config = gxPLNewConfigFromCommandArgs (argc, argv, gxPLConnectViaHub);
  test (config);

  // opens the xPL network
  test_count++;
  gxpl = gxPLOpen (config);
  test (gxpl);

  // Check if gxPLGenerateUniqueIdenti is Ok
  for (int i = 0; i < 32; i++) {
    
    ret = gxPLGenerateUniqueId (gxpl, str, GXPL_INSTANCEID_MAX);
    test (ret == GXPL_INSTANCEID_MAX);
    printf ("Unique id: %s\n", str);
    delay_ms (1);
  }

  // adds a message listener
  // test_count++;
  // ret = gxPLDeviceListenerAdd (gxpl, prvMessageHandler, hello);

  // View network information
  printf ("Starting test service on %s...\n", gxPLIoInterfaceGet (gxpl));
  printf ("  listen on  %s:%d\n", gxPLIoLocalAddrGet (gxpl),
          gxPLIoInfoGet (gxpl)->port);
  printf ("  broadcast on  %s\n", gxPLIoBcastAddrGet (gxpl));
  printf ("Press Ctrl+C to abort ...\n");

  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);

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

    case SIGTERM:
    case SIGINT:

      gxPLDeviceDelete(device);
      
      test_count++;
      ret = gxPLClose (gxpl);
      test (ret == 0);

      printf ("\neverything was closed.\nHave a nice day !\n");
      exit (EXIT_SUCCESS);
      break;
    default:
    break;
  }

}

// -----------------------------------------------------------------------------
// message handler
static int
prvMessageHandler (gxPLDevice * device, const gxPLMessage * msg, void * p) {

  return 0;
}

/* ========================================================================== */
