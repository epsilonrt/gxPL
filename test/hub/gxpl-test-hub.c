/**
 * @file
 * gxPLHub test
 *
 * Copyright 2015 (c), epsilonRT                
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
static gxPLHub * hub;
static int test_count;

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0;
  gxPLSetting * setting;

  // retrieved the requested configuration from the command line
  test_count++;
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectStandAlone);
  test (setting);

  // opens the xPL network
  test_count++;
  hub = gxPLHubOpen (setting);
  test (hub);


  // View network information
  gxPLApplication * app = gxPLHubApplication (hub);
  const xVector * ip_list = gxPLIoLocalAddrList (app);

  printf ("Starting hub test on xPL port %d for IP below:\n", 
  gxPLIoInfoGet (app)->port);
  for (int i = 0; i < iVectorSize (ip_list); i++) {
    printf ("  %s\n", (const char *) pvVectorGet(ip_list, i));
  }
  printf ("Broadcast on  %s - %s\n", gxPLIoInterfaceGet (app), 
  gxPLIoBcastAddrGet (app));
  printf ("Press Ctrl+C to abort ...\n");

  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);


  for (;;) {

    // Main loop
    ret = gxPLHubPoll (hub, 100);
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
      ret = gxPLHubClose (hub);
      test (ret == 0);

      printf ("\neverything was closed.\nHave a nice day !\n");
      exit (EXIT_SUCCESS);
      break;
  }

}

/* ========================================================================== */
