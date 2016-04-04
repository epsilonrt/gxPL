/*
 * template.h
 * >>> Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _TEMPLATE_HEADER_
#define _TEMPLATE_HEADER_
#include <stdlib.h>
#include <gxPL.h>
#include <gxPL/stdio.h>
#include "version-git.h"

/* constants ================================================================ */
#define TEMPLATE_VENDOR_ID      "epsirt"
#define TEMPLATE_DEVICE_ID      "sensor"
#define TEMPLATE_INSTANCE_ID    NULL // NULL for auto instance
#define TEMPLATE_DEVICE_VERSION VERSION_SHORT // VERSION_SHORT is automatically defined in version-git.h from git describe
#define TEMPLATE_LOG_LEVEL      LOG_INFO

#define SENSOR_NAME "vin"
#define SENSOR_TYPE "voltage"
#define SENSOR_GAP_NAME "gap"
#define SENSOR_GAP    0.1

#define MAX_DAEMON_RESTARTS 100
#define POLL_RATE_MS  1000

/* structures =============================================================== */
/* types ==================================================================== */
/* public variables ========================================================= */
/*
 * Configurable parameters
 */
extern double dSensorGap;

/* internal public functions ================================================ */
/*
 * Main Task
 */
void vMain (gxPLSetting * setting);

/*
 * xPL Device
 */
gxPLDevice * xDeviceCreate(gxPLSetting * setting);

/*
 * User Interface
 */
int iUiOpen (void);
int iUiPoll (gxPLDevice * device);
int iUiClose (void);

/*
 * Sensor Interface
 */
int iSensorOpen (gxPLDevice * device);
int iSensorClose (gxPLDevice * device);
int iSensorPoll (gxPLDevice * device);
  
// !!TODO!! Here you can add more interfaces...

/* ========================================================================== */
#endif /* _TEMPLATE_HEADER_ defined */
