/**
 * @file message.c
 * xPL Message support functions
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sysio/log.h>

#include <gxPL/util.h>
#include "message_p.h"

/* macros =================================================================== */
/* constants ================================================================ */
#ifndef CONFIG_ALLOC_STR_GROW
#define CONFIG_ALLOC_STR_GROW  256
#endif

/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static void
prvPairDelete (void * pair) {
  gxPLPair * p = (gxPLPair *) pair;

  free (p->name);
  free (p->value);
  free (p);
}

// -----------------------------------------------------------------------------
static int
prvPairMatch (const void *key1, const void *key2) {

  return strcmp ( (const char *) key1, (const char *) key2);
}

// -----------------------------------------------------------------------------
static const void *
prvPairKey (const void * pair) {
  gxPLPair * p = (gxPLPair *) pair;

  return p->name;
}

// -----------------------------------------------------------------------------
static int
prvSetVendorId (gxPLMessageId * id, const char * vendor_id) {
  if (vendor_id == NULL) {
    errno = EFAULT;
    return -1;
  }

  if (strlen (vendor_id) > GXPL_VENDORID_MAX) {
    errno = EINVAL;
    return -1;
  }
  return gxPLStrCpy (id->vendor, vendor_id);
}

// -----------------------------------------------------------------------------
static int
prvSetDeviceId (gxPLMessageId * id, const char * device_id) {
  if (device_id == NULL) {
    errno = EFAULT;
    return -1;
  }

  if (strlen (device_id) > GXPL_DEVICEID_MAX) {
    errno = EINVAL;
    return -1;
  }
  return gxPLStrCpy (id->device, device_id);
}

// -----------------------------------------------------------------------------
static int
prvSetInstanceId (gxPLMessageId * id, const char * instance_id) {
  if (instance_id == NULL) {
    errno = EFAULT;
    return -1;
  }

  if (strlen (instance_id) > GXPL_INSTANCEID_MAX) {
    errno = EINVAL;
    return -1;
  }
  return gxPLStrCpy (id->instance, instance_id);
}

// -----------------------------------------------------------------------------
static int
prvSetId (gxPLMessageId * dst, const gxPLMessageId * src) {
  if (prvSetVendorId (dst, src->vendor) == 0) {
    if (prvSetDeviceId (dst, src->device) == 0) {
      return prvSetInstanceId (dst, src->instance);
    }
  }
  return -1;
}

// -----------------------------------------------------------------------------
static int
prvStrPrintf (char ** buf, int * buf_size, int index, const char * format, ...) {
  va_list ap;
  int buf_free = *buf_size - index - 1;
  int size;

  va_start (ap, format);
  // trying to write the string in the buffer...
  size = vsnprintf (& (*buf) [index], buf_free, format, ap);

  while (size >= buf_free) {

    // the buffer is too small, it reallocates memory...
    *buf_size += CONFIG_ALLOC_STR_GROW;
    buf_free  += CONFIG_ALLOC_STR_GROW;
    *buf = realloc (*buf, *buf_size);
    assert (buf);

    //  and try again !
    va_end (ap);
    va_start (ap, format);
    size = vsnprintf (& (*buf) [index], buf_free, format, ap);
  }

  va_end (ap);
  return size;
}

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
char *
gxPLMessageToString (const gxPLMessage * message) {
  char * buf;

  if (message) {
    const char * str;
    int index = 0;
    int buf_size = CONFIG_ALLOC_STR_GROW;
    buf = malloc (buf_size);
    assert (buf);


    // Write the header block
    switch (message->type) {

      case gxPLMessageCommand:
        str = "xpl-cmnd";
        break;

      case gxPLMessageStatus:
        str = "xpl-stat";
        break;

      case gxPLMessageTrigger:
        str = "xpl-trig";
        break;

      default:
        vLog (LOG_ERR,
              "Unable to format message -- invalid/unknown message type %d",
              message->type);
        free (buf);
        return NULL;
    }

    // Writes message type and begins the header block
    index += prvStrPrintf (&buf, &buf_size, index, "%s\n{\nhop=%d\n",
                           str, message->hop);
    // Writes the source
    const gxPLMessageId * n = gxPLMessageSourceIdGet (message);
    index += prvStrPrintf (&buf, &buf_size, index, "source=%s-%s.%s\n",
                           n->vendor, n->device, n->instance);
    // Writes the target and ends the header
    if (message->isbroadcast) {

      index += prvStrPrintf (&buf, &buf_size, index, "target=*\n}\n");
    }
    else {

      n = gxPLMessageTargetIdGet (message);
      index += prvStrPrintf (&buf, &buf_size, index, "target=%s-%s.%s\n}\n",
                             n->vendor, n->device, n->instance);
    }

    // Writes the schema and begins the body
    const gxPLMessageSchema * s = gxPLMessageSchemaGet (message);
    index += prvStrPrintf (&buf, &buf_size, index, "%s.%s\n{\n", s->class, s->type);

    // Writes the name/value pairs (body)
    const xVector * body = gxPLMessageBodyGetConst (message);
    for (int i = 0; i < iVectorSize (body); i++) {
      const gxPLPair * p = (const gxPLPair *) pvVectorGet (body, i);

      index += prvStrPrintf (&buf, &buf_size, index, "%s=%s\n", p->name, p->value);
    }

    // Ends the body and message
    index += prvStrPrintf (&buf, &buf_size, index, "}\n", s->class, s->type);
    // shorten the memory block to the strict minimum
    buf = realloc (buf, index + 1);
    return buf;
  }
  else {

    errno = EFAULT;
  }
  return NULL;
}

// -----------------------------------------------------------------------------
gxPLMessage *
gxPLMessageFromString (const char * str) {

  if (str) {
  }

  errno = EFAULT;
  return NULL;
}

// -----------------------------------------------------------------------------
gxPLMessage *
gxPLMessageNew (gxPLMessageType type) {
  gxPLMessage * message = calloc (1, sizeof (gxPLMessage));

  if (message) {

    if (iVectorInit (&message->body, NULL, prvPairDelete) == 0) {

      if (iVectorInitSearch (&message->body, prvPairKey, prvPairMatch) == 0) {

        message->hop = 1;
        message->type = type;
        return message;
      }
    }
    free (message);
  }
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLMessageDelete (gxPLMessage * message) {

  if (message) {

    int ret = iVectorDestroy (&message->body);
    free (message);
    return ret;
  }
  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageSchemaClassSet (gxPLMessage * message, const char * schema_class) {
  if ( (message == NULL) || (schema_class == NULL)) {
    errno = EFAULT;
    return -1;
  }
  if (strlen (schema_class) > GXPL_CLASS_MAX) {
    errno = EINVAL;
    return -1;
  }
  return gxPLStrCpy (message->schema.class, schema_class);
}

// -----------------------------------------------------------------------------
int
gxPLMessageSchemaTypeSet (gxPLMessage * message, const char * schema_type) {

  if ( (message == NULL) || (schema_type == NULL)) {
    errno = EFAULT;
    return -1;
  }
  if (strlen (schema_type) > GXPL_TYPE_MAX) {
    errno = EINVAL;
    return -1;
  }
  return gxPLStrCpy (message->schema.type, schema_type);
}

// -----------------------------------------------------------------------------
int
gxPLMessageSourceVendorIdSet (gxPLMessage * message, const char * vendor_id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }
  return prvSetVendorId (&message->source, vendor_id);
}

// -----------------------------------------------------------------------------
int
gxPLMessageSourceDeviceIdSet (gxPLMessage * message, const char * device_id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }
  return prvSetDeviceId (&message->source, device_id);
}

// -----------------------------------------------------------------------------
int
gxPLMessageSourceInstanceIdSet (gxPLMessage * message, const char * instance_id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }
  return prvSetInstanceId (&message->source, instance_id);
}


// -----------------------------------------------------------------------------
int
gxPLMessageTargetVendorIdSet (gxPLMessage * message, const char * vendor_id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }
  return prvSetVendorId (&message->target, vendor_id);
}

// -----------------------------------------------------------------------------
int
gxPLMessageTargetDeviceIdSet (gxPLMessage * message, const char * device_id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }
  return prvSetDeviceId (&message->target, device_id);
}

// -----------------------------------------------------------------------------
int
gxPLMessageTargetInstanceIdSet (gxPLMessage * message, const char * instance_id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }
  return prvSetInstanceId (&message->target, instance_id);
}

// -----------------------------------------------------------------------------
int
gxPLMessagePairAdd (gxPLMessage * message, const char * name, const char * value) {

  if ( (message == NULL) || (name == NULL) || (value == NULL)) {

    errno = EFAULT;
    return -1;
  }

  if (strlen (name) > GXPL_NAME_MAX) {

    errno = EINVAL;
  }
  else {
    gxPLPair * p = calloc (1, sizeof (gxPLPair));

    if (p) {
      p->name = malloc (strlen (name) + 1);
      if (p->name) {
        if (gxPLStrCpy (p->name, name) == 0) {

          p->value = malloc (strlen (value) + 1);
          if (p->value) {
            strcpy (p->value, value);
            if (iVectorAppend (&message->body, p) == 0) {
              return 0;
            }
          }
        }
      }
      prvPairDelete (p);
    }
  }

  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessagePairValueSet (gxPLMessage * message, const char * name, const char * value) {

  if (message == NULL) {

    errno = EFAULT;
    return -1;
  }

  if (strlen (name) > GXPL_NAME_MAX) {

    errno = EINVAL;
  }
  else {
    gxPLPair * p = pvVectorFindFirst (&message->body, name);

    if (p == NULL) {

      return gxPLMessagePairAdd (message, name, value);
    }

    if (strcmp (p->name, name) != 0) {

      if (strlen (p->name) != strlen (name)) {

        p->name = realloc (p->name, strlen (name));
        if (p->name == NULL) {

          return -1;
        }
      }

      if (gxPLStrCpy (p->name, name) != 0) {

        return -1;
      }
    }

    if (strcmp (p->value, value) != 0) {
      if (strlen (p->value) != strlen (value)) {

        p->value = realloc (p->value, strlen (value));
      }
      if (p->value) {

        strcpy (p->value, value);
        return 0;
      }
    }
  }
  return -1;
}

// -----------------------------------------------------------------------------
int gxPLMessagePairValuePrintf (gxPLMessage * message, const char * name,
                                const char * format, ...) {

  if (message) {
    va_list ap;

    va_start (ap, format);
    int size = vsnprintf (NULL, 0, format, ap);
    va_end (ap);

    char * value = malloc (size + 1);
    assert (value);

    va_start (ap, format);
    vsprintf (value, format, ap);
    va_end (ap);

    int ret = gxPLMessagePairValueSet (message, name, value);
    free (value);

    return ret;
  }

  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessagePairValuesSet (gxPLMessage * message, ...) {

  if (message) {
    va_list ap;
    char * name;
    char * value;

    va_start (ap, message);
    for (;;) {

      if ( (name = va_arg (ap, char *)) == NULL) {

        break;
      }

      value = va_arg (ap, char *);

      if (gxPLMessagePairValueSet (message, name, value) != 0) {

        return -1;
      }
    }
    va_end (ap);
    return 0;
  }

  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
char *
gxPLMessagePairValueGet (const gxPLMessage * message, const char * name) {

  if ( (message) && (name)) {

    gxPLPair * p = (gxPLPair *) pvVectorFindFirst (&message->body, name);
    if (p) {
      return p->value;
    }
  }
  else {

    errno = EFAULT;
  }

  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLMessagePairExist (const gxPLMessage * message, const char * name) {

  return gxPLMessagePairValueGet (message, name) != NULL;
}

// -----------------------------------------------------------------------------
int
gxPLMessageSourceSet (gxPLMessage * message, const char * vendor_id,
                      const char * device_id, const char * instance_id) {
  gxPLMessageId id;
  strcpy (id.vendor, vendor_id);
  strcpy (id.device, device_id);
  strcpy (id.instance, instance_id);

  return gxPLMessageSourceIdSet (message, &id);
}

// -----------------------------------------------------------------------------
gxPLMessageType
gxPLMessageTypeGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  return message->type;
}

// -----------------------------------------------------------------------------
int
gxPLMessageHopGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  return message->hop;
}

// -----------------------------------------------------------------------------
const char *
gxPLMessageTargetVendorIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->target.vendor;
}

// -----------------------------------------------------------------------------
const char *
gxPLMessageTargetDeviceIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->target.device;
}

// -----------------------------------------------------------------------------
const char *
gxPLMessageTargetInstanceIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->target.instance;
}

// -----------------------------------------------------------------------------
const gxPLMessageId *
gxPLMessageSourceIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return &message->source;
}

// -----------------------------------------------------------------------------
const gxPLMessageId *
gxPLMessageTargetIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return &message->target;
}

// -----------------------------------------------------------------------------
const char *
gxPLMessageSourceVendorIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->source.vendor;
}

// -----------------------------------------------------------------------------
const char *
gxPLMessageSourceDeviceIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->source.device;
}

// -----------------------------------------------------------------------------
const char *
gxPLMessageSourceInstanceIdGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->source.instance;
}

// -----------------------------------------------------------------------------
int
gxPLMessageBroadcastGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  return message->isbroadcast;
}

// -----------------------------------------------------------------------------
const gxPLMessageSchema *
gxPLMessageSchemaGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return &message->schema;

}

// -----------------------------------------------------------------------------
const char *
gxPLMessageSchemaClassGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->schema.class;
}

// -----------------------------------------------------------------------------
const char *
gxPLMessageSchemaTypeGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return NULL;
  }

  return message->schema.type;
}

// -----------------------------------------------------------------------------
int
gxPLMessageReceivedGet (const gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  return message->isreceived;
}

// -----------------------------------------------------------------------------
xVector *
gxPLMessageBodyGet (gxPLMessage * message) {

  if (message == NULL) {

    errno = EFAULT;
    return NULL;
  }

  return &message->body;
}

// -----------------------------------------------------------------------------
const xVector *
gxPLMessageBodyGetConst (const gxPLMessage * message) {

  return (const xVector *) gxPLMessageBodyGet ( (gxPLMessage *) message);
}

// -----------------------------------------------------------------------------
int
gxPLMessageSchemaSetAll (gxPLMessage * message, const char * schema_class,
                         const char * schema_type) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  if (gxPLMessageSchemaClassSet (message, schema_class) == 0) {
    return gxPLMessageSchemaTypeSet (message, schema_type);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
gxPLMessageSchemaSet (gxPLMessage * message, const gxPLMessageSchema * schema) {

  return gxPLMessageSchemaSetAll (message, schema->class, schema->type);
}


// -----------------------------------------------------------------------------
int
gxPLMessageTypeSet (gxPLMessage * message, gxPLMessageType type) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  message->type = type;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLMessageBroadcastSet (gxPLMessage * message, bool isBroadcast) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  message->isbroadcast = isBroadcast;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLMessageReceivedSet (gxPLMessage * message, bool isReceived) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  message->isreceived = isReceived;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLMessageHopSet (gxPLMessage * message, int hop) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  message->hop = hop;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLMessageHopInc (gxPLMessage * message) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  message->hop++;
  return 0;
}

// -----------------------------------------------------------------------------
int
gxPLMessageSourceIdSet (gxPLMessage * message, const gxPLMessageId * id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  return prvSetId (&message->source, id);
}

// -----------------------------------------------------------------------------
int
gxPLMessageTargetIdSet (gxPLMessage * message, const gxPLMessageId * id) {
  if (message == NULL) {
    errno = EFAULT;
    return -1;
  }

  return prvSetId (&message->target, id);
}

// -----------------------------------------------------------------------------
int
gxPLMessageTargetSet (gxPLMessage * message, const char * vendor_id,
                      const char * device_id, const char * instance_id) {
  gxPLMessageId id;
  strcpy (id.vendor, vendor_id);
  strcpy (id.device, device_id);
  strcpy (id.instance, instance_id);

  return gxPLMessageTargetIdSet (message, &id);
}

// -----------------------------------------------------------------------------
int
gxPLMessageBodyClear (gxPLMessage * message) {

  if (message == NULL) {

    errno = EFAULT;
    return -1;
  }

  return iVectorClear (&message->body);
}

// -----------------------------------------------------------------------------
int
gxPLMessageIdCmp (const gxPLMessageId * n1, const gxPLMessageId * n2) {
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
int
gxPLMessageSchemaCmp (const gxPLMessageSchema * s1, const gxPLMessageSchema * s2) {
  int ret = strcmp (s1->class, s2->class);
  if (ret == 0) {
    ret = strcmp (s1->type, s2->type);
  }
  return ret;

}

// -----------------------------------------------------------------------------
int gxPLMessageBodySize (const gxPLMessage * message) {
  const xVector * v = gxPLMessageBodyGetConst (message);
  if (v) {
    return v->size;
  }
  return -1;
}

/* ========================================================================== */
