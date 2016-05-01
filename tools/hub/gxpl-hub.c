/**
 * @file
 * Implementation of an xPL Hub using gxPLib
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysio/log.h>
#include <getopt.h>

#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define MAX_HUB_RESTARTS 10000

/* private variables ======================================================== */
static pid_t hub_pid = 0;
static gxPLHub * hub;

/* private functions ======================================================== */
static void prvPrintUsage (void);
static void prvSupervisorSignalHandler (int sig);
static void prvHubSignalHandler (int sig);
static void prvSuperviseHub (gxPLSetting * setting);
static int prvRunHub (gxPLSetting * setting);
static void prvParseAdditionnalOptions (int argc, char *argv[]);

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  gxPLSetting * setting;

  vLogInit (LOG_UPTO (LOG_NOTICE));

  // retrieved the requested configuration from the command line
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectStandAlone);
  if (setting == NULL) {

    prvPrintUsage();
    exit (EXIT_FAILURE);
  }
  prvParseAdditionnalOptions (argc, argv);

  // Now we detach (daemonize ourself)
  if (setting->nodaemon == 0) {

    vLogDaemonize (true);

    // Fork ourselves
    switch (fork()) {
      case 0:            // child
        // Close standard I/O and become our own process group
        close (fileno (stdin));
        close (fileno (stdout));
        close (fileno (stderr));
        setpgrp();

        // Start he hub and keep it running
        prvSuperviseHub (setting);
        break;

      default:           // parent
        break;

      case -1:           // error
        PERROR ("unable to spawn gxpl-hub supervisor, %s (%d)",
                strerror (errno), errno);
        exit (EXIT_FAILURE);
    }
  }
  else {

    // When running non-detached, just do the hub work
    if (prvRunHub (setting) != 0) {

      exit (EXIT_FAILURE);
    }
  }
  return 0;
}

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
//  This is the hub code itself.
static int
prvRunHub (gxPLSetting * setting) {
  int ret;

  if ( (hub = gxPLHubOpen (setting)) == NULL) {

    PERROR ("Unable to start the hub");
    return -1;
  }

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvHubSignalHandler);
  signal (SIGINT, prvHubSignalHandler);

  if (setting->nodaemon) {
    gxPLApplication * app = gxPLHubApplication (hub);
    const xVector * ip_list = gxPLIoLocalAddrList (app);

    printf ("Starting hub test on xPL port %d for IP below:\n",
            gxPLIoInfoGet (app)->port);
    for (int i = 0; i < iVectorSize (ip_list); i++) {
      printf ("  %s\n", (const char *) pvVectorGet (ip_list, i));
    }
    printf ("Broadcast on  %s - %s\n", gxPLIoInterfaceGet (app),
            gxPLIoBcastAddrGet (app));
    printf ("Press Ctrl+C to abort ...\n");
  }
  else {

    vLog (LOG_NOTICE, "xPL Hub now running");
  }

  // Hand control over to gxPLib
  for (;;) {

    ret = gxPLHubPoll (hub, 100);
    if (ret != 0) {

      PERROR ("Hub failure, exiting !");
      return -1;
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------
//  Start the hub and monitor it.  If it stops for any reason, restart it
static void
prvSuperviseHub (gxPLSetting * setting) {
  int hub_restart_count = 0;
  int ret;

  // Begin hub supervision

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSupervisorSignalHandler);
  signal (SIGINT, prvSupervisorSignalHandler);

  // To supervise the hub, we repeatedly fork ourselves each time we
  // determine the hub is not running.  We maintain a circuit breaker
  // to prevent an endless spawning if the hub just can't run
  for (;;) {

    // See if we can still do this
    if (hub_restart_count == MAX_HUB_RESTARTS) {

      PERROR ("gxpl-hub has died %d times -- something may be wrong "
              "-- terminating supervisor", MAX_HUB_RESTARTS);
      exit (EXIT_FAILURE);
    }

    // Up the restart count
    hub_restart_count++;

    // Fork off the hub
    switch (hub_pid = fork()) {
      case 0:            // child
        // Close standard I/O and become our own process group
        close (fileno (stdin));
        close (fileno (stdout));
        close (fileno (stderr));
        setpgrp();

        // Run the hub
        if (prvRunHub (setting) != 0) {

          // If we come back, something bad likely happened
          exit (EXIT_FAILURE);
        }
        break;

      default:           // parent
        PDEBUG ("spawned gxpl-hub process, pid=%d, spawn count=%d",
                hub_pid, hub_restart_count);
        break;

      case -1:           // error
        PERROR ("unable to spawn gxpl-hub supervisor, %s (%d)",
                strerror (errno), errno);
        exit (EXIT_FAILURE);
    }

    // Now we just wait for something bad to happen to our hub
    waitpid (hub_pid, &ret, 0);

    if (WIFEXITED (ret)) {
      
      vLog (LOG_NOTICE, "gxpl-hub exited normally with status %d -- restarting...",
            WEXITSTATUS (ret));
    }
    else if (WIFSIGNALED (ret)) {
      
      vLog (LOG_NOTICE, "gxpl-hub died from by receiving unexpected signal %d"
            " -- restarting...", WTERMSIG (ret));
    }
    else {

      vLog (LOG_NOTICE, "gxpl-hub died from unknown causes -- restarting...");
    }
    delay_ms (1000);
  }
}

// -----------------------------------------------------------------------------
// Print usage info
static void
prvPrintUsage (void) {
  printf ("%s - xPL Hub\n", __progname);
  printf ("Copyright (c) 2015-2016 Pascal JEAN aka epsilonRT\n\n");
  printf ("Usage: %s [-i interface] [-d] [-D] [-h]\n", __progname);
  printf ("  -i interface - use interface named interface (i.e. eth0) as network interface\n");
  printf ("  -W timeout   - set the timeout at the opening of the io layer\n");
  printf ("  -D           - do not daemonize -- run from the console\n");
  printf ("  -d           - enable debugging, it can be doubled or tripled to"
          " increase the level of debug. \n");
  printf ("  -h           - print this message\n\n");
}

// -----------------------------------------------------------------------------
//  Shutdown gracefully if user hits ^C or received TERM signal
static void
prvHubSignalHandler (int sig) {
  int ret;
  switch (sig) {

    case SIGTERM:
    case SIGINT:
      ret = gxPLHubClose (hub);
      if (ret != 0) {

        PERROR ("unable to close hub");
        exit (EXIT_FAILURE);
      }

      vLog (LOG_NOTICE, "everything was closed. Have a nice day !");
      exit (EXIT_SUCCESS);
      break;

    default:
      break;
  }
}

// -----------------------------------------------------------------------------
// signal handler
static void
prvSupervisorSignalHandler (int sig) {
  int ret;

  vLog (LOG_NOTICE, "received termination signal -- starting shutdown");

  switch (sig) {

    case SIGTERM:
    case SIGINT:
      // Stop the child and wait for it
      if ( (hub_pid != 0) && (kill (hub_pid, SIGTERM) == 0)) {

        waitpid (hub_pid, &ret, 0);
        if (WIFEXITED (ret) == 0) {

          exit (EXIT_FAILURE);
        }
      }
      exit (EXIT_SUCCESS);
      break;

    default:
      break;
  }
}

// -----------------------------------------------------------------------------
static void
prvParseAdditionnalOptions (int argc, char *argv[]) {
  int c;

  static const char short_options[] = "h" GXPL_GETOPT;
  static struct option long_options[] = {
    {"help",     no_argument,        NULL, 'h' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

  do  {

    c = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (c) {

      case 'h':
        prvPrintUsage();
        exit (EXIT_SUCCESS);
        break;

      default:
        break;
    }
  }
  while (c != -1);
}
/* ========================================================================== */
