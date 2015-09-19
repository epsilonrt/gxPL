/**
 * @file gxPL/util.h
 * xPL Utilities functions
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_UTIL_HEADER_
#define _GXPL_UTIL_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/* internal public functions ================================================ */

/**
 * @defgroup xPLUtils Utilities
 * @{
 */

/**
 * @defgroup xPLUtilsText Text functions
 * @{
 */
 
/**
 * @brief Copy string in accordance with xPL
 * 
 * Copies the C string pointed by src into the array pointed by dst, 
 * including the terminating null character (and stopping at that point).
 * src must contain only the following characters: A-Z, a-z, 0-9 and "-" 
 * (letters, numbers and the hyphen/dash character - ASCII 45). If this is not 
 * the case, the copying is interrupted and -1 is returned.

 * @param dst
 * @param src
 * @return 0, -1 if error
 */
int gxPLStrCpy (char * dst, const char * src);

/**
 * @}
 */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_UTIL_HEADER_ defined */
