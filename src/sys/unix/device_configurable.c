/**
 * @file
 * High level interface to manage configurable devices (unix source code)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#if CONFIG_DEVICE_CONFIGURABLE && defined(__unix__)
/* ========================================================================== */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gxPL/util.h>
#include <gxPL/device.h>
#include "device_p.h"

/* private functions ======================================================== */
#ifndef DEFAULT_LINE_BUFSIZE
#define DEFAULT_LINE_BUFSIZE 256
#endif

/* api functions ============================================================ */
xVector *
gxPLConfigReadFile (const char * filename, const char * vendor_id,
                    const char * device_id) {
  FILE *file;
  char buf[DEFAULT_LINE_BUFSIZE];
  char *line, *p;
  char *vid;
  char *did;
  xVector * values;

  // Attempt to open the configuration file
  if ( (file = fopen (filename, "r")) == NULL) {

    return NULL;
  }

  // Get the header and insure it matches
  if (fgets (buf, sizeof (buf), file) == NULL) {

    PERROR ("%s (%s)", strerror (errno), filename);
    fclose (file);
    return NULL;
  }

  PDEBUG ("Parse file header: %s", buf);

  p = buf;
  line = strsep (&p, "]");
  if (p == NULL) {

    PERROR ("wrong file header");
    fclose (file);
    return NULL;
  }

  did = strchr (line, '[');
  if (did == NULL) {

    PERROR ("wrong file header");
    fclose (file);
    return NULL;
  }

  did++;
  vid = strsep (&did, "-");
  if ( (vid == NULL) || (did == NULL)) {

    PERROR ("wrong file header");
    fclose (file);
    return NULL;
  }

  // Insure it matches
  if ( (vendor_id) && (device_id)) {
    if ( (strcasecmp (vid, vendor_id) != 0)
         || (strcasecmp (did, device_id) != 0)) {

      PWARNING ("file header does not match this device");
      fclose (file);
      return NULL;
    }
    PDEBUG ("file header match this device");
  }

  values = malloc (sizeof (xVector));
  iVectorInit (values, 2, NULL, gxPLPairDelete);
  iVectorInitSearch (values, gxPLPairKey, gxPLPairMatch);

  // Read in each line and pick it apart into an NV pair
  while (fgets (buf, sizeof (buf), file)) {

    gxPLPair * pair = gxPLPairFromLine (buf);
    if (pair) {

      PDEBUG ("read line: %s=%s", pair->name, (pair->value ? pair->value : ""));
      iVectorAppend (values, pair);
    }
  }
  // Close the file
  fclose (file);

  // And we are done
  return values;
}

// -----------------------------------------------------------------------------
// Save out the current configuration
int
gxPLDeviceConfigSave (const gxPLDevice * device) {
  FILE *file;

  // Skip unless we have a local config file
  if (device->config->filename == NULL) {
    return -1;
  }

  // Attempt to create the configuration file
  if ( (file = fopen (device->config->filename, "w")) == NULL) {

    PERROR ("File %s could not be opened/created -- %s (%d)",
            device->config->filename,
            strerror (errno), errno);

    return -1;
  }

  // write simple header data
  fprintf (file, "[%s-%s]\n", device->id.vendor, device->id.device);
  fprintf (file, "reconf=newconf\n");
  fprintf (file, "reconf=interval\n");

#if CONFIG_DEVICE_GROUP
  fprintf (file, "option=group[%d]\n", device->group_max);
#endif
#if CONFIG_DEVICE_FILTER
  fprintf (file, "option=filter[%d]\n", device->filter_max);
#endif

  // write configurable headers
  for (int i = 0; i < iVectorSize (&device->config->items); i++) {

    gxPLDeviceConfigItem * item = pvVectorGet (&device->config->items, i);
    if (item) {

      const char * type;

      // Write out configurable type
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
          continue;
      }

      // See if there are multiple values allowed
      // Write item name
      if (item->values_max > 1) {

        fprintf (file, "%s=%s[%d]\n", type, item->name, item->values_max);
      }
      else {

        fprintf (file, "%s=%s\n", type, item->name);
      }
    }
  }

  // Now write current values out
  fprintf (file, "newconf=%s\n", device->id.instance);
  fprintf (file, "interval=%d\n", device->hbeat_interval / 60);

#if CONFIG_DEVICE_GROUP
  // Write groups, if any
  if (device->havegroup) {

    for (int i = 0; i < iVectorSize (&device->group); i++) {

      char * group = (char *) pvVectorGet (&device->group, i);
      fprintf (file, "group=xpl-group.%s\n", group);
    }
  }
  else {

    fprintf (file, "group=\n");
  }
#endif

#if CONFIG_DEVICE_FILTER
  // Write filters, if any
  if (device->havefilter) {

    for (int i = 0; i < iVectorSize (&device->filter); i++) {

      gxPLFilter * filter = (gxPLFilter *) pvVectorGet (&device->filter, i);
      fprintf (file, "filter=%s\n", gxPLDeviceFilterToString (filter));
    }
  }
  else {

    fprintf (file, "filter=\n");
  }
#endif

  // Write configurable values
  for (int i = 0; i < iVectorSize (&device->config->items); i++) {

    gxPLDeviceConfigItem * item = pvVectorGet (&device->config->items, i);
    if (item) {
      for (int v = 0; v < iVectorSize (&item->values); v++) {

        char * value =  pvVectorGet (&item->values, v);
        fprintf (file, "%s=%s\n", item->name, value);
      }
    }
  }

  // And we are done
  fflush (file);
  fclose (file);
  PINFO ("%s config file sucessfully saved", device->config->filename);
  return 0;
}

#endif /* __unix__ defined */
/* ========================================================================== */
