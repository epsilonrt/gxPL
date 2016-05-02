/**
 * @file
 * Misc support for gxPLib
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <gxPL/util.h>

/* macros =================================================================== */
#ifndef MAX_CHAR_LEN_DECIMAL_INTEGER
#define MAX_CHAR_LEN_DECIMAL_INTEGER(type) (80*sizeof(type)/33 + 2)
#endif

/* public api functions ===================================================== */

// -----------------------------------------------------------------------------
// name=value\0
gxPLPair *
gxPLPairFromString (char * str) {

  if (str) {
    char * name;

    char * value = str;

    name = strsep (&value, "=");
    if (value) {
      gxPLPair * p = malloc (sizeof (gxPLPair));
      assert (p);
      p->name = malloc (strlen (name) + 1);
      p->value = malloc (strlen (value) + 1);
      strcpy (p->name, name);
      strcpy (p->value, value);
      return p;
    }
    else {
      PERROR ("Unable to find '=' in %s", str);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------------------
gxPLPair *
gxPLPairFromLine (char * line) {
  char * p = strsep (&line, "\n");

  if (line) {
    // line found
    gxPLPair * pair = gxPLPairFromString (p);
    if (pair) {
      return pair;
    }
    PINFO ("unable to find a '=' in this line: %s", p);
  }
  return NULL;
}

// -----------------------------------------------------------------------------
void
gxPLPairDelete (void * pair) {
  if (pair) {
    gxPLPair * p = (gxPLPair *) pair;

    free (p->name);
    free (p->value);
    free (p);
  }
}

// -----------------------------------------------------------------------------
int
gxPLPairMatch (const void *key1, const void *key2) {

  return strcmp ( (const char *) key1, (const char *) key2);
}

// -----------------------------------------------------------------------------
const void *
gxPLPairKey (const void * pair) {
  gxPLPair * p = (gxPLPair *) pair;

  return p->name;
}

// -----------------------------------------------------------------------------
int
gxPLStrCpy (char * dst, const char * src) {
  int c;
  int count = 0;
  char * p = dst;

  while ( (c = *src) != 0) {

    if (isascii (c)) {

      if ( (!isalnum (c)) && (c != '-')) {

        // c is not a letter, number or hyphen/dash
        errno = EINVAL;
        return -1;
      }

      if (isupper (c)) {

        // convert to lowercase
        *p = tolower (c);
      }
      else {

        // raw copy
        *p = c;
      }

      p++;
      src++;
      count++;
    }
    else {

      // c is not a ASCII character
      errno = EINVAL;
      return -1;
    }
  }
  *p = '\0';
  return count;
}

// --------------------------------------------------------------------------
const char *
gxPLLongToStr (long value) {
  static char longBuffer[MAX_CHAR_LEN_DECIMAL_INTEGER (long) + 1];

  sprintf (longBuffer, "%ld", value);
  return longBuffer;
}

// --------------------------------------------------------------------------
const char *
gxPLDoubleToStr (double value, int precision) {
  static char doubleBuffer[MAX_CHAR_LEN_DECIMAL_INTEGER (int) + 8 + 2];

  precision = MIN (precision, 8);
  sprintf (doubleBuffer, "%.8f", value);
  if (precision < 8) {
    int len = strlen (doubleBuffer);
    
    doubleBuffer[len -8 + precision] = '\0';
  }
  return doubleBuffer;
}

// -----------------------------------------------------------------------------
int
gxPLIdSet (gxPLId * id, const char * vendor_id, const char * device_id, const char * instance_id) {

  if (gxPLIdVendorIdSet (id, vendor_id) == 0) {

    if (gxPLIdDeviceIdSet (id, device_id) == 0) {

      return gxPLIdInstanceIdSet (id, instance_id);
    }
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdVendorIdSet (gxPLId * id, const char * vendor_id) {

  if (vendor_id) {
    if (strlen (vendor_id) > GXPL_VENDORID_MAX) {

      errno = EINVAL;
      return -1;
    }
    if (strcmp (vendor_id, "*") == 0) {
      strcpy (id->vendor, "*");
      return 0;
    }
    return (gxPLStrCpy (id->vendor, vendor_id) > 0) ? 0 : -1;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdDeviceIdSet (gxPLId * id, const char * device_id) {

  if (device_id) {
    if (strlen (device_id) > GXPL_DEVICEID_MAX) {

      errno = EINVAL;
      return -1;
    }
    if (strcmp (device_id, "*") == 0) {
      strcpy (id->device, "*");
      return 0;
    }
    return (gxPLStrCpy (id->device, device_id) > 0) ? 0 : -1;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdInstanceIdSet (gxPLId * id, const char * instance_id) {

  if (instance_id) {
    if (strlen (instance_id) > GXPL_INSTANCEID_MAX) {

      errno = EINVAL;
      return -1;
    }
    if (strcmp (instance_id, "*") == 0) {
      strcpy (id->instance, "*");
      return 0;
    }
    return (gxPLStrCpy (id->instance, instance_id) > 0) ? 0 : -1;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdCopy (gxPLId * dst, const gxPLId * src) {

  if (gxPLIdVendorIdSet (dst, src->vendor) == 0) {

    if (gxPLIdDeviceIdSet (dst, src->device) == 0) {

      return gxPLIdInstanceIdSet (dst, src->instance);
    }
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLIdCmp (const gxPLId * n1, const gxPLId * n2) {

  int ret = strcmp (n1->vendor, n2->vendor);
  if (ret == 0) {

    ret = strcmp (n1->device, n2->device);
    if (ret == 0) {

      ret = strcmp (n1->instance, n2->instance);
    }
  }
  return ret;
}

// -----------------------------------------------------------------------------
// vendor-device.instance\0
int
gxPLIdFromString (gxPLId * id, char * str) {

  if ( (str) && (id)) {
    char *p, *n;

    n = str;
    p = strsep (&n, "-");
    if (n) {
      if (gxPLIdVendorIdSet (id, p) == 0) {

        p = strsep (&n, ".");
        if (n) {

          if (gxPLIdDeviceIdSet (id, p) == 0) {

            return gxPLIdInstanceIdSet (id, n);
          }
        }
      }

    }
  }
  errno = EINVAL;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaCmp (const gxPLSchema * s1, const gxPLSchema * s2) {

  int ret = strcmp (s1->class, s2->class);
  if (ret == 0) {

    ret = strcmp (s1->type, s2->type);
  }
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaMatch (const gxPLSchema * s, const char * schema_class,
                 const char * schema_type) {

  int ret = (strcmp (s->class, schema_class) == 0);
  if (ret == true) {

    ret = (strcmp (s->type, schema_type) == 0);
  }
  return ret;
}


// -----------------------------------------------------------------------------
int
gxPLSchemaClassSet (gxPLSchema * schema, const char * schema_class) {

  if (schema_class) {

    if (strlen (schema_class) > GXPL_CLASS_MAX) {
      errno = EINVAL;
      return -1;
    }
    if (strcmp (schema_class, "*") == 0) {
      strcpy (schema->class, "*");
      return 0;
    }
    return (gxPLStrCpy (schema->class, schema_class) > 0) ? 0 : -1;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaTypeSet (gxPLSchema * schema, const char * schema_type) {

  if (schema_type) {

    if (strlen (schema_type) > GXPL_TYPE_MAX) {
      errno = EINVAL;
      return -1;
    }
    if (strcmp (schema_type, "*") == 0) {
      strcpy (schema->type, "*");
      return 0;
    }
    return (gxPLStrCpy (schema->type, schema_type) > 0) ? 0 : -1;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaSet (gxPLSchema * schema, const char * schema_class,
               const char * schema_type) {
  int ret;

  if ( (ret = gxPLSchemaClassSet (schema, schema_class)) == 0) {

    return gxPLSchemaTypeSet (schema, schema_type);
  }
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaCopy (gxPLSchema * dst, const gxPLSchema * src) {
  int ret;

  if ( (ret = gxPLSchemaClassSet (dst, src->class)) == 0) {

    return gxPLSchemaTypeSet (dst, src->type);
  }
  return ret;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaIsEmpty (const gxPLSchema * schema) {

  if ( (strlen (schema->class) == 0) && (strlen (schema->type) == 0)) {

    return true;
  }
  return false;
}

// -----------------------------------------------------------------------------
int
gxPLSchemaFromString (gxPLSchema * schema, const char * str) {

  if ( (schema) && (str)) {
    if ( (strlen (str) < 18) && (strchr (str, '.'))) {
      char buffer[18];
      char * c;
      char * t = buffer;

      strcpy (buffer, str);
      c = strsep (&t, ".");

      if (gxPLSchemaClassSet (schema, c) == 0) {
        if (gxPLSchemaTypeSet (schema, t) == 0) {
          return 0;
        }
      }
    }
  }

  PERROR ("Unable to set schema from %s", str);
  return -1;
}

/* ========================================================================== */
