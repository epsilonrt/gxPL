/*
 * xPL StdIO functions
 *
 * Copyright 2016 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_STDIO_HEADER_
#define _GXPL_STDIO_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */
#include <stdio.h>

/* private variables ======================================================== */

/* constants ================================================================ */
#ifdef __AVR__
// -----------------------------------------------------------------------------
#include <avr/pgmspace.h>

/* constants ================================================================ */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif

/* macros =================================================================== */
#define gxPLExit(c) for(;;);
#define gxPLSprintf(str,fmt,...) sprintf_P(str,fmt,##__VA_ARGS__)

#ifdef AVR_INTERRUPT_BUTTON
#include <avrio/button.h>
#ifdef AVRIO_BUTTON_ENABLE
#define gxPLIsInterrupted() (xButGet(AVR_INTERRUPT_BUTTON) != 0)
#else
#define gxPLIsInterrupted() (0)
#endif
#endif

#ifndef NLOG
// -------------------------------------
#include <avrio/file.h>

/* macros =================================================================== */
#define gxPLPutchar(c) putchar(c)
#define gxPLPrintf(fmt,...) printf_P(PSTR(fmt),##__VA_ARGS__)
#define gxPLFflush(f) iFileFlush(f)
#define gxPLWait() getchar()

#else /* NLOG defined */
// -------------------------------------
#include <avrio/delay.h>
#include <avrio/led.h>

/* macros =================================================================== */
#define gxPLPutchar(c)
#define gxPLPrintf(fmt,...)
#define gxPLFflush(f)
#define gxPLWait() delay_ms(5000)

// -------------------------------------
#endif /* NLOG defined */

#else /* __AVR__ not defined */
// -----------------------------------------------------------------------------
#define gxPLPutchar(c) putchar(c)
#define gxPLPrintf(fmt,...) printf(fmt,##__VA_ARGS__)
#define gxPLFflush(f) fflush(f)
#define gxPLWait() getchar()
#define gxPLExit(c) exit(c)
#define gxPLIsInterrupted() (0)
#define gxPLSprintf(str,fmt,...) sprintf(str,fmt,##__VA_ARGS__)
// -----------------------------------------------------------------------------
#endif


/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
INLINE void
gxPLStdIoOpen (void) {
#if defined(__AVR__)

#if defined(AVR_INTERRUPT_BUTTON) && defined(AVRIO_BUTTON_ENABLE)
  vButInit();
#endif

#ifndef NLOG
  xSerialIos term_setting = {
    .baud = AVR_TERMINAL_BAUDRATE, .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE, .sbits = SERIAL_STOPBIT_ONE,
    .flow = AVR_TERMINAL_FLOW, .eol = SERIAL_CRLF
  };

  FILE * tc = xFileOpen (AVR_TERMINAL_PORT, O_RDWR, &term_setting);
  if (tc) {
    stdout = tc;
    stderr = tc;
    stdin = tc;
  }
#else
#ifdef AVRIO_LED_ENABLE
  vLedInit();
#endif
#endif
#endif
}

// -----------------------------------------------------------------------------
INLINE void
gxPLStdIoClose (void) {
#if defined(__AVR__) && !defined(NLOG)
  (void) iFileClose (stdout);
#endif
}


/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_STDIO_HEADER_ defined */
