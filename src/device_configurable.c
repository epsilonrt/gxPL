/**
 * @file
 * High level interface to manage configurable devices (source code)
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#if CONFIG_DEVICE_CONFIGURABLE
/* ========================================================================== */
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
  gxPLDeviceConfigListener func;
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

  return * ( (gxPLDeviceConfigListener *) key1) == * ( (gxPLDeviceConfigListener *) key2);
}

// -----------------------------------------------------------------------------
void
prvDeviceConfigItemDelete (void * item) {
  if (item) {
    gxPLDeviceConfigItem * p = (gxPLDeviceConfigItem *) item;

    free (p->name);
    vVectorDestroy (&p->values);
    free (p);
  }
}

// -----------------------------------------------------------------------------
int
prvDeviceConfigItemMatch (const void *key1, const void *key2) {

  return strcmp ( (const char *) key1, (const char *) key2);
}

// -----------------------------------------------------------------------------
const void *
prvDeviceConfigItemKey (const void * item) {
  gxPLDeviceConfigItem * p = (gxPLDeviceConfigItem *) item;

  return p->name;
}

// -----------------------------------------------------------------------------
// Send list of configurable items
static void
prvSendConfigList (gxPLDevice * device) {

  gxPLMessage * message = gxPLDeviceMessageNew (device, gxPLMessageStatus);
  gxPLMessageBroadcastSet (message, true);
  gxPLMessageSchemaSet (message, "config", "list");

  // Add in standard configurable items
  gxPLMessagePairAdd (message, "reconf", "newconf");
  gxPLMessagePairAdd (message, "reconf", "interval");

  gxPLDeviceGroupAddListOfItems (device, message);
  gxPLDeviceFilterAddListOfItems (device, message);

  for (int i = 0; i < iVectorSize (&device->config->items); i++) {

    gxPLDeviceConfigItem * item = pvVectorGet (&device->config->items, i);
    if (item) {
      const char * type = NULL;

      switch (item->type) {
        case gxPLConfigOptional:
          type = "option";
          break;
        case gxPLConfigMandatory:
          type = "config";
          break;
        case gxPLConfigReconf:
          type = "reconf";
          break;
        default:
          break;
      }

      if (type) {

        if (item->values_max > 1) {

          gxPLMessagePairAddFormat (message, type, "%s[%d]",
                                    item->name, item->values_max);
        }
        else {

          gxPLMessagePairAddFormat (message, type, "%s", item->name);
        }
      }
    }
  }
  gxPLDeviceMessageSend (device, message);
  gxPLMessageDelete (message);
}

// -----------------------------------------------------------------------------
// Send list of configurables
static void
prvDeviceConfigSendCurrent (gxPLDevice * device) {

  gxPLMessage * message = gxPLDeviceMessageNew (device, gxPLMessageStatus);
  gxPLMessageBroadcastSet (message, true);
  gxPLMessageSchemaSet (message, "config", "current");
  gxPLMessagePairAdd (message, "newconf", gxPLDeviceId (device)->instance);
  gxPLMessagePairAddFormat (message, "interval", "%d",
                            gxPLDeviceHeartbeatInterval (device) / 60);
  // Include groups
  gxPLDeviceGroupAddCurrentValues (device, message);
  // Include filters
  gxPLDeviceFilterAddCurrentValues (device, message);

  // Add in device configurable items
  for (int i = 0; i < iVectorSize (&device->config->items); i++) {

    gxPLDeviceConfigItem * item = pvVectorGet (&device->config->items, i);
    if (item) {

      for (int v = 0; v < iVectorSize (&item->values); v++) {

        char * value =  pvVectorGet (&item->values, v);
        gxPLMessagePairAdd (message, item->name, value);
      }
    }
  }
  gxPLDeviceMessageSend (device, message);
  gxPLMessageDelete (message);
}

// -----------------------------------------------------------------------------
// create new configuration items
// extract the corresponding pair configuration items for option reconf and config
// returns the index of the first item after the last element processed
static int
prvConfigNew (gxPLDevice * device, xVector * config) {

  iVectorClear (&device->config->items);

  PDEBUG ("Parse config[%d] to find items...", iVectorSize (config));

  for (int i = 0; i < iVectorSize (config); i++) {
    gxPLConfigurableType type;

    gxPLPair * pair = pvVectorGet (config, i);

    PDEBUG ("  item: %s=%s", pair->name, (pair->value ? pair->value : ""));

    if (strcmp (pair->name, "config") == 0) {
      type = gxPLConfigMandatory;
    }
    else if (strcmp (pair->name, "option") == 0) {
      type = gxPLConfigOptional;
    }
    else if (strcmp (pair->name, "reconf") == 0) {
      type = gxPLConfigReconf;
    }
    else {
      if (i < iVectorSize (config)) {
        PDEBUG ("Parse new config stopped at index = % d", i);
        return i;
      }
      return -1;
    }

    if (strcmp (pair->value, "newconf") && strcmp (pair->value, "interval") &&
        strncmp (pair->value, "filter", strlen ("filter")) &&
        strncmp (pair->value, "group", strlen ("group"))) {

      int values_max = -1;
      char * name;
      char * str_max = pair->value;

      gxPLDeviceConfigItem * item = calloc (1, sizeof (gxPLDeviceConfigItem));
      assert (item);

      item->type = type;

      // gets number of values if exists
      name = strsep (&str_max, "[");
      if (str_max) {
        char * endptr;

        values_max = strtol (str_max, &endptr, 10);
        if (*endptr == ']') {

          item->values_max = values_max;
        }
      }
      else {
        
          item->values_max = 1;
      }
      item->name = malloc (strlen (name) + 1);
      assert (item->name);
      strcpy (item->name, name);
      iVectorInit (&item->values, 1, NULL, free);
      iVectorAppend (&device->config->items, item);
      PDEBUG ("    item added to the list");
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------
static void
prvConfigSet (gxPLDevice * device, xVector * config, int index) {
  char * new_instance = NULL;
  int new_interval = -1;
  bool restart_needed = false;
  bool was_enabled = device->isenabled;

  PDEBUG ("Parse to set config at index = % d", index);
  for (int i = index; i < iVectorSize (config); i++) {

    gxPLPair * p = pvVectorGet (config, i);

    // Check for instance change
    if (strcmp (p->name, "newconf") == 0) {

      if (strcmp (p->value, device->id.instance) != 0) {
        new_instance = p->value;
        PDEBUG ("  Set new instance %s", new_instance);
        restart_needed = true;
      }
    }
    // Check for interval change
    else if (strcmp (p->name, "interval") == 0) {
      char * endptr;
      int i = strtol (p->value, &endptr, 10);
      if (*endptr == '\0') {
        i *= 60;
        if (i != device->hbeat_interval) {

          new_interval = i;
          PDEBUG ("  Set new interval %s s", new_interval);
          restart_needed = true;
        }
      }
    }
    // Check for groups
    else if (strcmp (p->name, "group") == 0) {

      gxPLDeviceGroupAddFromString (device, p->value);
    }
    // Check for filters
    else if (strcmp (p->name, "filter") == 0) {

      gxPLDeviceFilterAddFromStr (device, p->value);
    }
    // Anything else had better be a configurable
    else {
      gxPLDeviceConfigValueAdd (device, p->name, p->value);
    }
  }

  // See if we need to restart the device
  if (was_enabled && restart_needed) {

    gxPLDeviceEnabledSet (device, false);
    PDEBUG ("Need to restart the device");
  }
  // Install new values
  if (new_instance) {
    gxPLDeviceInstanceIdSet (device, new_instance);
  }
  if (new_interval > 0) {
    gxPLDeviceHeartbeatIntervalSet (device, new_interval);
  }
  // Flag the device as configured
  device->isconfigured = true;

  // Preserve settings
  gxPLDeviceConfigSave (device);

  // Restart device, if needed
  if (was_enabled && restart_needed) {

    gxPLDeviceEnabledSet (device, true);
    PDEBUG ("device restarted");
  }

  // Fire config changed message
  for (int i = 0; i < iVectorSize (&device->config->listener); i++) {
    listener_elmt * listener = pvVectorGet (&device->config->listener, i);
    if (listener->func) {

      // call the user listener
      listener->func (device, listener->data);
    }
  }
}

// -----------------------------------------------------------------------------
// Handle configuration messages -> schema.class = config
static void
prvConfigHandler (gxPLDevice * device, gxPLMessage * message, void * udata) {
  const char * schema_type = gxPLMessageSchemaTypeGet (message);
  const char * cmd = gxPLMessagePairGet (message, "command");

  // See if this is a request for a list of configurable elements
  if (gxPLMessageTypeGet (message) == gxPLMessageCommand) {
    if (strcasecmp (schema_type, "response") == 0) {

      gxPLDeviceConfigItemClearAll (device);
      gxPLDeviceFilterClearAll (device);
      gxPLDeviceGroupClearAll (device);
      prvConfigSet (device, gxPLMessageBodyGet (message), 0);
    }
    else {

      if (cmd != NULL) {

        if ( (strcasecmp (schema_type, "list") == 0) &&
             (strcasecmp (cmd, "request") == 0)) {

          prvSendConfigList (device);
        }
        else if ( (strcasecmp (schema_type, "current") == 0) &&
                  (strcasecmp (cmd, "request") == 0)) {

          prvDeviceConfigSendCurrent (device);
        }
      }
    }
  }
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
gxPLDevice *
gxPLDeviceConfigNew (gxPLApplication * app, const char * vendor_id,
                     const char * device_id,
                     const char * filename) {
  int ret = 0;

  // Create the device
  gxPLDevice * device = gxPLDeviceNew (app, vendor_id, device_id, NULL);
  assert (device);
  // adds a config block
  device->config = calloc (1, sizeof (gxPLDeviceConfig));
  assert (device->config);

  // setting up
  device->isconfigurable = 1;

  iVectorInit (&device->config->items, 2, NULL, prvDeviceConfigItemDelete);
  iVectorInitSearch (&device->config->items,
                     prvDeviceConfigItemKey, prvDeviceConfigItemMatch);

  // init listener vector
  iVectorInit (&device->config->listener, 1, NULL, free);
  iVectorInitSearch (&device->config->listener, prvListenerKey, prvListenerMatch);

  if (filename) {

    ret = gxPLDeviceConfigFilenameSet (device, filename);
  }

  if (ret == 0) {

    // Install a configuration listener for this device
    if (gxPLDeviceListenerAdd (device, prvConfigHandler, gxPLMessageAny,
                               "config", NULL, NULL) == 0) {

      // If there is a config file, attempt to load it
      if (device->config->filename != NULL) {

        xVector * values = gxPLDeviceConfigLoad (device);
        if (values) {

          // Parse the data into the device
          if (iVectorSize (values) > 0) {

            int i = prvConfigNew (device, values);
            if (i >= 0) {

              prvConfigSet (device, values, i);
            }
          }
          vVectorDestroy (values);
          free (values);
        }
      }


      return device;
    } // gxPLDeviceListenerAdd() failure
  } // gxPLDeviceConfigFilenameSet() failure

  gxPLDeviceDelete (device);
  return NULL;
}

// -----------------------------------------------------------------------------
void
gxPLDeviceConfigDelete (gxPLDevice * device) {

  if ( (device->isconfigurable) && (device->config)) {

    vVectorDestroy (&device->config->items);
    free (device->config);
  }
}

/* public api functions ===================================================== */

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigFilenameSet (gxPLDevice * device, const char * filename) {


  if (device->isconfigured == 1) {

    errno = EBUSY;
  }
  else {

    // Device not configured
    if (device->config->filename) {

      // there is already a file name
      if (strcmp (device->config->filename, filename) == 0) {

        // it is the same exit without error
        return 0;
      }
      free (device->config->filename);
    }

    device->config->filename = malloc (strlen (filename) + 1);
    assert (device->config->filename);
    strcpy (device->config->filename, filename);
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
const char *
gxPLDeviceConfigFilenameGet (const gxPLDevice * device) {

  return device->config->filename;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigListenerAdd (gxPLDevice * device,
                             gxPLDeviceConfigListener func,
                             void * udata) {

  listener_elmt * listener = calloc (1, sizeof (listener_elmt));
  assert (listener);

  listener->func = func;
  listener->data = udata;
  if (iVectorAppend (&device->config->listener, listener) == 0) {

    return 0;
  }
  free (listener);
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigListenerRemove (gxPLDevice * device,
                                gxPLDeviceConfigListener func) {

  int i = iVectorFindFirstIndex (&device->config->listener, &func);
  return iVectorRemove (&device->config->listener, i);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigItemCount (const gxPLDevice * device) {

  if (device->isconfigurable) {

    return iVectorSize (&device->config->items);
  }
  PERROR ("Device not configurable");
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigItemRemoveAll (gxPLDevice * device) {

  if (device->isconfigurable) {
    if (device->isenabled == 0) {


      return iVectorClear (&device->config->items);
    }
    PERROR ("Device was enabled");
  }
  PERROR ("Device not configurable");
  return -1;
}


// -----------------------------------------------------------------------------
int
gxPLDeviceConfigItemClearAll (gxPLDevice * device) {

  if (device->isconfigurable) {

    for (int i = 0; i < iVectorSize (&device->config->items); i++) {

      gxPLDeviceConfigItem * item = pvVectorGet (&device->config->items, i);
      if (iVectorClear (&item->values) != 0) {
        return -1;
      }
    }
    return 0;
  }
  PERROR ("Device not configurable");
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigItemAdd (gxPLDevice * device, const char * name,
                         gxPLConfigurableType type, int max_values) {

  if (device->isconfigurable) {


    gxPLDeviceConfigItem * item = gxPLDeviceConfigItemFind (device, name);
    if (item != NULL) {

      PERROR ("Item [ % s] already exists", name);
      return -1;
    }

    item = calloc (1, sizeof (gxPLDeviceConfigItem));
    assert (item);
    item->name = malloc (strlen (name) + 1);
    assert (item->name);
    strcpy (item->name, name);
    item->type = type;
    item->values_max = max_values;
    iVectorInit (&item->values, 1, NULL, free);
    return iVectorAppend (&device->config->items, item);
  }

  PERROR ("Device not configurable");
  return -1;
}
// -----------------------------------------------------------------------------
int
gxPLDeviceConfigItemRemove (gxPLDevice * device, const char * name) {

  if (device->isconfigurable) {

    int index = iVectorFindFirstIndex (&device->config->items, name);

    if (index < 0) {

      PERROR ("Item [ % s] does not exists", name);
      return -1;
    }

    return iVectorRemove (&device->config->items, index);
  }
  PERROR ("Device not configurable");
  return -1;
}

// -----------------------------------------------------------------------------
gxPLDeviceConfigItem *
gxPLDeviceConfigItemFind (const gxPLDevice * device, const char * name) {

  if (device->isconfigurable) {

    return (gxPLDeviceConfigItem *) pvVectorFindFirst (&device->config->items, name);
  }
  PERROR ("Device not configurable");
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigValueCount (const gxPLDevice * device, const char * name) {

  if (device->isconfigurable) {

    gxPLDeviceConfigItem * item = gxPLDeviceConfigItemFind (device, name);
    if (item) {

      return iVectorSize (&item->values);
    }
    else {

      PERROR ("Item [ % s] does not exists", name);
    }
  }
  PERROR ("Device not configurable");
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigValueClearAll (gxPLDevice * device, const char * name) {

  if (device->isconfigurable) {

    gxPLDeviceConfigItem * item = gxPLDeviceConfigItemFind (device, name);
    if (item) {

      return iVectorClear (&item->values);
    }
    else {

      PERROR ("Item [ % s] does not exists", name);
    }
  }
  PERROR ("Device not configurable");
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigValueAdd (gxPLDevice * device,
                          const char * name, const char * value) {

  if (device->isconfigurable) {

    gxPLDeviceConfigItem * item = gxPLDeviceConfigItemFind (device, name);
    if (item) {

      if (iVectorSize (&item->values) < item->values_max) {

        if (value == NULL) {
          value = "";
        }
        PDEBUG ("  Set new value %s=%s", name, value);
        char * p = malloc (strlen (value) + 1);
        assert (p);
        strcpy (p, value);
        return iVectorAppend (&item->values, p);
      }
      else {

        PERROR ("Item values overflow, max values % d", item->values_max);
      }
    }
    else {

      PERROR ("Item [ % s] does not exists", name);
    }
  }
  PERROR ("Device not configurable");
  return -1;
}

// -----------------------------------------------------------------------------
const char *
gxPLDeviceConfigValueGetAt (gxPLDevice * device,
                            const char * name, int index) {
  if (device->isconfigurable) {

    gxPLDeviceConfigItem * item = gxPLDeviceConfigItemFind (device, name);
    if (item) {

      return (const char *) pvVectorGet (&item->values, index);
    }
    else {

      PERROR ("Item [ % s] does not exists", name);
    }
  }
  PERROR ("Device not configurable");
  return NULL;
}

// -----------------------------------------------------------------------------
const char *
gxPLDeviceConfigValueGet (gxPLDevice * device, const char * name) {

  return gxPLDeviceConfigValueGetAt (device, name, 0);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigValueSetAt (gxPLDevice * device, const char * name,
                            int index, const char * value) {

  if (device->isconfigurable) {

    gxPLDeviceConfigItem * item = gxPLDeviceConfigItemFind (device, name);
    if (item) {
      char * p = (char *) pvVectorGet (&item->values, index);

      if (p) {
        if (strcmp (p, value) != 0) {
          if (value == NULL) {
            value = "";
          }
          p = realloc (p, strlen (value + 1));
          strcpy (p, value);
          return iVectorReplace (&item->values, index, p);
        }
        return 0;
      }
      return gxPLDeviceConfigValueAdd (device, name, value);
    }
  }

  PERROR ("Device not configurable");
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceConfigValueSet (gxPLDevice * device,
                          const char * name, const char * value) {
  return gxPLDeviceConfigValueSetAt (device, name, 0, value);
}

/* ========================================================================== */
#endif /** CONFIG_DEVICE_CONFIGURABLE true */
