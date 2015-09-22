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
/* structures =============================================================== */
typedef struct hitem {
  gxPLMessageListener func;
  void * data;
} hitem;

/* types ==================================================================== */
/* private variables ======================================================== */

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static const void *
prvHandlerKey (const void * item) {
  hitem * p = (hitem *) item;

  return & (p->func);
}

// -----------------------------------------------------------------------------
static int
prvHandlerMatch (const void *key1, const void *key2) {

  return * ( (gxPLMessageListener *) key1) == * ( (gxPLMessageListener *) key2);
}

/* -----------------------------------------------------------------------------
 * This will parse the passed command array for options and parameters
 * It supports the following options:
 *    -i / --interface xxx : interface or device used to access the network
 *    -h / --hal       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 */
static void
prvParseCommonArgs (gxPLConfig * config, int argc, char *argv[]) {
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

  prvParseCommonArgs (config, argc, argv);
  config->connecttype = type;
  config->malloc = 1;

  return config;
}

// -----------------------------------------------------------------------------
gxPL *
gxPLOpen (gxPLConfig * config) {
  gxPL * gxpl = calloc (1, sizeof (gxPL));
  assert (gxpl);

  if (config->debug) {

    vLogSetMask (LOG_UPTO (GXPL_LOG_DEBUG_LEVEL));
  }

  gxpl->io = gxPLIoOpen (config);

  if (gxpl->io) {
    
    if (config->malloc == 0) {
      
      gxpl->config = malloc (sizeof(gxPLConfig));
      memcpy (gxpl->config, config, sizeof(gxPLConfig));
      gxpl->config->malloc = 1;
    }
    else {
      gxpl->config = config;
    }
    if (gxPLIoCtl (gxpl, gxPLIoFuncGetLocalAddr, &gxpl->net_info) == 0) {
      if (iVectorInit (&gxpl->listeners, NULL, free) == 0) {
        if (iVectorInitSearch (&gxpl->listeners, prvHandlerKey, prvHandlerMatch) == 0) {
          return gxpl;
        }
      }
    }
  }
  vLog (LOG_ERR, "Unable to setup gxPL object");
  free (gxpl);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLClose (gxPL * gxpl) {
  int ret;

  ret = gxPLIoClose (gxpl->io);
  iVectorDestroy (&gxpl->listeners);
  if (gxpl->config->malloc) {

    free (gxpl->config);
  }
  free (gxpl);
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerAdd (gxPL * gxpl, gxPLMessageListener listener, void * udata) {
  hitem * h = malloc (sizeof (hitem));
  assert (h);

  h->func = listener;
  h->data = udata;

  return iVectorAppend (&gxpl->listeners, h);
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerRemove (gxPL * gxpl, gxPLMessageListener listener) {
  int i = iVectorFindFirstIndex (&gxpl->listeners, &listener);

  return iVectorRemove (&gxpl->listeners, i);
}

// -----------------------------------------------------------------------------
int
gxPLIoCtl (gxPL * gxpl, int c, ...) {
  int ret = 0;
  va_list ap;

  va_start (ap, c);
  switch (c) {

      // put here the requests should not be transmitted to the layer below.
      // case ...

    default:
      ret = gxPLIoIoCtl (gxpl->io, c, ap);
      if ( (ret == -1) && (errno == EINVAL)) {
        vLog (LOG_ERR, "gxPLIoCtl function not supported: %d", c);
      }
      break;
  }

  va_end (ap);
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLPoll (gxPL * gxpl, int timeout_ms) {
  int ret, size = 0;

  ret = gxPLIoCtl (gxpl, gxPLIoFuncPoll, &size, timeout_ms);

  if ( (ret == 0) && (size > 0)) {
    char * buffer = malloc (size + 1);
    assert (buffer);
    gxPLNetAddress source;

    ret = gxPLIoRecv (gxpl->io, buffer, size, &source);
    if (ret == size) {
      static gxPLMessage * msg;

      // We receive a message, append null character to terminate the string
      buffer[size] = '\0';
      vLog (LOG_DEBUG, "Just read %d bytes as packet:\n%s", size, buffer);

      // TODO: Send the raw message to any raw message listeners ?

      msg = gxPLMessageFromString (msg, buffer);
      if (msg) {

        if (gxPLMessageIsError (msg)) {
          vLog (LOG_INFO, "Error parsing network message - ignored");
        }
        else if (gxPLMessageIsValid (msg)) {

          // Dispatch the message
          vLog (LOG_DEBUG, "Now dispatching valid message");

          for (int i = 0; i < iVectorSize (&gxpl->listeners); i++) {

            hitem * h = pvVectorGet (&gxpl->listeners, i);
            if (h->func) {
              if (h->func (msg, &source, h->data) != 0) {
                vLog (LOG_INFO, "Handler message failed");
              }
            }
          }
        }

        if (gxPLMessageIsValid (msg) || gxPLMessageIsError (msg)) {
          // Release the message
          ret = gxPLMessageDelete (msg);
          msg = NULL;
        }
      }
      else {

        vLog (LOG_INFO, "Error parsing network message - ignored");
      }
    }
    free (buffer);
  }
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLSendMessage (gxPL * gxpl, gxPLMessage * message) {
  int ret = -1;
  int count;
  char * str = gxPLMessageToString (message);

  if (str) {

    count = strlen (str);
    ret = gxPLIoSend (gxpl->io, str, count, NULL);
    free (str);
  }
  return ret;
}


// -----------------------------------------------------------------------------
int
gxPLMessageIsHubEcho (const gxPL * gxpl, const gxPLMessage * msg,
                      const gxPLMessageId * my_id) {

  if (strcmp (gxPLMessageSchemaClassGet (msg), "hbeat") == 0) {
    if (strcmp (gxPLMessageSchemaTypeGet (msg), "app") == 0) {
      const char * remote_ip = gxPLMessagePairValueGet (msg, "remote-ip");

      if (remote_ip) {
        const char * str_port = gxPLMessagePairValueGet (msg, "port");

        if (str_port) {
          char *endptr;

          uint16_t port = strtol (str_port, &endptr, 10);

          if (*endptr == '\0') {
            if ( (port == gxpl->net_info.port) &&
                 (strcmp (gxPLLocalAddressString (gxpl), remote_ip) == 0)) {

              return true;
            }
          }
        }
      }
    }
    else if (strcmp (gxPLMessageSchemaClassGet (msg), "basic") == 0) {

      if (my_id) {
        if (gxPLMessageIdCmp (my_id, gxPLMessageSourceIdGet (msg)) == 0) {

          return true;
        }
      }
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
const char *
gxPLLocalAddressString (const gxPL * gxpl) {
  static char * str;


  if (gxPLIoCtl ((gxPL *)gxpl, gxPLIoFuncNetAddrToString, &gxpl->net_info, &str) == 0) {

    return str;
  }
  return "";
}

// -----------------------------------------------------------------------------
const char *
gxPLBroadcastAddressString (const gxPL * gxpl) {
  gxPLNetAddress addr;
  static char * str;

  if (gxPLIoCtl ((gxPL *)gxpl, gxPLIoFuncGetBcastAddr, &addr) == 0) {

    if (gxPLIoCtl ((gxPL *)gxpl, gxPLIoFuncNetAddrToString, &addr, &str) == 0) {

      return str;
    }
  }
  return "";
}

// -----------------------------------------------------------------------------
const gxPLNetAddress *
gxPLNetInfo (const gxPL * gxpl) {

  return &gxpl->net_info;
}

// -----------------------------------------------------------------------------
gxPLConnectType
gxPLGetConnectionType (const gxPL * gxpl) {

  return gxpl->config->connecttype;
}

// -----------------------------------------------------------------------------
const char *
gxPLGetInterfaceName (const gxPL * gxpl) {

  return gxpl->config->iface;
}

// -----------------------------------------------------------------------------
const char *
gxPLGetIoLayerName (const gxPL * gxpl) {

  return gxpl->config->iolayer;
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
