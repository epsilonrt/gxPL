/**
 * @file 
 * Low-level API, used by high level to access the hardware
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
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
 * @addtogroup gxPLIo
 * @{
 * @defgroup gxPLIoInterface Abstract interface
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
 * @brief 
 * @param setting
 * @return 
 */
gxPLIo * gxPLIoOpen (gxPLSetting * setting);

/**
 * @brief  Receive a message from the network
 * 
 * The call can be blocking, gxPLIoFuncPoll will be used before so you do not block.
 * 
 * @param io
 * @param buffer
 * @param count
 * @param source if not NULL, returns the source address.
 * @return 
 */
int gxPLIoRecv (gxPLIo * io, void * buffer, int count, gxPLIoAddr * source);

/**
 * @brief 
 * 
 * @param io
 * @param buffer
 * @param count
 * @param target if target is NULL or if her broadcast flag is set, the 
 * broadcast address of the network is used.
 * @return 
 */
int gxPLIoSend (gxPLIo * io, const void * buffer, int count, const gxPLIoAddr * target);

/**
 * @brief 
 * @param io
 * @return 
 */
int gxPLIoClose (gxPLIo * io);

/**
 * @brief 
 * @param io
 * @param c
 * @param ap
 * @return 
 */
int gxPLIoIoCtl (gxPLIo * io, int c, va_list ap);

/**
 *  @}
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_IO_HEADER_ defined */
