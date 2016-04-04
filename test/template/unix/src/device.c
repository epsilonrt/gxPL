/*
 * device.c
 * >>> Device side, Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "template.h"

/* constants ================================================================ */
/* public variables ========================================================= */
/* macros =================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
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
  // Create  a device for us
  device = gxPLAppAddDevice (app, TEMPLATE_VENDOR_ID, TEMPLATE_DEVICE_ID, NULL);
  if (device == NULL) {

    return NULL;
  }

  gxPLDeviceVersionSet (device, TEMPLATE_DEVICE_VERSION);
  
  return device;
}
/* ========================================================================== */
