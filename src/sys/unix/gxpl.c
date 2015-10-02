/**
 * @file src/sys/unix/gxpl.c
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
 *    -h / --hal       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 */
void
gxPLParseCommonArgs (gxPLSetting * config, int argc, char *argv[]) {
  int c;
  static const char short_options[] = "i:h:d";
  static struct option long_options[] = {
    {"interface", required_argument, NULL, 'i'},
    {"hal",       required_argument, NULL, 'h'},
    {"debug",     no_argument,       NULL, 'd' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

  do  {
    c = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (c) {

      case 'i':
        strcpy (config->iface, optarg);
        PDEBUG ("set interface to %s", config->iface);
        break;

      case 'h':
        strcpy (config->iolayer, optarg);
        PDEBUG ("set iolayer to %s", config->iolayer);
        break;

      case 'd':
        vLogSetMask (LOG_UPTO (LOG_DEBUG));
        config->debug = 1;
        PDEBUG ("enable debugging");
        break;

      default:
        break;
    }
  }
  while (c != -1);

  optind = 1; // rewinds to allow the user to analyze again the parameters
}

// -----------------------------------------------------------------------------
int 
gxPLRandomSeed (gxPL * gxpl) {
  
  return time (NULL);
}

#endif /* __unix__ defined */
/* ========================================================================== */
