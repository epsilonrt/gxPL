/**
 * @file
 * xPL hub on a system using ethernet networking
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_HUB_HEADER_
#define _GXPL_HUB_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */


/* internal public functions ================================================ */

/**
 * @defgroup gxPLHubDoc Hub
 * xPL hub on a system using ethernet networking. \n 
 * The primary purpose of a xPL hub is to bind to port 3865, receive xPL 
 * messages from the network at that port and redistribute those messages to all 
 * xPL applications running on the same computer. A hub must perform the 
 * following functions:
 * - Receive xPL network messages
 * - Deliver/Rebroadcast those messages to all xPL applications on the same 
 *   computer
 * - Discovery of new xPL applications on the computer and adding them to the 
 *   list of recipients of received xPL messages
 * - Track known local xPL applications and determine when they have died and 
 *   remove them from the list of local applications
 * - The hub is only involved in receiving messages, it is not involved in 
 *   sending messages whatsoever. 
 * .
 * @{
 */

/**
 * @brief Opens a new gxPLHub object
 * @param setting pointer to a configuration, this configuration can be modified
 * by the function to return the actual configuration.
 * @return the object or NULL if error occurs
 */
gxPLHub * gxPLHubOpen (gxPLSetting * setting);

/**
 * @brief Close a gxPLHub object and release all ressources
 * @param hub pointer to a gxPLHub object
 * @return 0, -1 if an error occurs
 */
int gxPLHubClose (gxPLHub * hub);

/**
 * @brief Polling event of a hub
 * @param hub pointer to a gxPLHub object
 * @param timeout_ms waiting period in ms before output if no event occurs
 * @return 0, -1 if an error occurs
 */
int gxPLHubPoll (gxPLHub * hub, int timeout_ms);

/**
 * @brief Returns the application
 * @param hub pointer to a gxPLHub object
 * @return the application
 */
gxPLApplication * gxPLHubApplication (gxPLHub * hub);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_HUB_HEADER_ defined */
