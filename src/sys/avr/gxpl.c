/**
 * @file
 * Top Layer of API (avr 8-bits source code)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifdef  __AVR__
#include "config.h"
#include <avrio/task.h>
#include <string.h>
#include <avrio/task.h>
#include <gxPL/util.h>
#include "gxpl_p.h"

/* api functions ============================================================ */

/* -----------------------------------------------------------------------------
 * provided only for compatibility.
 */
void
gxPLParseCommonArgs (gxPLSetting * setting, int argc, char *argv[]) {
  
  strcpy (setting->iolayer, DEFAULT_AVR8_IO_LAYER);
  setting->xbee.ios.baud = DEFAULT_XBEE_BAUDRATE;
  setting->xbee.ios.dbits = SERIAL_DATABIT_8;
  setting->xbee.ios.parity = SERIAL_PARITY_NONE;
  setting->xbee.ios.sbits = SERIAL_STOPBIT_ONE;
  setting->xbee.ios.flow = DEFAULT_XBEE_FLOW;
  setting->iosflag = 1;
}

// -----------------------------------------------------------------------------
int
gxPLRandomSeed (gxPLApplication * app) {
  unsigned long t;
  
  (void) gxPLTimeMs (&t);
  return t;
}

#endif /* __AVR__ defined */
/* ========================================================================== */
