/**
 * @file
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

/* private API functions ==================================================== */

// -----------------------------------------------------------------------------
static int
gxPLTemplateOpen (gxPLIo * io) {

  if (io->pdata == NULL) {
    io->pdata = calloc (1, sizeof (template_data));
    assert (io->pdata);
    // TODO
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateRecv (gxPLIo * io, void * buffer, int count, gxPLIoAddr * source) {

  // TODO
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateSend (gxPLIo * io, const void * buffer, int count, gxPLIoAddr * target) {

  // TODO
  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLTemplateClose (gxPLIo * io) {

  if (io->pdata) {

    // TODO
    free (io->pdata);
    io->pdata = NULL;
    return 0;
  }
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

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetNetInfo, gxPLIoAddr * local_addr)
    case gxPLIoFuncGetNetInfo: {
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

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncNetAddrFromString, gxPLIoAddr * net_addr, const char * str_addr)
    case gxPLIoFuncNetAddrFromString: {
      gxPLIoAddr * addr = va_arg (ap, gxPLIoAddr*);
      const char * str_addr = va_arg (ap, char*);

      // TODO
      // local_addr->size = ? ;
      // local_addr->family = ? ;
      // local_addr->flag = ? ;
      ret = -1;
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetLocalAddrList, const xVector ** addr_list)
    case gxPLIoFuncGetLocalAddrList: {
      const xVector ** addr_list = va_arg (ap, xVector**);
      // TODO
      ret = -1;
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
  .recv  = gxPLTemplateRecv,
  .send  = gxPLTemplateSend,
  .close = gxPLTemplateClose,
  .ctl   = gxPLTemplateCtl
};

/* public functions ========================================================= */

// -----------------------------------------------------------------------------
void __gxplio_init
gxPLTemplateInit (void) {

  (void) gxPLIoRegister (IO_NAME, &ops);
}

// -----------------------------------------------------------------------------
void __gxplio_exit
gxPLTemplateExit (void) {

  (void) gxPLIoUnregister (IO_NAME);
}

/* ========================================================================== */
