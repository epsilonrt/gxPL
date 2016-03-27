/*
 * gxpl-template-avr.h
 * >>> Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_TEMPLATE_AVR_HEADER_
#define _GXPL_TEMPLATE_AVR_HEADER_
#include <stdlib.h>
#include <gxPL.h>

/* constants ================================================================ */
#define GXPL_TEMPLATE_AVR_VENDOR_ID      "epsirt"
#define GXPL_TEMPLATE_AVR_DEVICE_ID      "sensor"
#define GXPL_TEMPLATE_AVR_INSTANCE_ID    NULL // NULL for auto instance
#define GXPL_TEMPLATE_AVR_DEVICE_VERSION VERSION_SHORT // VERSION_SHORT is automatically defined in version-git.h from git describe
#define GXPL_TEMPLATE_AVR_LOG_LEVEL      LOG_DEBUG
#define IOLAYER_NAME  "xbeezb"
#define IOLAYER_PORT  "tty1"
#define SENSOR_GAP    0.1
#define POLL_RATE_MS  1000

#define SENSOR_NAME "vin"
#define SENSOR_TYPE "voltage"
#define SENSOR_GAP_NAME "gap"

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
 * xPL Device
 */
gxPLDevice * xDeviceCreate(const char * iolayerport, const char * iolayername, int log);
void vDeviceConfigChanged (gxPLDevice * device, void * udata);
void vDeviceSetConfig (gxPLDevice * device);
/* ========================================================================== */
#endif /* _GXPL_TEMPLATE_AVR_HEADER_ defined */
