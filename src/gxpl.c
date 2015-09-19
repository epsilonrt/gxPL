/**
 * @file gxpl.c
 * Top Layer of API
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <getopt.h>
#include <sysio/log.h>
#include <sysio/dlist.h>
#include <gxPL.h>

#define GXPL_INTERNALS
#include <gxPL/io.h>
#include "config.h"
#include "gxpl_p.h"
#include "version-git.h"

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */

/* private functions ======================================================== */

/* -----------------------------------------------------------------------------
 * This will parse the passed command array for options and parameters
 * It supports the following options:
 *    -i / --interface xxx : interface or device used to access the network
 *    -h / --hal       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 */
static void
vParseCommonArgs (gxPLConfig * config, int argc, char *argv[]) {
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


#warning TODO
#if 0
/* -----------------------------------------------------------------------------
 * Check to see if the passed message is a hub echo.  */
static bool
isHubEcho (gxPLMessage * msg) {
  char * remoteIP;
  char * thePort;

  if (msg == NULL) {
    return false;
  }

  /* If this is not a heartbeat, ignore it */
  if (! (!strcasecmp (msg->schema.class, "hbeat")
         || !strcasecmp (msg->schema.class, "config"))) {
    return false;
  }

  /* Insure it has an IP address and port */
  if ( (remoteIP = gxPLMessagePairValueGet (msg, "remote-ip")) == NULL) {
    return false;
  }
  if ( (thePort = gxPLMessagePairValueGet (msg, "port")) == NULL) {
    return false;
  }

  /* Now See if the IP address & port matches ours */
  if (strcmp (remoteIP, xPL_getListenerIPAddr())) {
    return false;
  }
  if (strcmp (thePort, gxPLStrFromInt (xPL_getPort()))) {
    return false;
  }

  /* Clearly this is a message from us */
  return true;
}

#endif

/* internal public functions ================================================ */


/* api functions ============================================================ */
// -----------------------------------------------------------------------------
gxPLConfig *
gxPLNewConfig (const char * iface, const char * iolayer, gxPLConnectType type) {
  gxPLConfig * config = calloc (1, sizeof (gxPLConfig));
  assert (config);

  strcpy (config->iface, iface);
  strcpy (config->iolayer, iolayer);
  config->connecttype = type;
  config->malloc = 1;

  return config;
}

// -----------------------------------------------------------------------------
gxPLConfig *
gxPLNewConfigFromCommandArgs (int argc, char * argv[], gxPLConnectType type) {
  gxPLConfig * config = calloc (1, sizeof (gxPLConfig));
  assert (config);

  vParseCommonArgs (config, argc, argv);
  config->connecttype = type;
  config->malloc = 1;

  return config;
}

// -----------------------------------------------------------------------------
gxPL *
gxPLOpen (gxPLConfig * config) {
  gxPL * gxpl = gxPLIoOpen (config);

  if (gxpl) {
    // There's perhaps something to do ?
#warning TODO
#if 0

    // Install a listener for xPL oriented messages
    if (!xPL_IODeviceInstalled) {
      if (xPL_addIODevice (xPL_receiveMessage, -1, pconfig->bind_socket, true, false, false)) {

        xPL_IODeviceInstalled = true;
      }
    }

    // Add a message listener for services
    gxPLMessageAddListener (xPL_handleServiceMessage, NULL);
#endif
  }
  return gxpl;
}

// -----------------------------------------------------------------------------
int
gxPLClose (gxPL * gxpl) {

  // There's perhaps something to do ?
#warning TODO
#if 0
  // Shutdown all services
  xPL_disableAllServices();

  // Remove xPL Listener
  if (xPL_removeIODevice (xPLFD)) {

    xPL_IODeviceInstalled = false;
  }
#endif
  return gxPLIoClose (gxpl);
}

// -----------------------------------------------------------------------------
int
gxPLPoll (gxPL * gxpl, int timeout_ms) {
  int ret, size = 0;

  ret = gxPLIoCtl (gxpl, gxPLIoFuncPoll, &size, timeout_ms);

  if ( (ret == 0) && (size > 0)) {
    char * buffer = malloc (size + 1);
    assert (buffer);

    ret = gxPLIoRead (gxpl, buffer, size);
    if (ret == size) {

      /* We receive a message, append null character to terminate the string */
      buffer[size] = '\0';
      vLog (LOG_DEBUG, "Just read %d bytes as packet [%s]", size, buffer);
#warning TODO
#if 0
      int bytesRead;
      gxPLMessage * msg = NULL;

      /* Send the raw message to any raw message listeners */
      xPL_dispatchRawEvent (messageBuff, bytesRead);

      /* Parse the message */
      if ( (msg = gxPLMessageFromString (gxpl, buffer)) == NULL) {
        vLog (LOG_ERR, "Error parsing network message - ignored");
        continue;
      }

      /* See if we need to check the message for hub detection */
      if (!hubConfirmed && isHubEcho (msg)) {
        vLog (LOG_DEBUG, "Hub detected and confirmed existing");
        hubConfirmed = true;
      }

      /* Dispatch the message */
      vLog (LOG_DEBUG, "Now dispatching valid message");
      xPL_dispatchMessageEvent (msg);

      /* Release the message */
      gxPLMessageDelete (msg);
#endif
    }
    free (buffer);
  }
  return ret;
}


/* -----------------------------------------------------------------------------
 * Public
 * Send an xPL message.  If the message is valid and is successfully sent,
 * TRUE is returned.
 */
int
gxPLSendMessage (gxPL * gxpl, gxPLMessage * message) {

#warning TODO
#if 0
  /* Write the message to text */
  if (gxPLMessageToString (message) == NULL) {
    return -1;
  }

  /* Attempt to brodcast it */
  vLog (LOG_ERR, "About to broadcast %d bytes as [%s]", messageBytesWritten, messageBuff);
  if (!xPL_sendRawMessage (messageBuff, messageBytesWritten)) {
    return -1;
  }
#endif
  /* And we are done */
  return 0;
}


// -----------------------------------------------------------------------------
char *
gxPLLocalAddressString (gxPL * gxpl) {
  gxPLAddress addr;
  static char * str;

  if (gxPLIoCtl (gxpl, gxPLIoFuncGetLocalAddr, &addr) == 0) {

    if (gxPLIoCtl (gxpl, gxPLIoFuncNetAddrToString, &addr, &str) == 0) {

      return str;
    }
  }
  return "";
}

// -----------------------------------------------------------------------------
char *
gxPLBroadcastAddressString (gxPL * gxpl) {
  gxPLAddress addr;
  static char * str;

  if (gxPLIoCtl (gxpl, gxPLIoFuncGetBcastAddr, &addr) == 0) {

    if (gxPLIoCtl (gxpl, gxPLIoFuncNetAddrToString, &addr, &str) == 0) {

      return str;
    }
  }
  return "";
}

// -----------------------------------------------------------------------------
int
gxPLInetPort (gxPL * gxpl) {
  int iport;
  if (gxPLIoCtl (gxpl, gxPLIoFuncGetInetPort, &iport) == 0) {

    return iport;
  }
  return -1;
}

// -----------------------------------------------------------------------------
const char *
gxPLVersion (void) {

  return VERSION_SHORT;
}

// -----------------------------------------------------------------------------
int
gxPLVersionMajor (void) {

  return VERSION_MAJOR;
}

// -----------------------------------------------------------------------------
int
gxPLVersionMinor (void) {

  return VERSION_MINOR;
}

// -----------------------------------------------------------------------------
int
gxPLVersionPatch (void) {

  return VERSION_PATCH;
}

// -----------------------------------------------------------------------------
int
gxPLVersionSha1 (void) {

  return VERSION_SHA1;
}

/* ========================================================================== */
