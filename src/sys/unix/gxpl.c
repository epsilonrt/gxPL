/**
 * @file
 * Top Layer of API (unix source code)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifdef  __unix__
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <gxPL/util.h>
#include "gxpl_p.h"

/* private functions ======================================================== */


/* api functions ============================================================ */

/* -----------------------------------------------------------------------------
 * This will parse the passed command array for options and parameters
 * It supports the following options:
 *    -i / --interface xxx : interface or device used to access the network
 *    -n / --net       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 *    -b / --baudrate      : serial baudrate (if iolayer use serial port)
 *    -D / --nodaemon      : do not daemonize
 *    -r / --reset         : performed iolayer reset
 */
void
gxPLParseCommonArgs (gxPLSetting * setting, int argc, char *argv[]) {
  int c;
  bool reset = false;
  static const char short_options[] = GXPL_GETOPT;
  static struct option long_options[] = {
    {"interface", required_argument, NULL, 'i'},
    {"net",       required_argument, NULL, 'n'},
    {"baudrate",  required_argument, NULL, 'b'},
    {"debug",     no_argument,       NULL, 'd' },
    {"nodaemon",  no_argument,       NULL, 'D' },
    {"reset",     no_argument,       NULL, 'r' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

  // backup inital argv
  char * backup = malloc (sizeof (char *) * argc);
  memcpy (backup, argv, sizeof (char *) * argc);
  char * baudrate = NULL;
  int loglvl = LOG_WARNING;
#ifdef DEBUG
  vLogSetMask (LOG_UPTO (LOG_DEBUG));
#endif

  opterr = 0; // ignore unknown options
  do  {

    c = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (c) {

      case 'i':
        strcpy (setting->iface, optarg);
        PDEBUG ("set interface to %s", setting->iface);
        break;

      case 'n':
        strcpy (setting->iolayer, optarg);
        PDEBUG ("set iolayer to %s", setting->iolayer);
        break;

      case 'b':
        baudrate = optarg;
        PDEBUG ("set baudrate to %s", baudrate);
        break;

      case 'd':
        loglvl++;
        PDEBUG ("enable log %d", loglvl);
        break;

      case 'D':
        setting->nodaemon = 1;
        PDEBUG ("set nodaemon flag");
        break;

      case 'r':
        reset = true;
        PDEBUG ("enable reset flag");
        break;

      default:
        break;
    }
  }
  while (c != -1);

  setting->log = MIN (LOG_DEBUG, loglvl);

  if (strncmp (setting->iolayer, "xbee", 4) == 0) {
    
    setting->xbee.reset_sw = reset;
    
    if (baudrate) {
      int b;
      char * endptr;
      b = strtol (baudrate, &endptr, 10);

      if (*endptr == '\0') {

        setting->xbee.ios.baud = b;
        setting->xbee.ios.dbits = SERIAL_DATABIT_8;
        setting->xbee.ios.parity = SERIAL_PARITY_NONE;
        setting->xbee.ios.sbits = SERIAL_STOPBIT_ONE;
        setting->xbee.ios.flow = GXPL_DEFAULT_FLOW;
        setting->xbee.ios.flag = 0;
        setting->iosflag = 1;
      }
    }
  }

  // restore initial argv order
  memcpy (argv, backup, sizeof (char *) * argc);
  free (backup);

  optind = 1; // rewinds to allow the user to analyze again the parameters ?
  opterr = 1; // restore initial value
}


// -----------------------------------------------------------------------------
int
gxPLRandomSeed (gxPLApplication * app) {

  return time (NULL);
}

#endif /* __unix__ defined */
/* ========================================================================== */
