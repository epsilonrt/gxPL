/**
 * @file gxPL/io.h
 * API IO functions
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_IO_HEADER_
#define _GXPL_IO_HEADER_

#include <gxPL.h>
__BEGIN_C_DECLS
/* ========================================================================== */
#ifndef GXPL_INTERNALS
#warning You should not add the header file gxPL/io.h in your source code
#endif

/**
 * @defgroup xPLIo Internal Io Layer API
 * Description of the IO layer to the application layer.
   @warning The top user should not access this layer directly.
 * @{
 */

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
/* public variables ========================================================= */
/* internal public functions ================================================ */
/**
 * @brief 
 * @param config
 * @return 
 */
gxPL * gxPLIoOpen (gxPLConfig * config);

/**
 * @brief 
 * @param gxpl
 * @param buffer
 * @param count
 * @return 
 */
int gxPLIoRead (gxPL * gxpl, void * buffer, int count);

/**
 * @brief 
 * @param gxpl
 * @param buffer
 * @param count
 * @return 
 */
int gxPLIoWrite (gxPL * gxpl, const void * buffer, int count);

/**
 * @brief 
 * @param gxpl
 * @return 
 */
int gxPLIoClose (gxPL * gxpl);

/* --------------- Defined in gxPL.h
 * @brief 
 * @param gxpl
 * @param c
 * @return 
 * int gxPLIoCtl (gxPL * gxpl, int c, ...);
 */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_IO_HEADER_ defined */
