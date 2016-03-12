/**
 * @file
 * High level interface to manage xPL devices (source code)
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gxPL.h>
#include "device_p.h"

/* structures =============================================================== */
typedef struct _listener_elmt {
  gxPLDeviceListener func;
  void * data;
  gxPLSchema schema;
  gxPLMessageType msg_type;
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

  return * ( (gxPLDeviceListener *) key1) == * ( (gxPLDeviceListener *) key2);
}

// -----------------------------------------------------------------------------
// Create a heartbeat message
static gxPLMessage *
prvHeartbeatMessageNew (gxPLDevice * device, gxPLHeartbeatType type) {

  gxPLMessage * message = gxPLDeviceMessageNew (device, gxPLMessageStatus);
  assert (message);
  gxPLApplication * app = gxPLDeviceParentGet (device);
  assert (app);

  gxPLMessageBroadcastSet (message, true);

  if (device->isconfigurable && !device->isconfigured) {

    gxPLMessageSchemaClassSet (message, "config");
  }
  else {

    gxPLMessageSchemaClassSet (message, "hbeat");
  }

  if (type & gxPLHeartbeatGoodbye) {

    gxPLMessageSchemaTypeSet (message, "end");
  }
  else {

    if (gxPLIoInfoGet (app)->family & gxPLNetFamilyInet)  {

      gxPLMessageSchemaTypeSet (message, "app");
    }
    else {

      gxPLMessageSchemaTypeSet (message, "basic");
    }
  }

  gxPLMessagePairAddFormat (message, "interval", "%d",
                            device->hbeat_interval / 60);

  if (gxPLIoInfoGet (app)->family & gxPLNetFamilyInet)  {

    gxPLMessagePairAddFormat (message, "port", "%d", gxPLIoInfoGet (app)->port);
    gxPLMessagePairAdd (message, "remote-ip", gxPLIoLocalAddrGet (app));
  }

  if (device->version) {

    gxPLMessagePairAdd (message, "version", device->version);
  }

#if CONFIG_HBEAT_BASIC_EXTENSION != 0
  // add the "remote-addr" field in hbeat.basic
  if ((gxPLIoInfoGet (app)->family & gxPLNetFamilyInet) == 0) {
    const char * local_addr = gxPLIoLocalAddrGet (app);
    if (strlen (local_addr) > 0) {

      gxPLMessagePairAdd (message, "remote-addr", local_addr);
    }
  }
#endif /* CONFIG_HBEAT_BASIC_EXTENSION != 0 */
  return message;
}

// -----------------------------------------------------------------------------
// Send an xPL Heartbeat immediatly
int
prvHeartbeatMessageSendHello (gxPLDevice * device) {
  gxPLMessage * message;

  // Create the Heartbeat message, if needed
  if (device->hbeat_msg == NULL) {

    message = prvHeartbeatMessageNew (device, gxPLHeartbeatHello);
    if (message == NULL) {
      return -1;
    }

    // Install a new heartbeat message
    device->hbeat_msg = message;
#ifdef DEBUG
    char * str = gxPLMessageToString (message);
    PDEBUG ("Just allocated a new Heartbeat message:\n%s", str);
    free (str);
#else
    PDEBUG ("Just allocated a new Heartbeat message");
#endif
  }
  else {

    message = device->hbeat_msg;
  }

  // Send the message
  if (gxPLDeviceMessageSend (device, message) > 0) {

    // Update last heartbeat time
    long now = gxPLTime();
    PDEBUG ("Sent heartbeat message at %s", gxPLTimeStr (now, NULL));
    device->hbeat_last = now;
    return 0;
  }
  PERROR ("Unable to send heartbeat");
  return -1;
}

// -----------------------------------------------------------------------------
//  Send an Goodbye heartbeat immediatly
int
prvHeartbeatMessageSendGoodbye (gxPLDevice * device) {
  int ret = -1;
  gxPLMessage *  message = prvHeartbeatMessageNew (device, gxPLHeartbeatGoodbye);

  // Send the message
  PDEBUG ("Sent goodbye heartbeat");
  if (gxPLDeviceMessageSend (device, message) > 0) {

    gxPLMessageDelete (device->hbeat_msg);
    device->hbeat_msg = NULL;
    return 0;
  }
  gxPLMessageDelete (message);
  return ret;
}

// -----------------------------------------------------------------------------
// Dispatch device messages to appropriate listeners
static void
prvDeviceDispatchEvent (gxPLDevice * device, gxPLMessage * message) {

  for (int i = 0; i < iVectorSize (&device->listener); i++) {

    listener_elmt * listener = pvVectorGet (&device->listener, i);

    if ( (listener->msg_type != gxPLMessageAny) &&
         (listener->msg_type != gxPLMessageTypeGet (message))) {

      continue;
    }

    // the message type matches
    if ( (strlen (listener->schema.class) > 0) &&
         (strcmp (listener->schema.class, gxPLMessageSchemaClassGet (message)) != 0)) {

      continue;
    }
    if ( (strlen (listener->schema.type) > 0) &&
         (strcmp (listener->schema.type, gxPLMessageSchemaTypeGet (message)) != 0)) {

      continue;
    }

    // the schema matches
    if (listener->func) {

      // call the user listener
      listener->func (device, message, listener->data);
    }
  }
}

/* private api functions ==================================================== */

// -----------------------------------------------------------------------------
void
gxPLDeviceMessageHandler (gxPLDevice * device, gxPLMessage * message,
                          void * udata) {

  if ( (device->ishubconfirmed == 0) &&
       (gxPLAppIsHubEchoMessage (device->parent, message, &device->id) == true)) {

    device->ishubconfirmed = 1;
    PDEBUG ("Hub detected and confirmed existing");
  }

  // If we are not reporting your own messages, see if they originated with us
  // and if so, dump it
  if (device->isreportownmsg == false) {

    if (gxPLIdCmp (gxPLMessageSourceIdGet (message), &device->id) == 0) {

      PDEBUG ("Skipping message from self");
      return;
    }
  }

  // Is this a broadcast message?
  if (gxPLMessageIsBroadcast (message) == true) {

    // See if this is a request for a heartbeat
    if ( (gxPLMessageTypeGet (message) == gxPLMessageCommand)
         && !strcasecmp (gxPLMessageSchemaClassGet (message), "hbeat")
         && !strcasecmp (gxPLMessageSchemaTypeGet (message), "request")) {

      // Compute a response delay (.5 to 2.5 seconds)
      unsigned int ms = (unsigned int) ( ( (double) random() /
                                           (double) RAND_MAX) * 2000.0) + 500;
      PDEBUG ("Sending heartbeat in response to discovery request "
              "after a %u millisecond delay", ms);
      gxPLTimeDelayMs (ms);
      prvHeartbeatMessageSendHello (device);
    }
#if CONFIG_DEVICE_FILTER
// -----------------------------------------------------------------------------
    // If we have filters, see if they match // TODO
    if (device->havefilter) {
      bool isfound = false;

      // See if we have any groups and if any groups match
      for (int i = 0; i < iVectorSize (&device->filter); i++) {

        gxPLFilter * filter = (gxPLFilter *) pvVectorGet (&device->filter, i);

        if (gxPLMessageFilterMatch (message, filter) == true) {

          PDEBUG ("message matches my filters");
          isfound = true;
          break;
        }
      }

      // If we had filters and none match, skip this device
      if (isfound == false) {
        PINFO ("skip message, does not match my filters");
        return;
      }
    }
// -----------------------------------------------------------------------------
#endif /* CONFIG_DEVICE_FILTER true */

  }
  else {

    // Targeted message

    if (gxPLMessageIsGrouped (message) == false) {
      // Make sure this target matches
      if (gxPLIdCmp (gxPLMessageTargetIdGet (message), &device->id) != 0) {

        return;
      }
    }
#if CONFIG_DEVICE_GROUP
// -----------------------------------------------------------------------------
    else {
      // Grouped message
      bool isfound = false;

      if (device->havegroup) {

        // See if we have any groups and if any groups match
        for (int i = 0; i < iVectorSize (&device->group); i++) {

          char * group = (char *) pvVectorGet (&device->group, i);

          if (strcmp (gxPLMessageTargetInstanceIdGet (message), group) == 0) {
            PDEBUG ("message matches my groups");
            isfound = true;
            break;
          }
        }
      }

      // If there is no group or no matching group in this device, skip it
      if (isfound == false) {
        PDEBUG ("skip group message that does not match");
        return;
      }
    }
// -----------------------------------------------------------------------------
#endif /* CONFIG_DEVICE_GROUP true */
  }

  /* Message is :
   * - broadcasted (that matches filters, if any) OR
   * - grouped that matches a group assigned to this device OR
   * - targeted to this device
   * so dispatch it!
   */
  prvDeviceDispatchEvent (device, message);
}


// -----------------------------------------------------------------------------
gxPLDevice *
gxPLDeviceNew (gxPLApplication * app,
               const char * vendor_id,
               const char * device_id,
               const char * instance_id) {
  gxPLDevice * device = calloc (1, sizeof (gxPLDevice));
  assert (device);

  device->parent = app;
  device->hbeat_interval = DEFAULT_HEARTBEAT_INTERVAL;

  // init listener vector
  iVectorInit (&device->listener, 1, NULL, free);
  iVectorInitSearch (&device->listener, prvListenerKey, prvListenerMatch);

  // init vendor id
  if (gxPLIdVendorIdSet (&device->id, vendor_id) != 0) {
    free (device);
    return NULL;
  }
  // init device id
  if (gxPLIdDeviceIdSet (&device->id, device_id) != 0) {
    free (device);
    return NULL;
  }

  if (gxPLDeviceGroupInit (device) != 0) {
    free (device);
    return NULL;
  }

  if (gxPLDeviceFilterInit (device) != 0) {
    free (device);
    return NULL;
  }
  // init instance id
  if (instance_id != NULL) {

    // instance id provided
    if (gxPLIdInstanceIdSet (&device->id, instance_id) == 0) {

      // setting up successful
      return device;
    }

  }
  else {
    // instance id not provided, generates fairly unique id

    if (gxPLGenerateUniqueId (app, device->id.instance,
                              GXPL_INSTANCEID_MAX) == GXPL_INSTANCEID_MAX) {

      // setting up successful
      return device;
    }
  }
  // failure exit
  free (device);
  return NULL;
}

// -----------------------------------------------------------------------------
void
gxPLDeviceDelete (gxPLDevice * device) {

  if (device) {

    // Disable any heartbeats
    gxPLDeviceEnable (device, false);

    // Release heartbeat message, if any
    gxPLMessageDelete (device->hbeat_msg);

    // Release device resources
    free (device->version);

    // Release group info
    gxPLDeviceGroupDelete (device);

    // Release filters
    gxPLDeviceFilterDelete (device);

    // Release any listeners
    vVectorDestroy (&device->listener);

    // Release configuration data
    gxPLDeviceConfigDelete (device);

    // Free the device
    free (device);
  }
}

/* public api functions ===================================================== */

// -----------------------------------------------------------------------------
gxPLMessage *
gxPLDeviceMessageNew (gxPLDevice * device, gxPLMessageType type) {

  if (type != gxPLMessageAny) {
    gxPLMessage * message = gxPLMessageNew (type);
    assert (message);

    gxPLMessageSourceIdSet (message, &device->id);
    return message;
  }
  PERROR ("Unable to create new device message");
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceMessageSend (gxPLDevice * device, gxPLMessage * message) {

  return gxPLAppBroadcastMessage (gxPLDeviceParentGet (device), message);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceHeartbeatSend (gxPLDevice * device, gxPLHeartbeatType type) {

  if (type == gxPLHeartbeatGoodbye) {

    return prvHeartbeatMessageSendGoodbye (device);
  }
  return prvHeartbeatMessageSendHello (device);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceListenerAdd (gxPLDevice * device,
                       gxPLDeviceListener func,
                       gxPLMessageType type,
                       char * schema_class, char * schema_type,
                       void * udata) {

  listener_elmt * listener = calloc (1, sizeof (listener_elmt));
  assert (listener);

  listener->func = func;
  listener->data = udata;
  listener->msg_type = type;
  gxPLSchemaClassSet (&listener->schema, schema_class);
  gxPLSchemaTypeSet (&listener->schema, schema_type);
  if (iVectorAppend (&device->listener, listener) == 0) {

    return 0;
  }
  free (listener);
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceListenerRemove (gxPLDevice * device,
                          gxPLDeviceListener listener) {

  int i = iVectorFindFirstIndex (&device->listener, &listener);
  return iVectorRemove (&device->listener, i);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIdSet (gxPLDevice * device,  const gxPLId * id) {

  if (gxPLIdCmp (id, &device->id) != 0) {

    if (device->isenabled) {

      if (prvHeartbeatMessageSendGoodbye (device) != 0) {

        return -1;
      }
    }

    gxPLIdCopy (&device->id, id);

    if (device->isenabled) {

      if (prvHeartbeatMessageSendHello (device) != 0) {

        return -1;
      }
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceVendorIdSet (gxPLDevice * device, const char * vendor_id) {
  gxPLId id;

  gxPLIdCopy (&id, &device->id);

  if (gxPLIdVendorIdSet (&id, vendor_id) == 0) {

    return gxPLDeviceIdSet (device, &id);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceDeviceIdSet (gxPLDevice * device, const char * device_id) {
  gxPLId id;

  gxPLIdCopy (&id, &device->id);
  if (gxPLIdDeviceIdSet (&id, device_id) == 0) {

    return gxPLDeviceIdSet (device, &id);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceInstanceIdSet (gxPLDevice * device, const char * instance_id) {
  gxPLId id;

  gxPLIdCopy (&id, &device->id);
  if (gxPLIdInstanceIdSet (&id, instance_id) == 0) {

    return gxPLDeviceIdSet (device, &id);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceVersionSet (gxPLDevice * device, const char * version) {

  if (device->version) {

    if (strcmp (version, device->version) == 0) {

      return 0;
    }
    free (device->version);
  }

  device->version = malloc (strlen (version) + 1);
  assert (device->version);
  strcpy (device->version, version);
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceEnable (gxPLDevice * device, bool enabled) {

  // Skip if already enabled
  if (device->isenabled != enabled) {

    // Mark the device
    device->isenabled = enabled;

    // Handle enabling a disabled device
    if (device->isenabled) {

      // If there is an existing heartbeat, release it and rebuild it
      if (device->hbeat_msg != NULL) {

        gxPLMessageDelete (device->hbeat_msg);
        device->hbeat_msg = NULL;
      }

      // Start sending heartbeats
      return prvHeartbeatMessageSendHello (device);
    }
    else {

      // Send goodby heartbeat
      return prvHeartbeatMessageSendGoodbye (device);
    }
  }

  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceHeartbeatIntervalSet (gxPLDevice * device, int interval) {

  device->hbeat_interval = interval;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceRespondToBroadcastSet (gxPLDevice * device, bool respond) {

  device->nobroadcast = !respond;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceReportOwnMessagesSet (gxPLDevice * device, bool isreportownmsg) {

  device->isreportownmsg = isreportownmsg;
  return 0;
}

// -----------------------------------------------------------------------------
gxPLApplication *
gxPLDeviceParentGet (gxPLDevice * device) {

  return device->parent;
}

// -----------------------------------------------------------------------------
const gxPLId *
gxPLDeviceId (const gxPLDevice * device) {

  return &device->id;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIsEnabled (const gxPLDevice * device) {

  return device->isenabled;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceHeartbeatInterval (const gxPLDevice * device) {

  return device->hbeat_interval;
}

// -----------------------------------------------------------------------------
const char *
gxPLDeviceVersion (const gxPLDevice * device) {

  return device->version;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIsRespondToBroadcast (const gxPLDevice * device) {

  return device->nobroadcast;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIsReportOwnMessages (const gxPLDevice * device) {

  return device->isreportownmsg;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIsHubConfirmed (const gxPLDevice * device) {

  return device->ishubconfirmed;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIsConfigurale (const gxPLDevice * device) {

  return device->isconfigurable;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIsConfigured (const gxPLDevice * device) {

  return device->isconfigured;
}

// -----------------------------------------------------------------------------
long
gxPLDeviceHeartbeatLast (const gxPLDevice * device) {

  return device->hbeat_last;
}

// -----------------------------------------------------------------------------
gxPLSetting *
gxPLDeviceSetting (gxPLDevice * device) {
  
  return gxPLAppSetting (gxPLDeviceParentGet (device));
}

/* ========================================================================== */
