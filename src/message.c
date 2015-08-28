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
#include <sys/socket.h>

#include "message_p.h"
#include "service_p.h"
#include "io_p.h"

/* public variables ========================================================= */
char messageBuff[CONFIG_MSG_BUFF_SIZE];

/* Caches */
itemCache * MessageCache = NULL;
int totalMessageAlloc = 0;

/* private variables ======================================================== */
static int messageBytesWritten;

/* macros =================================================================== */
#define WRITE_TEXT(x) do { if (!appendText(x)) return FALSE; } while(0)
#define CONFIRM_CACHE_OK do { if (MessageCache == NULL) MessageCache = xPL_AllocItemCache(); } while(0)

/* static functions ========================================================= */

/* -----------------------------------------------------------------------------
 * Append text and keep track of what we've used */
static bool 
appendText (char * theText) {
  int textLen = strlen (theText);

  /* Make sure this will fit in the buffer */
  if ( (messageBytesWritten + textLen) >= MSG_MAX_SIZE) {
    xPL_Debug ("Message exceeds MSG_MAX_SIZE (%d) -- not sent!", MSG_MAX_SIZE);
    xPL_Debug ("** Too Long Message is, to date [%s]", messageBuff);
    return FALSE;
  }

  /* Copy the text in */
  memcpy (&messageBuff[messageBytesWritten], theText, textLen);
  messageBytesWritten += textLen;
  messageBuff[messageBytesWritten] = '\0';
  return TRUE;
}

/* Convert a binary value in xPL encoded data */
static bool writeBinaryValue (char * theData, int dataLen) {
  int dataIndex;

  for (dataIndex = 0; dataIndex < dataLen; dataIndex++) {
    WRITE_TEXT (xPL_intToHex (theData[dataIndex]));
  }

  return TRUE;
}


/* -----------------------------------------------------------------------------
 * Create a new message based on a service */
static xPL_Message * 
createSendableMessage (xPL_Service * theService, xPL_MessageType messageType) {
  xPL_Message * theMessage;

  /* Allocate the message */
  theMessage = xPL_AllocMessage();

  /* Set the version (NOT DYNAMIC) */
  theMessage->messageType = messageType;
  theMessage->hopCount = 1;
  theMessage->receivedMessage = FALSE;

  theMessage->sourceVendor = xPL_getServiceVendor (theService);
  theMessage->sourceDeviceID = xPL_getServiceDeviceID (theService);
  theMessage->sourceInstanceID = xPL_getServiceInstanceID (theService);

  /* And we are done */
  return theMessage;
}

/* public functions ========================================================= */


/* -----------------------------------------------------------------------------
 * Public
 */
xPL_MessageType xPL_getMessageType (xPL_Message * theMessage) {
  return theMessage->messageType;
}

/* -----------------------------------------------------------------------------
 * Public
 */
int xPL_getHopCount (xPL_Message * theMessage) {
  return theMessage->hopCount;
}

/* -----------------------------------------------------------------------------
 * Public
 */
bool xPL_isBroadcastMessage (xPL_Message * theMessage) {
  return theMessage->isBroadcastMessage;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getTargetGroup (xPL_Message * theMessage) {
  return theMessage->groupName;
}

/* -----------------------------------------------------------------------------
 * Public
 */
bool xPL_isGroupMessage (xPL_Message * theMessage) {
  return theMessage->isGroupMessage;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getTargetVendor (xPL_Message * theMessage) {
  return theMessage->targetVendor;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getTargetDeviceID (xPL_Message * theMessage) {
  return theMessage->targetDeviceID;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getTargetInstanceID (xPL_Message * theMessage) {
  return theMessage->targetInstanceID;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getSourceVendor (xPL_Message * theMessage) {
  return theMessage->sourceVendor;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getSourceDeviceID (xPL_Message * theMessage) {
  return theMessage->sourceDeviceID;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getSourceInstanceID (xPL_Message * theMessage) {
  return theMessage->sourceInstanceID;
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setSchemaClass (xPL_Message * theMessage, char * theSchemaClass) {
  /* Skip unless a real change */
  if ( (theMessage->schemaClass != NULL) && !xPL_strcmpIgnoreCase (theMessage->schemaClass, theSchemaClass)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->schemaClass);
  theMessage->schemaClass = xPL_StrDup (theSchemaClass);
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getSchemaClass (xPL_Message * theMessage) {
  return theMessage->schemaClass;
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setSchemaType (xPL_Message * theMessage, char * theSchemaType) {
  /* Skip unless a real change */
  if ( (theMessage->schemaType != NULL) && !xPL_strcmpIgnoreCase (theMessage->schemaType, theSchemaType)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->schemaType);
  theMessage->schemaType = xPL_StrDup (theSchemaType);
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getSchemaType (xPL_Message * theMessage) {
  return theMessage->schemaType;
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setSchema (xPL_Message * theMessage, char * theSchemaClass, char * theSchemaType) {
  xPL_setSchemaClass (theMessage, theSchemaClass);
  xPL_setSchemaType (theMessage, theSchemaType);
}

/* -----------------------------------------------------------------------------
 * Public
 */
xPL_NameValueList * xPL_getMessageBody (xPL_Message * theMessage) {
  if (theMessage->messageBody == NULL) {
    return NULL;
  }
  return theMessage->messageBody;
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getMessageNamedValue (xPL_Message * theMessage, char * theName) {
  if (theMessage->messageBody == NULL) {
    return NULL;
  }
  return xPL_getNamedValue (theMessage->messageBody, theName);
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_addMessageNamedValue (xPL_Message * theMessage, char * theName, char * theValue) {
  if (theMessage->messageBody == NULL) {
    theMessage->messageBody = xPL_newNamedValueList();
  }
  xPL_addNamedValue (theMessage->messageBody, theName, theValue);
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setMessageNamedValue (xPL_Message * theMessage, char * theName, char * theValue) {
  if (theMessage->messageBody == NULL) {
    xPL_addMessageNamedValue (theMessage, theName, theValue);
    return;
  }

  xPL_setNamedValue (theMessage->messageBody, theName, theValue);
}

/* -----------------------------------------------------------------------------
 * Public
 * Set a series of NameValue pairs for a message
 */
void xPL_setMessageNamedValues (xPL_Message * theMessage, ...) {
  va_list argPtr;
  char * theName;
  char * theValue;

  /* Handle the name/value pairs */
  va_start (argPtr, theMessage);
  for (;;) {
    /* Get the name.  NULL means End of List */
    if ( (theName = va_arg (argPtr, char *)) == NULL) {
      break;
    }

    /* Get the value */
    theValue = va_arg (argPtr, char *);

    /* Create a name/value pair */
    xPL_setMessageNamedValue (theMessage, theName, theValue);
  }
  va_end (argPtr);
}


/* -----------------------------------------------------------------------------
 * Public
 * Create a message suitable for sending to a specific receiver
 */
xPL_Message * xPL_createTargetedMessage (xPL_Service * theService, xPL_MessageType messageType,
    char * theVendor, char * theDevice, char * theInstance) {
  xPL_Message * theMessage = createSendableMessage (theService, messageType);
  xPL_setTarget (theMessage, theVendor, theDevice, theInstance);
  return theMessage;
}

/* -----------------------------------------------------------------------------
 * Public
 * Create a message suitable for broadcasting to all listeners
 */
xPL_Message * xPL_createBroadcastMessage (xPL_Service * theService, xPL_MessageType messageType) {
  xPL_Message * theMessage = createSendableMessage (theService, messageType);
  xPL_setBroadcastMessage (theMessage, TRUE);
  return theMessage;
}

/* -----------------------------------------------------------------------------
 * Public
 * Release a message and all it's resources
 */
void xPL_releaseMessage (xPL_Message * theMessage) {
  xPL_Debug ("Releasing message, TYPE=%d, RECEIVED=%d", theMessage->messageType, theMessage->receivedMessage);

  /* Free Parsed stuff */
  if (theMessage->receivedMessage) {
    STR_FREE (theMessage->sourceVendor);
    STR_FREE (theMessage->sourceDeviceID);
    STR_FREE (theMessage->sourceInstanceID);
    xPL_Debug ("Releasing received messages SOURCE parameters");
  }
  else {
    theMessage->sourceVendor = NULL;
    theMessage->sourceDeviceID = NULL;
    theMessage->sourceInstanceID = NULL;
    xPL_Debug ("NULLing out transmitted messages SOURCE parameters");
  }

  theMessage->isBroadcastMessage = FALSE;
  STR_FREE (theMessage->targetVendor);
  STR_FREE (theMessage->targetDeviceID);
  STR_FREE (theMessage->targetInstanceID);

  theMessage->isGroupMessage = FALSE;
  STR_FREE (theMessage->groupName);

  STR_FREE (theMessage->schemaClass);
  STR_FREE (theMessage->schemaType);

  xPL_clearAllNamedValues (theMessage->messageBody);

  /* Clear header info */
  theMessage->messageType = xPL_MESSAGE_ANY;
  theMessage->hopCount = 0;
  theMessage->receivedMessage = FALSE;

  /* Free Message */
  xPL_FreeMessage (theMessage);
}

/* -----------------------------------------------------------------------------
 * Public
 * Write out the message
 */
char * xPL_formatMessage (xPL_Message * theMessage) {
  xPL_NameValueList * nvList = xPL_getMessageBody (theMessage);
  xPL_NameValuePair * nvPair = NULL;
  int nvIndex = 0;
  int nvCount = xPL_getNamedValueCount (nvList);

  /* Init the write buffer */
  messageBytesWritten = 0;

  /* Write header */
  switch (theMessage->messageType) {
    case xPL_MESSAGE_COMMAND:
      WRITE_TEXT ("xpl-cmnd");
      break;
    case xPL_MESSAGE_STATUS:
      WRITE_TEXT ("xpl-stat");
      break;
    case xPL_MESSAGE_TRIGGER:
      WRITE_TEXT ("xpl-trig");
      break;
    default:
      xPL_Debug ("Unable to format message -- invalid/unknown message type %d", theMessage->messageType);
      return NULL;
  }

  /* Write hop and source info */
  WRITE_TEXT ("\n{\nhop=1\nsource=");
  WRITE_TEXT (xPL_getSourceVendor (theMessage));
  WRITE_TEXT ("-");
  WRITE_TEXT (xPL_getSourceDeviceID (theMessage));
  WRITE_TEXT (".");
  WRITE_TEXT (xPL_getSourceInstanceID (theMessage));
  WRITE_TEXT ("\n");

  /* Write target */
  if (xPL_isBroadcastMessage (theMessage)) {
    WRITE_TEXT ("target=*");
  }
  else {
    if (xPL_isGroupMessage (theMessage)) {
      WRITE_TEXT ("target=XPL-GROUP.");
      WRITE_TEXT (xPL_getTargetGroup (theMessage));
    }
    else {
      WRITE_TEXT ("target=");
      WRITE_TEXT (xPL_getTargetVendor (theMessage));
      WRITE_TEXT ("-");
      WRITE_TEXT (xPL_getTargetDeviceID (theMessage));
      WRITE_TEXT (".");
      WRITE_TEXT (xPL_getTargetInstanceID (theMessage));
    }
  }
  WRITE_TEXT ("\n}\n");

  /* Write the schema out */
  WRITE_TEXT (xPL_getSchemaClass (theMessage));
  WRITE_TEXT (".");
  WRITE_TEXT (xPL_getSchemaType (theMessage));
  WRITE_TEXT ("\n{\n");

  /* Write Name/Value Pairs out */
  for (nvIndex = 0; nvIndex < nvCount; nvIndex++) {
    nvPair = xPL_getNamedValuePairAt (nvList, nvIndex);
    WRITE_TEXT (nvPair->itemName);
    WRITE_TEXT ("=");

    /* Write data content out */
    if (nvPair->itemValue != NULL) {
      if (nvPair->isBinary) {
        writeBinaryValue (nvPair->itemValue, nvPair->binaryLength);
      }
      else {
        WRITE_TEXT (nvPair->itemValue);
      }
    }

    /* Terminate line/entry */
    WRITE_TEXT ("\n");
  }

  /* Write message terminator */
  WRITE_TEXT ("}\n");

  /* Terminate and return text */
  messageBuff[messageBytesWritten] = '\0';
  return messageBuff;
}

/* -----------------------------------------------------------------------------
 * Public
 * Send an xPL message.  If the message is valid and is successfully sent,
 * TRUE is returned.
 */
bool xPL_sendMessage (xPL_Message * theMessage) {
  /* Write the message to text */
  if (xPL_formatMessage (theMessage) == NULL) {
    return FALSE;
  }

  /* Attempt to brodcast it */
  xPL_Debug ("About to broadcast %d bytes as [%s]", messageBytesWritten, messageBuff);
  if (!xPL_sendRawMessage (messageBuff, messageBytesWritten)) {
    return FALSE;
  }

  /* And we are done */
  return TRUE;
}


/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setMessageType (xPL_Message * theMessage, xPL_MessageType messageType) {
  if (theMessage->messageType == messageType) {
    return;
  }
  theMessage->messageType = messageType;
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
bool xPL_isReceivedMessage (xPL_Message * theMessage) {
  return theMessage->receivedMessage;
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setBroadcastMessage (xPL_Message * theMessage, bool isBroadcast) {
  if (theMessage->isBroadcastMessage == isBroadcast) {
    return;
  }
  theMessage->isBroadcastMessage = isBroadcast;
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setTargetGroup (xPL_Message * theMessage, char * theGroup) {
  if (theGroup == NULL) {
    STR_FREE (theMessage->groupName);
    theMessage->isGroupMessage = FALSE;
  }
  else {
    if ( (theMessage->groupName != NULL) && !xPL_strcmpIgnoreCase (theMessage->groupName, theGroup)) {
      return;
    }
    STR_FREE (theMessage->targetVendor);
    STR_FREE (theMessage->targetDeviceID);
    STR_FREE (theMessage->targetInstanceID);
    STR_FREE (theMessage->groupName);

    theMessage->isGroupMessage = TRUE;
    theMessage->groupName = xPL_StrDup (theGroup);
  }
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setTargetVendor (xPL_Message * theMessage, char * theVendor) {
  /* Skip unless a real change */
  if ( (theMessage->targetVendor != NULL) && !xPL_strcmpIgnoreCase (theMessage->targetVendor, theVendor)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->targetVendor);
  theMessage->targetVendor = xPL_StrDup (theVendor);

  /* Clear any group */
  if (theMessage->isGroupMessage) {
    STR_FREE (theMessage->groupName);
    theMessage->isGroupMessage = FALSE;
  }
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setTargetDeviceID (xPL_Message * theMessage, char * theDeviceID) {
  /* Skip unless a real change */
  if ( (theMessage->targetDeviceID != NULL) && !xPL_strcmpIgnoreCase (theMessage->targetDeviceID, theDeviceID)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->targetDeviceID);
  theMessage->targetDeviceID = xPL_StrDup (theDeviceID);

  /* Clear any group */
  if (theMessage->isGroupMessage) {
    STR_FREE (theMessage->groupName);
    theMessage->isGroupMessage = FALSE;
  }
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setTargetInstanceID (xPL_Message * theMessage, char * theInstanceID) {
  /* Skip unless a real change */
  if ( (theMessage->targetInstanceID != NULL) && !xPL_strcmpIgnoreCase (theMessage->targetInstanceID, theInstanceID)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->targetInstanceID);
  theMessage->targetInstanceID = xPL_StrDup (theInstanceID);

  /* Clear any group */
  if (theMessage->isGroupMessage) {
    STR_FREE (theMessage->groupName);
    theMessage->isGroupMessage = FALSE;
  }
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setTarget (xPL_Message * theMessage, char * theVendor, char * theDeviceID, char * theInstanceID) {
  xPL_setTargetVendor (theMessage, theVendor);
  xPL_setTargetDeviceID (theMessage, theDeviceID);
  xPL_setTargetInstanceID (theMessage, theInstanceID);
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setSourceVendor (xPL_Message * theMessage, char * theVendor) {
  /* Skip unless a real change and this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) {
    return;
  }
  if ( (theMessage->sourceVendor != NULL) && !xPL_strcmpIgnoreCase (theMessage->sourceVendor, theVendor)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->sourceVendor);
  theMessage->sourceVendor = xPL_StrDup (theVendor);
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setSourceDeviceID (xPL_Message * theMessage, char * theDeviceID) {
  /* Skip unless a real change and this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) {
    return;
  }
  if ( (theMessage->sourceDeviceID != NULL) && !xPL_strcmpIgnoreCase (theMessage->sourceDeviceID, theDeviceID)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->sourceDeviceID);
  theMessage->sourceDeviceID = xPL_StrDup (theDeviceID);
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setSourceInstanceID (xPL_Message * theMessage, char * theInstanceID) {
  /* Skip unless a real change and this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) {
    return;
  }
  if ( (theMessage->sourceInstanceID != NULL) && !xPL_strcmpIgnoreCase (theMessage->sourceInstanceID, theInstanceID)) {
    return;
  }

  /* Install new value */
  STR_FREE (theMessage->sourceInstanceID);
  theMessage->sourceInstanceID = xPL_StrDup (theInstanceID);
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_setSource (xPL_Message * theMessage, char * theVendor, char * theDeviceID, char * theInstanceID) {
  /* Skip unless this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) {
    return;
  }

  xPL_setSourceVendor (theMessage, theVendor);
  xPL_setSourceDeviceID (theMessage, theDeviceID);
  xPL_setSourceInstanceID (theMessage, theInstanceID);
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
void xPL_clearMessageNamedValues (xPL_Message * theMessage) {
  xPL_clearAllNamedValues (theMessage->messageBody);
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 */
bool xPL_doesMessageNamedValueExist (xPL_Message * theMessage, char * theName) {
  if (theMessage->messageBody == NULL) {
    return FALSE;
  }
  return xPL_doesNamedValueExist (theMessage->messageBody, theName);
}
/* -----------------------------------------------------------------------------
 * Public: Not used
 * Create a message suitable for sending to a group
 */
xPL_Message * xPL_createGroupTargetedMessage (xPL_Service * theService, xPL_MessageType messageType, char * theGroup) {
  xPL_Message * theMessage = createSendableMessage (theService, messageType);
  xPL_setTargetGroup (theMessage, theGroup);
  return theMessage;
}

/* private functions ======================================================== */

/* -----------------------------------------------------------------------------
 * Private
 */
void 
xPL_FreeMessage (xPL_Message * theMessage) {
  
  CONFIRM_CACHE_OK;
  xPL_Debug ("STORE:: Relesing xPL_Message @ %p back to cache pool", theMessage);
  xPL_ReleaseItem (theMessage, MessageCache);
}

/* -----------------------------------------------------------------------------
 * Private
 */
xPL_Message * 
xPL_AllocMessage (void) {
  xPL_Message * theMessage;

  CONFIRM_CACHE_OK;
  if ( (theMessage = (xPL_Message *) xPL_AllocItem (MessageCache)) == NULL) {
    theMessage = malloc (sizeof (xPL_Message));
    bzero (theMessage, sizeof (xPL_Message));
    xPL_Debug ("STORE:: Allocated new xPL_Message, now %d messages allocated", ++totalMessageAlloc);
  }
  else {
    xPL_Debug ("STORE:: Reused xPL_Message @ %p from cache", theMessage);
  }

  return theMessage;
}

/* ========================================================================== */
