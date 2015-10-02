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

/* macros =================================================================== */
#define test_ok(t) do { \
    if (!(t)) { \
      fprintf (stderr, "line %d in %s: test %d failed !\n",  __LINE__, \
               __FUNCTION__, test_count); \
      exit (EXIT_FAILURE); \
    } \
    else { \
      printf("\tOk\n"); \
    } \
  } while (0)

#define test(t) do { \
    if (!(t)) { \
      fprintf (stderr, "line %d in %s: test %d failed !\n",  __LINE__, \
               __FUNCTION__, test_count); \
      exit (EXIT_FAILURE); \
    } \
  } while (0)

#define test_new(fmt,...) printf("\nTest %d: " fmt "\n", ++test_count, ##__VA_ARGS__)

/* private variables ======================================================== */
static gxPL * gxpl;
static gxPLDevice * device1, * device2;
static int test_count;

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;
static void prvDeviceHandler (gxPLDevice * device, gxPLMessage * msg, void * p);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0;
  gxPLSetting * config;
  char hello1[] = "hello1";
  char hello2[] = "hello2";
  char str[GXPL_INSTANCEID_MAX + 1];

  // retrieved the requested configuration from the command line
  test_new ("retrieved the requested configuration from the command line");
  config = gxPLSettingNewFromCommandArgs (argc, argv, gxPLConnectViaHub);
  test_ok (config);

  // opens the xPL network
  test_new ("opens the xPL network");
  gxpl = gxPLOpen (config);
  test_ok (gxpl);


  // View network information
  printf ("\nStarting test on %s...\n", gxPLIoInterfaceGet (gxpl));
  printf ("  listen on  %s:%d\n", gxPLIoLocalAddrGet (gxpl),
          gxPLIoInfoGet (gxpl)->port);
  printf ("  broadcast on  %s\n", gxPLIoBcastAddrGet (gxpl));

  // gxPLGenerateUniqueId() Test
  test_new ("generates a fairly unique identifier");
  for (int i = 0; i < 32; i++) {

    ret = gxPLGenerateUniqueId (gxpl, str, GXPL_INSTANCEID_MAX);
    test_ok (ret == GXPL_INSTANCEID_MAX);
    printf ("Unique id: %s\n", str);
    gxPLTimeDelayMs (1);
  }

  test_new ("adds a new device on the network");
  device1 = gxPLDeviceAdd (gxpl, "epsirt", "test", NULL);
  test_ok (device1);
  test_ok (gxPLDeviceCount (gxpl) == 1);
  test_ok (gxPLDeviceAt (gxpl, 0) == device1);

  test_new ("adds a device listener");
  ret = gxPLDeviceListenerAdd (device1, prvDeviceHandler, gxPLMessageAny, NULL, NULL, hello1);
  test_ok (ret == 0);


  test_new ("adds a new device on the network");
  device2 = gxPLDeviceAdd (gxpl, "epsirt", "test", NULL);
  test_ok (device2);
  test_ok (gxPLDeviceCount (gxpl) == 2);
  test_ok (gxPLDeviceAt (gxpl, 1) == device2);

  test_new ("adds a device listener");
  ret = gxPLDeviceListenerAdd (device2, prvDeviceHandler, gxPLMessageAny, NULL, NULL, hello2);
  test_ok (ret == 0);

  test_new ("enable the new device");
  ret = gxPLDeviceEnabledSet (device1, true);
  test_ok (ret == 0);

  test_new ("enable the new device");
  ret = gxPLDeviceEnabledSet (device2, true);
  test_ok (ret == 0);

  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);
  printf ("Press Ctrl+C to abort ...\n");

  for (;;) {

    // Main loop
    ret = gxPLPoll (gxpl, 100);
    test (ret == 0);
/*    if (gxPLDeviceIsHubConfirmed (device2)) {
      prvSignalHandler (SIGTERM);
    }*/
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

      test_new ("close the xPL network");
      ret = gxPLClose (gxpl);
      test_ok (ret == 0);

      printf ("\neverything was closed.\nHave a nice day !\n");
      exit (EXIT_SUCCESS);
      break;
    default:
      break;
  }

}

// -----------------------------------------------------------------------------
// message handler
static void
prvDeviceHandler (gxPLDevice * device, gxPLMessage * msg, void * p) {
  char * str = (char *) p;
  int i = gxPLDeviceIndex (gxPLDeviceParentGet (device), device) + 1;

  printf ("\n********* device%d: [%s] *********\n", i, str);
}

/* ========================================================================== */
