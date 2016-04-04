/*
 * main.c
 * >>> Main task, Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <signal.h>
#include <unistd.h>
#include "template.h"

/* constants ================================================================ */
/* public variables ========================================================= */
/* macros =================================================================== */
/* private variables ======================================================== */
static bool bMainIsRun = true;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
prvSignalHandler (int sig) {
  
  if ( (sig == SIGTERM) || (sig == SIGINT)) {

    bMainIsRun = false;
  }
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
void
vMain (gxPLSetting * setting) {
  int ret;
  gxPLDevice * device;
  gxPLApplication * app;

  PNOTICE ("starting template... (%s log)", sLogPriorityStr (setting->log));
  
  // Create xPL application and device
  device = xDeviceCreate (setting);
  if (device == NULL) {

    PERROR ("Unable to start xPL");
    exit (EXIT_FAILURE);
  }

  // take the application to be able to close
  app = gxPLDeviceParent (device);

  // Open user interfaces (buttons, keyboard, lcd, terminal...)
  ret = iUiOpen ();
  if (ret != 0) {

    PERROR ("Unable to setting up user interface, error %d", ret);
    gxPLAppClose (app);
    exit (EXIT_FAILURE);
  }

  // Sensor init.
  ret = iSensorOpen (device);
  if (ret != 0) {

    PERROR ("Unable to setting up sensor, error %d", ret);
    iUiClose();
    gxPLAppClose (app);
    exit (EXIT_FAILURE);
  }

  /*
   * !!!!!!!!!!!! TODO !!!!!!!!!!!! 
   * Here you can add more blocks to initialize or open
   */

  // Enable the device to do the job
  gxPLDeviceEnable (device, true);

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);

  while (bMainIsRun) {

    // Main Loop
    ret = gxPLAppPoll (app, POLL_RATE_MS);
    if (ret != 0) {

      PWARNING ("Unable to poll xPL network, error %d", ret);
    }

    if (gxPLDeviceIsHubConfirmed (device)) {

      // if the hub is confirmed, performs xPL tasks...
      ret = iSensorPoll (device);
      if (ret != 0) {

        PWARNING ("Unable to poll sensor, error %d", ret);
      }
    }

    ret = iUiPoll (device);
    if (ret != 0) {

      PWARNING ("Unable to poll user interface, error %d", ret);
    }
  }

  ret = iSensorClose (device);
  if (ret != 0) {

    PWARNING ("Unable to close sensor, error %d", ret);
  }

  ret = iUiClose ();
  if (ret != 0) {

    PWARNING ("Unable to close user interface, error %d", ret);
  }

  // Sends heartbeat end messages to all devices
  ret = gxPLAppClose (app);
  if (ret != 0) {

    PWARNING ("Unable to close xPL network, error %d", ret);
  }

  PNOTICE ("template closed, Have a nice day !");
  exit (EXIT_SUCCESS);
}

/* ========================================================================== */
