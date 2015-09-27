/**
 * @file device.c
 * xPL Service Management
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
#include <assert.h>
#include <gxPL.h>
#include "device_p.h"

/* constants ================================================================ */
/* macros =================================================================== */
/* private variables ======================================================== */
/* structures =============================================================== */
typedef struct _listener_elmt {
  gxPLDeviceListener func;
  void * data;
  gxPLSchema schema;
  gxPLMessageType msg_type;
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

  return * ( (gxPLDeviceListener *) key1) == * ( (gxPLDeviceListener *) key2);
}

// -----------------------------------------------------------------------------
// Create a heartbeat message
static gxPLMessage *
prvHeartbeatMessageNew (gxPLDevice * device, gxPLHeartbeatType type) {

  gxPLMessage * message = gxPLDeviceMessageNew (device, gxPLMessageStatus);
  if (message) {
    gxPL * gxpl = gxPLDeviceParentGet (device);

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

      if (gxPLIoInfoGet (gxpl)->family & gxPLNetFamilyInet)  {

        gxPLMessageSchemaTypeSet (message, "app");
      }
      else {

        gxPLMessageSchemaTypeSet (message, "basic");
      }
    }

    gxPLMessagePairValuePrintf (message, "interval", "%d",
                                device->hbeat_interval / 60);

    if (gxPLIoInfoGet (gxpl)->family & gxPLNetFamilyInet)  {

      gxPLMessagePairValuePrintf (message, "port", "%d", gxPLIoInfoGet (gxpl)->port);
      gxPLMessagePairAdd (message, "remote-ip", gxPLIoLocalAddrGet (gxpl));
    }
    if (device->version) {

      gxPLMessagePairAdd (message, "version", device->version);
    }
  }
  return message;
}

// -----------------------------------------------------------------------------
// Send an XPL Heartbeat immediatly
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
    vLog (LOG_DEBUG, "Just allocated a new Heartbeat message");
  }
  else {

    message = device->hbeat_msg;
  }

  // Send the message
  if (gxPLDeviceMessageSend (device, message) > 0) {

    // Update last heartbeat time
    long now = gxPLTime();
    vLog (LOG_DEBUG, "Sent heartbeat message at %s", gxPLTimeStr (now));
    device->hbeat_last = now;
    return 0;
  }
  vLog (LOG_ERR, "Unable to send heartbeat");
  return -1;
}

// -----------------------------------------------------------------------------
//  Send an Goodbye heartbeat immediatly
int
prvHeartbeatMessageSendGoodbye (gxPLDevice * device) {

  gxPLMessage *  message = prvHeartbeatMessageNew (device, gxPLHeartbeatGoodbye);
  if (message) {

    // Send the message
    vLog (LOG_DEBUG, "Sent goodbye heartbeat");
    if (gxPLDeviceMessageSend (device, message) > 0) {

      gxPLMessageDelete (device->hbeat_msg);
      device->hbeat_msg = NULL;
      return 0;
    }
  }
  return -1;
}

// -----------------------------------------------------------------------------
// Dispatch device messages to appropriate listeners
static void
prvDeviceDispatchEvent (gxPLDevice * device, const gxPLMessage * message) {

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
gxPLDeviceMessageHandler (gxPLDevice * device, const gxPLMessage * message,
                          void * udata) {

  if ( (device->ishubconfirmed == 0) &&
       (gxPLMessageIsHubEcho (device->parent, message, &device->id) == true)) {

    device->ishubconfirmed = 1;
    vLog (LOG_DEBUG, "Hub detected and confirmed existing");
  }

  // If we are not reporting your own messages, see if they originated with us
  // and if so, dump it
  if (device->isreportownmsg == false) {

    if (gxPLIdCmp (gxPLMessageSourceIdGet (message), &device->id) == 0) {

      vLog (LOG_DEBUG, "Skipping message from self");
      return;
    }
  }

  // Is this a broadcast message?
  if (gxPLMessageBroadcastGet (message) == true) {

    // See if this is a request for a heartbeat
    if ( (gxPLMessageTypeGet (message) == gxPLMessageCommand)
         && !strcasecmp (gxPLMessageSchemaClassGet (message), "hbeat")
         && !strcasecmp (gxPLMessageSchemaTypeGet (message), "request")) {

      // Compute a response delay (.5 to 2.5 seconds)
      unsigned int ms = (unsigned int) ( ( (double) random() /
                                           (double) RAND_MAX) * 2000.0) + 500;
      vLog (LOG_DEBUG, "Sending heartbeat in response to discovery request "
            "after a %u millisecond delay", ms);
      gxPLTimeDelayMs (ms);
      prvHeartbeatMessageSendHello (device);
    }

#if 0
    // If we have filters, see if they match // TODO
    if (device->filterCount != 0) {
      for (filterIndex = 0; filterIndex < device->filterCount; filterIndex++) {
        if (prvDoesFilterMatch (& (device->messageFilterList[filterIndex]), message)) {
          foundFilter = TRUE;
          break;
        }
      }

      // If we had filters and none match, skip this device
      if (!foundFilter) {
        return;
      }
    }
#endif
  }
  else {

    // Targeted message

    // Make sure this target matches
    if (gxPLIdCmp (gxPLMessageTargetIdGet (message), &device->id) != 0) {

      return;
    }

#if 0
    if (message->isgrouped == false) {

      // Make sure this target matches
      if (gxPLIdCmp (&message->target, &device->id) != 0) {

        return;
      }
    }
    else {
      // See if we have any groups and if any groups match // TODO
      for (groupIndex = 0; groupIndex < device->groupCount; groupIndex++) {
        if (!strcasecmp (message->group, device->groupList[groupIndex])) {
          foundGroup = TRUE;
          break;
        }
      }

      // If there is no group or no matching group in this device, skip it
      if (!foundGroup) {
        return;
      }
    }
#endif
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
gxPLDeviceNew (gxPL * gxpl,
               const char * vendor_id,
               const char * device_id,
               const char * instance_id) {
  gxPLDevice * device = calloc (1, sizeof (gxPLDevice));
  assert (device);

  device->parent = gxpl;
  device->hbeat_interval = DEFAULT_HEARTBEAT_INTERVAL;

  // init listener vector
  if (iVectorInit (&device->listener, 2, NULL, free) == 0) {

    if (iVectorInitSearch (&device->listener, prvListenerKey,
                           prvListenerMatch) == 0) {

      // init vendor id
      if (gxPLIdVendorIdSet (&device->id, vendor_id) == 0) {

        // init device id
        if (gxPLIdDeviceIdSet (&device->id, device_id) == 0) {

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

            if (gxPLGenerateUniqueId (gxpl, device->id.instance,
                                      GXPL_INSTANCEID_MAX) == GXPL_INSTANCEID_MAX) {

              // setting up successful
              return device;
            }
          }
        }
      }
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
    gxPLDeviceEnabledSet (device, false);

    // Release heartbeat message, if any
    gxPLMessageDelete (device->hbeat_msg);

    // Release device resources
    free (device->version);

    // Release group info // TODO
    // Release filters // TODO

    // Release any listeners
    vVectorDestroy (&device->listener);

    // Release configuration data // TODO

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
    if (message) {

      gxPLMessageSourceIdSet (message, &device->id);
      return message;
    }
    vLog (LOG_ERR, "Unable to create new device message");
  }
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceMessageSend (gxPLDevice * device, gxPLMessage * message) {

  return gxPLMessageSend (gxPLDeviceParentGet (device), message);
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
  (void) gxPLSchemaClassSet (&listener->schema, schema_class);
  (void) gxPLSchemaTypeSet (&listener->schema, schema_type);
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
  if ( (device == NULL) || (id == NULL)) {
    errno = EFAULT;
    return -1;
  }

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

  if ( (device) && (vendor_id)) {
    gxPLId id;

    gxPLIdCopy (&id, &device->id);
    if (gxPLIdVendorIdSet (&id, vendor_id) == 0) {
      return gxPLDeviceIdSet (device, &id);
    }
  }

  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceDeviceIdSet (gxPLDevice * device, const char * device_id) {

  if ( (device) && (device_id)) {
    gxPLId id;

    gxPLIdCopy (&id, &device->id);
    if (gxPLIdDeviceIdSet (&id, device_id) == 0) {
      return gxPLDeviceIdSet (device, &id);
    }
  }

  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceInstanceIdSet (gxPLDevice * device, const char * instance_id) {

  if ( (device) && (instance_id)) {
    gxPLId id;

    gxPLIdCopy (&id, &device->id);
    if (gxPLIdInstanceIdSet (&id, instance_id) == 0) {
      return gxPLDeviceIdSet (device, &id);
    }
  }

  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceVersionSet (gxPLDevice * device, const char * version) {

  if ( (device) && (version)) {

    if (device->version) {

      if (strcmp (version, device->version) == 0) {

        return 0;
      }
      free (device->version);
    }

    device->version = malloc (strlen (version) + 1);
    strcpy (device->version, version);
    return 0;
  }

  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceEnabledSet (gxPLDevice * device, bool enabled) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

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
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
// TODO
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLRespondToBroadcastSet (gxPLDevice * device, bool respond) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  device->nobroadcast = !respond;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLReportOwnMessagesSet (gxPLDevice * device, bool isreportownmsg) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  device->isreportownmsg = isreportownmsg;
  return 0;
}

// -----------------------------------------------------------------------------
gxPL *
gxPLDeviceParentGet (gxPLDevice * device) {
  if (device == NULL) {
    errno = EFAULT;
    return NULL;
  }
  return device->parent;
}

// -----------------------------------------------------------------------------
const gxPLId *
gxPLDeviceIdGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return NULL;
  }

  return &device->id;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceEnabledGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->isenabled;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceHeartbeatIntervalGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->hbeat_interval;
}

// -----------------------------------------------------------------------------
const char *
gxPLDeviceVersionGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return NULL;
  }

  return device->version;
}

// -----------------------------------------------------------------------------
int
gxPLRespondToBroadcastGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->nobroadcast;
}

// -----------------------------------------------------------------------------
int
gxPLReportOwnMessagesGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->isreportownmsg;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceHubConfirmedGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->ishubconfirmed;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfiguraleGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->isconfigurable;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfiguredGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->isconfigured;
}

// -----------------------------------------------------------------------------
long gxPLDeviceHeartbeatLastGet (const gxPLDevice * device) {

  if (device == NULL) {

    errno = EFAULT;
    return -1;
  }

  return device->hbeat_last;
}
/* ========================================================================== */
