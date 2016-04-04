/*
 * template-daemon.c
 * >>> Daemon application, Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sysio/log.h>
#include "template.h"

/* private variables ======================================================== */
static pid_t daemon_pid = 0;
static gxPLSetting * setting;

/* private functions ======================================================== */
static void prvDaemonSignalHandler (int sig);
static void prvDaemon (gxPLSetting * setting);

/* main ===================================================================== */
int
main (int argc, char **argv) {

  vLogInit (LOG_UPTO (TEMPLATE_LOG_LEVEL));

  // Read arguments from the command line
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  if (setting == NULL) {

    PERROR ("Unable to get settings from command line");
    exit (EXIT_FAILURE);
  }
  vLogSetMask (LOG_UPTO (setting->log));

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

        // Start the daemon and keep it running
        prvDaemon (setting);
        break;

      default:           // parent
        break;

      case -1:           // error
        PERROR ("unable to spawn daemon supervisor, %s (%d)",
                strerror (errno), errno);
        exit (EXIT_FAILURE);
    }
  }
  else {

    // When running non-detached, start the main task
    vMain (setting);
  }

  return 0;
}

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
//  Start the daemon and monitor it. If it stops for any reason, restart it
static void
prvDaemon (gxPLSetting * setting) {
  int daemon_restart_count = 0;
  int ret;

  setting->malloc = 0; // the settings are not to be freed at the network closure

  // Begin daemon supervision

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvDaemonSignalHandler);
  signal (SIGINT, prvDaemonSignalHandler);

  // To supervise the daemon, we repeatedly fork ourselves each time we
  // determine the daemon is not running.  We maintain a circuit breaker
  // to prevent an endless spawning if the daemon just can't run
  for (;;) {

    // See if we can still do this
    if (daemon_restart_count == MAX_DAEMON_RESTARTS) {

      PERROR ("template has died %d times -- something may be wrong "
              "-- terminating supervisor", MAX_DAEMON_RESTARTS);
      exit (EXIT_FAILURE);
    }

    // Up the restart count
    daemon_restart_count++;

    // Fork off the daemon
    switch (daemon_pid = fork()) {

      case 0:            // child
        // Close standard I/O and become our own process group
        close (fileno (stdin));
        close (fileno (stdout));
        close (fileno (stderr));
        setpgrp();

        // start the main task
        vMain (setting);
        break;

      default:           // parent
        PINFO ("spawned template process, pid=%d, spawn count=%d",
               daemon_pid, daemon_restart_count);
        break;

      case -1:           // error
        PERROR ("unable to spawn template-daemon supervisor, %s (%d)",
                strerror (errno), errno);
        exit (EXIT_FAILURE);
    }

    // Now we just wait for something bad to happen to our daemon
    waitpid (daemon_pid, &ret, 0);

    if (WIFEXITED (ret)) {
      PNOTICE ("template exited normally with status %d -- restarting...",
               WEXITSTATUS (ret));
      continue;
    }

    if (WIFSIGNALED (ret)) {
      PNOTICE ("template died from by receiving unexpected signal %d"
               " -- restarting...", WTERMSIG (ret));
      continue;
    }

    PNOTICE ("template died from unknown causes -- restarting...");
    continue;
  }

}

// -----------------------------------------------------------------------------
// Daemon signal handler
static void
prvDaemonSignalHandler (int sig) {

  if ( (sig == SIGTERM) || (sig == SIGINT)) {
    int ret;
    
    PINFO ("received termination signal -- starting shutdown");
    
    ret = EXIT_SUCCESS;
    // Stop the child and wait for it
    if ( (daemon_pid != 0) && (kill (daemon_pid, SIGTERM) == 0)) {

      waitpid (daemon_pid, &ret, 0);
      if (WIFEXITED (ret) == 0) {

        ret = EXIT_FAILURE;
      }
    }

    free (setting);
    exit (ret);
  }
}

/* ========================================================================== */
