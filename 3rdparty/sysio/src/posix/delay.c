/*
 * sysio/posix/delay.c
 * @brief fonctions de temporisation
 * Copyright © 2014 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sysio/delay.h>

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
// Must be thread friendly
int
delay_ms (long lMs) {

  if (lMs) {

    if (lMs == -1) {

      sleep (-1);
    }
    else {
      struct timespec dt;

      dt.tv_nsec = (lMs % 1000UL) * 1000000UL;
      dt.tv_sec  = lMs / 1000UL;
      nanosleep (&dt, NULL);
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------
// Must be thread friendly
int
delay_us (long lUs) {

  if (lUs) {

    if (lUs == -1) {

      sleep (-1);
    }
    else {
      unsigned long s, us;
      struct timeval t1;

      gettimeofday (&t1, NULL);
      s  = lUs / 1000000UL;
      us = lUs % 1000000UL;

      if (lUs < 100) {
        struct timeval dt, t2;

        dt.tv_sec = s;
        dt.tv_usec = us;
        timeradd (&t1, &dt, &t2) ; // TODO: NO POSIX COMPLIANT
        while (timercmp (&t1, &t2, <)) {  // TODO: NO POSIX COMPLIANT

          gettimeofday (&t1, NULL);
        }
      }
      else {
        struct timespec dt;

        dt.tv_nsec = us * 1000UL;
        dt.tv_sec = s;
        nanosleep (&dt, NULL);
      }
    }
  }
  return 0;
}

/* ========================================================================== */
