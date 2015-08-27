/**
 * @file xPL/hub.h
 * xPL Hub
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _XPL4LINUX_HUB_HEADER_
#define _XPL4LINUX_HUB_HEADER_

#include <xPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup xPLHub Hub
 */

/* internal public functions ================================================ */

/**
 * @brief running a hub process
 *
 * Once called, this instance of xPLLib will be running a hub process.
 * @note The xPLLib must be started in standalone mode for this to work
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
#endif /* _XPL4LINUX_HUB_HEADER_ defined */
