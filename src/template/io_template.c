/**
 * @file io_template.c
 * xPL Hardware Layer, Template
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define GXPL_IO_INTERNALS
#include "io_p.h"

/* constants ================================================================ */
#define IO_NAME "template"

/* structures =============================================================== */
typedef struct template_data {
  int dummy;
} template_data;

/* macros =================================================================== */
#define dp ((template_data *)io->pdata)

/* types ==================================================================== */
/* private variables ======================================================== */

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static int
gxPLTemplateOpen (gxPLIo * io) {

  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateRead (gxPLIo * io, void * buffer, int count, gxPLIoAddr * source) {

  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateWrite (gxPLIo * io, const void * buffer, int count, gxPLIoAddr * target) {

  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateClose (gxPLIo * io) {

  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateCtl (gxPLIo * io, int c, va_list ap) {
  int ret = 0;

  switch (c) {

      // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncPoll, int * available_bytes, int timeout_ms)
    case gxPLIoFuncPoll: {
      int * available_bytes = va_arg (ap, int*);
      int timeout_ms = va_arg (ap, int);
      // TODO
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetInetPort, int * iport)
    case gxPLIoFuncGetInetPort: {
      int * iport = va_arg (ap, int*);
      // TODO
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetBcastAddr, gxPLIoAddr * bcast_addr)
    case gxPLIoFuncGetBcastAddr: {
      gxPLIoAddr * bcast_addr = va_arg (ap, gxPLIoAddr*);
      // TODO
      // bcast_addr->size = ? ;
      // bcast_addr->family = ? ;
      // bcast_addr->flag = ? ;
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetLocalAddr, gxPLIoAddr * local_addr)
    case gxPLIoFuncGetLocalAddr: {
      gxPLIoAddr * local_addr = va_arg (ap, gxPLIoAddr*);
      // TODO
      // local_addr->size = ? ;
      // local_addr->family = ? ;
      // local_addr->flag = ? ;
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncNetAddrToString, gxPLIoAddr * net_addr, char ** str_addr)
    case gxPLIoFuncNetAddrToString: {
      gxPLIoAddr * addr = va_arg (ap, gxPLIoAddr*);

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
  .recv  = gxPLTemplateRead,
  .send = gxPLTemplateWrite,
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
