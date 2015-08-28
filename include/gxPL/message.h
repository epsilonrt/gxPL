/**
 * @file gxPL/message.h
 * xPL Messages
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_MESSAGE_HEADER_
#define _GXPL_MESSAGE_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup gxPLMessage Messages
 * @{
 */

/* structures =============================================================== */

/**
 * @brief Describe a received message
 */
typedef struct _xPL_Message {

  xPL_MessageType messageType;

  int hopCount;
  bool receivedMessage; /* TRUE if received, FALSE if being sent */

  char * sourceVendor;
  char * sourceDeviceID;
  char * sourceInstanceID;

  bool isGroupMessage;
  char * groupName;

  bool isBroadcastMessage;
  char * targetVendor;
  char * targetDeviceID;
  char * targetInstanceID;

  char * schemaClass;
  char * schemaType;

  xPL_NameValueList * messageBody;
} xPL_Message;

/* internal public functions ================================================ */
/**
 * @brief Create a message suitable for broadcasting to all listeners
 * @param theService
 * @param messageType
 * @return
 */
xPL_Message * xPL_createBroadcastMessage (xPL_Service * theService,
    xPL_MessageType messageType);

/**
 * @brief Create a message suitable for sending to a specific receiver
 * @param theService
 * @param messageType
 * @param theVendor
 * @param theDevice
 * @param theInstance
 * @return
 */
xPL_Message * xPL_createTargetedMessage (xPL_Service * theService,
    xPL_MessageType messageType,
    char * theVendor, char * theDevice, char * theInstance);

/**
 * @brief
 * @param theService
 * @param messageType
 * @param theGroup
 * @return
 */
xPL_Message * xPL_createGroupTargetedMessage (xPL_Service * theService,
    xPL_MessageType messageType, char * theGroup);

/**
 * @brief Release a message and all it's resources
 * @param theMessage
 */
void xPL_releaseMessage (xPL_Message * theMessage);

/**
 * @brief Write out the message
 * @param theMessage
 * @return
 */
char * xPL_formatMessage (xPL_Message * theMessage);

/**
 * @brief Send an xPL message
 * If the message is valid and is successfully sent, TRUE is returned
 * @param theMessage
 * @return
 */
bool xPL_sendMessage (xPL_Message * theMessage);

/**
 * @brief Process xPL messages and I/O process any pending messages and then
 * immediatly return. If theTimeout > 0 then we process any pending messages,
 * waiting up to theTimeout milliseconds and then return. If theTimeout is -1,
 * then we process messages and wait and do not return until gxPLib is stopped.
 * In all cases, if at lease one xPL message was processed during the duration
 * of this call, TRUE is returned.  otherwise, FALSE
 * If theTimeout is 0, then we
 * @param theTimeout
 * @return TRUE if at lease one xPL message was processed, otherwise, FALSE
 */
bool xPL_processMessages (int theTimeout);


/**
 * @brief
 * @param theMessage
 * @return
 */
bool xPL_isReceivedMessage (xPL_Message * theMessage);


/**
 * @brief
 * @param theMessage
 * @return
 */
bool xPL_isGroupMessage (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
int xPL_getHopCount (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param messageType
 */
void xPL_setMessageType (xPL_Message * theMessage, xPL_MessageType messageType);

/**
 * @brief
 * @param theMessage
 * @return
 */
xPL_MessageType xPL_getMessageType (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theSchemaClass
 * @param theSchemaType
 */
void xPL_setSchema (xPL_Message * theMessage,
                    char * theSchemaClass, char * theSchemaType);

/**
 * @brief
 * @param theMessage
 * @param theSchemaClass
 */
void xPL_setSchemaClass (xPL_Message * theMessage, char * theSchemaClass);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSchemaClass (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theSchemaType
 */
void xPL_setSchemaType (xPL_Message * theMessage, char * theSchemaType);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSchemaType (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theVendor
 */
void xPL_setSourceVendor (xPL_Message * theMessage, char * theVendor);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSourceVendor (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theDeviceID
 */
void xPL_setSourceDeviceID (xPL_Message * theMessage, char * theDeviceID);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSourceDeviceID (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theInstanceID
 */
void xPL_setSourceInstanceID (xPL_Message * theMessage, char * theInstanceID);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSourceInstanceID (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theVendor
 * @param theDeviceID
 * @param theInstanceID
 */
void xPL_setSource (xPL_Message * theMessage, char * theVendor, char * theDeviceID, char * theInstanceID);

/**
 * @brief
 * @param theMessage
 * @param isBroadcast
 */
void xPL_setBroadcastMessage (xPL_Message * theMessage, bool isBroadcast);

/**
 * @brief
 * @param theMessage
 * @return
 */
bool xPL_isBroadcastMessage (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theGroup
 */
void xPL_setTargetGroup (xPL_Message * theMessage, char * theGroup);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getTargetGroup (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theVendor
 */
void xPL_setTargetVendor (xPL_Message * theMessage, char * theVendor);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getTargetVendor (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theDeviceID
 */
void xPL_setTargetDeviceID (xPL_Message * theMessage, char * theDeviceID);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getTargetDeviceID (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theInstanceID
 */
void xPL_setTargetInstanceID (xPL_Message * theMessage, char * theInstanceID);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getTargetInstanceID (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theVendor
 * @param theDeviceID
 * @param theInstanceID
 */
void xPL_setTarget (xPL_Message * theMessage, char * theVendor, char * theDeviceID, char * theInstanceID);


/**
 * @brief
 * @param theMessage
 * @param theName
 * @param theValue
 */
void xPL_addMessageNamedValue (xPL_Message * theMessage, char * theName, char * theValue);

/**
 * @brief
 * @param theMessage
 * @param theName
 * @param theValue
 */
void xPL_setMessageNamedValue (xPL_Message * theMessage,
                               char * theName, char * theValue);

/**
 * @brief Set a series of NameValue pairs for a message
 * @param theMessage
 */
void xPL_setMessageNamedValues (xPL_Message * theMessage, ...);

/**
 * @brief
 * @param theMessage
 * @param theName
 * @return
 */
char * xPL_getMessageNamedValue (xPL_Message * theMessage, char * theName);

/**
 * @brief
 * @param theMessage
 * @return
 */
xPL_NameValueList * xPL_getMessageBody (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 */
void xPL_clearMessageNamedValues (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @param theName
 * @return
 */
bool xPL_doesMessageNamedValueExist (xPL_Message * theMessage, char * theName);


/**
 * @defgroup xPLMessageListener Listeners
 * Message Listener SupportService Listener Support
 * @{
 */

/* types ==================================================================== */
/**
 * @brief
 */
typedef void (* xPL_messageListener) (struct _xPL_Message *, xPL_Object *);

/* internal public functions ================================================ */

/**
 * @brief Add a message listener
 * @param theHandler
 * @param userValue
 */
void xPL_addMessageListener (xPL_messageListener theHandler, xPL_Object * userValue);

/**
 * @brief
 * @param theHandler
 * @return
 */
bool xPL_removeMessageListener (xPL_messageListener theHandler);


/**
 * @}
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_MESSAGE_HEADER_ defined */
