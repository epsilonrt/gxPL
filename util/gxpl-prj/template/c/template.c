/*
 * template.c
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <gxPL.h>

/* constants ================================================================ */
/* macros =================================================================== */
/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */
/* public variables ========================================================= */
/* internal public functions ================================================ */

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  printf("\neverything was closed.\nHave a nice day !\n");
  exit(EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {

  // vSigIntHandler() intercepte le CTRL+C
  signal(SIGINT, vSigIntHandler);

  printf("Press Ctrl+C to abort ...\n");
  for (;;) {
    // Replace the 2 lines below with your source code
    putchar('.'); fflush(stdout);
    sleep (1);
  }

  return 0;
}
/* ========================================================================== */
