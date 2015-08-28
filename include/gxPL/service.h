/**
 * @file gxPL/service.h
 * xPL Services
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_SERVICE_HEADER_
#define _GXPL_SERVICE_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup xPLService Services
 * @{
 */

/* types ==================================================================== */

/**
 * @brief Service Listener Support
 */
typedef void (* xPL_ServiceListener) (struct _xPL_Service *, struct _xPL_Message *, xPL_Object *);

/* structures =============================================================== */

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
typedef struct _xPL_ServiceListenerDef {

  xPL_MessageType matchMessageType;
  char *matchSchemaClass;
  char *matchSchemaType;
  xPL_Object * userValue;
  xPL_ServiceListener serviceListener;
} xPL_ServiceListenerDef;


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

/**
 * @brief Return number of services
 * @return
 */
int xPL_getServiceCount (void);

/**
 * @brief Return a service at a given index.
 * If the index is out of range, return NULL
 * @param serviceIndex
 * @return
 */
xPL_Service * xPL_getServiceAt (int serviceIndex);

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
 * @brief Remove a service listener
 * @param theService
 * @param theListener
 * @return
 */
bool xPL_removeServiceListener (xPL_Service * theService,
                                xPL_ServiceListener theListener);

/**
 * @brief
 * @param theService
 * @param isEnabled
 */
void xPL_setServiceEnabled (xPL_Service * theService, bool isEnabled);

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
 * @brief Clear out any/all installed filters
 * @param theService
 */
void xPL_clearServiceFilters (xPL_Service * theService);

/**
 * @brief Clear out all groups
 * @param theService
 */
void xPL_clearServiceGroups (xPL_Service * theService);


/**
 * @brief
 * @param theService
 * @return
 */
bool xPL_isServiceEnabled (xPL_Service * theService);


/**
 * @brief
 * @param theService
 * @return
 */
bool xPL_isServiceFiltered (xPL_Service * theService);

/**
 * @brief
 * @param theService
 * @param respondToBroadcasts
 */
void xPL_setRespondingToBroadcasts (xPL_Service * theService, bool respondToBroadcasts);

/**
 * @brief
 * @param theService
 * @return
 */
bool xPL_isRespondingToBroadcasts (xPL_Service * theService);

/**
 * @brief
 * @param theService
 * @param reportOwnMessages
 */
void xPL_setReportOwnMessages (xPL_Service * theService, bool reportOwnMessages);

/**
 * @brief
 * @param theService
 * @return
 */
bool xPL_isReportOwnMessages (xPL_Service * theService);

/**
 * @brief
 * @param theService
 * @return
 */
bool xPL_doesServiceHaveGroups (xPL_Service * theService);

/**
 * @brief Release an xPL service
 * @param theService
 */
void xPL_releaseService (xPL_Service * theService);

/**
 * @brief Change the current
 * @param theService
 * @param newVendor
 */
void xPL_setServiceVendor (xPL_Service * theService, char * newVendor);

/**
 * @brief
 * @param theService
 * @return
 */
char * xPL_getServiceVendor (xPL_Service * theService);

/**
 * @brief Change the current
 * @param theService
 * @param newDeviceID
 */
void xPL_setServiceDeviceID (xPL_Service * theService, char * newDeviceID);

/**
 * @brief
 * @param theService
 * @return
 */
char * xPL_getServiceDeviceID (xPL_Service * theService);

/**
 * @brief Change the current
 * @param theService
 * @param newInstanceID
 */
void xPL_setServiceInstanceID (xPL_Service * theService, char * newInstanceID);

/**
 * @brief
 * @param theService
 * @return
 */
char * xPL_getServiceInstanceID (xPL_Service * theService);

/**
 * @brief Change the current heartbeat interval
 * @param theService
 * @param newInterval
 */
/*  */
void xPL_setHeartbeatInterval (xPL_Service * theService, int newInterval);

/**
 * @brief Return services heartbeat interval
 * @param theService
 * @return
 */
int xPL_getHeartbeatInterval (xPL_Service * theService);

/**
 * @brief
 * @param theService
 * @param theVersion
 */
void xPL_setServiceVersion (xPL_Service * theService, char * theVersion);

/**
 * @brief
 * @param theService
 * @return
 */
char * xPL_getServiceVersion (xPL_Service * theService);

/**
 * @defgroup xPLServiceConfig Configurable services
 * @{
 */

/* types ==================================================================== */

/**
 * @brief Changes to a services configuration
 */
typedef void (* xPL_ServiceConfigChangedListener) (struct _xPL_Service *, xPL_Object *);

/* structures =============================================================== */
/**
 * @brief
 */
typedef struct _xPL_ServiceChangedListenerDef {

  xPL_ServiceConfigChangedListener changeListener;
  xPL_Object * userValue;
} xPL_ServiceChangedListenerDef;

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

/* internal public functions ================================================ */

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
 * @param theVendor
 * @param theDeviceID
 * @param localConfigFile
 * @return
 */
xPL_Service * xPL_createConfigurableService (char * theVendor,
    char * theDeviceID,
    char * localConfigFile);

/**
 * @brief Return TRUE if this is a configured service
 * @param theService
 * @return
 */
bool xPL_isConfigurableService (xPL_Service * theService);

/**
 * @brief Return the installed config file, if any
 * @param theService
 * @return
 */
char * xPL_getServiceConfigFile (xPL_Service * theService);

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
 * @brief Remove a config change listener
 * @param theService
 * @param theListener
 * @return
 */
bool xPL_removeServiceConfigChangedListener (xPL_Service * theService,
    xPL_ServiceConfigChangedListener theListener);

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
 * @brief Remove a configurable.
 * @param theService
 * @param itemName
 * @return Return TRUE if item found and removed, FALSE if not found
 */
bool xPL_removeServiceConfigurable (xPL_Service * theService, char * itemName);

/**
 * @brief Remove all configurables
 * @param theService
 */
void xPL_removeAllServiceConfigurables (xPL_Service * theService);

/**
 * @brief Search for a configurable and return it (or NULL)
 * @param theService
 * @param itemName
 * @return
 */
xPL_ServiceConfigurable * xPL_findServiceConfigurable (xPL_Service * theService,
    char * itemName);

/**
 * @brief Add a service item value
 *
 * If there are already values this is added to it, up to the limit defined for
 * the configurable.  If the item is "full", then the value is discarded.
 * @param theService
 * @param itemName
 * @param itemValue
 * @return
 */
bool xPL_addServiceConfigValue (xPL_Service * theService,
                                char * itemName, char * itemValue);


/**
 * @brief Simple form to set first/only value in an item
 *
 * @param theService
 * @param itemName
 * @param itemValue
 */
void xPL_setServiceConfigValue (xPL_Service * theService,
                                char * itemName, char * itemValue);

/**
 * @brief Set a item value at a given index.
 *
 * If that index is above the actual number of values, the value is appeneded
 * (i.e. may not be the same index as passed)
 * @param theService
 * @param itemName
 * @param valueIndex
 * @param itemValue
 */
void xPL_setServiceConfigValueAt (xPL_Service * theService, char * itemName,
                                  int valueIndex, char * itemValue);

/**
 * @brief Clear values for a given configurable
 * @param theService
 * @param itemName
 */
void xPL_clearServiceConfigValues (xPL_Service * theService, char * itemName);

/**
 * @brief Return the value of the first/only index for an item
 * @param theService
 * @param itemName
 * @return
 */
char * xPL_getServiceConfigValue (xPL_Service * theService, char * itemName);

/**
 * @brief Return the value at the given index.
 * If the value is NULL of the index is out of range, NULL is returned
 * @param theService
 * @param itemName
 * @param valueIndex
 * @return
 */
char * xPL_getServiceConfigValueAt (xPL_Service * theService, char * itemName, int valueIndex);

/**
 * @brief Clear all configurable values out
 *
 * The configurable definitions remain intact
 * @param theService
 */
void xPL_clearAllServiceConfigValues (xPL_Service * theService);

/**
 * @brief Return the number of values for a given configurable
 * @param theService
 * @param itemName
 * @return
 */
int xPL_getServiceConfigValueCount (xPL_Service * theService, char * itemName);

/**
 * @brief Return TRUE if this is a configured service and a configuration has been received
 * @param theService
 * @return
 */
bool xPL_isServiceConfigured (xPL_Service * theService);

/**
 * @}
 */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_SERVICE_HEADER_ defined */
