/**
 * @file gxPL/hub.h
 * xPL Hub
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_HUB_HEADER_
#define _GXPL_HUB_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup gxpl-hub Hub
 * @{
 */

/* internal public functions ================================================ */

/**
 * @brief running a hub process
 *
 * Once called, this instance of gxPLib will be running a hub process.
 * @note The gxPLib must be started in standalone mode for this to work
 * (which is NOT the default)
 *
 * @return
 */
bool xPL_startHub (void);

/**
 * @brief Disable the hub
 */
void xPL_stopHub (void);

/**
 * @brief Return if hub is running or not
 * @return 
 */
bool xPL_isHubRunning (void);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_HUB_HEADER_ defined */
