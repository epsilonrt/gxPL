/*
 * sysio/src/log.c
 * @brief Gestion des logs
 * Copyright © 2014 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */

#define SYSLOG_NAMES 1
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <sysio/log.h>

/* constants ================================================================ */
#define ERROR_BUFFER_SIZE 256

#ifndef CONFIG_LOG_DEFAULT_MASK
#define CONFIG_LOG_DEFAULT_MASK  LOG_UPTO (LOG_ERR)
#endif

/* structures =============================================================== */
struct xLog {

  bool bIsDaemon;
  bool bIsInit;
  int iMask;
};

/* private variables ======================================================== */
static struct xLog xMyLog = {
  .bIsDaemon = false,
  .bIsInit = true,
  .iMask = CONFIG_LOG_DEFAULT_MASK
};
static char sError[ERROR_BUFFER_SIZE];

/* private functions ======================================================== */
/* -----------------------------------------------------------------------------
 * Copyright (C) 2005-2007 Rune Saetre
 *
 * This function is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Rune Saetre <rst@folesvaert.no>
 *
 */
static char*
get_priority_name(int pri) {
  CODE* n = prioritynames;

  while(  (n->c_name) && (n->c_val!=pri) )
    n++;
  return n->c_name;
}
// -----------------------------------------------------------------------------

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
void
vLogInit (int mask) {

  xMyLog.iMask = mask;
  xMyLog.bIsInit = true;
}

// -----------------------------------------------------------------------------
void vLogDaemonize (bool daemonize) {

  if (xMyLog.bIsInit) {

    xMyLog.bIsDaemon = daemonize;
    if (daemonize) {

      (void)setlogmask (xMyLog.iMask);
      openlog (__progname, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
    }
  }
  else {

    fprintf (stderr, "Error: xMyLog is not initialized, you must call first vLogInit()\n");
  }
}

// -----------------------------------------------------------------------------
void
vLogSetMask (int mask) {

  xMyLog.iMask = mask;
}

// -----------------------------------------------------------------------------
int
iLogMask (void) {

  return xMyLog.iMask;
}

// -----------------------------------------------------------------------------
void
vLog (int priority, const char *format, ...) {
  va_list va;

  va_start(va, format);
  if (xMyLog.bIsInit) {

    if (xMyLog.bIsDaemon) {

      vsyslog (priority, format, va);
    }
    else {

      if (xMyLog.iMask & LOG_MASK(priority)) {

        (void)fprintf(stderr,"%s(%s): ", __progname, get_priority_name(priority));
        (void)vfprintf(stderr, format, va);
        (void)fputc('\n', stderr);
        fflush (stderr);
      }
    }
  }
  else {

    (void)fprintf(stderr,"%s(%s): ", __progname, get_priority_name(priority));
    (void)vfprintf(stderr, format, va);
    (void)fputc('\n', stderr);
    fflush (stderr);
  }

  va_end(va);
}


// -----------------------------------------------------------------------------
const char *
sSysIoStrError (void) {
  return sError;
}

// -----------------------------------------------------------------------------
int
iSysIoSetStrError (const char *format, ...) {
  va_list va;
  int size;

  va_start (va, format);
  size = vsnprintf (sError, ERROR_BUFFER_SIZE - 1, format, va);
  va_end (va);
  return size;
}

// -----------------------------------------------------------------------------
bool
bSysIoLogAssert (void) {
#ifdef LOG_ASSERT
  return true;
#else
  return false;
#endif
}

#ifdef LOG_ASSERT
/* private functions ======================================================== */

void __vLogAssertFail (__const char *__assertion, __const char *__file,
         unsigned int __line, __const char *__function) {

  if (xMyLog.bIsInit) {

    vLog (LOG_CRIT, "%s:%d: %s: Assertion '%s' failed.",
              __file, __line, __function, __assertion);
  }
  else {

    (void)fprintf(stderr, "%s(%s): %s:%d: %s: Assertion '%s' failed.\n",
              __progname, get_priority_name(LOG_CRIT),
              __file, __line, __function, __assertion);
  }
  abort();
}

void __vLogAssertPerrorFail (int __errnum, __const char *__file,
          unsigned int __line, __const char *__function) {


  if (xMyLog.bIsInit) {

    vLog (LOG_CRIT, "%s:%d: %s: %s.",
              __file, __line, __function, strerror(__errnum));
  }
  else {

    (void)fprintf(stderr, "%s(%s): %s:%d: %s: %s.\n",
              __progname, get_priority_name(LOG_CRIT),
              __file, __line, __function, strerror(__errnum));
  }
  abort();
}
#endif

/* ========================================================================== */
