/**
 * @file
 * Utilities, (avr 8-bits source code)
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifdef  __AVR__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avrio/delay.h>
#include <avrio/task.h>
#include <gxPL/util.h>
#include <avr/pgmspace.h>

/* constants ================================================================ */
#define MIN_SEC 60UL
#define HOUR_SEC (MIN_SEC * 60UL)
#define DAY_SEC (HOUR_SEC * 24UL)

/* structures =============================================================== */
struct tm {
  int s;
  int m;
  int h;
  int d;
};

/* private functions ======================================================== */
/*
 * TODO: RTC with asynchronous 8 bits timer
 */
// -----------------------------------------------------------------------------
struct tm *
prvTime (unsigned long t) {
  static struct tm clk;
  unsigned long mod;

  clk.d = t / DAY_SEC;
  mod = (clk.d * DAY_SEC);
  clk.h = (t % mod) / HOUR_SEC;
  mod += (clk.h * HOUR_SEC);
  clk.m = (t % mod) / MIN_SEC;
  mod += (clk.m * MIN_SEC);
  clk.s = t % mod;

  return &clk;
}

/* api functions ============================================================ */

/* ----------------------------------------------------------------------------
 * @brief System time
 * This time can be an absolute time (a date) or relative to the system startup
 * (on embedded platform, for example)
 * @return time in seconds
 */
unsigned long
gxPLTime (void) {
  unsigned long t;

  (void) gxPLTimeMs (&t);
  return t / 1000UL;
}

/* ----------------------------------------------------------------------------
 * @brief System time in milliseconds
 * @param ms pointer on the result
 * @return 0, < 0 if error occurs
 */
int
gxPLTimeMs (unsigned long * ms) {

  *ms = xTaskConvertTicks (xTaskSystemTime ());
  return 0;
}

/* ----------------------------------------------------------------------------
 * @brief converts the system time t into a null-terminated string
 * @param t time return by gxPLTime
 * @return system time t into a null-terminated string
 */
char *
gxPLDateTimeStr (unsigned long t, const char * format) {
  static char buf[16];

  struct tm * clk = prvTime (t);

  snprintf_P (buf, sizeof (buf), PSTR ("------%02d%02d%02d%02d"),
              clk->d, clk->h, clk->m, clk->s);

  return buf;
}

// -----------------------------------------------------------------------------
char *
gxPLTimeStr (unsigned long t, const char * format) {
  static char buf[8];

  struct tm * clk = prvTime (t);

  snprintf_P (buf, sizeof (buf), PSTR ("%02d%02d%02d"),
              clk->h, clk->m, clk->s);

  return buf;
}

// -----------------------------------------------------------------------------
char *
gxPLDateStr (unsigned long t, const char * format) {
  static char buf[10];

  struct tm * clk = prvTime (t);

  snprintf_P (buf, sizeof (buf), PSTR ("------%02d"), clk->d);
  return buf;
}

/* ----------------------------------------------------------------------------
 * @brief suspends execution for (at least) ms milliseconds
 * @param ms delay in milliseconds
 * @return 0, < 0 if error occurs
 */
int
gxPLTimeDelayMs (unsigned long ms) {

  delay_ms (ms);
  return 0;
}

/* ----------------------------------------------------------------------------
 * @brief Returns the path of a configuration file
 *
 * @param filename the file name
 * @return If filename is a basename, add a directory that depends on the
 * context and the host system. On a Unix system, the added directory is
 * /etc/gxpl if the user is root, $HOME/.gxpl] otherwise. \n
 * If filename is not a basename, filename is returned
 */
const char *
gxPLConfigPath (const char * filename) {

  return "eefile";
}

#endif /* __AVR__ defined */
/* ========================================================================== */
