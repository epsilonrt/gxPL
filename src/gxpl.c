/**
 * @file
 * Top Layer of API (source code)
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gxPL.h>
#define GXPL_INTERNALS
#include <gxPL/io.h>
#include <gxPL/util.h>
#include <gxPL/device.h>
#include "gxpl_p.h"
#include "version-git.h"

/* structures =============================================================== */
typedef struct _listener_elmt {
  gxPLMessageListener func;
  void * data;
} listener_elmt;

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

  return gxPLDeviceId (d);
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

// -----------------------------------------------------------------------------
// Public
// Stop (disable) all services, usually in preparation for shutdown, but
// that isn't the only possible reason
int gxPLAppDisableAllDevice (gxPLApplication * app) {
  gxPLDevice * device;

  for (int i = 0; i < iVectorSize (&app->device); i++) {

    device = pvVectorGet (&app->device, i);
    if (gxPLDeviceEnable (device, false) != 0) {
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
prvHeartbeatPoll (gxPLApplication * app) {
  gxPLDevice * device;
  long now = gxPLTime();
  long elapsed;

  for (int i = 0; i < iVectorSize (&app->device); i++) {

    device = pvVectorGet (&app->device, i);

    if (gxPLDeviceIsEnabled (device)) {
      int interval = gxPLDeviceHeartbeatInterval (device);
      time_t last = gxPLDeviceHeartbeatLast (device);

      // See how much time has gone by
      elapsed = now - last;

      if (gxPLDeviceIsHubConfirmed (device)) {

        if (last >= 1) {

          if (gxPLDeviceIsConfigurale (device) &&
              !gxPLDeviceIsConfigured (device)) {

            // If we are in configuration mode, we send out once a minute
            if (elapsed < DEFAULT_CONFIG_HEARTBEAT_INTERVAL) {

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
        if (elapsed < DEFAULT_HUB_DISCOVERY_INTERVAL) {

          continue;
        }
      }

      gxPLDeviceHeartbeatSend (device, gxPLHeartbeatHello);
    }
  }
}

// -----------------------------------------------------------------------------
// Run the passed message by each device and see who is interested
static void
prvDeviceMessageDispatcher (gxPLApplication * app, gxPLMessage * message,
                            void * udata) {
  gxPLDevice * device;

  for (int i = 0; i < iVectorSize (&app->device); i++) {

    device = pvVectorGet (&app->device, i);
    gxPLDeviceMessageHandler (device, message, udata);
  }
}

// -----------------------------------------------------------------------------
static void
prvEncodeLong (unsigned long value, char * str, int size) {
  int i, len, str_len = strlen (str);
  static const char alphanum[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz";
  const int base = sizeof (alphanum) - 1;

  // Fill with zeros
  for (i = str_len; i < size; i++) {

    str[i] = '0';
  }
  str[size] = '\0';
  len = strlen (str);

  // Handle the simple case
  if (value == 0) {
    return;
  }

  for (i = len - 1; i >= (len - str_len); i--) {

    str[i] = alphanum[value % base];
    if (value < base) {

      break;
    }
    value = value / base;
  }
}

/* internal public functions ================================================ */


/* api functions ============================================================ */
// -----------------------------------------------------------------------------
gxPLSetting *
gxPLSettingNew (const char * iface, const char * iolayer, gxPLConnectType type) {
  gxPLSetting * setting = calloc (1, sizeof (gxPLSetting));
  assert (setting);

  if (iface) {
    strcpy (setting->iface, iface);
  }
  if (iolayer) {
    strcpy (setting->iolayer, iolayer);
  }
  setting->connecttype = type;
  setting->malloc = 1;

  return setting;
}

// -----------------------------------------------------------------------------
gxPLSetting *
gxPLSettingFromCommandArgs (int argc, char * argv[], gxPLConnectType type) {
  gxPLSetting * setting = calloc (1, sizeof (gxPLSetting));
  assert (setting);

  gxPLParseCommonArgs (setting, argc, argv);
  if (strlen (setting->iolayer) == 0) {

    strcpy (setting->iolayer, DEFAULT_IO_LAYER);
  }
  setting->connecttype = type;
  setting->malloc = 1;

  return setting;
}

// -----------------------------------------------------------------------------
gxPLApplication *
gxPLAppOpen (gxPLSetting * setting) {
  gxPLApplication * app = calloc (1, sizeof (gxPLApplication));
  assert (app);

  if (setting->debug) {

    vLogSetMask (LOG_UPTO (GXPL_LOG_DEBUG_LEVEL));
  }

  app->io = gxPLIoOpen (setting);

  if (app->io) {

    if (setting->malloc == 0) {

      app->setting = malloc (sizeof (gxPLSetting));
      memcpy (app->setting, setting, sizeof (gxPLSetting));
      app->setting->malloc = 1;
    }
    else {

      app->setting = setting;
    }

    if (iVectorInit (&app->msg_listener, 2, NULL, free) == 0) {

      if (iVectorInitSearch (&app->msg_listener, prvListenerKey,
                             prvListenerMatch) == 0) {

        if (iVectorInit (&app->device, 2, NULL, prvDeviceDelete) == 0) {

          if (iVectorInitSearch (&app->device, prvDeviceKey,
                                 prvDeviceMatch) == 0) {

            // everything was done, we copy the network information and returns.
            (void) gxPLIoCtl (app, gxPLIoFuncGetNetInfo, &app->net_info);
            if (gxPLMessageListenerAdd (app, prvDeviceMessageDispatcher, NULL) == 0) {

              srand (gxPLRandomSeed (app));
              return app;
            }
          }
        }
      }
    }
  }
  PERROR ("Unable to setting up gxPLApplication object");
  free (app);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLAppClose (gxPLApplication * app) {

  if (app) {
    int ret;

    // for each device, sends a goodbye heartbeat and removes all listeners,
    vVectorDestroy (&app->device);
    // then releases all message listeners
    vVectorDestroy (&app->msg_listener);
    // an close !
    ret = gxPLIoClose (app->io);
    if (app->setting->malloc) {

      free (app->setting);
    }
    free (app);
    return ret;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerAdd (gxPLApplication * app, gxPLMessageListener listener, void * udata) {
  listener_elmt * h = malloc (sizeof (listener_elmt));
  assert (h);

  h->func = listener;
  h->data = udata;

  if (iVectorAppend (&app->msg_listener, h) == 0) {
    return 0;
  }
  free (h);
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageListenerRemove (gxPLApplication * app, gxPLMessageListener listener) {
  int i = iVectorFindFirstIndex (&app->msg_listener, &listener);


  return iVectorRemove (&app->msg_listener, i);
}

// -----------------------------------------------------------------------------
int
gxPLAppPoll (gxPLApplication * app, int timeout_ms) {
  int ret, size = 0;

  ret = gxPLIoCtl (app, gxPLIoFuncPoll, &size, timeout_ms);

  if (ret == 0)  {

    if (size > 0) {
      char * buffer = malloc (size + 1);
      assert (buffer);

      ret = gxPLIoRecv (app->io, buffer, size, NULL);
      if (ret == size) {
        static gxPLMessage * msg;

        // We receive a message, append null character to terminate the string
        buffer[size] = '\0';
        PDEBUG ("Just read %d bytes, raw buffer below >>>\n%s<<<", size, buffer);

        // TODO: Send the raw message to any raw message msg_listener ?

        msg = gxPLMessageFromString (msg, buffer);
        if (msg) {

          if (gxPLMessageIsError (msg)) {
            PINFO ("Error parsing network message - ignored");
          }
          else if (gxPLMessageIsValid (msg)) {

            // Dispatch the message
            PDEBUG ("Now dispatching valid message");

            for (int i = 0; i < iVectorSize (&app->msg_listener); i++) {

              listener_elmt * h = pvVectorGet (&app->msg_listener, i);
              if (h->func) {

                h->func (app, msg, h->data);
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

          PINFO ("Error parsing network message - ignored");
        }
      }
      free (buffer);
    }
    else {

      prvHeartbeatPoll (app);
    }
  }
  else {

    PINFO ("Error Polling");
  }
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLAppSendMessage (gxPLApplication * app, const gxPLMessage * message,
                    const gxPLIoAddr * client) {
  int ret = -1;
  int count;
  char * str = gxPLMessageToString (message);

  if (str) {

    count = strlen (str);
    ret = gxPLIoSend (app->io, str, count, client);
    if (ret < 0) {
      PERROR ("Unable to send message: [%10s...]", str);
    }
    free (str);
  }

  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLAppBroadcastMessage (gxPLApplication * app, const gxPLMessage * message) {

  return gxPLAppSendMessage (app, message, NULL);
}


// -----------------------------------------------------------------------------
int
gxPLAppIsHubEchoMessage (const gxPLApplication * app, const gxPLMessage * msg,
                         const gxPLId * my_id) {

  if ( (strcmp (gxPLMessageSchemaClassGet (msg), "hbeat") == 0) ||
       (strcmp (gxPLMessageSchemaClassGet (msg), "config") == 0)) {

    if (strcmp (gxPLMessageSchemaTypeGet (msg), "app") == 0) {
      const char * remote_ip = gxPLMessagePairGet (msg, "remote-ip");

      if (remote_ip) {
        const char * str_port = gxPLMessagePairGet (msg, "port");

        if (str_port) {
          char *endptr;

          uint16_t port = strtol (str_port, &endptr, 10);

          if (*endptr == '\0') {
            if ( (port == app->net_info.port) &&
                 (strcmp (gxPLIoLocalAddrGet (app), remote_ip) == 0)) {

              return true;
            }
          }
        }
      }
    }
    else if (strcmp (gxPLMessageSchemaTypeGet (msg), "basic") == 0) {
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
gxPLAppAddDevice (gxPLApplication * app, const char * vendor_id,
                  const char * device_id, const char * instance_id) {

  gxPLDevice * device = gxPLDeviceNew (app, vendor_id, device_id, instance_id);
  if (device) {

    // verifies that the device does not exist
    if (pvVectorFindFirst (&app->device, gxPLDeviceId (device)) == NULL) {

      // if not, add it to the list
      if (iVectorAppend (&app->device, device) == 0) {

        return device;
      }
    }
  }
  // failure exit
  gxPLDeviceDelete (device);
  return NULL;
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLAppAddConfigurableDevice (gxPLApplication * app, const char * vendor_id,
                              const char * device_id, const char * filename) {
  gxPLDevice * device = gxPLDeviceConfigNew (app,
                        vendor_id, device_id, filename);
  if (device) {

    // verifies that the device does not exist
    if (pvVectorFindFirst (&app->device, gxPLDeviceId (device)) == NULL) {

      // if not, add it to the list
      if (iVectorAppend (&app->device, device) == 0) {

        return device;
      }
    }
  }
  // failure exit
  gxPLDeviceDelete (device);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLAppRemoveDevice (gxPLApplication * app, gxPLDevice * device) {

  int index = iVectorFindFirstIndex (&app->device, device);
  if (index >= 0) {
    return iVectorRemove (&app->device, index);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLAppDeviceCount (gxPLApplication * app) {

  return iVectorSize (&app->device);
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLAppDeviceAt (gxPLApplication * app, int index) {

  return (gxPLDevice *) pvVectorGet (&app->device, index);
}

// -----------------------------------------------------------------------------
int
gxPLAppDeviceIndex (gxPLApplication * app, const gxPLDevice * device) {

  return iVectorFindFirstIndex (&app->device, device);
}

// -----------------------------------------------------------------------------
int
gxPLIoCtl (gxPLApplication * app, int c, ...) {
  int ret = 0;
  va_list ap;

  va_start (ap, c);
  switch (c) {

      // put here the requests should not be transmitted to the layer below.
      // case ...

    default:
      ret = gxPLIoIoCtl (app->io, c, ap);
      if ( (ret == -1) && (errno == EINVAL)) {
        PERROR ("gxPLIoCtl function not supported: %d", c);
      }
      break;
  }

  va_end (ap);
  return ret;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoLocalAddrGet (const gxPLApplication * app) {
  static char * str;

  if (gxPLIoCtl ( (gxPLApplication *) app, gxPLIoFuncNetAddrToString, &app->net_info, &str) == 0) {

    return str;
  }
  return "";
}

// -----------------------------------------------------------------------------
const xVector *
gxPLIoLocalAddrList (const gxPLApplication * app) {
  const xVector * v;

  if (gxPLIoCtl ( (gxPLApplication *) app, gxPLIoFuncGetLocalAddrList, &v) == 0) {

    return v;
  }
  return NULL;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoBcastAddrGet (const gxPLApplication * app) {
  gxPLIoAddr addr;
  static char * str;

  if (gxPLIoCtl ( (gxPLApplication *) app, gxPLIoFuncGetBcastAddr, &addr) == 0) {

    if (gxPLIoCtl ( (gxPLApplication *) app, gxPLIoFuncNetAddrToString, &addr, &str) == 0) {

      return str;
    }
  }
  return "";
}

// -----------------------------------------------------------------------------
const gxPLIoAddr *
gxPLIoInfoGet (const gxPLApplication * app) {

  return &app->net_info;
}

// -----------------------------------------------------------------------------
gxPLConnectType
gxPLAppConnectionType (const gxPLApplication * app) {

  return app->setting->connecttype;
}

// -----------------------------------------------------------------------------
gxPLSetting * 
gxPLAppSetting (gxPLApplication * app) {
  
  return app->setting;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoInterfaceGet (const gxPLApplication * app) {

  return app->setting->iface;
}

// -----------------------------------------------------------------------------
const char *
gxPLIoLayerGet (const gxPLApplication * app) {

  return app->setting->iolayer;
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
unsigned long
gxPLVersionSha1 (void) {

  return VERSION_SHA1;
}

// -----------------------------------------------------------------------------
int
gxPLGenerateUniqueId (const gxPLApplication * app, char * s, int size) {
  int max, len = 0;

  if (app->net_info.addrlen > 0) {

    for (int i = 0; (i < app->net_info.addrlen) && (len < size); i++) {

      max = size - len + 1;
      len += snprintf (&s[len], max, "%02x", app->net_info.addr[i]);
    }
    if (len > size) {

      len = size;
    }
  }
  if (len < size) {
    unsigned long ms;

    if (gxPLTimeMs (&ms) == 0) {

      prvEncodeLong (ms, s, size);
      gxPLTimeDelayMs (1);
    }
  }

  return strlen (s);
}

/* ========================================================================== */
