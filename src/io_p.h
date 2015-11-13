/**
 * @file
 * gxPLIo internal include
 * 
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_IO_PRIVATE_HEADER_
#define _GXPL_IO_PRIVATE_HEADER_

#include <stdarg.h>
#include <gxPL/defs.h>
#if !defined(GXPL_IO_INTERNALS) && !defined(__DOXYGEN__)
#warning You should not add the header file gxPL/io.h in your source code
#endif

#ifndef __AVR__
/* constants ================================================================ */
#define GXPL_PRIORITY   101
#define GXPLIO_PRIORITY 102

/* macros =================================================================== */
#define __gxpl_init __attribute__ ((constructor(GXPL_PRIORITY)))
#define __gxpl_exit __attribute__ ((destructor(GXPL_PRIORITY)))
#define __gxplio_init __attribute__ ((constructor(GXPLIO_PRIORITY)))
#define __gxplio_exit __attribute__ ((destructor(GXPLIO_PRIORITY)))

#else /* __AVR__ defined */
#define __gxpl_init __attribute__ ((section (".init5")))
#define __gxpl_exit __attribute__ ((section (".fini5")))
#define __gxplio_init __attribute__ ((section (".init7")))
#define __gxplio_exit __attribute__ ((section (".fini7")))
#endif  /* __AVR__ defined */

/* structures =============================================================== */

/*
 * @brief xPL Hardware Layer Operations
 */
typedef struct _gxPLIoOps {
  int (*open)   (gxPLIo * io);
  int (*recv)   (gxPLIo * io, void * buffer, int count, gxPLIoAddr * source);
  int (*send)   (gxPLIo * io, const void * buffer, int count, const gxPLIoAddr * target);
  int (*close)  (gxPLIo * io);
  int (*ctl)    (gxPLIo * io, int c, va_list ap);
} gxPLIoOps;

/*
 * @brief xPL Hardware Abstraction Layer
 */
typedef struct _gxPLIo {
  void * pdata; /**< Private data used internally by layer io */
  gxPLIoOps * ops;
  gxPLSetting * setting; /**< pointer to the top-level configuration */
} gxPLIo;

/* internal private functions (low level service) =========================== */

/*
 * @brief Register an io layer
 * 
 * Should be called in the constructor of each io layer ...
 * 
 * @param iolayer name used to identify this layer
 * @param ops layer's function that performs the operations
 * @return 0, -1 on error
 */
int gxPLIoRegister (const char * iolayer, gxPLIoOps * ops);

/*
 * @brief Register an io layer
 * 
 * Should be called in the destructor of each io layer ...
 * 
 * @param iolayer name used to identify the layer
 * @return 0, -1 on error
 */
int gxPLIoUnregister (const char * iolayer);

/* ========================================================================== */
#endif /* _GXPL_IO_PRIVATE_HEADER_ defined */
