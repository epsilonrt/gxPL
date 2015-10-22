/**
 * @file
 * xPL bridge on a system using ethernet networking
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <gxPL.h>
#include "bridge_p.h"

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static const void *
prvClientKey (const void * elmt) {
  gxPLBridgeClient * client = (gxPLBridgeClient *) elmt;

  return &client->addr;
}

// -----------------------------------------------------------------------------
static int
prvClientMatch (const void *key1, const void *key2) {
  const gxPLIoAddr * a1 = (const gxPLIoAddr *) key1;
  const gxPLIoAddr * a2 = (const gxPLIoAddr *) key2;

  return memcmp (a1->addr, a2->addr, a1->addrlen);
}

// -----------------------------------------------------------------------------
// Receive xPL messages from inner side
static void
prvHandleInnerMessage (gxPLApplication * app, gxPLMessage * message, void * udata) {
  gxPLBridge * bridge = (gxPLBridge *) udata;
  gxPLBridgeClient * client = NULL;

  if ( (strcmp (gxPLMessageSchemaClassGet (message), "hbeat") == 0) ||
       (strcmp (gxPLMessageSchemaClassGet (message), "config") == 0)) {

    // When the bridge receives a hbeat.app or config.app message
    // the bridge should extract the "remote-addr" value from the message body
    const char * str_addr = gxPLMessagePairGet (message, "remote-addr");

    if (str_addr) {
      gxPLIoAddr clinfo;
      char * endptr;
      long now;

      now = gxPLTime();

      // remote-ip matches with local address, converts to gxPLIoAddr
      if (gxPLIoCtl (bridge->in, gxPLIoFuncNetAddrFromString, &clinfo, str_addr) != 0) {

        vLog (LOG_ERR, "unable to convert %s to remote address", str_addr);
        return;
      }

      if (strcmp (gxPLMessageSchemaTypeGet (message), "basic") == 0) {
        int interval;
        const char * str_interval;

        // Gets heartbeat interval for update
        str_interval = gxPLMessagePairGet (message, "interval");
        interval =  strtol (str_interval, &endptr, 10);
        if (endptr == NULL) {

          vLog (LOG_ERR, "unable to convert %s to heartbeat interval", str_interval);
          return;
        }

        client = pvVectorFindFirst (&bridge->clients, &clinfo);
        if (client == NULL) {

          // New client
          client = calloc (1, sizeof (gxPLBridgeClient));
          assert (client);

          // Copies address for this client
          memcpy (&client->addr, &clinfo, sizeof (clinfo));
          gxPLIdCopy (&client->id, gxPLMessageSourceIdGet (message));

          // then adds to the list
          if (iVectorAppend (&bridge->clients, client) != 0) {

            vLog (LOG_ERR, "unable to append client");
            free (client);
            return;
          }
          vLog (LOG_INFO, "New client %s, processing %d clients",
                str_addr, iVectorSize (&bridge->clients));
        }

        client->hbeat_period_max = interval * 60 * 2 + 60;
        client->hbeat_last = now;
      }
      else if (strcmp (gxPLMessageSchemaTypeGet (message), "end") == 0) {
        int c = iVectorFindFirstIndex (&bridge->clients, &clinfo);

        if (c >= 0) {

          iVectorRemove (&bridge->clients, c);
          vLog (LOG_INFO, "Delete client %s after receiving his"
                " heartbeat end , processing %d clients",
                str_addr, iVectorSize (&bridge->clients));
        }
      }
    }
  }

  if (gxPLAppSetting (bridge->in)->broadcast) {

    // Deliver this message to all clients on inner side
    PDEBUG ("Broadcasts message from inner to inner");
    for (int i = 0; i < iVectorSize (&bridge->clients); i++) {

      client = pvVectorGet (&bridge->clients, i);
      gxPLAppSendMessage (bridge->in, message, &client->addr);
    }
  }
  else if (client) {

    // if inner side broadcast is disabled, echoes hbeat.basic message to the client only
    PDEBUG ("Echoes message to the client");
    gxPLAppSendMessage (bridge->in, message, &client->addr);
  }

  if (gxPLMessageHopGet (message) <= bridge->max_hop) {

    // Broadcasts this message to the outer side
    gxPLMessageHopInc (message);
    PDEBUG ("[OUT<--IN] Deliver message from inner to outer");
    gxPLAppSendMessage (bridge->out, message, NULL);
  }
}

// -----------------------------------------------------------------------------
// Receive xPL messages from outer side
static void
prvHandleOuterMessage (gxPLApplication * app, gxPLMessage * message, void * udata) {
  gxPLBridge * bridge = (gxPLBridge *) udata;

  if (gxPLMessageHopGet (message) <= bridge->max_hop) {

    // Broadcasts this message to the inner side
    gxPLMessageHopInc (message);

    // Deliver this message if outer side broadcast is enabled or if the target is on inner side
    for (int i = 0; i < iVectorSize (&bridge->clients); i++) {

      gxPLBridgeClient * client = pvVectorGet (&bridge->clients, i);
      if ( (gxPLAppSetting (bridge->out)->broadcast) ||
           (gxPLIdCmp (&client->id, gxPLMessageTargetIdGet (message)) == 0)) {

        PDEBUG ("[OUT-->IN] Deliver message from outer to inner");
        gxPLAppSendMessage (bridge->in, message, &client->addr);
      }
    }
  }
}

/* public api functions ===================================================== */

// -----------------------------------------------------------------------------
gxPLBridge *
gxPLBridgeOpen (gxPLSetting * insetting, gxPLSetting * outsetting, uint8_t max_hop) {

  if ( (strcmp (insetting->iolayer, "udp") == 0) &&
       (strlen (insetting->iolayer) == 0)) {

    vLog (LOG_ERR, "iolayer of inner side  must be provided and different of udp");
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

      if (iVectorInit (&bridge->clients, 1, NULL, free) == 0) {

        if (iVectorInitSearch (&bridge->clients, prvClientKey, prvClientMatch) == 0) {

          if (gxPLMessageListenerAdd (bridge->in, prvHandleInnerMessage, bridge) == 0) {

            if (gxPLMessageListenerAdd (bridge->out, prvHandleOuterMessage, bridge) == 0) {

              if (max_hop == 0) {

                max_hop = 1;
              }
              else if (max_hop > 9) {

                max_hop = 9;
              }
              bridge->max_hop = max_hop;
              return bridge;
            }
          }
        }
      }
    }
    free (bridge);
  }
  vLog (LOG_ERR, "unable to open bridge");
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLBridgeClose (gxPLBridge * bridge) {

  if (bridge) {
    int ret;

    ret = gxPLAppClose (bridge->in);
    if (ret != 0) {
      vLog (LOG_NOTICE, "Unable to close inner application");
    }
    ret = gxPLAppClose (bridge->out);
    if (ret != 0) {
      vLog (LOG_NOTICE, "Unable to close outer application");
    }

    vVectorDestroy (&bridge->clients);
    free (bridge);
    return ret;
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
    vLog (LOG_NOTICE, "Unable to poll inner application");
  }
  
  i = gxPLAppPoll (bridge->out, timeout_ms);
  if (i != 0) {
    
    ret = i;
    vLog (LOG_NOTICE, "Unable to poll outer application");
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
        char * str;
        if (gxPLIoCtl (bridge->in, gxPLIoFuncNetAddrToString, &client->addr, &str) == 0) {

          vLog (LOG_INFO, "Delete client %s:%d after heartbeat timeout, "
                "processing %d clients",
                str, client->addr.port,
                iVectorSize (&bridge->clients) - 1);
        }
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
/* ========================================================================== */
