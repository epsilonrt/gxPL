/*
 * device-config.c
 * >>> Configurable device side, Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <string.h>
#include "template.h"

/* constants ================================================================ */
/* public variables ========================================================= */
/* macros =================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
// --------------------------------------------------------------------------
//  It's best to put the logic for reading the device configuration
//  and parsing it into your code in a seperate function so it can
//  be used by your prvDeviceConfigChanged and your startup code that
//  will want to parse the same data after a setting file is loaded
void
prvDeviceSetConfig (gxPLDevice * device) {
  double new_gap;
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
  dSensorGap = new_gap;
}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration
static void
prvDeviceConfigChanged (gxPLDevice * device, void * udata) {

  // Read setting items for device and install
  prvDeviceSetConfig (device);
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
// Create xPL application and device
gxPLDevice *
xDeviceCreate (gxPLSetting * setting) {
  gxPLApplication * app;
  gxPLDevice * device;

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    return NULL;
  }

  // Initialize sensor device
  // Create a device for us
  // Create a configurable device and set our application version
  device = gxPLAppAddConfigurableDevice (app, TEMPLATE_VENDOR_ID,
                                         TEMPLATE_DEVICE_ID,
                                         gxPLConfigPath ("sensor.xpl"));
  if (device == NULL) {

    return NULL;
  }

  gxPLDeviceVersionSet (device, TEMPLATE_DEVICE_VERSION);

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
  prvDeviceSetConfig (device);

  // Add a device change listener we'll use to pick up a new gap
  gxPLDeviceConfigListenerAdd (device, prvDeviceConfigChanged, NULL);

  return device;
}

/* ========================================================================== */
