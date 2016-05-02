/**
 * @file 
 * High level interface to manage xPL messages (public header)
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_MESSAGE_HEADER_
#define _GXPL_MESSAGE_HEADER_

#include <gxPL/defs.h>

__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup gxPLMessageDoc Messages
 * gxPLMessage is a message circulating on xPL network.
 * @{
 */


/* internal public functions ================================================ */

/**
 * @brief Create a new empty message
 * 
 * All fields are set to zero except the hop count is set to 1 and the type that
 * is set with the value passed as parameter. The message should be released 
 * with gxPLMessageDelete after use.
 *
 * @param type the type of message
 * @return  the message, NULL if an error occurs
 */
gxPLMessage * gxPLMessageNew (gxPLMessageType type);

/**
 * @brief Release a message and all it's resources
 *
 * @param message pointer to the message
 */
void gxPLMessageDelete (gxPLMessage * message);

/**
 * @brief Returns xPL message as text
 * 
 * @param message pointer to the message
 * @return  xPL message as text, NULL if an error occurs. This character buffer 
 * must be released after use.
 */
char * gxPLMessageToString (const gxPLMessage * message);

/**
 * @brief Parse a list of lines as text  to extract a message
 * 
 * Parse the string, line per line. I do this because the bottom layer can make 
 * repeated calls with parts of the message. The smallest portion that can be 
 * passed is a line. Do not pass incomplete line ! \n
 * This function must be called again with the returned pointer as the message 
 * is not valid and is not in error.
 * 
 * @param message pointer to the message returned by a previous call or 
 * NULL if the first call.
 * @param lines the list of lines as text, this string is modified by the 
 * function and is no longer  valid after apple.
 * @return  the message, NULL if an error occurs
 */
gxPLMessage * gxPLMessageFromString (gxPLMessage * message, char * line);

/**
 * @brief Check if the passed message matches the passed filter
 * @param message pointer to the message
 * @param filter pointer to the filter
 * @return true, false, -1 if an error occurs
 */
int gxPLMessageFilterMatch (const gxPLMessage * message, const gxPLFilter * filter);

/**
 * @brief Gets message type
 * @param message pointer to the message
 * @return message type, -1 if an error occurs
 */
gxPLMessageType gxPLMessageTypeGet (const gxPLMessage * message);

/**
 * @brief Sets message type
 * @param message pointer to the message
 * @param type message type
 * @return 0, -1 if an error occurs
 */
int gxPLMessageTypeSet (gxPLMessage * message, gxPLMessageType type);

/**
 * @brief string from a message type
 * @param type message type
 * @return the string (xpl-cmnd, ...) or NULL if error occurs
 */
const char * gxPLMessageTypeToString (gxPLMessageType type);

/**
 * @brief message type from a string
 * @param str string that starts with 8 characters corresponding to xpl-cmnd, 
 * xpl gold xpl-stat-trig
 * @return the type, -1 if unknown
 */
gxPLMessageType gxPLMessageTypeFromString (const char * str);

/**
 * @brief Gets hop count
 * @param message pointer to the message
 * @return hop count, -1 if an error occurs
 */
int gxPLMessageHopGet (const gxPLMessage * message);

/**
 * @brief Sets hop count
 * @param message pointer to the message
 * @return hop hop count
 * @return 0, -1 if an error occurs
 */
int gxPLMessageHopSet (gxPLMessage * message, int hop);

/**
 * @brief Increments hop count
 * @param message pointer to the message
 * @return 0, -1 if an error occurs
 */
int gxPLMessageHopInc (gxPLMessage * message);

/**
 * @brief Source identifier
 * @param message pointer to the message
 * @return pointer to the id, must not be released.  NULL if an error occurs
 */
const gxPLId * gxPLMessageSourceIdGet (const gxPLMessage * message);

/**
 * @brief Source vendor identifier
 * @param message pointer to the message
 * @return pointer to the vendor id, must not be released. NULL if an error occurs
 */
const char * gxPLMessageSourceVendorIdGet (const gxPLMessage * message);

/**
 * @brief Source device identifier
 * @param message pointer to the message
 * @return pointer to the device id, must not be released. NULL if an error occurs
 */
const char * gxPLMessageSourceDeviceIdGet (const gxPLMessage * message);

/**
 * @brief Source instance identifier
 * @param message pointer to the message
 * @return pointer to the instance id, must not be released. NULL if an error occurs
 */
const char * gxPLMessageSourceInstanceIdGet (const gxPLMessage * message);

/**
 * @brief Sets source identifier
 * @param message pointer to the message
 * @param id pointer to the source identifier
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSourceIdSet (gxPLMessage * message, const gxPLId * id);

/**
 * @brief Sets source identifier
 * @param message pointer to the message
 * @param vendor_id pointer to the vendor id
 * @param device_id pointer to the device id
 * @param instance_id pointer to the instance id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSourceSet (gxPLMessage * message, const char * vendor_id,
                          const char * device_id, const char * instance_id);

/**
 * @brief Sets source vendor identifier
 * @param message pointer to the message
 * @param vendor_id pointer to the vendor id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSourceVendorIdSet (gxPLMessage * message, const char * vendor_id);

/**
 * @brief Sets source device identifier
 * @param message pointer to the message
 * @param device_id pointer to the device id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSourceDeviceIdSet (gxPLMessage * message, const char * device_id);

/**
 * @brief Sets source instance identifier
 * @param message pointer to the message
 * @param instance_id pointer to the instance id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSourceInstanceIdSet (gxPLMessage * message, const char * instance_id);

/**
 * @brief Target identifier
 * @param message pointer to the message
 * @return pointer to the id, must not be released.  NULL if an error occurs
 */
const gxPLId * gxPLMessageTargetIdGet (const gxPLMessage * message);

/**
 * @brief Target vendor identifier
 * @param message pointer to the message
 * @return pointer to the vendor id, must not be released. NULL if an error occurs
 */
const char * gxPLMessageTargetVendorIdGet (const gxPLMessage * message);

/**
 * @brief Target device identifier
 * @param message pointer to the message
 * @return pointer to the device id, must not be released. NULL if an error occurs
 */
const char * gxPLMessageTargetDeviceIdGet (const gxPLMessage * message);

/**
 * @brief Target instance identifier
 * @param message pointer to the message
 * @return pointer to the instance id, must not be released. NULL if an error occurs
 */
const char * gxPLMessageTargetInstanceIdGet (const gxPLMessage * message);

/**
 * @brief Sets target identifier
 * @param message pointer to the message
 * @param id pointer to the target identifier
 * @return 0, -1 if an error occurs
 */
int gxPLMessageTargetIdSet (gxPLMessage * message, const gxPLId * id);

/**
 * @brief Sets target identifier
 * @param message pointer to the message
 * @param vendor_id pointer to the vendor id
 * @param device_id pointer to the device id
 * @param instance_id pointer to the instance id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageTargetSet (gxPLMessage * message, const char * vendor_id,
                          const char * device_id, const char * instance_id);

/**
 * @brief Sets target vendor identifier
 * @param message pointer to the message
 * @param vendor_id pointer to the vendor id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageTargetVendorIdSet (gxPLMessage * message, const char * vendor_id);

/**
 * @brief Sets target device identifier
 * @param message pointer to the message
 * @param device_id pointer to the device id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageTargetDeviceIdSet (gxPLMessage * message, const char * device_id);

/**
 * @brief Sets target instance identifier
 * @param message pointer to the message
 * @param instance_id pointer to the instance id
 * @return 0, -1 if an error occurs
 */
int gxPLMessageTargetInstanceIdSet (gxPLMessage * message, const char * instance_id);

/**
 * @brief Gets the schema 
 * @param message pointer to the message
 * @return pointer to the schema, must not be released. NULL if an error occurs
 */
const gxPLSchema * gxPLMessageSchemaGet (const gxPLMessage * message);

/**
 * @brief Gets the schema class
 * @param message pointer to the message
 * @return pointer to the schema class, must not be released. NULL if an error occurs
 */
const char * gxPLMessageSchemaClassGet (const gxPLMessage * message);

/**
 * @brief Gets the schema type
 * @param message pointer to the message
 * @return pointer to the schema type, must not be released. NULL if an error occurs
 */
const char * gxPLMessageSchemaTypeGet (const gxPLMessage * message);

/**
 * @brief Sets the schema
 * @param message pointer to the message
 * @param schema pointer to the schema
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSchemaCopy (gxPLMessage * message, const gxPLSchema * schema);

/**
 * @brief Sets the schema
 * @param message pointer to the message
 * @param schema_class pointer to the schema class
 * @param schema_type pointer to the schema type
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSchemaSet (gxPLMessage * message,
                          const char * schema_class, const char * schema_type);

/**
 * @brief Sets the schema class
 * @param message pointer to the message
 * @param schema_class pointer to the schema class
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSchemaClassSet (gxPLMessage * message, const char * schema_class);

/**
 * @brief Sets the schema type
 * @param message pointer to the message
 * @param schema_type pointer to the schema type
 * @return 0, -1 if an error occurs
 */
int gxPLMessageSchemaTypeSet (gxPLMessage * message, const char * schema_type);

/**
 * @brief Returns body of message as a const vector of gxPLPair
 * @param message pointer to the message
 * @return pointer body of message as a const vector of gxPLPair, must not be 
 * released. NULL if an error occurs
 */
const xVector * gxPLMessageBodyGetConst (const gxPLMessage * message);

/**
 * @brief Returns body of message as a vector of gxPLPair
 * 
 * All added pairs will be released during the destruction of the message.
 * If a pair is changed, it will reallocate the memory of the modified parameter 
 * if it is longer.
 * 
 * @param message pointer to the message
 * @return pointer body of message as a vector of gxPLPair, must not be 
 * released. NULL if an error occurs
 */
xVector * gxPLMessageBodyGet (gxPLMessage * message);

/**
 * @brief Number of pairs of the body
 * @param message pointer to the message
 * @return the value, -1 if an error occurs
 */
int gxPLMessageBodySize (const gxPLMessage * message);

/**
 * @brief Clear, release a body and all it's resources
 * @param message pointer to the message
 * @return 0, -1 if an error occurs
 */
int gxPLMessageBodyClear (gxPLMessage * message);

/**
 * @brief Gets the value of a name/value pair
 * @param message pointer to the message
 * @param name the name
 * @return pointer to the schema type, must not be released. NULL if an error occurs
 */
const char * gxPLMessagePairGet (const gxPLMessage * message, const char * name);

/**
 * @brief Check if a pair exist
 * @param message pointer to the message
 * @param name the name
 * @return true if exists, false in the other cases, -1 if an error occurs
 */
int gxPLMessagePairExist (const gxPLMessage * message, const char * name);

/**
 * @brief Adds a pair to the body
 * @param message pointer to the message
 * @param name the name
 * @param value the value, If NULL is supplied, a zero-length string is assigned ("")
 * @return 0, -1 if an error occurs
 */
int gxPLMessagePairAdd (gxPLMessage * message, const char * name, const char * value);

/**
 * @brief Sets the value of a name/value pair
 * @param message pointer to the message
 * @param name the name
 * @param value the value, If NULL is supplied, a zero-length string is assigned ("")
 * @return 0, -1 if an error occurs
 */
int gxPLMessagePairSet (gxPLMessage * message, const char * name, const char * value);

/**
 * @brief Produce value according to a format 
 * @param message pointer to the message
 * @param name the name
 * @param format format as described in the sprintf() function man page
 * @return 0, -1 if an error occurs
 */
int gxPLMessagePairAddFormat (gxPLMessage * message, const char * name, const char * format, ...);

/**
 * @brief Sets the value according to a format 
 * @param message pointer to the message
 * @param name the name
 * @param format format as described in the sprintf() function man page
 * @return 0, -1 if an error occurs
 */
int gxPLMessagePairSetFormat (gxPLMessage * message, const char * name, const char * format, ...);

/**
 * @brief Set a series of NameValue pairs for a message
 * @param message pointer to the message
 * @return 0, -1 if an error occurs
 */
int gxPLMessagePairValuesSet (gxPLMessage * message, ...);

/**
 * @brief Check if a message is valid
 * @param message pointer to the message
 * @return true if valid, false if not, -1 if an error occurs
 */
int gxPLMessageIsValid (const gxPLMessage * message);

/**
 * @brief Check if a message is in error
 * @param message pointer to the message
 * @return true if error, false if not, -1 if an error occurs
 */
int gxPLMessageIsError (const gxPLMessage * message);

/**
 * @brief gets the decoding state 
 * @param message pointer to the message
 * @return the state, -1 if an error occurs
 */
gxPLMessageState gxPLMessageStateGet (const gxPLMessage * message);

/**
 * @brief Clear all flags
 * @param message pointer to the message
 * @return 0, -1 if an error occurs
 */
int gxPLMessageFlagClear (gxPLMessage * message);

/**
 * @brief Check if a message is received
 * @param message pointer to the message
 * @return true if received, false if not, -1 if an error occurs
 */
int gxPLMessageIsReceived (const gxPLMessage * message);

/**
 * @brief Sets if a message is received
 * @param message pointer to the message
 * @param isReceived the value
 * @return 0, -1 if an error occurs
 */
int gxPLMessageReceivedSet (gxPLMessage * message, bool isReceived);

/**
 * @brief Check if a message is broadcast
 * @param message pointer to the message
 * @return true if broadcast, false if not, -1 if an error occurs
 */
int gxPLMessageIsBroadcast (const gxPLMessage * message);

/**
 * @brief Sets if a message is grouped
 * @param message pointer to the message
 * @param isGrouped the value
 * @return 0, -1 if an error occurs
 */
int gxPLMessageGroupedSet (gxPLMessage * message, bool isGrouped);

/**
 * @brief Check if a message is for a group.
 * @param message pointer to the message
 * @return true, false, -1 if an error occurs
 */
int gxPLMessageIsGrouped (const gxPLMessage * message);

/**
 * @brief Check if a message is broadcast
 * @param message pointer to the message
 * @return true if broadcast, false if not, -1 if an error occurs
 */
int gxPLMessageIsBroadcast (const gxPLMessage * message);

/**
 * @brief Sets if a message is broadcast
 * @param message pointer to the message
 * @param isBroadcast the value
 * @return 0, -1 if an error occurs
 */
int gxPLMessageBroadcastSet (gxPLMessage * message, bool isBroadcast);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_MESSAGE_HEADER_ defined */
