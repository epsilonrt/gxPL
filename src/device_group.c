/**
 * @file
 * High level interface to manage xPL devices (source code for groups)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#if CONFIG_DEVICE_GROUP
/* ========================================================================== */
#include <stdlib.h>
#include <string.h>
#include <gxPL.h>
#include "device_p.h"

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
int
gxPLDeviceGroupInit (gxPLDevice * device) {

  device->group_max = DEFAULT_MAX_DEVICE_GROUP;
  return iVectorInit (&device->group, 1, NULL, free);
}

// -----------------------------------------------------------------------------
void
gxPLDeviceGroupDelete (gxPLDevice * device) {

  device->havegroup = 0;
  vVectorDestroy (&device->group);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceGroupAddListOfItems (gxPLDevice * device, gxPLMessage * message) {

  return gxPLMessagePairAddFormat (message, "option", "group[%d]", device->group_max);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceGroupAddCurrentValues (gxPLDevice * device, gxPLMessage * message) {

  if (device->havegroup) {

    for (int i = 0; i < iVectorSize (&device->group); i++) {

      char * group = pvVectorGet (&device->group, i);
      gxPLMessagePairAddFormat (message, "group", "xpl-group.%s", group);
    }
  }
  else {

    gxPLMessagePairAdd (message, "group", NULL);
  }
  return 0;
}

/* public api functions ===================================================== */
// -----------------------------------------------------------------------------
int
gxPLDeviceGroupAdd (gxPLDevice * device, const char * group_name) {

  if (group_name) {
    if (strlen (group_name) > 0) {
      if (iVectorSize (&device->group) < device->group_max) {
        int ret;
        PDEBUG ("  Set new group %s", group_name);
        char * new_group = malloc (strlen (group_name) + 1);
        assert (new_group);
        strcpy (new_group, group_name);
        ret = iVectorAppend (&device->group, new_group);
        if ( (ret == 0) && (iVectorSize (&device->group) > 0)) {
          device->havegroup = 1;
        }
        return ret;
      }
    }
    else {
      // group_name == NULL
      return 0;
    }
  }
  else {
    // strlen(group_name) <= 0
    return 0;
  }
  PERROR ("Group overflow, max values %d", device->group_max);
  return -1;
}

// -----------------------------------------------------------------------------
// xpl-group.name
int
gxPLDeviceGroupAddFromString (gxPLDevice * device, const char * str) {

  if (str) {
    const int name_index =  strlen ("xpl-group.");

    if (strncmp (str, "xpl-group.", name_index) == 0) {

      if (iVectorSize (&device->group) < device->group_max) {
        const char * name = &str[name_index];
        if (strlen (name) > 0) {
          int ret;

          PDEBUG ("  Set new group %s", name);
          char * new_group = malloc (strlen (name) + 1);
          assert (new_group);
          strcpy (new_group, name);
          ret = iVectorAppend (&device->group, new_group);
          if ( (ret == 0) && (iVectorSize (&device->group) > 0)) {
            device->havegroup = 1;
          }
          return ret;
        }
        return 0;
      }
    }
    else {
      // group_name == NULL
      return 0;
    }
  }
  else {
    // strncmp (str, "xpl-group.", strlen("xpl-group.")) != 0
    return 0;
  }
  PERROR ("Group overflow, max values %d", device->group_max);
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceGroupHave (const gxPLDevice * device) {

  return device->havegroup;
}

// -----------------------------------------------------------------------------
int
gxPLDeviceGroupClearAll (gxPLDevice * device) {

  device->havegroup = 0;
  return iVectorClear (&device->group);
}

// -----------------------------------------------------------------------------
int
gxPLDeviceGroupCount (const gxPLDevice * device) {

  return iVectorSize (&device->group);
}

// -----------------------------------------------------------------------------
const char *
gxPLDeviceGroupGet (const gxPLDevice * device, int index) {

  return (const char *) pvVectorGet (&device->group, index);
}


/* ========================================================================== */
#endif /* CONFIG_DEVICE_GROUP true */
