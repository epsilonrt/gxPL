/*
 * template.c
 * >>> Application without daemon, Describe the contents of your file here
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <sysio/log.h>
#include "template.h"

/* main ===================================================================== */
int
main (int argc, char **argv) {
  gxPLSetting * setting;

  // Read arguments from the command line
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  if (setting == NULL) {

    PERROR ("Unable to get settings from command line");
    exit (EXIT_FAILURE);
  }
  
  // start the main task
  vMain (setting);
  return 0;
}

/* ========================================================================== */
