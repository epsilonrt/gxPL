/**
 * @file
 * gxPLBridge internal include
 * 
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_BRIDGE_PRIVATE_HEADER_
#define _GXPL_BRIDGE_PRIVATE_HEADER_

#include <gxPL/defs.h>

/* structures =============================================================== */

/**
 * @brief Describes a bridge client
 */
typedef struct _gxPLBridgeClient {
  
  gxPLIoAddr addr;
  gxPLId id;
  int hbeat_period_max; /**< (hbeat_interval * 2 + 60) */
  long hbeat_last;
} gxPLBridgeClient;

/**
 * @brief Describes a xPL to xPL bridge
 */
typedef struct _gxPLBridge {
  gxPLApplication * in;
  gxPLApplication * out;
  xVector clients;
  long timeout;
  uint8_t max_hop;
} gxPLBridge;

/* ========================================================================== */
#endif /* _GXPL_BRIDGE_PRIVATE_HEADER_ defined */
