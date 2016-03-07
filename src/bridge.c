/**
 * @file
 * xPL bridge on a system using ethernet networking
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef  __AVR__
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <gxPL.h>
#include "bridge_p.h"

/* constants ================================================================ */
#define BROADCAST_KEY "broadcast"
#define ALLOW_KEY     "allow"

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static const void *
prvAllowKey (const void * allow) {

  return allow;
}

// -----------------------------------------------------------------------------
static int
prvAllowMatch (const void *key1, const void *key2) {
  const gxPLSchema * s1 = (const gxPLSchema *) key1;
  const gxPLSchema * s2 = (const gxPLSchema *) key2;

  return gxPLSchemaCmp (s1, s2);
}

// -----------------------------------------------------------------------------
static const void *
prvClientKey (const void * client) {

  return client;
}

// -----------------------------------------------------------------------------
static int
prvClientMatch (const void *key1, const void *key2) {
  const gxPLBridgeClient * c1 = (const gxPLBridgeClient *) key1;
  const gxPLBridgeClient * c2 = (const gxPLBridgeClient *) key2;
  int ret;

  ret = gxPLIdCmp (&c1->id, &c2->id);
  if ( (ret == 0) && (c1->addr.family != gxPLNetFamilyUnknown) &&
       (c1->addr.family == c2->addr.family)) {

    ret = memcmp (c1->addr.addr, c2->addr.addr, c1->addr.addrlen);
  }
  return ret;
}

// -----------------------------------------------------------------------------
// Receive xPL messages from inside
static void
prvHandleInnerMessage (gxPLApplication * app, gxPLMessage * message, void * udata) {
  gxPLBridge * bridge = (gxPLBridge *) udata;
  gxPLBridgeClient * client = NULL;

  if ( (strcmp (gxPLMessageSchemaClassGet (message), "hbeat") == 0) ||
       (strcmp (gxPLMessageSchemaClassGet (message), "config") == 0)) {
    char * endptr;
    long now;
    bool new_client = false;

    gxPLBridgeClient * src = calloc (1, sizeof (gxPLBridgeClient));
    assert (src);

    now = gxPLTime();
    gxPLIdCopy (&src->id, gxPLMessageSourceIdGet (message));

    if (strcmp (gxPLMessageSchemaTypeGet (message), "basic") == 0) {
      int interval;
      const char * str_interval;

      /* When the bridge receives a hbeat with remote-addr field (extension of
       * hbeat.basic schema) the bridge should extract the "remote-addr" value
       * from the message body
       */
      const char * str_addr = gxPLMessagePairGet (message, "remote-addr");

      if (str_addr) {

        // remote-ip matches with local address, converts to gxPLIoAddr
        if (gxPLIoCtl (bridge->in, gxPLIoFuncNetAddrFromString, &src->addr, str_addr) != 0) {

          PERROR ("unable to convert %s to remote address", str_addr);
          return;
        }
      }

      // Gets heartbeat interval for update
      str_interval = gxPLMessagePairGet (message, "interval");
      interval =  strtol (str_interval, &endptr, 10);
      if (endptr == NULL) {

        PERROR ("unable to convert %s to heartbeat interval", str_interval);
        return;
      }

      client = pvVectorFindFirst (&bridge->clients, src);
      if (client == NULL) {

        // New client
        client = src;
        new_client = true;

        // then adds to the list
        if (iVectorAppend (&bridge->clients, client) != 0) {

          PERROR ("unable to append client");
          free (client);
          return;
        }
        PINFO ("New client %s.%s.%s, processing %d clients",
               src->id.vendor, src->id.device, src->id.instance,
               iVectorSize (&bridge->clients));
      }

      client->hbeat_period_max = interval * 60 * 2 + 60;
      client->hbeat_last = now;
    }
    else if (strcmp (gxPLMessageSchemaTypeGet (message), "end") == 0) {
      int c = iVectorFindFirstIndex (&bridge->clients, src);

      if (c >= 0) {

        iVectorRemove (&bridge->clients, c);
        PINFO ("Delete client %s.%s.%s after receiving his"
               " heartbeat end, processing %d clients",
               src->id.vendor, src->id.device, src->id.instance,
               iVectorSize (&bridge->clients));
      }
    }

    if (!new_client) {

      free (src);
    }
  }

  if (gxPLAppSetting (bridge->in)->broadcast) {

    // Deliver this message to all inside clients
    PINFO ("IN  --- IN  > Broadcast");
    gxPLAppSendMessage (bridge->in, message, NULL);
  }
  else if (client) {

    if (client->addr.family != gxPLNetFamilyUnknown) {

      // if inside broadcast is disabled, echoes hbeat.basic message to the client only
      PINFO ("IN  --- IN  > Deliver");
      gxPLAppSendMessage (bridge->in, message, &client->addr);
    }
    else {

      PERROR ("IN  --- IN  > Broadcast disable, unable to find the address of the client to respond.");
    }
  }

  if (gxPLMessageHopGet (message) <= bridge->max_hop) {

    // Broadcasts this message outside
    gxPLMessageHopInc (message);
    PINFO ("OUT <-- IN  > Deliver");
    gxPLAppSendMessage (bridge->out, message, NULL);
  }
}

// -----------------------------------------------------------------------------
// Receive xPL messages from outside
static void
prvHandleOuterMessage (gxPLApplication * app, gxPLMessage * message, void * udata) {
  gxPLBridge * bridge = (gxPLBridge *) udata;

  if (gxPLMessageHopGet (message) <= bridge->max_hop) {

    // the message should go on another network, increment the number of hops
    gxPLMessageHopInc (message);

    if (gxPLAppSetting (bridge->in)->broadcast) {

      // Deliver this message to all inside clients
      PINFO ("OUT --> IN  > Broadcast");
      gxPLAppSendMessage (bridge->in, message, NULL);
    }
    else {
      /*
       * Broadcast disabled, deliver this message if:
       * - target is inside
       * - schema message is allowed to cross the bridge
       */
      const gxPLSchema * s = gxPLMessageSchemaGet (message);

      int allow = iVectorFindFirstIndex (&bridge->allow, s);
      if (allow >= 0) {

        PINFO ("OUT --> IN  > %s.%s allowed to cross", s->class, s->type);
      }

      for (int i = 0; i < iVectorSize (&bridge->clients); i++) {

        gxPLBridgeClient * client = pvVectorGet (&bridge->clients, i);
        if ( (gxPLIdCmp (&client->id, gxPLMessageTargetIdGet (message)) == 0) ||
             (allow >= 0)) {

          PINFO ("OUT --> IN  > Deliver");
          gxPLAppSendMessage (bridge->in, message, &client->addr);
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
static void
prvSetConfig (gxPLBridge * bridge) {
  const char * str;

  PDEBUG ("Starting bridge configuration");
  if ( (str = gxPLDeviceConfigValueGet (bridge->device, BROADCAST_KEY)) != NULL) {

    gxPLAppSetting (bridge->in)->broadcast = strcasecmp (str, "true") == 0;
  }

  iVectorClear (&bridge->allow);
  for (int i = 0; i < gxPLDeviceConfigValueCount (bridge->device, ALLOW_KEY); i++) {
    gxPLSchema * schema = malloc (sizeof (gxPLSchema));
    assert (schema);

    if (gxPLSchemaFromString (schema, gxPLDeviceConfigValueGetAt (bridge->device, ALLOW_KEY, i)) == 0) {

      iVectorAppend (&bridge->allow, schema);
    }
    else {

      free (schema);
    }
  }
}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration */
static void
prvConfigChanged (gxPLDevice * device, void * udata) {

  // Read setting items for device and install */
  prvSetConfig (udata);
}

/* public api functions ===================================================== */

// -----------------------------------------------------------------------------
gxPLBridge *
gxPLBridgeOpen (gxPLSetting * insetting, gxPLSetting * outsetting, uint8_t max_hop) {

  if ( (strcmp (insetting->iolayer, "udp") == 0) &&
       (strlen (insetting->iolayer) == 0)) {

    PERROR ("iolayer for inside must be provided and different of udp");
  }
  else {
    gxPLBridge * bridge = calloc (1, sizeof (gxPLBridge));
    assert (bridge);

    // ignore connection type for inner
    insetting->connecttype = gxPLConnectStandAlone;

    // ignore iolayer and connection type for outer
    strcpy (outsetting->iolayer, "udp");
    outsetting->connecttype = gxPLConnectViaHub;

    bridge->in  = gxPLAppOpen (insetting);
    bridge->out = gxPLAppOpen (outsetting);

    if ( (bridge->in) && (bridge->out)) {

      iVectorInit (&bridge->clients, 1, NULL, free);
      iVectorInitSearch (&bridge->clients, prvClientKey, prvClientMatch);
      iVectorInit (&bridge->allow, 1, NULL, free);
      iVectorInitSearch (&bridge->allow, prvAllowKey, prvAllowMatch);
      gxPLMessageListenerAdd (bridge->in, prvHandleInnerMessage, bridge);
      gxPLMessageListenerAdd (bridge->out, prvHandleOuterMessage, bridge);

      if (max_hop == 0) {

        max_hop = 1;
      }
      else if (max_hop > 9) {

        max_hop = 9;
      }
      bridge->max_hop = max_hop;
      return bridge;
    }
    free (bridge);
  }
  PERROR ("unable to open bridge");
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLBridgeClose (gxPLBridge * bridge) {

  if (bridge) {
    int ret;

    ret = gxPLAppClose (bridge->in);
    if (ret != 0) {
      PNOTICE ("Unable to close inner application");
    }
    ret = gxPLAppClose (bridge->out);
    if (ret != 0) {
      PNOTICE ("Unable to close outer application");
    }

    vVectorDestroy (&bridge->clients);
    free (bridge);
    return ret;
  }
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLBridgeSetNewInSetting (gxPLBridge * bridge, gxPLSetting * insetting) {

  if ( (strcmp (insetting->iolayer, "udp") == 0) &&
       (strlen (insetting->iolayer) == 0)) {

    PERROR ("iolayer for inside must be provided and different of udp");
    return -1;
  }

  if (memcmp (insetting, gxPLAppSetting (bridge->in), sizeof (gxPLSetting)) != 0) {
    if (gxPLAppClose (bridge->in) != 0) {

      PERROR ("Unable close inside application");
      return -1;
    }
    bridge->in  = gxPLAppOpen (insetting);
    if (bridge->in == NULL) {

      PERROR ("Unable open inside application, this bridge was closed in emergency");
      gxPLBridgeClose (bridge);
      return -1;
    }
    gxPLMessageListenerAdd (bridge->in, prvHandleInnerMessage, bridge);
  }
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLBridgePoll (gxPLBridge * bridge, int timeout_ms) {
  int ret = 0, i;

  timeout_ms /= 2;

  if (timeout_ms < 1) {
    timeout_ms = 1;
  }

  i = gxPLAppPoll (bridge->in, timeout_ms);
  if (i != 0) {

    ret = i;
    PNOTICE ("Unable to poll inner application");
  }

  i = gxPLAppPoll (bridge->out, timeout_ms);
  if (i != 0) {

    ret = i;
    PNOTICE ("Unable to poll outer application");
  }
  bridge->timeout += timeout_ms;

  if (bridge->timeout >= 60000) { // every minute
    long now;

    // Track known inner xPL clients and determine when they have died and
    // remove them from the list of local clients
    bridge->timeout = 0;
    now = gxPLTime();
    for (int i = 0; i < iVectorSize (&bridge->clients); i++) {
      gxPLBridgeClient * client = pvVectorGet (&bridge->clients, i);

      if ( (now - client->hbeat_last) > client->hbeat_period_max) {

        PINFO ("Delete client %s.%s.%s after heartbeat timeout, "
               "processing %d clients",
               client->id.vendor, client->id.device, client->id.instance,
               iVectorSize (&bridge->clients) - 1);

        iVectorRemove (&bridge->clients, i);
      }
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
gxPLApplication *
gxPLBridgeInApp (gxPLBridge * bridge) {

  return bridge->in;
}

// -----------------------------------------------------------------------------
gxPLApplication *
gxPLBridgeOutApp (gxPLBridge * bridge) {

  return bridge->out;
}

// -----------------------------------------------------------------------------
int
gxPLBridgeDeviceSet (gxPLBridge * bridge,
                     const char * vendor_id, const char * device_id,
                     const char * filename, const char * version) {

  // Create a configurable device and set our application version
  bridge->device = gxPLAppAddConfigurableDevice (bridge->out,
                   vendor_id, device_id, filename);

  if (bridge->device) {

    if (version) {

      gxPLDeviceVersionSet (bridge->device, version);
    }

    if (gxPLDeviceIsConfigured (bridge->device) == false) {

      // Define the configurable items and give it a default
      gxPLDeviceConfigItemAdd (bridge->device, BROADCAST_KEY, gxPLConfigReconf, 1);
      gxPLDeviceConfigValueSet (bridge->device, BROADCAST_KEY, "false");

      gxPLDeviceConfigItemAdd (bridge->device, ALLOW_KEY, gxPLConfigReconf, 16);
      gxPLDeviceConfigValueSet (bridge->device, ALLOW_KEY, "hbeat.request");
    }

    // Parse the device configurables into a form this program
    // can use (whether we read a config or not)
    prvSetConfig (bridge);

    // Add a device change listener we'll use to pick up changes
    return gxPLDeviceConfigListenerAdd (bridge->device, prvConfigChanged, bridge);
  }
  return -1;
}

// -----------------------------------------------------------------------------
gxPLDevice *
gxPLBridgeDevice (gxPLBridge * bridge) {

  return bridge->device;
}

// -----------------------------------------------------------------------------
int
gxPLBridgeDeviceEnable (gxPLBridge * bridge, bool enable) {

  return gxPLDeviceEnable (bridge->device, enable);
}

// -----------------------------------------------------------------------------
int
gxPLBridgeDeviceIsEnabled (const gxPLBridge * bridge) {

  return gxPLDeviceIsEnabled (bridge->device);
}

/* ========================================================================== */
#endif /*  __AVR__ not defined */
