/**
 * @file xPL/io.h
 * xPLLib IO functions
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _XPL4LINUX_IO_HEADER_
#define _XPL4LINUX_IO_HEADER_

#include <xPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup xPLIo IO
 * @{
 */

/* types ==================================================================== */

/**
 * @brief Event management of user timeouts
 */
typedef void (* xPL_TimeoutHandler) (int, xPL_Object *);

/**
 * @brief Raw Listener Support
 */
typedef void (* xPL_rawListener) (char *, int, xPL_Object *);

/**
 * @brief Event handler for user-registered I/O management
 */
typedef void (* xPL_IOHandler) (int, int, int);

/* internal public functions ================================================ */

/**
 * @brief Set the interface
 * @param newInterfaceName
 */
void xPL_setBroadcastInterface (char * newInterfaceName);

/**
 * @brief Get the xPL Interface
 * @return
 */
char * xPL_getBroadcastInterface (void);

/**
 * @brief Return the xPL FD
 * @return
 */
int xPL_getFD (void);

/**
 * @brief Get the connection port
 * @return
 */
int xPL_getPort (void);

/**
 * @brief Return listing IP address
 * @return
 */
char * xPL_getListenerIPAddr (void);

/**
 * @brief Return IP address
 * @return
 */
char * xPL_getBroadcastIPAddr (void);

/**
 * @brief Hub detected and confirmed existing
 * @return
 */
bool xPL_isHubConfirmed (void);

/**
 * @brief Allocate a new timeout handler and install it into the list
 * @param timeoutHandler
 * @param timeoutInSeconds
 * @param userValue
 */
void xPL_addTimeoutHandler (xPL_TimeoutHandler timeoutHandler, int timeoutInSeconds, xPL_Object * userValue);

/**
 * @brief Remove a previously allocated timeout handler
 * @param timeoutHandler
 * @return
 */
bool xPL_removeTimeoutHandler (xPL_TimeoutHandler timeoutHandler);

/**
 * @brief Add an IO channel to monitor/dispatch to.
 *
 * theFD is the FD that is open and should be monitored.  ioHandler is the
 * routine that is is called when there is activity on the channel.
 * userValue is an integer passed directly to the ioHandler, frequently used to
 * track context information.
 * watchRead, watchWrite, watchError tell what sort of things need to be monitored.
 * @param theIOHandler
 * @param userValue
 * @param theFD
 * @param watchRead
 * @param watchWrite
 * @param watchError
 * @return
 */
bool xPL_addIODevice (xPL_IOHandler theIOHandler, int userValue, int theFD,
                      bool watchRead, bool watchWrite, bool watchError);

/**
 * @brief Remove an IO channel based on the passed fd.
 * If the fd exists, it's removed and TRUE is returned.
 * If the fd doesn't exist, FALSE is returned.
 * @param theFD
 */
bool xPL_removeIODevice (int theFD);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _XPL4LINUX_IO_HEADER_ defined */
