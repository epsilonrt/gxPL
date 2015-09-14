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

#include "config.h"
#include "gxpl_p.h"
#include "io_p.h"

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

#if 0
/* -----------------------------------------------------------------------------
 * Check to see if the passed message is a hub echo.  */
static bool
isHubEcho (gxPLMessage * theMessage) {
  char * remoteIP;
  char * thePort;

  if (theMessage == NULL) {
    return false;
  }

  /* If this is not a heartbeat, ignore it */
  if (! (!xPL_strcmpIgnoreCase (theMessage->schemaClass, "hbeat")
         || !xPL_strcmpIgnoreCase (theMessage->schemaClass, "config"))) {
    return false;
  }

  /* Insure it has an IP address and port */
  if ( (remoteIP = xPL_getMessageNamedValue (theMessage, "remote-ip")) == NULL) {
    return false;
  }
  if ( (thePort = xPL_getMessageNamedValue (theMessage, "port")) == NULL) {
    return false;
  }

  /* Now See if the IP address & port matches ours */
  if (strcmp (remoteIP, xPL_getListenerIPAddr())) {
    return false;
  }
  if (strcmp (thePort, xPL_intToStr (xPL_getPort()))) {
    return false;
  }

  /* Clearly this is a message from us */
  return true;
}

/* -----------------------------------------------------------------------------
 * Convert a text message into a xPL message.
 * Return the message or NULL if there is a parse error
 */
static gxPLMessage *
parseMessage (char * theText) {
  int textLen = strlen (theText);
  int parsedChars, parsedThisTime;
  char * blockHeaderKeyword;
  char * blockDelimStr;
  char * periodstr = NULL;
  gxPLMessage * theMessage;

  /* Allocate a message */
  theMessage = createReceivedMessage (xPLMessageAny);

  /* Parse the header */
  if ( (parsedThisTime = parseBlock (theText, &blockHeaderKeyword, theMessage->messageBody, false)) <= 0) {
    xPL_Debug ("Error parsing message header");
    xPL_releaseMessage (theMessage);
    return NULL;
  }
  parsedChars = parsedThisTime;

  /* Parse the header */
  if (!xPL_strcmpIgnoreCase (blockHeaderKeyword, "XPL-CMND")) {
    xPL_setMessageType (theMessage, xPLMessageCommand);
  }
  else if (!xPL_strcmpIgnoreCase (blockHeaderKeyword, "XPL-STAT")) {
    xPL_setMessageType (theMessage, xPLMessageStatus);
  }
  else if (!xPL_strcmpIgnoreCase (blockHeaderKeyword, "XPL-TRIG")) {
    xPL_setMessageType (theMessage, xPLMessageTrigger);
  }
  else {
    xPL_Debug ("Unknown message header of %s - bad message", blockHeaderKeyword);
    STR_FREE (blockHeaderKeyword);
    xPL_releaseMessage (theMessage);
    return NULL;
  }

  /* We are done with this now - drop it while we are still thinking about it */
  STR_FREE (blockHeaderKeyword);

  /* Parse the name/values into the message */
  if (!parseMessageHeader (theMessage, theMessage->messageBody)) {
    xPL_Debug ("Unable to parse message header");
    xPL_releaseMessage (theMessage);
    return NULL;
  }

  /* Parse multiple blocks until we are done */
  for (; parsedChars < textLen;) {
    /* Clear the name/value list for the message */
    xPL_clearAllNamedValues (theMessage->messageBody);
    periodstr = NULL;

    /* Parse the next block */
    if ( (parsedThisTime = parseBlock (& (theText[parsedChars]), &blockHeaderKeyword, theMessage->messageBody, false)) < 0) {
      xPL_Debug ("Error parsing message block");
      xPL_releaseMessage (theMessage);
      STR_FREE (blockHeaderKeyword);
      return NULL;
    }

    /* If we ran out of characters, no more blocks */
    if (parsedThisTime == 0) {
      break;
    }

    /* Up Parsed count */
    parsedChars += parsedThisTime;

    /* Parse the block header */
    if ( (blockDelimStr = strchr (blockHeaderKeyword, '.')) == NULL) {
      xPL_Debug ("Malformed message block header - %s", blockHeaderKeyword);
      xPL_releaseMessage (theMessage);
      STR_FREE (blockHeaderKeyword);
      return NULL;
    }
    periodstr = blockDelimStr;
    *blockDelimStr++ = '\0';

    /* Record the message schema class/type */
    xPL_setSchemaClass (theMessage, blockHeaderKeyword);
    xPL_setSchemaType (theMessage, blockDelimStr);

    /* Fix mangled string & release string */
    if (periodstr != NULL) {
      *periodstr = '.';
    }
    STR_FREE (blockHeaderKeyword);
    break;
  }

  /* Return the message */
  return theMessage;
}
#endif

/* internal public functions ================================================ */

/* -----------------------------------------------------------------------------
 * Parse and dispatch an xPL message
 */
int
gxPLParseDatagram (gxPL * gxpl, char * data) {
  
#if 0
  int bytesRead;
  gxPLMessage * theMessage = NULL;

  for (;;) {


    /* Send the raw message to any raw message listeners */
    xPL_dispatchRawEvent (messageBuff, bytesRead);

    /* Parse the message */
    if ( (theMessage = parseMessage (messageBuff)) == NULL) {
      vLog (LOG_ERR, "Error parsing network message - ignored");
      continue;
    }

    /* See if we need to check the message for hub detection */
    if (!hubConfirmed && isHubEcho (theMessage)) {
      vLog (LOG_DEBUG, "Hub detected and confirmed existing");
      hubConfirmed = true;
    }

    /* Dispatch the message */
    vLog (LOG_DEBUG, "Now dispatching valid message");
    xPL_dispatchMessageEvent (theMessage);

    /* Release the message */
    xPL_releaseMessage (theMessage);
  }
#endif

  return 0;
}

/* api functions ============================================================ */
// -----------------------------------------------------------------------------
gxPL *
gxPLOpen (const char * iface, const char * iolayer, gxPLConnectType type) {
  gxPLConfig * config = calloc (1, sizeof (gxPLConfig));
  assert (config);

  strcpy (config->iface, iface);
  strcpy (config->iolayer, iolayer);
  config->connecttype = type;
  config->malloc = 1;

  return gxPLOpenWithConfig (config);
}

// -----------------------------------------------------------------------------
gxPL *
gxPLOpenWithArgs (int argc, char * argv[], gxPLConnectType type) {
  gxPLConfig * config = calloc (1, sizeof (gxPLConfig));
  assert (config);

  vParseCommonArgs (config, argc, argv);
  config->connecttype = type;
  config->malloc = 1;

  return gxPLOpenWithConfig (config);
}

// -----------------------------------------------------------------------------
gxPL *
gxPLOpenWithConfig (gxPLConfig * config) {
  gxPLIo * io = calloc (1, sizeof (gxPLIo));
  assert (io);

  if (strlen (config->iolayer) == 0) {
    strcpy (config->iolayer, DEFAULT_IO_LAYER);
    PDEBUG ("set iolayer to default (%s)", config->iolayer);
  }

  io->ops = gxPLIoGetOps (config->iolayer);
  if (io->ops) {
    gxPL * gxpl = calloc (1, sizeof (gxPL));
    assert (gxpl);

    gxpl->config = config;
    gxpl->io = io;

    if (io->ops->open (gxpl) == 0) {
#if 0

      // Install a listener for xPL oriented messages
      if (!xPL_IODeviceInstalled) {
        if (xPL_addIODevice (xPL_receiveMessage, -1, pconfig->bind_socket, true, false, false)) {

          xPL_IODeviceInstalled = true;
        }
      }

      // Add a message listener for services
      xPL_addMessageListener (xPL_handleServiceMessage, NULL);

      // We are ready to go
      return 0;
#endif
      return gxpl;
    }
    free (gxpl);
  }
  free (io);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLClose (gxPL * gxpl) {
  int ret;

  /*
    // Shutdown all services
    xPL_disableAllServices();

    // Remove xPL Listener
    if (xPL_removeIODevice (xPLFD)) {

      xPL_IODeviceInstalled = false;
    }
  */

  ret = gxpl->io->ops->close (gxpl);
  free (gxpl->io);
  if (gxpl->config->malloc) {
    free (gxpl->config);
  }
  free (gxpl);
  return ret;
}


// -----------------------------------------------------------------------------
int
gxPLPoll (gxPL * gxpl, int timeout_ms) {
  int ret, size;

  ret = gxpl->io->ops->ctl (gxpl, xPLIoFuncPoll, &size, timeout_ms);

  if ( (ret == 0) && (size > 0)) {
    char * buffer = malloc (size + 1);
    assert (buffer);

    ret = gxpl->io->ops->read (gxpl, buffer, size);
    if (ret == size) {
      
      /* We receive a message, append null character to terminate the string */
      buffer[size] = '\0';
      vLog (LOG_DEBUG, "Just read %d bytes as packet [%s]", size, buffer);
      ret = gxPLParseDatagram (gxpl, buffer);
    }
    free (buffer);
  }
  return ret;
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
