/**
 * @file io.c
 * xPL I/O handling
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sysio/dlist.h>
#include <sysio/log.h>

#include "config.h"
#define GXPL_IO_INTERNALS
#include "io_p.h"

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
typedef struct ioitem {
  char * name;
  gxPLIoOps * ops;
} ioitem;

/* types ==================================================================== */
/* private variables ======================================================== */
static xDList iolist;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
prvIoDestroy (void *item) {
  free ( ( (ioitem *) item)->name);
  free (item);
}

// -----------------------------------------------------------------------------
static const void *
prvIoKey (const xDListElmt * element) {
  return pxDListElmtDataPtr (element, ioitem)->name;
}

// -----------------------------------------------------------------------------
static int
prvIoMatch (const void *key1, const void *key2) {
  return strcmp ( (const char *) key1, (const char *) key2);
}


// -----------------------------------------------------------------------------
static gxPLIoOps *
prvGetOps (const char * iolayer) {
  gxPLIoOps * ops = NULL;
  xDListElmt * element = pxDListFindFirst (&iolist, iolayer);

  if (element) {
    ops = pxDListElmtDataPtr (element, ioitem)->ops;
  }
  return ops;
}

/* internal api functions =================================================== */

// -----------------------------------------------------------------------------
gxPLIo *
gxPLIoOpen (gxPLConfig * config) {
  gxPLIo * io = calloc (1, sizeof (gxPLIo));
  assert (io);

  if (strlen (config->iolayer) == 0) {
    
    strcpy (config->iolayer, DEFAULT_IO_LAYER);
    PDEBUG ("set iolayer to default (%s)", config->iolayer);
  }

  io->ops = prvGetOps (config->iolayer);
  io->config = config;
  if (io->ops) {

    if (io->ops->open (io) == 0) {
      
      // great ! everything was done
      return io;
    }
  }
  free (io);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLIoClose (gxPLIo * io) {
  int ret;

  ret = io->ops->close (io);
  free (io);
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLIoRecv (gxPLIo * io, void * buffer, int count, gxPLNetAddress * source) {

  return io->ops->recv (io, buffer, count, source);
}

// -----------------------------------------------------------------------------
int
gxPLIoSend (gxPLIo * io, const void * buffer, int count, gxPLNetAddress * target) {

  return io->ops->send (io, buffer, count, target);
}

// -----------------------------------------------------------------------------
int 
gxPLIoIoCtl (gxPLIo * io, int c, va_list ap) {
  
  return io->ops->ctl (io, c, ap);
}

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
int
gxPLIoRegister (const char * iolayer, gxPLIoOps * ops) {

  if (pxDListFindFirst (&iolist, iolayer) == NULL) {
    struct ioitem * item;

    item = malloc (sizeof (struct ioitem));
    assert (item);

    item->name = malloc (strlen (iolayer) + 1);
    assert (item->name);
    strcpy (item->name, iolayer);

    item->ops = ops;
    return iDListAppend (&iolist, item);
  }
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLIoUnregister (const char * iolayer) {
  xDListElmt * element = pxDListFindFirst (&iolist, iolayer);

  if (element) {
    void * item;

    (void) iDListRemove (&iolist, element, &item);
    prvIoDestroy (item);
    return 0;
  }
  return -1;
}

/*constructor/destructor ==================================================== */

// -----------------------------------------------------------------------------
int __gxpl_init
gxPLIoInit (void) {

  if (iDListInit (&iolist, prvIoDestroy) == 0) {
    
    return iDListInitSearch (&iolist, prvIoKey, prvIoMatch);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int __gxpl_exit
gxPLIoExit (void) {

  return iDListDestroy (&iolist);
}

/* ========================================================================== */
