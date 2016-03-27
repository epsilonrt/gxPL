/*
 * gxpl-template-avr.c
 * >>> Main application, Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "gxpl-template-avr.h"
#include "version-git.h"

/* constants ================================================================ */
/*
 * All definitions are in gxpl-template-avr.h
 */

/* public variables ========================================================= */
/* macros =================================================================== */
/* private variables ======================================================== */

/* main ===================================================================== */
int
main (void) {
  int ret;
  gxPLDevice * device;
  gxPLApplication * app;

  // User interface init (buttons, keyboard, lcd, terminal...)
  vUiInit ();

  // Create xPL application and device
  device = xDeviceCreate (IOLAYER_PORT, IOLAYER_NAME, GXPL_TEMPLATE_AVR_LOG_LEVEL);
  assert (device);

  app = gxPLDeviceParent (device);

  ret = iSensorInit (device);
  assert (ret == 0);
  // !!TODO!! Here you can add more blocks to initialize

  // Enable the device to do the job
  ret = gxPLDeviceEnable (device, true);
  assert (ret == 0);

  // Main Loop
  for (;;) {

    ret = gxPLAppPoll (app, POLL_RATE_MS);
    assert (ret == 0);

    if (gxPLDeviceIsHubConfirmed (device)) {

      // if the hub is confirmed, performs xPL tasks...
      ret = iSensorTask (device);
      assert (ret >= 0);
    }
    ret = iUiTask (device);
    assert (ret >= 0);
  }
}

// -----------------------------------------------------------------------------
// Create xPL application and device
gxPLDevice *
xDeviceCreate (const char * iolayerport, const char * iolayername, int log) {
  int ret;
  gxPLApplication * app;
  gxPLSetting * setting;
  gxPLDevice * device;

  setting = gxPLSettingNew (iolayerport, iolayername, gxPLConnectViaHub);
  assert (setting);
  setting->log = log;

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    PERROR ("Unable to start xPL");
    gxPLExit (EXIT_FAILURE);
  }

  // Initialize sensor device
  // Create  a device for us
  device = gxPLAppAddDevice (app, GXPL_TEMPLATE_AVR_VENDOR_ID, GXPL_TEMPLATE_AVR_DEVICE_ID, NULL);
  assert (device);

  ret = gxPLDeviceVersionSet (device, GXPL_TEMPLATE_AVR_DEVICE_VERSION);
  assert (ret == 0);
  return device;
}

/* ========================================================================== */
