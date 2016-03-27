/*
 * ui.c
 * >>> User Interface, describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/button.h>
#include "gxpl-template-avr.h"

/* macros =================================================================== */
/* structures =============================================================== */
/* types ==================================================================== */
/* public variables ========================================================= */
/* private variables ======================================================== */
/* private functions ======================================================== */

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
void
vUiInit (void) {
  
  vLedInit();
  vButInit();
  gxPLStdIoOpen();
}

// -----------------------------------------------------------------------------
int
iUiTask (gxPLDevice * device) {
#if 0
  int ret;

  // Sends heartbeat end messages to all devices
  ret = gxPLAppDisableAllDevices (app);
  assert (ret == 0);

  gxPLPrintf ("\nPress any key to close...\n");
  gxPLWait();

  ret = gxPLAppClose (app);
  assert (ret == 0);

  gxPLMessageDelete (message);

  gxPLPrintf ("\neverything was closed.\nHave a nice day !\n");
#endif
  return 0;
}

/* ========================================================================== */
