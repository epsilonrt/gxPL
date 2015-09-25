/**
 * @file device.c
 * xPL Service Management
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
// #include <unistd.h>

#include <gxPL.h>
#include "device_p.h"
#include "config.h"

/* constants ================================================================ */
/* macros =================================================================== */
/* private variables ======================================================== */
/* structures =============================================================== */
typedef struct _listener_elmt {
  gxPLDeviceListener func;
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

  return * ( (gxPLDeviceListener *) key1) == * ( (gxPLDeviceListener *) key2);
}

/* public api functions ===================================================== */

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

    // Release group info
    // Release filters
    // Release any listeners
    (void) iVectorDestroy (&device->listener);
    // Release configuration data
    // Free the service
  }
}

// -----------------------------------------------------------------------------
int
gxPLDeviceIdSet (gxPLDevice * device,  const gxPLId * id) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  return 0;
}
// -----------------------------------------------------------------------------
int
gxPLDeviceVendorIdSet (gxPLDevice * device, const char * vendor_id) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  return 0;
}
// -----------------------------------------------------------------------------
int
gxPLDeviceDeviceIdSet (gxPLDevice * device, const char * device_id) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  return 0;
}
// -----------------------------------------------------------------------------
int
gxPLDeviceInstanceIdSet (gxPLDevice * device, const char * instance_id) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  return 0;
}
// -----------------------------------------------------------------------------
int
gxPLDeviceEnabledSet (gxPLDevice * device, bool enabled) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
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
  return 0;
}
// -----------------------------------------------------------------------------
int
gxPLRespondToBroadcastSet (gxPLDevice * device, bool respond) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  return 0;
}
// -----------------------------------------------------------------------------
int
gxPLReportOwnMessagesSet (gxPLDevice * device, bool reportmsg) {
  if (device == NULL) {
    errno = EFAULT;
    return -1;
  }
  return 0;
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
  return device->reportmsg;
}

/* ========================================================================== */
