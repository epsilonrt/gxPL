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
 *    -n / --net       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 */
void
gxPLParseCommonArgs (gxPLSetting * setting, int argc, char *argv[]) {
  int c;
  static const char short_options[] = GXPL_GETOPT;
  static struct option long_options[] = {
    {"interface", required_argument, NULL, 'i'},
    {"net",       required_argument, NULL, 'n'},
    {"debug",     no_argument,       NULL, 'd' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

  // backup inital argv
  char * backup = malloc (sizeof(char *) * argc);
  memcpy (backup, argv, sizeof(char *) * argc);
  
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

      case 'd':
        vLogSetMask (LOG_UPTO (GXPL_LOG_DEBUG_LEVEL));
        setting->debug = 1;
        PDEBUG ("enable debugging");
        break;

      default:
        break;
    }
  }
  while (c != -1);
  
  // restore initial argv order
  memcpy (argv, backup, sizeof(char *) * argc);
  free(backup);
  
  optind = 1; // rewinds to allow the user to analyze again the parameters ?
  opterr = 1; // restore initial value
}


// -----------------------------------------------------------------------------
int
gxPLRandomSeed (gxPL * gxpl) {

  return time (NULL);
}

#endif /* __unix__ defined */
/* ========================================================================== */
