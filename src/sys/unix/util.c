/**
 * @file unix/util.c
 * Top Layer of API, Unix code
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifdef  __unix__
#include "config.h"
#include <time.h>
#include <sys/time.h>
#include <sysio/delay.h>
#include <gxPL/util.h>

/* private functions ======================================================== */


/* api functions ============================================================ */

// -----------------------------------------------------------------------------
long
gxPLTime (void) {

  return time (NULL);
}

// -----------------------------------------------------------------------------
int
gxPLTimeMs (unsigned long * ms) {
  int ret;
  struct timeval tv;

  if ( (ret = gettimeofday (&tv, NULL)) == 0) {

    *ms = (tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL);
    return 0;
  }
  return ret;
}

// -----------------------------------------------------------------------------
char *
gxPLTimeStr (unsigned long time) {
  return ctime ( (time_t *) &time);
}

// -----------------------------------------------------------------------------
int
gxPLTimeDelayMs (unsigned long ms) {
  return delay_ms (ms);
}

#endif /* __unix__ defined */
/* ========================================================================== */
