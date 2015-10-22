/** 
 * @file 
 * gxPLDevice test
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
static gxPLApplication * app;
static gxPLDevice * device1, * device2;
static int test_count;

/* private functions ======================================================== */
static void prvSignalHandler (int sig) ;
static void prvDeviceHandler (gxPLDevice * device, gxPLMessage * msg, void * p);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0;
  gxPLSetting * setting;
  char hello1[] = "hello1";
  char hello2[] = "hello2";
  char str[GXPL_INSTANCEID_MAX + 1];

  // retrieved the requested configuration from the command line
  test_new ("retrieved the requested configuration from the command line");
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  test_ok (setting);

  // opens the xPL network
  test_new ("opens the xPL network");
  app = gxPLAppOpen (setting);
  test_ok (app);


  // View network information
  printf ("\nStarting test on %s...\n", gxPLIoInterfaceGet (app));
  printf ("  listen on  %s:%d\n", gxPLIoLocalAddrGet (app),
          gxPLIoInfoGet (app)->port);
  printf ("  broadcast on  %s\n", gxPLIoBcastAddrGet (app));

  // gxPLGenerateUniqueId() Test
  test_new ("generates a fairly unique identifier");
  for (int i = 0; i < 32; i++) {

    ret = gxPLGenerateUniqueId (app, str, GXPL_INSTANCEID_MAX);
    test_ok (ret == GXPL_INSTANCEID_MAX);
    printf ("Unique id: %s\n", str);
    gxPLTimeDelayMs (1);
  }

  test_new ("adds a new device on the network");
  device1 = gxPLAppAddDevice (app, "epsirt", "test", NULL);
  test_ok (device1);
  test_ok (gxPLAppDeviceCount (app) == 1);
  test_ok (gxPLAppDeviceAt (app, 0) == device1);

  test_new ("adds a device listener");
  ret = gxPLDeviceListenerAdd (device1, prvDeviceHandler, gxPLMessageAny, NULL, NULL, hello1);
  test_ok (ret == 0);


  test_new ("adds a new device on the network");
  device2 = gxPLAppAddDevice (app, "epsirt", "test", NULL);
  test_ok (device2);
  test_ok (gxPLAppDeviceCount (app) == 2);
  test_ok (gxPLAppDeviceAt (app, 1) == device2);

  test_new ("adds a device listener");
  ret = gxPLDeviceListenerAdd (device2, prvDeviceHandler, gxPLMessageAny, NULL, NULL, hello2);
  test_ok (ret == 0);

  test_new ("enable the new device");
  ret = gxPLDeviceEnable (device1, true);
  test_ok (ret == 0);

  test_new ("enable the new device");
  ret = gxPLDeviceEnable (device2, true);
  test_ok (ret == 0);

  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);
  printf ("Press Ctrl+C to abort ...\n");

  for (;;) {

    // Main loop
    ret = gxPLAppPoll (app, 100);
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
      ret = gxPLAppClose (app);
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
  int i = gxPLAppDeviceIndex (gxPLDeviceParentGet (device), device) + 1;

  printf ("\n********* device%d: [%s] *********\n", i, str);
}

/* ========================================================================== */
