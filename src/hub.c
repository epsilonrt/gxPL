/**
 * @file
 * xPL hub on a system using ethernet networking
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
#include "hub_p.h"

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static const void *
prvClientKey (const void * elmt) {
  gxPLHubClient * client = (gxPLHubClient *) elmt;

  return &client->addr;
}

// -----------------------------------------------------------------------------
static int
prvClientMatch (const void *key1, const void *key2) {
  int ret;
  const gxPLIoAddr * a1 = (const gxPLIoAddr *) key1;
  const gxPLIoAddr * a2 = (const gxPLIoAddr *) key2;

  if ( (ret = memcmp (a1->addr, a2->addr, a1->addrlen)) != 0) {
    return ret;
  }

  if (a1->port > a2->port) {
    return 1;
  }
  else if (a1->port < a2->port) {
    return -1;
  }
  return 0;
}

// --------------------------------------------------------------------------
// Receive xPL network messages
static void
prvHandleMessage (gxPLApplication * app, gxPLMessage * message, void * udata) {
  gxPLHub * hub = (gxPLHub *) udata;

  if ( (strcmp (gxPLMessageSchemaClassGet (message), "hbeat") == 0) ||
       (strcmp (gxPLMessageSchemaClassGet (message), "config") == 0)) {

    // When the hub receives a hbeat.app or config.app message
    // the hub should extract the "remote-ip" value from the message body
    const char * str_addr = gxPLMessagePairGet (message, "remote-ip");
    const char * str_port = gxPLMessagePairGet (message, "port");

    if ( (str_addr) && (str_port)) {

      // and compare the IP address with the list of addresses the hub is
      // currently bound to for the  local computer.
      if (iVectorFindFirstIndex (hub->local_addr_list, str_addr) >= 0) {
        gxPLHubClient * client;
        gxPLIoAddr clinfo;
        char * endptr;
        long now;

        now = gxPLTime();

        // remote-ip matches with local address, converts to gxPLIoAddr
        if (gxPLIoCtl (hub->app, gxPLIoFuncNetAddrFromString, &clinfo, str_addr) != 0) {

          PERROR ("unable to convert %s to ip address", str_addr);
          return;
        }

        // Gets the ip port
        clinfo.port = strtol (str_port, &endptr, 10);
        if (endptr == NULL) {

          PERROR ("unable to convert %s to udp port", str_port);
          return;
        }

        if (strcmp (gxPLMessageSchemaTypeGet (message), "app") == 0) {
          int interval;
          const char * str_interval;

          // Gets heartbeat interval for update
          str_interval = gxPLMessagePairGet (message, "interval");
          interval =  strtol (str_interval, &endptr, 10);
          if (endptr == NULL) {

            PERROR ("unable to convert %s to heartbeat interval", str_interval);
            return;
          }

          client = pvVectorFindFirst (&hub->clients, &clinfo);
          if (client == NULL) {

            // New client
            client = calloc (1, sizeof (gxPLHubClient));
            assert (client);

            // Copies address and port for this client
            memcpy (&client->addr, &clinfo, sizeof (clinfo));

            // then adds to the list
            if (iVectorAppend (&hub->clients, client) != 0) {

              PERROR ("unable to append client");
              free (client);
              return;
            }
            PINFO ("add application %s:%s, processing %d applications",
                  str_addr, str_port,
                  iVectorSize (&hub->clients));
          }

          client->hbeat_period_max = interval * 60 * 2 + 60;
          client->hbeat_last = now;
        }
        else if (strcmp (gxPLMessageSchemaTypeGet (message), "end") == 0) {
          int c = iVectorFindFirstIndex (&hub->clients, &clinfo);

          if (c >= 0) {

            iVectorRemove (&hub->clients, c);
            PINFO ("remove application %s:%s after receiving his"
                  " heartbeat end , processing %d applications",
                  str_addr, str_port,
                  iVectorSize (&hub->clients));
          }
        }
      }
      // If the address does not match any local addresses, the packet moves on
      // to the delivery/rebroadcast step.
    }
  }

  // Deliver/Rebroadcast those messages to all xPL applications on the same computer
  for (int i = 0; i < iVectorSize (&hub->clients); i++) {

    gxPLHubClient * client = pvVectorGet (&hub->clients, i);
    gxPLAppSendMessage (hub->app, message, &client->addr);
  }
}

/* public api functions ===================================================== */

// -----------------------------------------------------------------------------
gxPLHub *
gxPLHubOpen (gxPLSetting * setting) {
  gxPLHub * hub = calloc (1, sizeof (gxPLHub));
  assert (hub);

  // ignore iolayer and connection type
  strcpy (setting->iolayer, "udp");
  setting->connecttype = gxPLConnectStandAlone;

  hub->app = gxPLAppOpen (setting);
  if (hub->app) {

    if (iVectorInit (&hub->clients, 1, NULL, free) == 0) {
      if (iVectorInitSearch (&hub->clients, prvClientKey, prvClientMatch) == 0) {
        // Add a listener for all xPL messages
        if (gxPLMessageListenerAdd (hub->app, prvHandleMessage, hub) == 0) {

          hub->local_addr_list = gxPLIoLocalAddrList (hub->app);
          return hub;
        }
      }
    }
  }
  PERROR ("unable to open hub");
  free (hub);
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLHubClose (gxPLHub * hub) {

  if (hub) {

    int ret = gxPLAppClose (hub->app);
    vVectorDestroy (&hub->clients);
    free (hub);
    return ret;
  }
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLHubPoll (gxPLHub * hub, int timeout_ms) {
  int ret;

  ret = gxPLAppPoll (hub->app, timeout_ms);
  hub->timeout += timeout_ms;

  if (hub->timeout >= 60000) {
    long now;

    // Track known local xPL applications and determine when they have died and
    // remove them from the list of local applications
    hub->timeout = 0;
    now = gxPLTime();
    for (int i = 0; i < iVectorSize (&hub->clients); i++) {
      gxPLHubClient * client = pvVectorGet (&hub->clients, i);

      if ( (now - client->hbeat_last) > client->hbeat_period_max) {
        char * str;
        if (gxPLIoCtl (hub->app, gxPLIoFuncNetAddrToString, &client->addr, &str) == 0) {

          PINFO ("remove application %s:%d after heartbeat timeout, "
                "processing %d applications",
                str, client->addr.port,
                iVectorSize (&hub->clients) - 1);
        }
        iVectorRemove (&hub->clients, i);
      }
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
gxPLApplication *
gxPLHubApplication (gxPLHub * hub) {

  return hub->app;
}
/* ========================================================================== */
#endif /*  __AVR__ not defined */
