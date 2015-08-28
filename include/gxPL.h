/**
 * @file gxPL.h
 * gxPL API
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_HEADER_
#define _GXPL_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup gxPLib Library Context
 * @{
 */

/* internal public functions ================================================ */

/**
 * @brief Parse options and parameters and sets xPL if found
 * This will parse the passed command array for options and parameters
 * and install them into xPL if found.  It supports the following switches:
 * - -interface x - Change the default interface gxPLib uses
 * - -xpldebug - enable gxPLib debugging
 * .
 *
 * This function will remove each recognized switch from the parameter
 * list so the returned arg list may be smaller than before. This generally
 * makes life easier for all involved.
 * @param argc
 * @param argv
 * @param silentErrors
 * @return If there is an error parsing the command line, FALSE is returned
 */
bool xPL_parseCommonArgs (int *argc, char *argv[], bool silentErrors);

/**
 * @brief Accessors for context
 * @return
 */
xPL_ConnectType xPL_getParsedConnectionType (void);

/**
 * @brief Attempt to start the xPL library
 * If we are already "started" then bail out
 * @param theConnectType
 * @return
 */
bool xPL_initialize (xPL_ConnectType theConnectType);

/**
 * @brief Stop the xPL library
 * 
 * If already stopped, bail.  Otherwise, we close our connection, release
 * any/all resources and reset
 * @return
 */
bool xPL_shutdown (void);

/**-
 * @defgroup gxPLibVersion Version
 * @{
 */

/**
 * @brief 
 * @return 
 */
const char * xPL_Version (void);

/**
 * @brief 
 * @return 
 */
int xPL_VersionMajor (void);

/**
 * @brief 
 * @return 
 */
int xPL_VersionMinor (void);

/**
 * @brief 
 * @return 
 */
int xPL_VersionPatch (void);

/**
 * @brief 
 * @return 
 */
int xPL_VersionSha1 (void);
/**
 * @}
 */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS

#include <gxPL/utils.h>
#include <gxPL/service.h>
#include <gxPL/message.h>
#include <gxPL/io.h>
#include <gxPL/hub.h>
#include <gxPL/compat.h>

#endif /* _GXPL_HEADER_ defined */
