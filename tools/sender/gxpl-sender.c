/**
 * @file gxpl-sender.c
 * Command Line xPL message sending tool
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
/* macros =================================================================== */
/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */
char msgSource[64] = "cdp1802-xplsend.default";
char * srcVendor = NULL;
char * srcDeviceId = NULL;
char * srcInstanceId = NULL;
char * msgTarget = NULL;
char * tgtVendor = NULL;
char * tgtDeviceId = NULL;
char * tgtInstanceId = NULL;
gxPLMessageType msgType = gxPLMessageCommand;
char * msgSchemaClass = NULL;
char * msgSchemaType = NULL;

/* public variables ========================================================= */
/* internal public functions ================================================ */

/* private functions ======================================================== */
// -----------------------------------------------------------------------------

/* -----------------------------------------------------------------------------
 * Print usage info */
void printUsage (char * commandName) {
  fprintf (stderr, "%s - xPL Message Sender\n", commandName);
  fprintf (stderr, "Copyright (c) 2005, Gerald R Duprey Jr\n\n");
  fprintf (stderr, "Usage: %s [-s source] [-b] [-t target] [-m msgType] -c schema  name=value name=value ...\n", commandName);
  fprintf (stderr, "  -s source - source of message in vendor-device.instance (default: cdp1802-xplsend.default)\n");
  fprintf (stderr, "  -b send broadcast message (default)\n");
  fprintf (stderr, "  -t target - target of message in vendor-device.instance format.  (default: broadcast)\n");
  fprintf (stderr, "  -m message type: cmnd, trig or stat (default: cmnd)\n");
  fprintf (stderr, "  -c schema class and type formatted as class.type - REQUIRED\n\n");
}

// -----------------------------------------------------------------------------
bool 
parseSourceIdent (void) {
  char * dashstr, * periodstr;

  /* Make sure we have something to work with */
  if ( (msgSource == NULL) || (strlen (msgSource) < 5)) {
    fprintf (stderr, "Empty or too short message source ID\n");
    return FALSE;
  }

  /* Locate the delimiters */
  if ( (dashstr = strstr (msgSource, "-")) == NULL) {
    fprintf (stderr, "Missing dash in source ident -- invalid source ID\n");
    return FALSE;
  }
  if ( (periodstr = strstr (dashstr, ".")) == NULL) {
    fprintf (stderr, "Missing period in source ident -- invalid source ID\n");
    return FALSE;
  }

  /* Install pointers */
  *dashstr++ = '\0';
  *periodstr++ = '\0';
  srcVendor = msgSource;
  srcDeviceId = dashstr;
  srcInstanceId = periodstr;

  return TRUE;
}

// -----------------------------------------------------------------------------
bool 
parseTargetIdent (void) {
  char * dashstr, * periodstr;

  /* Make sure we have something to work with */
  if (msgTarget == NULL) {
    return TRUE;
  }
  if (strlen (msgTarget) < 5) {
    fprintf (stderr, "Empty or too short message target ID\n");
    return FALSE;
  }

  /* Locate the delimiters */
  if ( (dashstr = strstr (msgTarget, "-")) == NULL) {
    fprintf (stderr, "Missing dash in target ident -- invalid target ID\n");
    return FALSE;
  }
  if ( (periodstr = strstr (dashstr, ".")) == NULL) {
    fprintf (stderr, "Missing period in target ident -- invalid target ID\n");
    return FALSE;
  }

  /* Install pointers */
  *dashstr++ = '\0';
  *periodstr++ = '\0';
  tgtVendor = msgTarget;
  tgtDeviceId = dashstr;
  tgtInstanceId = periodstr;

  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Parse command line for switches 
 * -s - source of message ident 
 * -t - target of message ident 
 * -b - target is broadcast 
 * -m - message type 
 * -c - schema class / type */
bool parseCmdLine (int *argc, char *argv[]) {
  int swptr;
  int newcnt = 0;
  char * delim;


  /* Handle each item of the command line.  If it starts with a '-', then */
  /* process it as a switch.  If not, then copy it to a new position in   */
  /* the argv list and up the new parm counter.                           */
  for (swptr = 0; swptr < *argc; swptr++) {
    /* If it doesn't begin with a '-', it's not a switch. */
    if (argv[swptr][0] != '-') {
      if (swptr != newcnt) {
        argv[++newcnt] = argv[swptr];
      }
    }
    else {
      if (!strcmp (argv[swptr], "-s")) {
        swptr++;
        strcpy (msgSource, argv[swptr]);
        continue;
      }

      if (!strcmp (argv[swptr], "-t")) {
        swptr++;
        msgTarget = argv[swptr];
        continue;
      }

      if (!strcmp (argv[swptr], "-b")) {
        msgTarget = NULL;
        continue;
      }

      if (!strcmp (argv[swptr], "-m")) {
        swptr++;

        if (strcasecmp (argv[swptr], "cmnd") == 0) {
          msgType = gxPLMessageCommand;
        }
        else if (strcasecmp (argv[swptr], "trig") == 0) {
          msgType = gxPLMessageTrigger;
        }
        else if (strcasecmp (argv[swptr], "stat") == 0) {
          msgType = gxPLMessageStatus;
        }
        else {
          fprintf (stderr, "Unknown message type of %s for -m", argv[swptr]);
          return FALSE;
        }

        continue;
      }

      if (!strcmp (argv[swptr], "-c")) {
        swptr++;

        if ( (delim = strstr (argv[swptr], ".")) == NULL) {
          fprintf (stderr, "Improperly formatted schema class.type of %s", argv[swptr]);
          return FALSE;
        }

        *delim++ = '\0';
        msgSchemaClass = strdup (argv[swptr]);
        msgSchemaType = strdup (delim);
        continue;
      }


      /* Anything left is unknown */
      fprintf (stderr, "Unknown switch `%s'", argv[swptr]);
      return FALSE;
    }
  }

  /* Set in place the new argument count and exit */
  *argc = newcnt + 1;
  return TRUE;
}

// -----------------------------------------------------------------------------
bool 
sendMessage (int argc, char * argv[]) {
  int argIndex = 0;
  gxPLService * service = NULL;
  gxPLMessage * message = NULL;
  char * delim;

  /* Create service so we can create messages */
  if ( (service = xPL_createService (srcVendor, srcDeviceId, srcInstanceId)) == NULL) {
    fprintf (stderr, "Unable to create xPL service\n");
    return FALSE;
  }

  /* Create an appropriate message */
  if (msgTarget == NULL) {
    if ( (message = gxPLMessageNewBroadcast (service, msgType)) == NULL) {
      fprintf (stderr, "Unable to create broadcast message\n");
      return FALSE;
    }
  }
  else {
    if ( (message = gxPLMessageNewTargeted (service, msgType, tgtVendor, tgtDeviceId, tgtInstanceId)) == NULL) {
      fprintf (stderr, "Unable to create targetted message\n");
      return FALSE;
    }
  }

  /* Install the schema */
  gxPLMessageSchemaSetAll (message, msgSchemaClass, msgSchemaType);

  /* Install named values */
  for (argIndex = 1; argIndex < argc; argIndex++) {
    if ( (delim = strstr (argv[argIndex], "=")) == NULL) {
      fprintf (stderr, "Improperly formatted name/value pair %s\n", argv[argIndex]);
      return FALSE;
    }

    /* Break them up  and add it */
    *delim++ = '\0';
    gxPLMessagePairAdd (message, argv[argIndex], delim);
  }

  /* Send the message */
  if (!gxPLSendMessage (message)) {
    fprintf (stderr, "Unable to send xPL message\n");
    return FALSE;
  }

  return TRUE;
}

/* main ===================================================================== */
int 
main (int argc, char * argv[]) {

  /* Handle a plea for help */
  if (argc == 1) {
    printUsage (argv[0]);
    exit (1);
  }

  /* Parse xPL & program command line parms */
  if (!xPL_parseCommonArgs (&argc, argv, FALSE)) {
    exit (1);
  }
  if (!parseCmdLine (&argc, argv)) {
    exit (1);
  }

  /* Ensure we have a class */
  if ( (msgSchemaClass == NULL) || (msgSchemaType == NULL)) {
    fprintf (stderr, "The -c schema class.type is REQUIRED\n");
    exit (1);
  }

  /* Start xPL up */
  if (!gxPLNewConfig (gxPLGetConnectionType())) {
    fprintf (stderr, "Unable to start xPL");
    exit (1);
  }

  /* Parse the source */
  if (!parseSourceIdent()) {
    exit (1);
  }

  /* Parse the target */
  if (!parseTargetIdent()) {
    exit (1);
  }

  /* Send the message */
  if (!sendMessage (argc, argv)) {
    exit (1);
  }
  return 0;
}
/* ========================================================================== */
