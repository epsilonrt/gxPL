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
#include <sysio/log.h>

#include <gxPL.h>
#include "io_p.h"

/* constants ================================================================ */
#define IO_NAME "template"

/* structures =============================================================== */
typedef struct template_data {
  int dummy;
} template_data;

/* macros =================================================================== */
#define dp ((template_data *)gxpl->io->pdata)

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
gxPLTemplateCtl (gxPL * gxpl, int c, va_list ap) {
  int ret = 0;

  switch (c) {

      // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncPoll, int * available_bytes, int timeout_ms)
    case gxPLIoFuncPoll: {
      int * available_bytes = va_arg (ap, int*);
      int timeout_ms = va_arg (ap, int);
      // TODO
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetInetPort, int * iport)
    case gxPLIoFuncGetInetPort: {
      int * iport = va_arg (ap, int*);
      // TODO
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetBcastAddr, gxPLAddress * bcast_addr)
    case gxPLIoFuncGetBcastAddr: {
      gxPLAddress * bcast_addr = va_arg (ap, gxPLAddress*);
      // TODO
      // bcast_addr->family = ? ;
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetLocalAddr, gxPLAddress * local_addr)
    case gxPLIoFuncGetLocalAddr: {
      gxPLAddress * local_addr = va_arg (ap, gxPLAddress*);
      // TODO
      // local_addr->family = ? ;
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncNetAddrToString, gxPLAddress * net_addr, char ** str_addr)
    case gxPLIoFuncNetAddrToString: {
      gxPLAddress * addr = va_arg (ap, gxPLAddress*);

      if (addr->family == ??) {
        char ** str_addr = va_arg (ap, char**);
        // TODO
        ret = -1;
      }
      else {

        errno = EINVAL;
        ret = -1;
      }
    }
    break;

    default:
      errno = EINVAL;
      ret = -1;
      break;
  }

  return ret;
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
