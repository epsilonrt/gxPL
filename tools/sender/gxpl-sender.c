/* xPLSend.c - Command Line xPL message sending tool */
/* Copyright (c) 2005, Gerald R Duprey Jr. */


#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <gxPL.h>

char msgSource[64] = "cdp1802-xplsend.default";
char * srcVendor = NULL;
char * srcDeviceID = NULL;
char * srcInstanceID = NULL;

char * msgTarget = NULL;
char * tgtVendor = NULL;
char * tgtDeviceID = NULL;
char * tgtInstanceID = NULL;

xPL_MessageType msgType = xPL_MESSAGE_COMMAND;
char * msgSchemaClass = NULL;
char * msgSchemaType = NULL;

/* Print usage info */
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

bool parseSourceIdent(void) {
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
  srcDeviceID = dashstr;
  srcInstanceID = periodstr;

  return TRUE;
}

bool parseTargetIdent(void) {
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
  tgtDeviceID = dashstr;
  tgtInstanceID = periodstr;

  return TRUE;
}

/** Parse command line for switches */
/** -s - source of message ident */
/** -t - target of message ident */
/** -b - target is broadcast */
/** -m - message type */
/** -c - schema class/type */
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

        if (xPL_strcmpIgnoreCase (argv[swptr], "cmnd") == 0) {
          msgType = xPL_MESSAGE_COMMAND;
        }
        else if (xPL_strcmpIgnoreCase (argv[swptr], "trig") == 0) {
          msgType = xPL_MESSAGE_TRIGGER;
        }
        else if (xPL_strcmpIgnoreCase (argv[swptr], "stat") == 0) {
          msgType = xPL_MESSAGE_STATUS;
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


bool sendMessage (int argc, char * argv[]) {
  int argIndex = 0;
  xPL_Service * theService = NULL;
  xPL_Message * theMessage = NULL;
  char * delim;

  /* Create service so we can create messages */
  if ( (theService = xPL_createService (srcVendor, srcDeviceID, srcInstanceID)) == NULL) {
    fprintf (stderr, "Unable to create xPL service\n");
    return FALSE;
  }

  /* Create an appropriate message */
  if (msgTarget == NULL) {
    if ( (theMessage = xPL_createBroadcastMessage (theService, msgType)) == NULL) {
      fprintf (stderr, "Unable to create broadcast message\n");
      return FALSE;
    }
  }
  else {
    if ( (theMessage = xPL_createTargetedMessage (theService, msgType, tgtVendor, tgtDeviceID, tgtInstanceID)) == NULL) {
      fprintf (stderr, "Unable to create targetted message\n");
      return FALSE;
    }
  }

  /* Install the schema */
  xPL_setSchema (theMessage, msgSchemaClass, msgSchemaType);

  /* Install named values */
  for (argIndex = 1; argIndex < argc; argIndex++) {
    if ( (delim = strstr (argv[argIndex], "=")) == NULL) {
      fprintf (stderr, "Improperly formatted name/value pair %s\n", argv[argIndex]);
      return FALSE;
    }

    /* Break them up  and add it */
    *delim++ = '\0';
    xPL_addMessageNamedValue (theMessage, argv[argIndex], delim);
  }

  /* Send the message */
  if (!xPL_sendMessage (theMessage)) {
    fprintf (stderr, "Unable to send xPL message\n");
    return FALSE;
  }

  return TRUE;
}

int main (int argc, char * argv[]) {
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
  if (!xPL_initialize (xPL_getParsedConnectionType())) {
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
