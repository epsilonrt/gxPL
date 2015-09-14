/**
 * @file io_template.c
 * xPL Hardware Layer, Template
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "io_p.h"

/* macros =================================================================== */
/* constants ================================================================ */
#define IO_NAME "template"

/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static int
gxPLTemplateOpen (gxPL * gxpl) {
  
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateRead (gxPL * gxpl, void * buffer, int count) {
  
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateWrite (gxPL * gxpl, const void * buffer, int count) {
  
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateClose (gxPL * gxpl) {
  
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateCtl (gxPL * gxpl, int c, ...) {
  
  return -1;
}

/* private variables ======================================================== */
static gxPLIoOps
ops = {
  .open  = gxPLTemplateOpen,
  .read  = gxPLTemplateRead,
  .write = gxPLTemplateWrite,
  .close = gxPLTemplateClose,
  .ctl   = gxPLTemplateCtl
};

/* public functions ========================================================= */

// -----------------------------------------------------------------------------
int __gxplio_init
gxPLTemplateInit (void) {
  
  return gxPLIoRegister (IO_NAME, &ops);
}

// -----------------------------------------------------------------------------
int __gxplio_exit
gxPLTemplateExit (void) {
  
  return gxPLIoUnregister (IO_NAME);
}

/* ========================================================================== */
