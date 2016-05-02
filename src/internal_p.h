/**
 * @file
 * Internal private header
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_INTERNAL_PRIVATE_HEADER_
#define _GXPL_INTERNAL_PRIVATE_HEADER_

#include <gxPL.h>

__BEGIN_C_DECLS
/* ========================================================================== */


/**
 * @brief Create a new xPL device
 * @param app
 * @param vendor_id
 * @param device_id
 * @param instance_id
 * @return
 */
gxPLDevice * gxPLDeviceNew (gxPLApplication * app,
                            const char * vendor_id,
                            const char * device_id,
                            const char * instance_id);
/**
 * @brief Create a new device and prepare it for configuration
 *
 * Like other devices, this will still require being enabled to start.
 * Before it's started, you need to define and attach the configurable items
 * for the device.   When the device is enabled, if there is a non-null
 * configFile, it's values are read.  The devices instance value will be
 * created in a fairly unique method for devices that have not yet been
 * configured.
 *
 * @param vendor_id
 * @param device_id
 * @param filename
 * @return
 */
gxPLDevice * gxPLDeviceConfigNew (gxPLApplication * app, const char * vendor_id,
                                  const char * device_id,
                                  const char * filename);

/**
 * @brief Release an xPL device
 * @param device
 */
void gxPLDeviceDelete (gxPLDevice * device);

/**
 * @brief Messages handler
 * @param device
 * @param message
 * @param udata
 */
void gxPLDeviceMessageHandler (gxPLDevice * device, gxPLMessage * message,
                               void * udata);

/**
 * @brief Sends an heartbeat immediately
 * @param device pointer on the device
 * @param type
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceHeartbeatSend (gxPLDevice * device, gxPLHeartbeatType type);

/**
 * @brief
 * @param setting
 * @param argc
 * @param argv
 */
void gxPLParseCommonArgs (gxPLSetting * setting, int argc, char *argv[]);

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_INTERNAL_PRIVATE_HEADER_ defined */
