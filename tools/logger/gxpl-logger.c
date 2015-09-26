/**
 * @file gxpl-logger.c
 * Simple program to monitor for any message and any device changes and print them
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define LOGGER_VERSION VERSION_SHORT
#define LOG_FILE_CFG_NAME "logFilename"
#define LOG_APPEND_CFG_NAME "append2Log"

/* private variables ======================================================== */
static FILE * logFile = NULL;
static char * logFileName = "";
static bool appendToLog = FALSE;
static gxPLDevice * loggerService = NULL;

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
void
shutdownHandler (int onSignal) {
  gxPLDeviceEnabledSet (loggerService, FALSE);
  gxPLDelete (loggerService);
  gxPLClose();
  exit (0);
}

/* -----------------------------------------------------------------------------
 * It's best to put the logic for reading the service configuration
 * and parsing it into your code in a seperate function so it can
 * be used by your configChangedHandler and your startup code that
 * will want to parse the same data after a config file is loaded */
static void
parseConfig (gxPLDevice * service) {
  char * configValue;
  FILE * newLogFile = NULL;

  /* Get append status */
  if ( (configValue = gxPLgetServiceConfigValue (service, LOG_APPEND_CFG_NAME)) != NULL) {

    appendToLog = strcasecmp (configValue, "true") == 0;
  }

  /* Get log file name and see if it's changed */
  if ( (configValue = gxPLgetServiceConfigValue (service, LOG_FILE_CFG_NAME)) != NULL) {

    /* Log file changed and not blank -- try to open the new one */
    if ( (strlen (configValue) != 0) && (strcmp (logFileName, configValue) != 0)) {
      /* Attempt to open new log file */
      if (strcasecmp (configValue, "stderr") == 0) {
        newLogFile = stderr;
      }
      else if (strcasecmp (configValue, "stdout") == 0) {
        newLogFile = stdout;
      }
      else {
        newLogFile = fopen (configValue, (appendToLog ? "a" : "w"));
      }

      /* If there is no new log file, it's bad -- ignore this and move on */
      if (newLogFile == NULL) {
        return;
      }

      /* Close out old log file (unless it was the console */
      if ( (logFile != NULL) && (logFile != stderr) && (logFile != stdout)) {
        fflush (logFile);
        fclose (logFile);
      }

      /* Install new file name and handle */
      logFileName = configValue;
      logFile = newLogFile;
    }
  }

  /* Other configurables */
}

/* --------------------------------------------------------------------------
 * Handle a change to the logger service configuration */
static void
configChangedHandler (gxPLDevice * service, void * userData) {

  /* Read config items for service and install */
  parseConfig (service);
}

/* --------------------------------------------------------------------------
 * Write a date/time stamp */
void printTimestamp (void) {
  char dateTimeBuffer[41];
  time_t rightNow;

  time (&rightNow);
  strftime (dateTimeBuffer, 40, "%y/%m/%d %H:%M:%S ", localtime (&rightNow));
  fprintf (logFile, dateTimeBuffer);
}

/* --------------------------------------------------------------------------
 * Print info on incoming messages */
void printXPLMessage (gxPLMessage * message, void * userValue) {

  printTimestamp();
  fprintf (logFile, "[gxPLMSG] TYPE=");
  switch (gxPLMessageTypeGet (message)) {
    case gxPLMessageCommand:
      fprintf (logFile, "xpl-cmnd");
      break;
    case gxPLMessageStatus:
      fprintf (logFile, "xpl-stat");
      break;
    case gxPLMessageTrigger:
      fprintf (logFile, "xpl-trig");
      break;
    default:
      fprintf (logFile, "!UNKNOWN!");
      break;
  }


  /* Print hop count, if interesting */
  if (gxPLMessageHopGet (message) != 1) {
    fprintf (logFile, ", HOPS=%d", gxPLMessageHopGet (message));
  }

  /* Source Info */
  fprintf (logFile, ", SOURCE=%s-%s.%s, TARGET=",
           gxPLMessageSourceVendorIdGet (message),
           gxPLMessageSourceDeviceIdGet (message),
           gxPLMessageSourceInstanceIdGet (message));

  /* Handle various target types */
  if (gxPLMessageBroadcastGet (message)) {
    fprintf (logFile, "*");
  }
  else {
    if (gxPLMessageIsGroup (message)) {
      fprintf (logFile, "XPL-GROUP.%s", gxPLMessageTargetGetGroup (message));
    }
    else {
      fprintf (logFile, "%s-%s.%s",
               gxPLMessageTargetVendorIdGet (message),
               gxPLMessageTargetDeviceIdGet (message),
               gxPLMessageTargetInstanceIdGet (message));
    }
  }

  /* Echo Schema Info */
  fprintf (logFile, ", CLASS=%s, TYPE=%s", gxPLMessageSchemaClassGet (message), gxPLMessageSchemaTypeGet (message));
  fprintf (logFile, "\n");
}

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
  /* Parse the command line */
  if (!gxPLparseCommonArgs (&argc, argv, FALSE)) {
    exit (1);
  }

  /* Start gxPLib */
  if (!gxPLConfigNew (gxPLConnectionTypeGet())) {
    fprintf (stderr, "Unable to start gxPLib\n");
    exit (1);
  }

  /* And a listener for all xPL messages */
  gxPLMessageListenerAdd (printXPLMessage, NULL);

  /* Create a service so the hubs know to send things to us        */
  /* While we are not relaly using he service, xPL hubs will not   */
  /* forward messages to us until they have seen a xPL-looking     */
  /* device on the end of a hub connection, so this just gets us a */
  /* place at the table, so to speak                               */
  loggerService = gxPLcreateConfigurableService ("cdp1802", "logger", "logger.xpl");
  gxPLDeviceVersionSet (loggerService, LOGGER_VERSION);

  if (!gxPLIsServiceConfigured (loggerService)) {
    
    /* Define a configurable item and give it a default */
    gxPLaddServiceConfigurable (loggerService, LOG_FILE_CFG_NAME, gxPLConfigReconf, 1);
    gxPLsetServiceConfigValue (loggerService, LOG_FILE_CFG_NAME, "stderr");

    gxPLaddServiceConfigurable (loggerService, LOG_APPEND_CFG_NAME, gxPLConfigReconf, 1);
    gxPLsetServiceConfigValue (loggerService, LOG_APPEND_CFG_NAME, "false");
  }

  /* Parse the service configurables into a form this program */
  /* can use (whether we read a config or not)                */
  parseConfig (loggerService);

  /* Add a service change listener we'll use to pick up changes */
  gxPLaddServiceConfigChangedListener (loggerService, configChangedHandler, NULL);

  /* Enable the service */
  gxPLDeviceEnabledSet (loggerService, TRUE);

  /* Install signal traps for proper shutdown */
  signal (SIGTERM, shutdownHandler);
  signal (SIGINT, shutdownHandler);

  /* Hand control over to gxPLib */
  gxPLprocessMessages (-1);
  exit (0);
}
/* ========================================================================== */
