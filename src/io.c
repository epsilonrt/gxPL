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
#include <gxPL.h>
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
void
destroy (void *item) {
  free ( ( (ioitem *) item)->name);
  free (item);
}

// -----------------------------------------------------------------------------
const void *
key (const xDListElmt * element) {
  return pxDListElmtDataPtr (element, ioitem)->name;
}

// -----------------------------------------------------------------------------
int
match (const void *key1, const void *key2) {
  return strcmp ( (const char *) key1, (const char *) key2);
}

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
gxPL *
gxPLIoOpen (gxPLConfig * config) {
  gxPLIo * io = calloc (1, sizeof (gxPLIo));
  assert (io);

  if (strlen (config->iolayer) == 0) {
    
    strcpy (config->iolayer, DEFAULT_IO_LAYER);
    PDEBUG ("set iolayer to default (%s)", config->iolayer);
  }

  io->ops = gxPLIoGetOps (config->iolayer);
  if (io->ops) {
    gxPL * gxpl = calloc (1, sizeof (gxPL));
    assert (gxpl);

    gxpl->config = config;
    gxpl->io = io;

    if (io->ops->open (gxpl) == 0) {
      
      // great ! everything was done
      return gxpl;
    }
    free (gxpl);
  }
  free (io);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLIoClose (gxPL * gxpl) {
  int ret;

  ret = gxpl->io->ops->close (gxpl);
  free (gxpl->io);
  if (gxpl->config->malloc) {
    
    free (gxpl->config);
  }
  free (gxpl);
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLIoRead (gxPL * gxpl, void * buffer, int count) {

  return gxpl->io->ops->read (gxpl, buffer, count);
}

// -----------------------------------------------------------------------------
int
gxPLIoWrite (gxPL * gxpl, const void * buffer, int count) {

  return gxpl->io->ops->write (gxpl, buffer, count);
}

// -----------------------------------------------------------------------------
int
gxPLIoCtl (gxPL * gxpl, int c, ...) {
  int ret = 0;
  va_list ap;

  va_start (ap, c);
  switch (c) {
    
    // put here the requests should not be transmitted to the layer below.
    // case ...
    
    default:
      ret = gxpl->io->ops->ctl (gxpl, c, ap);
      if ((ret == -1) && (errno == EINVAL)) {
        vLog (LOG_ERR, "gxPLIoCtl function not supported: %d", c);
      }
      break;
  }

  va_end (ap);
  return ret;
}

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
    destroy (item);
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
gxPLIoOps *
gxPLIoGetOps (const char * iolayer) {
  gxPLIoOps * ops = NULL;
  xDListElmt * element = pxDListFindFirst (&iolist, iolayer);

  if (element) {
    ops = pxDListElmtDataPtr (element, ioitem)->ops;
  }
  return ops;
}

/*constructor/destructor ==================================================== */

// -----------------------------------------------------------------------------
int __gxpl_init
gxPLIoInit (void) {

  if (iDListInit (&iolist, destroy) == 0) {
    
    return iDListInitSearch (&iolist, key, match);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int __gxpl_exit
gxPLIoExit (void) {

  return iDListDestroy (&iolist);
}

/* ========================================================================== */
