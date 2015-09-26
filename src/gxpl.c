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
#include <gxPL/util.h>
#include <gxPL/device.h>
#include "config.h"
#include "gxpl_p.h"
#include "version-git.h"

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
/* structures =============================================================== */
typedef struct _listener_elmt {
  gxPLMessageListener func;
  void * data;
} listener_elmt;

/* types ==================================================================== */
/* private variables ======================================================== */

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static const void *
prvListenerKey (const void * elmt) {
  listener_elmt * p = (listener_elmt *) elmt;

  return & (p->func);
}

// -----------------------------------------------------------------------------
static int
prvListenerMatch (const void *key1, const void *key2) {

  return * ( (gxPLMessageListener *) key1) == * ( (gxPLMessageListener *) key2);
}

// -----------------------------------------------------------------------------
static const void *
prvDeviceKey (const void * elmt) {
  gxPLDevice * d = (gxPLDevice *) elmt;

  return gxPLDeviceIdGet (d);
}

// -----------------------------------------------------------------------------
static int
prvDeviceMatch (const void *key1, const void *key2) {

  return gxPLIdCmp ( (gxPLId *) key1, (gxPLId *) key2);
}

// -----------------------------------------------------------------------------
static void
prvDeviceDelete (void * d) {

  gxPLDeviceDelete ( (gxPLDevice *) d);
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

// -----------------------------------------------------------------------------
// Public
// Stop (disable) all services, usually in preparation for shutdown, but
// that isn't the only possible reason
int gxPLDeviceDisableAll (gxPL * gxpl) {
  gxPLDevice * device;

  for (int i = 0; i < iVectorSize (&gxpl->device); i++) {

    device = pvVectorGet (&gxpl->device, i);
    if (gxPLDeviceEnabledSet (device, false) != 0) {
      return -1;
    }
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * Private
 * Check each known device for when it last sent a heart
 * beat and if it's time to send another, do it. */
static void
prvHeartbeatPoll (gxPL * gxpl) {
  gxPLDevice * device;
  time_t now = time (NULL);
  time_t elapsed;

  for (int i = 0; i < iVectorSize (&gxpl->device); i++) {

    device = pvVectorGet (&gxpl->device, i);

    if (gxPLDeviceEnabledGet (device)) {
      int interval = gxPLDeviceHeartbeatIntervalGet (device);
      time_t last = gxPLDeviceHeartbeatLastGet (device);

      // See how much time has gone by
      elapsed = now - last;

      if (gxPLDeviceHubConfirmedGet (device)) {

        if (last >= 1) {

          if (gxPLDeviceConfiguraleGet (device) &&
              !gxPLDeviceConfiguredGet (device)) {

            // If we are in configuration mode, we send out once a minute
            if (elapsed < CONFIG_HEARTBEAT_INTERVAL) {

              continue;
            }
          }

          // For normal heartbeats, once each "hbeat_interval"
          if (elapsed < interval) {

            continue;
          }
        }
      }
      else {

        // If we are still waiting to hear from the hub,
        // then send a message every 3 seconds until we do
        if (elapsed < CONFIG_HUB_DISCOVERY_INTERVAL) {

          continue;
        }
      }

      gxPLDeviceHeartbeatSend (device, gxPLHeartbeatHello);
    }
  }
}

// -----------------------------------------------------------------------------
// Run the passed message by each device and see who is interested
void prvDeviceMessageDispatcher (gxPL * gxpl, const gxPLMessage * message,
                                 void * udata) {
  gxPLDevice * device;

  for (int i = 0; i < iVectorSize (&gxpl->device); i++) {

    device = pvVectorGet (&gxpl->device, i);
    gxPLDeviceMessageHandler (device, message, udata);
  }
}


/* internal public functions ================================================ */


/* api functions ============================================================ */
// -----------------------------------------------------------------------------
gxPLConfig *
gxPLConfigNew (const char * iface, const char * iolayer, gxPLConnectType type) {
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
gxPLConfigNewFromCommandArgs (int argc, char * argv[], gxPLConnectType type) {
  gxPLConfig * config = calloc (1, sizeof (gxPLConfig));
  assert (config);

  prvParseCommonArgs (config, argc, argv);
  if (strlen (config->iolayer) == 0) {

    strcpy (config->iolayer, DEFAULT_IO_LAYER);
  }
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

      gxpl->config = malloc (sizeof (gxPLConfig));
      memcpy (gxpl->config, config, sizeof (gxPLConfig));
      gxpl->config->malloc = 1;
    }
    else {

      gxpl->config = config;
    }

    if (iVectorInit (&gxpl->msg_listener, 2, NULL, free) == 0) {

      if (iVectorInitSearch (&gxpl->msg_listener, prvListenerKey,
                             prvListenerMatch) == 0) {

        if (iVectorInit (&gxpl->device, 2, NULL, prvDeviceDelete) == 0) {

          if (iVectorInitSearch (&gxpl->device, prvDeviceKey,
                                 prvDeviceMatch) == 0) {

            // everything was done, we copy the network information and returns.
            (void) gxPLIoCtl (gxpl, gxPLIoFuncGetLocalAddr, &gxpl->net_info);
            if (gxPLMessageListenerAdd (gxpl, prvDeviceMessageDispatcher, NULL) == 0) {

              return gxpl;
            }
          }
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

  if (gxpl) {
    int ret;
    
    // for each device, sends a goodbye heartbeat and removes all listeners,
    vVectorDestroy (&gxpl->device);
    // then releases all message listeners
    vVectorDestroy (&gxpl->msg_listener);
    // an close !
    ret = gxPLIoClose (gxpl->io);
    if (gxpl->config->malloc) {

      free (gxpl->config);
    }
    free (gxpl);
    return ret;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerAdd (gxPL * gxpl, gxPLMessageListener listener, void * udata) {
  listener_elmt * h = malloc (sizeof (listener_elmt));
  assert (h);

  h->func = listener;
  h->data = udata;

  if (iVectorAppend (&gxpl->msg_listener, h) == 0) {
    return 0;
  }
  free(h);
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerRemove (gxPL * gxpl, gxPLMessageListener listener) {
  int i = iVectorFindFirstIndex (&gxpl->msg_listener, &listener);
  

  return iVectorRemove (&gxpl->msg_listener, i);
}

// -----------------------------------------------------------------------------
int
gxPLPoll (gxPL * gxpl, int timeout_ms) {
  int ret, size = 0;

  ret = gxPLIoCtl (gxpl, gxPLIoFuncPoll, &size, timeout_ms);

  if (ret == 0)  {

    if (size > 0) {
      char * buffer = malloc (size + 1);
      assert (buffer);

      ret = gxPLIoRecv (gxpl->io, buffer, size, NULL);
      if (ret == size) {
        static gxPLMessage * msg;

        // We receive a message, append null character to terminate the string
        buffer[size] = '\0';
        vLog (LOG_DEBUG, "Just read %d bytes as packet:\n%s", size, buffer);

        // TODO: Send the raw message to any raw message msg_listener ?

        msg = gxPLMessageFromString (msg, buffer);
        if (msg) {

          if (gxPLMessageIsError (msg)) {
            vLog (LOG_INFO, "Error parsing network message - ignored");
          }
          else if (gxPLMessageIsValid (msg)) {

            // Dispatch the message
            vLog (LOG_DEBUG, "Now dispatching valid message");

            for (int i = 0; i < iVectorSize (&gxpl->msg_listener); i++) {

              listener_elmt * h = pvVectorGet (&gxpl->msg_listener, i);
              if (h->func) {

                h->func (gxpl, msg, h->data);
              }
            }
          }

          if (gxPLMessageIsValid (msg) || gxPLMessageIsError (msg)) {

            // Release the message
            gxPLMessageDelete (msg);
            msg = NULL;
            ret = 0;
          }
        }
        else {

          vLog (LOG_INFO, "Error parsing network message - ignored");
        }
      }
      free (buffer);
    }
    else {

      prvHeartbeatPoll (gxpl);
    }
  }
  else {

    vLog (LOG_INFO, "Error Polling");
  }
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLMessageSend (gxPL * gxpl, gxPLMessage * message) {
  int ret = -1;
  int count;
  char * str = gxPLMessageToString (message);

  if (str) {

    count = strlen (str);
    ret = gxPLIoSend (gxpl->io, str, count, NULL);
    if (ret < 0) {
      vLog (LOG_ERR, "Unable to send message: [%10s...]", str);
    }
    free (str);
  }

  return ret;
}


// -----------------------------------------------------------------------------
int
gxPLMessageIsHubEcho (const gxPL * gxpl, const gxPLMessage * msg,
                      const gxPLId * my_id) {

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
                 (strcmp (gxPLIoLocalAddrGet (gxpl), remote_ip) == 0)) {

              return true;
            }
          }
        }
      }
    }
    else if (strcmp (gxPLMessageSchemaClassGet (msg), "basic") == 0) {

      if (my_id) {
        if (gxPLIdCmp (my_id, gxPLMessageSourceIdGet (msg)) == 0) {

          return true;
        }
      }
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLDeviceAdd (gxPL * gxpl, const char * vendor_id,
               const char * device_id, const char * instance_id) {

  gxPLDevice * device = gxPLDeviceNew (gxpl, vendor_id, device_id, instance_id);
  if (device) {

    // verifies that the device does not exist
    if (pvVectorFindFirst (&gxpl->device, gxPLDeviceIdGet (device)) == NULL) {

      // if not, add it to the list
      if (iVectorAppend (&gxpl->device, device) == 0) {

        return device;
      }
    }
  }
  // failure exit
  (void) gxPLDeviceDelete (device);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceRemove (gxPL * gxpl, gxPLDevice * device) {

  int index = iVectorFindFirstIndex (&gxpl->device, device);
  if (index >= 0) {
    return iVectorRemove (&gxpl->device, index);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceCount (gxPL * gxpl) {

  return iVectorSize (&gxpl->device);
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLDeviceAt (gxPL * gxpl, int index) {

  return (gxPLDevice *) pvVectorGet (&gxpl->device, index);
}

// -----------------------------------------------------------------------------
int 
gxPLDeviceIndex (gxPL * gxpl, const gxPLDevice * device) {
  return iVectorFindFirstIndex(&gxpl->device, device);
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
const char *
gxPLIoLocalAddrGet (const gxPL * gxpl) {
  static char * str;


  if (gxPLIoCtl ( (gxPL *) gxpl, gxPLIoFuncNetAddrToString, &gxpl->net_info, &str) == 0) {

    return str;
  }
  return "";
}

// -----------------------------------------------------------------------------
const char *
gxPLIoBcastAddrGet (const gxPL * gxpl) {
  gxPLIoAddr addr;
  static char * str;

  if (gxPLIoCtl ( (gxPL *) gxpl, gxPLIoFuncGetBcastAddr, &addr) == 0) {

    if (gxPLIoCtl ( (gxPL *) gxpl, gxPLIoFuncNetAddrToString, &addr, &str) == 0) {

      return str;
    }
  }
  return "";
}

// -----------------------------------------------------------------------------
const gxPLIoAddr *
gxPLIoInfoGet (const gxPL * gxpl) {

  return &gxpl->net_info;
}

// -----------------------------------------------------------------------------
gxPLConnectType
gxPLConnectionTypeGet (const gxPL * gxpl) {

  return gxpl->config->connecttype;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoInterfaceGet (const gxPL * gxpl) {

  return gxpl->config->iface;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoLayerGet (const gxPL * gxpl) {

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
