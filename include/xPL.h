#ifndef _XPL_HEADER_
#define _XPL_HEADER_

#include <stdint.h>
#include <stdarg.h>

#ifndef __DOXYGEN__
# if defined(__cplusplus)
extern "C" {
# else
#   include <stdbool.h>
# endif
# ifndef TRUE
#   define TRUE true
# endif
# ifndef FALSE
#   define FALSE false
# endif
struct _xPL_Service;
struct _xPL_Message;
#endif
/* ========================================================================== */

/**
 * @defgroup xPL4Linux Tools, Application and Framework for xPL on Linux
 * 
 * This is a C based xPL framework that hides most of the details of dealing 
 * with xPL.  It will handle filtering messages, sending heartbeats, formatting 
 * and parsing messages, directing messages to handlers based on where they were
 * bound from, etc.  You can use it as the simplest level to 
 * format/send/receive/parse xPL messages or add various layers of additional 
 * management to simplify more complex xPL idioms.  It's designed to be easy to
 * design a new program around or to integrate into an existing program.
 * It includes some example applications showing how to use various features 
 * of the framework, including the source to the xPLHub.
 * @{
 */

/* macros =================================================================== */

/* private variables ======================================================== */
/* private functions ======================================================== */
/* public variables ========================================================= */

/* constants ================================================================ */
#define XPL_PORT 3865


/**
 * @brief xPL Connection mode
 */
typedef enum {
  xcStandAlone,
  xcViaHub,
  xcAuto
} xPL_ConnectType;

/**
 * @brief
 */
typedef enum {
  xPL_CONFIG_OPTIONAL,
  xPL_CONFIG_MANDATORY,
  xPL_CONFIG_RECONF
} xPL_ConfigurableType;

/**
 * @brief Possible xPL message types
 */
typedef enum {
  xPL_MESSAGE_ANY,
  xPL_MESSAGE_COMMAND,
  xPL_MESSAGE_STATUS,
  xPL_MESSAGE_TRIGGER
} xPL_MessageType;


/* types ==================================================================== */
/**
 * @brief Generic xPL object, void for now, but could change in the future
 */
typedef void xPL_Object;

/**
 * @brief Changes to a services configuration
 */
typedef void (* xPL_ServiceConfigChangedListener) (struct _xPL_Service *, xPL_Object *);

/**
 * @brief Service Listener Support
 */
typedef void (* xPL_ServiceListener) (struct _xPL_Service *, struct _xPL_Message *, xPL_Object *);

/**
 * @brief Message Listener SupportService Listener Support
 */
typedef void (* xPL_messageListener) (struct _xPL_Message *, xPL_Object *);

/**
 * @brief Event management of user timeouts
 */
typedef void (* xPL_TimeoutHandler)(int, xPL_Object *);

/**
 * @brief Raw Listener Support
 */
typedef void (* xPL_rawListener)(char *, int, xPL_Object *);

/* structures =============================================================== */
/**
 * @brief A discrete name/value structure
 */
typedef struct _xPL_NameValuePair {

  char * itemName;
  char * itemValue;
  bool isBinary;
  int binaryLength;
} xPL_NameValuePair;

/**
 * @brief A list of name/value pairs
 */
typedef struct _xPL_NameValueList {

  int namedValueCount;
  int namedValueAlloc;
  xPL_NameValuePair **namedValues;
} xPL_NameValueList;

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

/**
 * @brief
 */
typedef struct _xPL_ServiceFilter {

  xPL_MessageType matchOnMessageType;
  char * matchOnVendor;
  char * matchOnDeviceID;
  char * matchOnInstanceID;
  char * matchOnSchemaClass;
  char * matchOnSchemaType;
} xPL_ServiceFilter;

/**
 * @brief
 */
typedef struct  _xPL_ServiceConfigurable {

  char * itemName;
  xPL_ConfigurableType itemType;
  int maxValueCount;

  int valueCount;
  int valueAllocCount;
  char **valueList;
} xPL_ServiceConfigurable;

/**
 * @brief
 */
typedef struct _xPL_ServiceListenerDef {

  xPL_MessageType matchMessageType;
  char *matchSchemaClass;
  char *matchSchemaType;
  xPL_Object * userValue;
  xPL_ServiceListener serviceListener;
} xPL_ServiceListenerDef;

/**
 * @brief
 */
typedef struct _xPL_ServiceChangedListenerDef {

  xPL_ServiceConfigChangedListener changeListener;
  xPL_Object * userValue;
} xPL_ServiceChangedListenerDef;

/**
 * @brief Describe a xPL service
 */
typedef struct _xPL_Service {
  bool serviceEnabled;

  char * serviceVendor;
  char * serviceDeviceID;
  char * serviceInstanceID;
  char * serviceVersion;

  int groupCount;
  int groupAllocCount;
  char **groupList;

  bool ignoreBroadcasts;

  int heartbeatInterval;
  time_t lastHeartbeatAt;
  xPL_Message * heartbeatMessage;

  bool configurableService;
  bool serviceConfigured;
  char * configFileName;
  int configChangedCount;
  int configChangedAllocCount;
  xPL_ServiceChangedListenerDef * changedListenerList;

  int configCount;
  int configAllocCount;
  xPL_ServiceConfigurable * configList;

  int filterCount;
  int filterAllocCount;
  xPL_ServiceFilter * messageFilterList;

  bool reportOwnMessages;
  int listenerCount;
  int listenerAllocCount;
  xPL_ServiceListenerDef * serviceListenerList;
} xPL_Service;

/* internal public functions ================================================ */

/*------------------------------------------------------------------------------
 *
 *                                xPL Library
 *
 *----------------------------------------------------------------------------*/

/**
 * @brief Parse options and parameters and sets xPL if found
 * This will parse the passed command array for options and parameters
 * and install them into xPL if found.  It supports the following switches:
 * * -interface x - Change the default interface xPLLib uses
 * * -xpldebug - enable xPLLib debugging
 * .
 *
 * This function will remove each recognized switch from the parameter
 * list so the returned arg list may be smaller than before. This generally
 * makes life easier for all involved.
 * @param argc
 * @param argv
 * @param silentErrors
 * @return If there is an error parsing the command line, FALSE is returned
 */
bool xPL_parseCommonArgs (int *argc, char *argv[], bool silentErrors);

/**
 * @brief Accessors for context
 * @return
 */
xPL_ConnectType xPL_getParsedConnectionType (void);

/**
 * @brief Attempt to start the xPL library
 * If we are already "started" then bail out
 * @param theConnectType
 * @return
 */
bool xPL_initialize (xPL_ConnectType theConnectType);

/**
 * @brief Stop the xPL library
 * If already stopped, bail.  Otherwise, we close our connection, release
 * any/all resources and reset
 * @return
 */
bool xPL_shutdown (void);

/* Return services heartbeat interval */
int xPL_getHeartbeatInterval(xPL_Service * theService);

const char * xPL_Version (void);
int xPL_VersionMajor (void);
int xPL_VersionMinor (void);
int xPL_VersionPatch (void);
int xPL_VersionSha1 (void);

/*------------------------------------------------------------------------------
 *
 *                                xPL Service
 *
 *----------------------------------------------------------------------------*/
/**
 * @brief Create a new xPL service
 *
 * @param theVendor
 * @param theDeviceID
 * @param theInstanceID
 * @return
 */
xPL_Service * xPL_createService (char * theVendor, char * theDeviceID,
                                 char * theInstanceID);
/**
 * @brief Create a new service and prepare it for configuration
 *
 * Like other services, this will still require being enabled to start.
 * Before it's started, you need to define and attach the configurable items
 * for the service.   When the service is enabled, if there is a non-null
 * configFile, it's values are read.  The services instance value will be
 * created in a fairly unique method for services that have not yet been
 * configured.
 *
 * @param vendorName
 * @param deviceID
 * @param localConfigFile
 * @return
 */
xPL_Service * xPL_createConfigurableService (char * vendorName, char * deviceID, char * localConfigFile);

/**
 * @brief
 * @param theService
 * @param theVersion
 */
void xPL_setServiceVersion (xPL_Service * theService, char * theVersion);

/* Clear out any/all installed filters */
void xPL_clearServiceFilters(xPL_Service * theService);

/* Clear out all groups */
void xPL_clearServiceGroups(xPL_Service * theService);

/* Add a service item value.  If there are already values */
/* this is added to it, up to the limit defined for the   */
/* configurable.  If the item is "full", then the value is */
/* discarded                                              */
bool xPL_addServiceConfigValue(xPL_Service * theService, char * itemName, char * itemValue);

/**
 * @brief Add a service listener
 * @param theService
 * @param theListener
 * @param messageType
 * @param schemaClass
 * @param schemaType
 * @param userValue
 */
void xPL_addServiceListener (xPL_Service * theService,
                             xPL_ServiceListener theListener,
                             xPL_MessageType messageType,
                             char * schemaClass, char * schemaType,
                             xPL_Object * userValue);
/**
 * @brief
 * @param theService
 * @param isEnabled
 */
void xPL_setServiceEnabled (xPL_Service * theService, bool isEnabled);

/**
 * @brief Return the value of the first/only index for an item
 * @param theService
 * @param itemName
 * @return
 */
char * xPL_getServiceConfigValue (xPL_Service * theService, char * itemName);

/**
 * @brief Simple form to set first/only value in an item
 * @param theService
 * @param itemName
 * @param itemValue
 */
void xPL_setServiceConfigValue (xPL_Service * theService,
                                char * itemName, char * itemValue);

/**
 * @brief Send a message out from this service
 *
 * If the message has not had it's source set or the source does not match the
 * sending service, it is updated and the message sent
 * @param theService
 * @param theMessage
 * @return
 */
bool xPL_sendServiceMessage (xPL_Service * theService, xPL_Message * theMessage);

/**
 * @brief Add a service config change listener
 * @param theService
 * @param theListener
 * @param userValue
 */
void xPL_addServiceConfigChangedListener (xPL_Service * theService,
    xPL_ServiceConfigChangedListener theListener,
    xPL_Object * userValue) ;

/**
 * @brief Return TRUE if this is a configured service and a configuration has been received
 * @param theService
 * @return
 */
bool xPL_isServiceConfigured (xPL_Service * theService);

/**
 * @brief Add a new configurable
 *
 * If the item is added, TRUE is returned.  If the item already exists,
 * FALSE is returned and it's not added or altered
 * @param theService
 * @param itemName
 * @param itemType
 * @param maxValues
 * @return
 */
bool xPL_addServiceConfigurable (xPL_Service * theService, char * itemName,
                                 xPL_ConfigurableType itemType, int maxValues);

/**
 * @brief Return the number of values for a given configurable
 * @param theService
 * @param itemName
 * @return
 */
int xPL_getServiceConfigValueCount (xPL_Service * theService, char * itemName);

/**
 * @brief 
 * @param theService
 * @return 
 */
char * xPL_getServiceInstanceID(xPL_Service * theService);

/**
 * @brief Release an xPL service
 * @param theService
 */
void xPL_releaseService (xPL_Service * theService);

/* Clear all configurable values out.  The configurable definitions */
/* remain in tact                                                 */
void xPL_clearAllServiceConfigValues(xPL_Service * theService);

char * xPL_getServiceVendor(xPL_Service * theService);

char * xPL_getServiceDeviceID(xPL_Service * theService);

char * xPL_getServiceInstanceID(xPL_Service * theService);

bool xPL_isServiceEnabled(xPL_Service * theService);

void xPL_setServiceVendor (xPL_Service * theService, char * newVendor);
void xPL_setServiceDeviceID (xPL_Service * theService, char * newDeviceID);
/* Change the current heartbeat interval */
void xPL_setHeartbeatInterval (xPL_Service * theService, int newInterval);
void xPL_setServiceInstanceID (xPL_Service * theService, char * newInstanceID);

/*------------------------------------------------------------------------------
 *
 *                                xPL Message
 *
 *----------------------------------------------------------------------------*/
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
xPL_Message * xPL_createTargetedMessage (xPL_Service * theService, xPL_MessageType messageType,
    char * theVendor, char * theDevice, char * theInstance);

/**
 * @brief Release a message and all it's resources
 * @param theMessage
 */
void xPL_releaseMessage(xPL_Message * theMessage);

char * xPL_getMessageNamedValue(xPL_Message * theMessage, char * theName);

xPL_NameValueList * xPL_getMessageBody(xPL_Message * theMessage);


/**
 * @brief
 * @param theMessage
 * @param theName
 * @param theValue
 */
void xPL_setMessageNamedValue (xPL_Message * theMessage,
                               char * theName, char * theValue);

/**
 * @brief 
 * @param theMessage
 * @param theName
 * @param theValue
 */
void xPL_addMessageNamedValue (xPL_Message * theMessage, char * theName, char * theValue);

/**
 * @brief Set a series of NameValue pairs for a message
 * @param theMessage
 */
void xPL_setMessageNamedValues (xPL_Message * theMessage, ...);

/**
 * @brief
 * @param theMessage
 * @param theSchemaClass
 * @param theSchemaType
 */
void xPL_setSchema (xPL_Message * theMessage,
                    char * theSchemaClass, char * theSchemaType);

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
 * then we process messages and wait and do not return until xPLLib is stopped.
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
char * xPL_getSourceVendor (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSourceDeviceID (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSourceInstanceID (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
xPL_MessageType xPL_getMessageType (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSchemaClass (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getSchemaType (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
int xPL_getHopCount (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
bool xPL_isBroadcastMessage (xPL_Message * theMessage);

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
char * xPL_getTargetGroup (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getTargetVendor (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getTargetDeviceID (xPL_Message * theMessage);

/**
 * @brief
 * @param theMessage
 * @return
 */
char * xPL_getTargetInstanceID (xPL_Message * theMessage);

/* Write out the message */
char * xPL_formatMessage(xPL_Message * theMessage);

void xPL_setSchemaClass(xPL_Message * theMessage, char * theSchemaClass);

void xPL_setSchemaType(xPL_Message * theMessage, char * theSchemaType);

bool xPL_isHubConfirmed(void);

/*------------------------------------------------------------------------------
 *
 *                                xPL Listeners
 *
 *----------------------------------------------------------------------------*/

/**
 * @brief Add a message listener
 * @param theHandler
 * @param userValue
 */
void xPL_addMessageListener (xPL_messageListener theHandler, xPL_Object * userValue);

bool xPL_removeMessageListener(xPL_messageListener theHandler);



/*------------------------------------------------------------------------------
 *
 *                                xPL Io
 *
 *----------------------------------------------------------------------------*/

/* Set the interface */
void xPL_setBroadcastInterface (char * newInterfaceName);

/* Return the xPL FD */
int xPL_getFD(void);

/* Get the connection port */
int xPL_getPort(void);

/* Return listing IP address */
char * xPL_getListenerIPAddr(void);

/* Allocate a new timeout handler and install it into the list */
void xPL_addTimeoutHandler (xPL_TimeoutHandler timeoutHandler, int timeoutInSeconds, xPL_Object * userValue);

/* Remove a previously allocated timeout handler */
bool xPL_removeTimeoutHandler (xPL_TimeoutHandler timeoutHandler);

/*------------------------------------------------------------------------------
 *
 *                                xPL Hub
 *
 *----------------------------------------------------------------------------*/

/**
 * @brief running a  hub process
 *
 * Once called, this instance of xPLLib will be running a hub process.
 * @note The xPLLib must be started in standalone mode for this to work
 * (which is NOT the default)
 *
 * @return
 */
bool xPL_startHub (void);

/**
 * @brief Disable the hub
 */
void xPL_stopHub (void);

/* Return if hub is running or not */
bool xPL_isHubRunning(void);

/*------------------------------------------------------------------------------
 *
 *                                xPL Uils and Debug
 *
 *----------------------------------------------------------------------------*/
/**
 * @brief Set Debugging Mode
 * @param isDebugging
 */
void xPL_setDebugging (bool isDebugging);

/**
 * @brief Write a debug message out (if we are debugging)
 * @param theFormat
 */
void xPL_Debug (char * theFormat, ...);

/**
 * @brief Write an error message out
 * @param theFormat
 */
void xPL_Error(char * theFormat, ...);

/* Return number of name value pairs */
int xPL_getNamedValueCount(xPL_NameValueList * theList);

/* Return the value indexed at */
xPL_NameValuePair * xPL_getNamedValuePairAt(xPL_NameValueList * theList, int listIndex);

/* Just add a simple entry to the list */
void xPL_addNamedValue(xPL_NameValueList * theList, char * theName, char * theValue);

/* Remove All name/value pairs from the passed list */
void xPL_clearAllNamedValues(xPL_NameValueList * theList);

/* Return true if there is a matching named value */
bool xPL_doesNamedValueExist(xPL_NameValueList * theList, char * theName);

/* Search for a name in a list of name values and return the */
/* index into the list of the value or -1 if not found       */
int xPL_getNamedValueIndex(xPL_NameValueList * theList, char * theName);

/* Attempt to update an existing name/value and if it is not */
/* existing, create and add a new one                        */
void xPL_setNamedValue(xPL_NameValueList * theList, char * theName, char * theValue);

/* Find the specified name in the name/value pair or return NULL */
xPL_NameValuePair * xPL_getNamedValuePair(xPL_NameValueList * theList, char * theName);

/* Search for a name in a list of name values and return the */
/* index into the list of the value or -1 if not found       */
int xPL_getNamedValueIndex(xPL_NameValueList * theList, char * theName);

/* Find the specified name in the list and return it's value or NULL */
char * xPL_getNamedValue(xPL_NameValueList * theList, char * theName);

/**
 * @brief Do a string comparison ignoring upper/lower case difference
 * @param textA
 * @param textB
 * @return
 */
int xPL_strcmpIgnoreCase (char * textA, char * textB);

/**
 * @brief Do a string comparison ignoring upper/lower case difference
 * @param textA
 * @param textB
 * @param maxChars
 * @return 
 */
int xPL_strncmpIgnoreCase(char * textA, char * textB, int maxChars);

/**
 * @brief Convert a passed string into a number
 * store in the passed value.  If the number is OK, then TRUE is returned.  
 * If there is an error, FALSE
 * @param theValue
 * @param theResult
 * @return 
 */
bool xPL_strToInt(char * theValue, int *theResult);

/**
 * @brief Convert an integer into a string
 * @param theValue
 * @return 
 */
char * xPL_intToStr(int theValue);

/* Convert a two digit hex string to an integer value */
/* If the string is invalid, FALSE is returned        */
bool xPL_hexToInt(char * theHexValue, int *theValue);

/* Convert a number to hex.  Only the lower 8 bits */
/* of the passed value are examined and converted  */
char * xPL_intToHex(int theValue);

/**
 * @}
 */

/* ========================================================================== */
#if defined(__cplusplus)
}
#endif
#endif /* _XPL_HEADER_ defined */
