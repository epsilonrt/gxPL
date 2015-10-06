/*
 * app-template.c
 * @brief gxPLApplication template project
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <gxPL.h>
#include <sysio/delay.h>

/* constants ================================================================ */
/* macros =================================================================== */
#define test(t) do { \
    if (!t) { \
      fprintf (stderr, "line %d in %s: test %d failed !\n",  __LINE__, \
               __FUNCTION__, test_count); \
      exit (EXIT_FAILURE); \
    } \
  } while (0)

/* private variables ======================================================== */
static int test_count;

/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */
/* public variables ========================================================= */

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {

  // vSigIntHandler() intercepte le CTRL+C
  signal (SIGINT, vSigIntHandler);

  printf ("Press Ctrl+C to abort ...\n");
  for (;;) {
    // Remplacer les lignes ci-dessous par votre code
    putchar ('.');
    fflush (stdout);
    gxPLTimeDelayMs (1000);
  }

  return 0;
}
/* ========================================================================== */
