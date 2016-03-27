/*
 * gxpl-template-avr-config.c
 * >>> Main application, Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <string.h>
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

    if (gxPLDeviceIsHubConfirmed (device) && gxPLDeviceIsConfigured (device)) {

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
  // Create a device for us
  // Create a configurable device and set our application version
  device = gxPLAppAddConfigurableDevice (app, GXPL_TEMPLATE_AVR_VENDOR_ID,
                                         GXPL_TEMPLATE_AVR_DEVICE_ID,
                                         gxPLConfigPath ("sensor.xpl"));
  assert (device);

  gxPLDeviceVersionSet (device, GXPL_TEMPLATE_AVR_DEVICE_VERSION);

  // If the configuration was not reloaded, then this is our first time and
  // we need to define what the configurables are and what the default values
  // should be.
  if (gxPLDeviceIsConfigured (device) == false) {

    // Define a configurable item and give it a default
    gxPLDeviceConfigItemAdd (device, SENSOR_GAP_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, SENSOR_GAP_NAME,
                              gxPLDoubleToStr (SENSOR_GAP, 1));
  }

  // Parse the device configurables into a form this program
  // can use (whether we read a setting or not)
  vDeviceSetConfig (device);

  // Add a device change listener we'll use to pick up a new gap
  gxPLDeviceConfigListenerAdd (device, vDeviceConfigChanged, NULL);

  return device;
}

// --------------------------------------------------------------------------
//  It's best to put the logic for reading the device configuration
//  and parsing it into your code in a seperate function so it can
//  be used by your vDeviceConfigChanged and your startup code that
//  will want to parse the same data after a setting file is loaded
void
vDeviceSetConfig (gxPLDevice * device) {
  float new_gap;
  char * endptr;

  // Get the gap
  const char * gap_str = gxPLDeviceConfigValueGet (device, SENSOR_GAP_NAME);

  // Handle bad configurable (override it)
  if ( (gap_str == NULL) || (strlen (gap_str) == 0)) {
    gxPLDeviceConfigValueSet (device, SENSOR_GAP_NAME,
                              gxPLDoubleToStr (SENSOR_GAP, 1));
    return;
  }

  // Convert text to a number
  new_gap = strtod (gap_str, &endptr);

  if (*endptr != '\0') {
    // Bad value -- override it
    gxPLDeviceConfigValueSet (device, SENSOR_GAP_NAME,
                              gxPLDoubleToStr (SENSOR_GAP, 1));
    return;
  }

  // Install new gap
  fSensorGap = new_gap;
}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration
void
vDeviceConfigChanged (gxPLDevice * device, void * udata) {

  // Read setting items for device and install
  vDeviceSetConfig (device);
}

/* ========================================================================== */
