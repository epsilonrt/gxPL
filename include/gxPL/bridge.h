/**
 * @file
 * xPL Bridge 
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
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
 * @defgroup xPLBridge Bridge
 * A bridge connects two networks do not have the same physical layer. \n
 * The outer side of a bridge uses UDP and behaves like a device associated 
 * with a schema. The inner side of a bridge acts as a hub and does not use UDP.
 * @{
 */

/**
 * @brief Opens a new gxPLBridge object
 * @param setting pointer to a configuration, this configuration can be modified
 * by the function to return the actual configuration.
 * @return the object or NULL if error occurs
 */
gxPLBridge * gxPLBridgeOpen (gxPLSetting * insetting, gxPLSetting * outsetting, uint8_t max_hop);

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
 * @brief 
 * @param bridge
 * @return 
 */
gxPLApplication * gxPLBridgeInApp (gxPLBridge * bridge);

/**
 * @brief 
 * @param bridge
 * @return 
 */
gxPLApplication * gxPLBridgeOutApp (gxPLBridge * bridge);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_BRIDGE_HEADER_ defined */
