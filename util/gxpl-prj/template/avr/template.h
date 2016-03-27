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

/* constants ================================================================ */
#define TEMPLATE_VENDOR_ID      "epsirt"
#define TEMPLATE_DEVICE_ID      "sensor"
#define TEMPLATE_INSTANCE_ID    NULL // NULL for auto instance
#define TEMPLATE_DEVICE_VERSION VERSION_SHORT // VERSION_SHORT is automatically defined in version-git.h from git describe
#define TEMPLATE_LOG_LEVEL      LOG_DEBUG
#define IOLAYER_NAME  "xbeezb"
#define IOLAYER_PORT  "tty1"
#define SENSOR_GAP    0.1
#define POLL_RATE_MS  1000

#define SENSOR_NAME "vin"
#define SENSOR_TYPE "voltage"

/*
 * AVR_TERMINAL used for gxPL debug purpose only ...
 * AVR has no command line or terminal, constants below allow to setup the
 * the serial port terminal, if NLOG is defined in the makefile,
 * no terminal is used.
 */
#define AVR_TERMINAL_PORT     "tty0"
#define AVR_TERMINAL_BAUDRATE 500000
#define AVR_TERMINAL_FLOW     SERIAL_FLOW_NONE
#include <gxPL/stdio.h>

/* structures =============================================================== */
/* types ==================================================================== */

/* public variables ========================================================= */
extern float fSensorGap;

/* internal public functions ================================================ */
/*
 * User Interface
 */
void vUiInit (void);
int iUiTask (gxPLDevice * device);
/*
 * Sensor Interface
 */
int iSensorInit (gxPLDevice * device);
int iSensorTask (gxPLDevice * device);
  
// !!TODO!! Here you can add more interfaces...

/*
 * xPL
 */
gxPLDevice * xDeviceCreate(const char * iolayerport, const char * iolayername, int log);

/* ========================================================================== */
#endif /* _TEMPLATE_HEADER_ defined */
