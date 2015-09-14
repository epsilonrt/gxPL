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
#include <string.h>
#include <sysio/dlist.h>

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
static xDList list;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
void 
destroy (void *item) {
  free (((ioitem *)item)->name);
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
  return strcmp ((const char *)key1, (const char *)key2);
}

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
int
gxPLIoRegister (const char * iolayer, gxPLIoOps * ops) {

  if (pxDListFindFirst(&list, iolayer) == NULL) {
    struct ioitem * item;

    item = malloc (sizeof (struct ioitem));
    assert (item);

    item->name = malloc (strlen (iolayer) + 1);
    assert (item->name);
    strcpy (item->name, iolayer);

    item->ops = ops;
    return iDListAppend(&list, item);
  }
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLIoUnregister (const char * iolayer) {
  xDListElmt * element = pxDListFindFirst(&list, iolayer);

  if (element) {
    void * item;
    
    (void) iDListRemove (&list, element, &item);
    destroy (item);
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
gxPLIoOps *
gxPLIoGetOps (const char * iolayer) {
  gxPLIoOps * ops = NULL;
  xDListElmt * element = pxDListFindFirst(&list, iolayer);

  if (element) {
    ops = pxDListElmtDataPtr (element, ioitem)->ops;
  }
  return ops;
}

/*constructor/destructor ==================================================== */

// -----------------------------------------------------------------------------
int __gxpl_init
gxPLIoInit (void) {

  vDListInit (&list, destroy);
  vDListInitSearch (&list, key, match);
  return 0;
}

// -----------------------------------------------------------------------------
int __gxpl_exit
gxPLIoExit (void) {
  
  vDListDestroy(&list);
  return 0;
}

/* ========================================================================== */
