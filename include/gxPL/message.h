/**
 * @file gxPL/message.h
 * xPL Messages
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_MESSAGE_HEADER_
#define _GXPL_MESSAGE_HEADER_

#include <sysio/vector.h>
#include <gxPL/defs.h>

__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup gxPLMessage Messages
 * @{
 */

/**
 * @brief Maximum number of characters allowed for vendor ID
 */
#define GXPL_VENDORID_MAX   8

/**
 * @brief Maximum number of characters allowed for device ID
 */
#define GXPL_DEVICEID_MAX   8

/**
 * @brief Maximum number of characters allowed for instance ID
 */
#define GXPL_INSTANCEID_MAX 16

/**
 * @brief Maximum number of characters allowed for schema class
 */
#define GXPL_CLASS_MAX      8

/**
 * @brief Maximum number of characters allowed for schema type
 */
#define GXPL_TYPE_MAX       8

/**
 * @brief Maximum number of characters allowed for a name of a name/value pair
 */
#define GXPL_NAME_MAX       16

/**
 * @brief Maximum number of hop count
 */
#define GXPL_HOP_MAX   9

/* structures =============================================================== */

/**
 * @brief Describe a source or destination
 */
typedef struct _gxPLMessageId {
  char vendor[GXPL_VENDORID_MAX + 1];
  char device[GXPL_DEVICEID_MAX + 1];
  char instance[GXPL_INSTANCEID_MAX + 1];
} gxPLMessageId;

/**
 * @brief Describe a schema
 */
typedef struct _gxPLMessageSchema {
  char class[GXPL_CLASS_MAX + 1];
  char type[GXPL_TYPE_MAX + 1];
} gxPLMessageSchema;

/**
 * @brief Describe a name=value pair
 */
typedef struct _gxPLPair {
  char * name;
  char * value;
} gxPLPair;

/* internal public functions ================================================ */

/**
 * @brief Create a new empty message
 *
 * @return  the message, NULL if an error occurs, in which case errno contains
 * the error code.
 */
gxPLMessage * gxPLMessageNew (gxPLMessageType type);

/**
 * @brief Release a message and all it's resources
 *
 * @param message
 * @return 0, -1 if an error occurs, in which case errno contains
 * the error code.
 */
int gxPLMessageDelete (gxPLMessage * message);

/**
 * @brief Returns xPL message as text
 * 
 * @param message
 * @return  xPL message as text, NULL if an error occurs, in which case errno 
 * contains the error code. this character buffer must be released after use.
 */
char * gxPLMessageToString (const gxPLMessage * message);

/**
 * @brief Convert a text message into a xPL message.
 * @param data
 * @return  the message, NULL if an error occurs, in which case errno contains
 * the error code.
 */
gxPLMessage * gxPLMessageFromString (const char * data);

/**
 * @brief Gets message type
 * @param message
 * @return
 */
gxPLMessageType gxPLMessageTypeGet (const gxPLMessage * message);

/**
 * @brief Sets message type
 * @param message
 * @param type
 */
int gxPLMessageTypeSet (gxPLMessage * message, gxPLMessageType type);

/**
 * @brief Gets hop count
 * @param message
 * @return
 */
int gxPLMessageHopGet (const gxPLMessage * message);

/**
 * @brief Sets hop count
 * @param message
 * @return
 */
int gxPLMessageHopSet (gxPLMessage * message, int hop);

/**
 * @brief Increments hop count
 * @param message
 * @return
 */
int gxPLMessageHopInc (gxPLMessage * message);

/**
 * @brief
 * @param message
 * @return
 */
const gxPLMessageId * gxPLMessageSourceIdGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @return
 */
const char * gxPLMessageSourceVendorIdGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @return
 */
const char * gxPLMessageSourceDeviceIdGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @return
 */
const char * gxPLMessageSourceInstanceIdGet (const gxPLMessage * message);

/**
 * @brief 
 * @param message
 * @param id
 * @return 
 */
int gxPLMessageSourceIdSet (gxPLMessage * message, const gxPLMessageId * id);

/**
 * @brief
 * @param message
 * @param vendor_id
 * @param device_id
 * @param instance_id
 */
int gxPLMessageSourceSet (gxPLMessage * message, const char * vendor_id,
                          const char * device_id, const char * instance_id);

/**
 * @brief
 * @param message
 * @param vendor_id
 */
int gxPLMessageSourceVendorIdSet (gxPLMessage * message, const char * vendor_id);

/**
 * @brief
 * @param message
 * @param device_id
 */
int gxPLMessageSourceDeviceIdSet (gxPLMessage * message, const char * device_id);

/**
 * @brief
 * @param message
 * @param instance_id
 */
int gxPLMessageSourceInstanceIdSet (gxPLMessage * message, const char * instance_id);

/**
 * @brief
 * @param message
 * @return
 */
const gxPLMessageId * gxPLMessageTargetIdGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @return
 */
const char * gxPLMessageTargetVendorIdGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @return
 */
const char * gxPLMessageTargetDeviceIdGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @return
 */
const char * gxPLMessageTargetInstanceIdGet (const gxPLMessage * message);

/**
 * @brief 
 * @param message
 * @param id
 * @return 
 */
int gxPLMessageTargetIdSet (gxPLMessage * message, const gxPLMessageId * id);

/**
 * @brief
 * @param message
 * @param vendor_id
 * @param device_id
 * @param instance_id
 */
int gxPLMessageTargetSet (gxPLMessage * message, const char * vendor_id,
                          const char * device_id, const char * instance_id);

/**
 * @brief
 * @param message
 * @param vendor_id
 */
int gxPLMessageTargetVendorIdSet (gxPLMessage * message, const char * vendor_id);

/**
 * @brief
 * @param message
 * @param device_id
 */
int gxPLMessageTargetDeviceIdSet (gxPLMessage * message, const char * device_id);

/**
 * @brief
 * @param message
 * @param instance_id
 */
int gxPLMessageTargetInstanceIdSet (gxPLMessage * message, const char * instance_id);

/**
 * @brief Gets the schema 
 * @param message
 * @return
 */
const gxPLMessageSchema * gxPLMessageSchemaGet (const gxPLMessage * message);

/**
 * @brief 
 * @param s1
 * @param s2
 * @return 
 */
int gxPLMessageSchemaCmp (const gxPLMessageSchema * s1, const gxPLMessageSchema * s2);

/**
 * @brief Gets the schema class
 * @param message
 * @return
 */
const char * gxPLMessageSchemaClassGet (const gxPLMessage * message);

/**
 * @brief Gets the schema type
 * @param message
 * @return
 */
const char * gxPLMessageSchemaTypeGet (const gxPLMessage * message);

/**
 * @brief 
 * @param message
 * @param schema
 * @return 
 */
int gxPLMessageSchemaSet (gxPLMessage * message, const gxPLMessageSchema * schema);

/**
 * @brief Sets the schema
 * @param message
 * @param schema_class
 * @param schema_type
 */
int gxPLMessageSchemaSetAll (gxPLMessage * message,
                          const char * schema_class, const char * schema_type);

/**
 * @brief Sets the schema class
 * @param message
 * @param schema_class
 */
int gxPLMessageSchemaClassSet (gxPLMessage * message, const char * schema_class);

/**
 * @brief Sets the schema type
 * @param message
 * @param schema_type
 */
int gxPLMessageSchemaTypeSet (gxPLMessage * message, const char * schema_type);

/**
 * @brief Returns body of message as a const vector of gxPLPair
 * @param message
 * @return
 */
const xVector * gxPLMessageBodyGetConst (const gxPLMessage * message);

/**
 * @brief Returns body of message as a vector of gxPLPair
 * @param message
 * @return
 */
xVector * gxPLMessageBodyGet (gxPLMessage * message);

/**
 * @brief
 * @param message
 * @param name
 * @return
 */
int gxPLMessageBodySize (const gxPLMessage * message);

/**
 * @brief Clear the body
 * @param message
 */
int gxPLMessageBodyClear (gxPLMessage * message);

/**
 * @brief
 * @param message
 * @param name
 * @return
 */
char * gxPLMessagePairValueGet (const gxPLMessage * message, const char * name);

/**
 * @brief
 * @param message
 * @param name
 * @return
 */
int gxPLMessagePairExist (const gxPLMessage * message, const char * name);


/**
 * @brief
 * @param message
 * @param name
 * @param value
 */
int gxPLMessagePairAdd (gxPLMessage * message, const char * name, const char * value);

/**
 * @brief
 * @param message
 * @param name
 * @param value
 */
int gxPLMessagePairValueSet (gxPLMessage * message, const char * name, const char * value);

/**
 * @brief
 * @param message
 * @param name
 * @param value
 */
int gxPLMessagePairValuePrintf (gxPLMessage * message, const char * name, const char * format, ...);

/**
 * @brief Set a series of NameValue pairs for a message
 * @param message
 */
int gxPLMessagePairValuesSet (gxPLMessage * message, ...);

/**
 * @brief
 * @param message
 * @return
 */
int gxPLMessageReceivedGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @param isBroadcast
 */
int gxPLMessageReceivedSet (gxPLMessage * message, bool isReceived);

/**
 * @brief
 * @param message
 * @return
 */
int gxPLMessageBroadcastGet (const gxPLMessage * message);

/**
 * @brief
 * @param message
 * @param isBroadcast
 */
int gxPLMessageBroadcastSet (gxPLMessage * message, bool isBroadcast);

/**
 * @brief 
 * @param n1
 * @param n2
 * @return 
 */
int gxPLMessageIdCmp (const gxPLMessageId * n1, const gxPLMessageId * n2);


/**
 * @}
 */

#ifndef __DOXYGEN__
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
#endif /* __DOXYGEN__ not defined */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_MESSAGE_HEADER_ defined */
