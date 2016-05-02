/**
 * @file
 * Low-level API, used by high level to access the hardware
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_IO_HEADER_
#define _GXPL_IO_HEADER_

#include <stdarg.h>
#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */
#if !defined(GXPL_INTERNALS) && !defined(__DOXYGEN__)
#warning You should not add the header file gxPL/io.h in your source code
#endif
/**
 * @defgroup gxPLIoDoc Hardware Layer
 * Allows end-user to read information and to control the hardware layer.
 * @{
 * @defgroup gxPLIoInterfaceDoc Hardware Abstract Layer
 *
 * Description of the IO layer to the application layer.
 * The application layer uses functions described here to access the io
 * layer.
 *
 * @warning This part concerns the developers wishing to create a new io
 * layer. The end-user should not access this layer directly.
 * @{
 */

/* internal public functions ================================================ */
/**
 * @brief Opening of the input-output layer.
 * @param setting pointer to a configuration, this configuration can be modified
 * by the function to return the actual configuration.
 * @return the object or NULL if error occurs
 */
gxPLIo * gxPLIoOpen (gxPLSetting * setting);

/**
 * @brief  Receive a message from the network
 *
 * The call can be blocking, gxPLIoFuncPoll will be used before so you do not block.
 *
 * @param io io layer
 * @param buffer a buffer in which bytes are stored
 * @param count number of bytes requested
 * @param source if not NULL, returns the source address.
 * @return number of bytes read, a negative value if error
 */
int gxPLIoRecv (gxPLIo * io, void * buffer, int count, gxPLIoAddr * source);

/**
 * @brief Send a message to the network
 *
 * @param io io layer
 * @param buffer buffer where the bytes were stored
 * @param count number of bytes to send
 * @param target if target is NULL or if her broadcast flag is set, the
 * broadcast address of the network is used.
 * @return number of bytes sent or frame identifier (greater than or equal to
 * one), a negative value if error occurs
 */
int gxPLIoSend (gxPLIo * io, const void * buffer, int count, const gxPLIoAddr * target);

/**
 * @brief Close the input-output layer.
 * @param io io layer
 * @return 0, -1 if error occurs
 */
int gxPLIoClose (gxPLIo * io);

/**
 * @brief device-specific input/output operations
 *
 * @param io io layer
 * @param c device-dependent request code
 * @param ap iterating arguments from gxPLIoCtl()
 * @return 0, -1 if error occurs
 */
int gxPLIoIoCtl (gxPLIo * io, int c, va_list ap);

/**
 *  @}
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_IO_HEADER_ defined */
