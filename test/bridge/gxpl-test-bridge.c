/**
 * @file
 * gxPLBridge test
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
#include "version-git.h"

/* constants ================================================================ */

/* macros =================================================================== */
#define test(t) do { \
    if (!(t)) { \
      fprintf (stderr, "line %d in %s: test %d failed !\n",  __LINE__, \
               __FUNCTION__, test_count); \
      exit (EXIT_FAILURE); \
    } \
  } while (0)

/* private variables ======================================================== */
static gxPLBridge * bridge;
static int test_count;

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0;
  gxPLSetting * insetting;
  gxPLSetting * outsetting;

  // retrieved the requested configuration from the command line
  test_count++;
  insetting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectStandAlone);
  test (insetting);

  test_count++;
  outsetting = gxPLSettingNew (NULL, "udp", gxPLConnectViaHub);
  test (outsetting);

  // opens the bridge
  test_count++;
  bridge = gxPLBridgeOpen (insetting, outsetting, 1);
  test (bridge);

  // View network information
  gxPLApplication * in = gxPLBridgeInApp (bridge);
  gxPLApplication * out = gxPLBridgeOutApp (bridge);

  printf ("Starting bridge test for %s network\n", insetting->iolayer);
  printf ("  inner side listen on  %s\n", gxPLIoLocalAddrGet (in));
  printf ("  outer side listen on  %s:%d\n", gxPLIoLocalAddrGet (out),
          gxPLIoInfoGet (out)->port);
  printf ("Press Ctrl+C to abort ...\n");

  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);


  for (;;) {

    // Main loop
    ret = gxPLBridgePoll (bridge, 100);
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

      test_count++;
      ret = gxPLBridgeClose (bridge);
      test (ret == 0);

      printf ("\neverything was closed.\nHave a nice day !\n");
      exit (EXIT_SUCCESS);
      break;
  }

}

/* ========================================================================== */
