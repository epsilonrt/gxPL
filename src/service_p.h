/**
 * @file private.h
 * gxPLib internal include
 * 
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_SERVICE_PRIVATE_HEADER_
#define _GXPL_SERVICE_PRIVATE_HEADER_

#include <gxPL/service.h>
#include "utils_p.h"

/* public variables ========================================================= */
extern itemCache * ServiceCache;
extern int totalServiceAlloc;

/* internal public functions ================================================ */

/* service.c */
bool xPL_sendHeartbeat(xPL_Service *);
bool xPL_sendGoodbyeHeartbeat(xPL_Service *);
void xPL_sendTimelyHeartbeats(void);
void xPL_handleServiceMessage(xPL_Message *, xPL_Object *);
void xPL_releaseServiceConfigurables(xPL_Service * theService);
void xPL_disableAllServices(void);
xPL_Service * xPL_AllocService(void);
void xPL_FreeService(xPL_Service *);

/* config.c */
void xPL_releaseServiceConfig(xPL_Service *);

/* ========================================================================== */
#endif /* _GXPL_SERVICE_PRIVATE_HEADER_ defined */
