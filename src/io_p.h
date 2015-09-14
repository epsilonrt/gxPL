/**
 * @file io_p.h
 * gxPLIo internal include
 * 
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_IO_PRIVATE_HEADER_
#define _GXPL_IO_PRIVATE_HEADER_

#include <gxPL/defs.h>

/* constants ================================================================ */
#define GXPL_PRIORITY   101
#define GXPLIO_PRIORITY 102

/* macros =================================================================== */
#define __gxpl_init __attribute__ ((constructor(GXPL_PRIORITY)))
#define __gxpl_exit __attribute__ ((destructor(GXPL_PRIORITY)))
#define __gxplio_init __attribute__ ((constructor(GXPLIO_PRIORITY)))
#define __gxplio_exit __attribute__ ((destructor(GXPLIO_PRIORITY)))

/* structures =============================================================== */

/**
 * @brief xPL Hardware Layer Operations
 */
typedef struct _gxPLIoOps {
  int (*open)   (gxPL * gxpl);
  int (*read)   (gxPL * gxpl, void * buffer, int count);
  int (*write)  (gxPL * gxpl, const void * buffer, int count);
  int (*close)  (gxPL * gxpl);
  int (*ctl)    (gxPL * gxpl, int c, ...);
} gxPLIoOps;

/**
 * @brief xPL Hardware Abstraction Layer
 */
typedef struct _gxPLIo {
  void * pdata; /**< Private data used internally by layer io */
  gxPLIoOps * ops;
} gxPLIo;

/* internal public functions ================================================ */
/**
 * @brief Register an io layer
 * @param iolayer name used to identify this layer
 * @param ops layer's function that performs the operations
 * @return 0, -1 on error
 */
int gxPLIoRegister (const char * iolayer, gxPLIoOps * ops);

/**
 * @brief Register an io layer
 * @param iolayer name used to identify the layer
 * @return 0, -1 on error
 */
int gxPLIoUnregister (const char * iolayer);

/**
 * @brief Returns io layer's functions
 * @param iolayer name used to identify the layer
 * @return pointer on layer's function that performs the operations
 */
gxPLIoOps * gxPLIoGetOps (const char * iolayer);

/* ========================================================================== */
#endif /* _GXPL_IO_PRIVATE_HEADER_ defined */
