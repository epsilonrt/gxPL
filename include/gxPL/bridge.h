/**
 * @file
 * xPL Bridge
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_BRIDGE_HEADER_
#define _GXPL_BRIDGE_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */


/* internal public functions ================================================ */

/**
 * @defgroup gxPLBridgeDoc Bridge
 * A bridge connects two networks do not have the same physical layer. \n
 * The outside of a bridge uses UDP and behaves like a device associated
 * with a schema. The inside of a bridge acts as a hub and does not use UDP.
 * @{
 */

/**
 * @brief Opens a new gxPLBridge object
 * @param insetting pointer to the inside configuration, this configuration can be 
 * modified by the function to return the actual configuration.
 * @param outsetting pointer to the outside configuration (udp), this 
 * configuration can be  modified by the function to return the actual configuration
 * @param max_hop only messages with a hop count less than or equal to max_hop cross the bridge
 * @return the object or NULL if error occurs
 */
gxPLBridge * gxPLBridgeOpen (gxPLSetting * insetting, gxPLSetting * outsetting, uint8_t max_hop);

/**
 * @brief Sets new setting for inside network
 * If configuration differ, the network will be closed then opened with the new
 * configuration
 * @param bridge pointer to a gxPLBridge object
 * @param insetting new inside network setting
 * @return 0, -1 if an error occurs
 */
int gxPLBridgeSetNewInSetting (gxPLBridge * bridge, gxPLSetting * insetting);

/**
 * @brief Sets the device for the outsite network
 * @param bridge pointer to a gxPLBridge object
 * @param vendor_id
 * @param device_id
 * @param filename
 * @param version
 * @return 0, -1 if an error occurs
 */
int gxPLBridgeDeviceSet (gxPLBridge * bridge,
                         const char * vendor_id, const char * device_id,
                         const char * filename, const char * version);

/**
 * @brief Returns the device for the outsite network
 * @param bridge pointer to a gxPLBridge object
 * @return
 */
gxPLDevice * gxPLBridgeDevice (gxPLBridge * bridge);

/**
 * @brief Enable device for the outsite network
 * @param bridge pointer to a gxPLBridge object
 * @param enable
 * @return 0, -1 if an error occurs
 */
int gxPLBridgeDeviceEnable (gxPLBridge * bridge, bool enable);

/**
 * @brief Checks if device is enabled
 * @param bridge pointer to a gxPLBridge object
 * @return false, true, -1 if an error occurs
 */
int gxPLBridgeDeviceIsEnabled (const gxPLBridge * bridge);

/**
 * @brief Close a gxPLBridge object and release all ressources
 * @param bridge pointer to a gxPLBridge object
 * @return 0, -1 if an error occurs
 */
int gxPLBridgeClose (gxPLBridge * bridge);

/**
 * @brief Polling event of a bridge
 * @param bridge pointer to a gxPLBridge object
 * @param timeout_ms waiting period in ms before output if no event occurs
 * @return 0, -1 if an error occurs
 */
int gxPLBridgePoll (gxPLBridge * bridge, int timeout_ms);

/**
 * @brief Returns the inside application
 * @param bridge pointer to a gxPLBridge object
 * @return the application
 */
gxPLApplication * gxPLBridgeInApp (gxPLBridge * bridge);

/**
 * @brief Returns the outside application
 * @param bridge pointer to a gxPLBridge object
 * @return the application
 */
gxPLApplication * gxPLBridgeOutApp (gxPLBridge * bridge);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_BRIDGE_HEADER_ defined */
