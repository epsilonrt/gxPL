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
#include <gxPL/stdio.h>

/* private variables ======================================================== */
static int UTEST_COUNTER;

#ifndef NLOG
static int heap_before;
#endif

/* constants ================================================================ */
#ifdef __AVR__
// -----------------------------------------------------------------------------

#ifndef NLOG
// -------------------------------------

/* macros =================================================================== */
#define UTEST_NEW(fmt,...) gxPLPrintf("\nTest %d: "fmt,++UTEST_COUNTER,##__VA_ARGS__)
#define UTEST_SUCCESS() gxPLPrintf ("Success\n")

#else /* NLOG defined */
// -------------------------------------

// -----------------------------------------------------------------------------
INLINE void
prvLedFlash (void) {

  for (;;) {

    vLedSetAll (UTEST_COUNTER);
    delay_ms (250);
    vLedSetAll (0);
    delay_ms (250);
  }
}

/* macros =================================================================== */
#define UTEST_NEW(fmt,...) vLedSetAll(++UTEST_COUNTER)
#define UTEST_SUCCESS()

#undef assert
#define assert(n) if ((n) == 0) { prvLedFlash(); }
// -------------------------------------
#endif /* NLOG defined */

#else /* __AVR__ not defined */
// -----------------------------------------------------------------------------
#define UTEST_NEW(fmt,...) gxPLPrintf("\nTest %d: "fmt,++UTEST_COUNTER,##__VA_ARGS__)
#define UTEST_SUCCESS() gxPLPrintf("Success\n")
// -----------------------------------------------------------------------------
#endif

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
INLINE void
UTEST_PMEM_BEFORE (void) {
#if !defined(NLOG)

  heap_before = gxPLDynamicMemoryUsed();
  gxPLPrintf ("Dynamic memory used before %d bytes\n", heap_before);
#endif
}

// -----------------------------------------------------------------------------
INLINE void
UTEST_PMEM_AFTER (void) {
#if !defined(NLOG)

  static int heap_after;
  heap_after = gxPLDynamicMemoryUsed();
  gxPLPrintf ("Dynamic memory used: before %d - after %d - loss %d\n",
                heap_before, heap_after, heap_after - heap_before);
#endif
}

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_UTEST_HEADER_ defined */
