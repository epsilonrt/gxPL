/**
 * @file
 * High level interface to manage xPL devices (source code for filters)
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#if CONFIG_DEVICE_FILTER
/* ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gxPL.h>
#include "device_p.h"

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
int
gxPLDeviceFilterInit (gxPLDevice * device) {

  device->filter_max = DEFAULT_MAX_DEVICE_FILTER;
  return iVectorInit (&device->filter, 1, NULL, free);
}

// -----------------------------------------------------------------------------
void
gxPLDeviceFilterDelete (gxPLDevice * device) {

  device->havefilter = 0;
  vVectorDestroy (&device->filter);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceFilterAddListOfItems (gxPLDevice * device, gxPLMessage * message) {

  return gxPLMessagePairAddFormat (message, "option", "filter[%d]", device->filter_max);
}

// -----------------------------------------------------------------------------
const char * gxPLDeviceFilterToString (const gxPLFilter * filter) {
  static char buf[62];
  snprintf (buf, sizeof (buf), "%s.%s.%s.%s.%s.%s",
            gxPLMessageTypeToString (filter->type),
            filter->source.vendor,
            filter->source.device,
            filter->source.instance,
            filter->schema.class,
            filter->schema.type
           );
  return buf;

}
// -----------------------------------------------------------------------------
int
gxPLDeviceFilterAddCurrentValues (gxPLDevice * device, gxPLMessage * message) {

  if (device->havefilter) {

    for (int i = 0; i < iVectorSize (&device->filter); i++) {

      gxPLFilter * filter = pvVectorGet (&device->filter, i);
      gxPLMessagePairAdd (message, "filter", gxPLDeviceFilterToString (filter));
    }
  }
  else {

    gxPLMessagePairAdd (message, "filter", NULL);
  }
  return 0;
}

/* public api functions ===================================================== */
// -----------------------------------------------------------------------------
int
gxPLDeviceFilterAdd (gxPLDevice * device, gxPLMessageType type,
                     const gxPLId * source, const gxPLSchema * schema) {

  if (iVectorSize (&device->filter) < device->filter_max) {
    int ret;

    gxPLFilter * filter = malloc (sizeof (gxPLFilter));
    assert (filter);
    filter->type = type;
    gxPLIdCopy (&filter->source, source);
    gxPLSchemaCopy (&filter->schema, schema);
    ret = iVectorAppend (&device->filter, filter);
    if ( (ret == 0) && (iVectorSize (&device->filter) > 0)) {

      device->havefilter = 1;
    }
    return ret;
  }
  PERROR ("Unable to add a new filter, overflow !");
  return -1;

}

// -----------------------------------------------------------------------------
int
gxPLDeviceFilterAddFromStr (gxPLDevice * device, char * str) {
  if (str) {
    if (strlen (str) > 0) {
      if (iVectorSize (&device->filter) < device->filter_max) {
        gxPLMessageType type;
        char * stype;
        char * id_vendor;
        char * id_device;
        char * id_instance;
        char * schema_class;
        char * schema_type = str;

        // [msgtype].[vendor].[device].[instance].[class].[type]

        PDEBUG ("  Set new filter %s", str);
        stype = strsep (&schema_type, ".");
        if (schema_type == NULL) {
          return -1;
        }
        id_vendor = strsep (&schema_type, ".");
        if (schema_type == NULL) {
          return -1;
        }
        id_device = strsep (&schema_type, ".");
        if (schema_type == NULL) {
          return -1;
        }
        id_instance = strsep (&schema_type, ".");
        if (schema_type == NULL) {
          return -1;
        }
        schema_class = strsep (&schema_type, ".");
        if (schema_type == NULL) {
          return -1;
        }

        type = gxPLMessageTypeFromString (stype);
        if (type != gxPLMessageUnknown) {
          gxPLFilter * filter = malloc (sizeof (gxPLFilter));
          assert (filter);

          filter->type = type;
          if (gxPLIdSet (&filter->source, id_vendor, id_device, id_instance) == 0) {
            if (gxPLSchemaSet (&filter->schema, schema_class, schema_type) == 0) {

              int ret = iVectorAppend (&device->filter, filter);
              if ( (ret == 0) && (iVectorSize (&device->filter) > 0)) {

                device->havefilter = 1;
              }
              return ret;
            }
            else {

              PERROR ("Unable to set schema filter");
            }
          }
          else {

            PERROR ("Unable to set id filter");
          }
          free (filter);
        }
      }
      else {

        PERROR ("Filter overflow, max values %d", device->filter_max);
      }
    }
    else {
      // strlen(str) <= 0
      return 0;
    }
  }
  else {
    // str == NULL
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceFilterHave (const gxPLDevice * device) {

  return device->havefilter;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceFilterClearAll (gxPLDevice * device) {

  device->havefilter = 0;
  return iVectorClear (&device->filter);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceFilterCount (const gxPLDevice * device) {

  return iVectorSize (&device->filter);
}

// -----------------------------------------------------------------------------
const char *
gxPLDeviceFilterGet (const gxPLDevice * device, int index) {

  return (const char *) pvVectorGet (&device->filter, index);
}

/* ========================================================================== */
#endif /* CONFIG_DEVICE_FILTER true */
