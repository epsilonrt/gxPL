/**
 * @file
 * gxPLHub internal include
 * 
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_HUB_PRIVATE_HEADER_
#define _GXPL_HUB_PRIVATE_HEADER_

#include <gxPL/defs.h>

/* structures =============================================================== */

/**
 * @brief Describes a hub client
 */
typedef struct _gxPLHubClient {
  
  gxPLIoAddr addr;
  int hbeat_period_max; /**< (hbeat_interval * 2 + 60) */
  long hbeat_last;
} gxPLHubClient;

/**
 * @brief Describes a hub
 */
typedef struct _gxPLHub {
  gxPLApplication * app;
  xVector clients;
  const xVector * local_addr_list;
  long timeout;
} gxPLHub;

/* ========================================================================== */
#endif /* _GXPL_HUB_PRIVATE_HEADER_ defined */
