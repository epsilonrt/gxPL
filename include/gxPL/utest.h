/*
 * xPL Unity test functions
 *
 * Copyright 2016 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_UTEST_HEADER_
#define _GXPL_UTEST_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */
#include <stdio.h>

/* private variables ======================================================== */
static int UTEST_COUNTER;
#ifndef NLOG
static int heap_before;
#endif

/* constants ================================================================ */
#ifdef __AVR__
// -----------------------------------------------------------------------------
#include <avr/pgmspace.h>
#include <avrio/memdebug.h>

/* macros =================================================================== */
#define SPRINTF(str,fmt,...) sprintf_P(str,fmt,##__VA_ARGS__)
#define UTEST_STOP() for(;;);

#ifndef NLOG
// -------------------------------------
#include <avrio/file.h>

/* macros =================================================================== */
#define UTEST_PRINTF(fmt,...) printf_P(PSTR(fmt),##__VA_ARGS__)
#define UTEST_NEW(fmt,...) UTEST_PRINTF("\nTest %d: "fmt,++UTEST_COUNTER,##__VA_ARGS__)
#define UTEST_SUCCESS() UTEST_PRINTF ("Success\n")
#define UTEST_FFLUSH(f) iFileFlush(f)
#define UTEST_WAIT() getchar()

#else /* NLOG defined */
// -------------------------------------
#include <avrio/delay.h>
// -----------------------------------------------------------------------------
INLINE void
prvSetLed (int d) {

  AVR_UTEST_LED_PORT = d & 0xFF;
}

// -----------------------------------------------------------------------------
INLINE void
prvLedFlash (void) {

  for (;;) {

    prvSetLed (UTEST_COUNTER);
    delay_ms (250);
    prvSetLed (0);
    delay_ms (250);
  }
}

/* macros =================================================================== */
#define UTEST_SUCCESS()
#define UTEST_PRINTF(fmt,...)
#define UTEST_FFLUSH(f)
#define UTEST_WAIT() delay_ms(5000)
#define UTEST_NEW(fmt,...) prvSetLed(++UTEST_COUNTER)

#undef assert
#define assert(n) if ((n) == 0) { prvLedFlash(); }
// -------------------------------------
#endif /* NLOG defined */

#else /* __AVR__ not defined */
// -----------------------------------------------------------------------------
#include <malloc.h>
#define UTEST_PRINTF(fmt,...) printf(fmt,##__VA_ARGS__)
#define UTEST_NEW(fmt,...) UTEST_PRINTF("\nTest %d: "fmt,++UTEST_COUNTER,##__VA_ARGS__)
#define UTEST_SUCCESS() UTEST_PRINTF("Success\n")
#define UTEST_FFLUSH(f) fflush(f)
#define UTEST_WAIT() getchar()
#define UTEST_STOP()
#define sprintf_P(str,fmt,...) sprintf(str,fmt,##__VA_ARGS__)
#define PROGMEM
// -----------------------------------------------------------------------------
#endif


/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
INLINE void
UTEST_INIT (void) {
#if defined(__AVR__)
#if defined(NLOG)
  AVR_UTEST_LED_DDR = 0xFF;
#else
  xSerialIos term_setting = {
    .baud = AVR_UTEST_TERM_BAUDRATE, .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE, .sbits = SERIAL_STOPBIT_ONE,
    .flow = AVR_UTEST_TERM_FLOW, .eol = SERIAL_CRLF
  };

  FILE * tc = xFileOpen (AVR_UTEST_TERM_PORT, O_RDWR, &term_setting);
  if (tc) {
    stdout = tc;
    stderr = tc;
    stdin = tc;
  }
#endif
#endif
}

// -----------------------------------------------------------------------------
INLINE void
UTEST_PMEM_BEFORE (void) {
#if !defined(NLOG)

#if defined(__AVR__)
  heap_before = ulMemoryUsed();
#else
  struct mallinfo mi = mallinfo();
  heap_before = mi.uordblks;
#endif

  UTEST_PRINTF ("Dynamic memory used before %d bytes\n", heap_before);
#endif
}

// -----------------------------------------------------------------------------
INLINE void
UTEST_PMEM_AFTER (void) {
#if !defined(NLOG)
  static int heap_after;
#if defined(__AVR__)
  heap_after = ulMemoryUsed();
#else
  struct mallinfo mi = mallinfo();
  heap_after = mi.uordblks;
#endif

  UTEST_PRINTF ("Dynamic memory used: before %d - after %d - loss %d\n",
                heap_before, heap_after, heap_after - heap_before);
#endif
}

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_UTEST_HEADER_ defined */
