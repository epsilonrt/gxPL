/*
 * template.cpp
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <csignal>
#include <unistd.h>
#include <xPL.hpp>

using namespace std; 

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

  cout << endl << "everything was closed." << endl << "Have a nice day !" << endl;
  exit(EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {

  // vSigIntHandler() intercepte le CTRL+C
  signal(SIGINT, vSigIntHandler);

  cout << "Press Ctrl+C to abort ..." << endl;
  for (;;) {
    // Remplacer les lignes ci-dessous par votre code
    cout << '.' << flush;
    sleep (1);
  }

  return 0;
}
/* ========================================================================== */
