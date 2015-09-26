/**
 * @file gxPL/device.h
 * High level interface for manage xPL devices
 *
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
 * @defgroup xPLDevices Devices
 * High level interface for manage xPL devices
 * @{
 */

/* types ==================================================================== */

/**
 * @brief Listener for a device
 */
typedef void (* gxPLDeviceListener) (gxPLDevice *, const gxPLMessage *, void *);

/* structures =============================================================== */


/* internal public functions ================================================ */

/**
 * @brief Create a message for the device
 * 
 * The message can be modified using the functions of the message module 
 * before being sent with gxPLDeviceMessageSend(). A message to send can not 
 * be gxPLMessageAny type. The message should be released with gxPLMessageDelete
 * after use.
 * 
 * @param device pointer on the device
 * @param type the type of message
 * @return  the message, NULL if an error occurs
 */
gxPLMessage * gxPLDeviceMessageNew (gxPLDevice * device, gxPLMessageType type);

/**
 * @brief Send a message out from this device
 *
 * The source of the message should be the device but no check is performed.
 * 
 * @param device pointer on the device
 * @param message
 * @return number of bytes send, -1 if error occurs
 */
int gxPLDeviceMessageSend (gxPLDevice * device, gxPLMessage * message);

/**
 * @brief Add a listener for the device
 * 
 * This function allows the user to install a listener that will be called for 
 * each message received for the service. \n
 * Only messages matching type, schema_class and schema_type are forwarded to 
 * the listener.
 * 
 * @param device pointer on the device
 * @param listener the function listening device messages
 * @param type type of message to be processed, gxPLMessageAny to manage any
 * @param schema_class schema class to process, NULL to manage everything
 * @param schema_type schema type to process, NULL to manage everything
 * @param udata pointer to the user data that will be passed to the listener, 
 * NULL if not used.
 * @return 0, -1 if error occurs
 */
int gxPLDeviceListenerAdd (gxPLDevice * device,
                           gxPLDeviceListener listener,
                           gxPLMessageType type,
                           char * schema_class, char * schema_type,
                           void * udata);

/**
 * @brief Remove a device listener
 * @param device pointer on the device
 * @param listener the listener
 * @return 0, -1 if error occurs
 */
int gxPLDeviceListenerRemove (gxPLDevice * device,
                              gxPLDeviceListener listener);

/**
 * @brief Gets the parent gxPL object
 * 
 * @param device pointer on the device
 * @return pointer to an gxPL object that is the parent of the device, NULL if error occurs
 */
gxPL * gxPLDeviceParentGet (gxPLDevice * device);

/**
 * @brief Gets the identifier
 * @param device pointer on the device
 * @return the identifier, NULL if error occurs
 */
const gxPLId * gxPLDeviceIdGet (const gxPLDevice * device);

/**
 * @brief Indicates whether the device is enabled.
 * @param device pointer on the device
 * @return true if enabled, false if not, -1 if an error occurs
 */
int gxPLDeviceEnabledGet (const gxPLDevice * device);

/**
 * @brief Gets the heartbeat interval
 * 
 * @param device pointer on the device
 * @return the interval in seconds, -1
 */
int gxPLDeviceHeartbeatIntervalGet (const gxPLDevice * device);

/**
 * @brief Gets the time of the last heartbeat sent
 * 
 * The time is given in a unit dependent on the host system, most of the time 
 * is given in seconds. On a unix system with a real time clock that value is 
 * the number of seconds since the first second of January 1, 1970.
 * 
 * @param device pointer on the device
 * @return the last time, -1 if error occurs
 */
long gxPLDeviceHeartbeatLastGet (const gxPLDevice * device);

/**
 * @brief Gets the version string
 * 
 * This information is transmitted with the heartbeat.
 * @param device pointer on the device
 * @return the last time, NULL if error occurs
 */
const char * gxPLDeviceVersionGet (const gxPLDevice * device);

/**
 * @brief Indicates whether the device will respond to broadcast messages.
 * @param device pointer on the device
 * @return true if respond, false if not, -1 if an error occurs
 */
int gxPLRespondToBroadcastGet (const gxPLDevice * device);

/**
 * @brief Indicates whether the device will transmit its own messages to the listeners
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLReportOwnMessagesGet (const gxPLDevice * device);

/**
 * @brief Indicates whether the device has detected a hub.
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLDeviceHubConfirmedGet (const gxPLDevice * device);

/**
 * @brief Indicates whether the device is configurable
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLDeviceConfiguraleGet (const gxPLDevice * device);

/**
 * @brief Indicates whether the device is configur
 * @param device pointer on the device
 * @return true, false, -1 if an error occurs
 */
int gxPLDeviceConfiguredGet (const gxPLDevice * device);

/**
 * @brief Sets the identifier
 * @param device pointer on the device
 * @param id pointer to identifier
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceIdSet (gxPLDevice * device,  const gxPLId * id);

/**
 * @brief Sets the vendor identifier
 * @param device pointer on the device
 * @param vendor_id pointer to the vendor id
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceVendorIdSet (gxPLDevice * device, const char * vendor_id);

/**
 * @brief Sets the device identifier
 * @param device pointer on the device
 * @param device_id pointer to the device id
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceDeviceIdSet (gxPLDevice * device, const char * device_id);

/**
 * @brief Sets the instance identifier
 * @param device pointer on the device
 * @param instance_id pointer to the instance id
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceInstanceIdSet (gxPLDevice * device, const char * instance_id);

/**
 * @brief Sets the version
 * @param device pointer on the device
 * @param version
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceVersionSet (gxPLDevice * device, const char * version);

/**
 * @brief Enabled or not a device
 * @param device pointer on the device
 * @param enabled true for eanbled
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceEnabledSet (gxPLDevice * device, bool enabled);

/**
 * @brief Sets the heartbeat interval
 * @param device pointer on the device
 * @param interval interval in seconds
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceHeartbeatIntervalSet (gxPLDevice * device, int interval);

/**
 * @brief Enable the response to broadcast messages
 * @param device pointer on the device
 * @param respond true for respond
 * @return 0, -1 if an error occurs
 */
int gxPLRespondToBroadcastSet (gxPLDevice * device, bool respond);

/**
 * @brief Process own messages
 * @param device pointer on the device
 * @param isreportownmsg true or flase
 * @return 0, -1 if an error occurs
 */
int gxPLReportOwnMessagesSet (gxPLDevice * device, bool isreportownmsg);



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


















/**
 * @brief
 * @param device pointer on the device
 * @return
 */
bool gxPLdoesServiceHaveGroups (gxPLDevice * device);

/**
 * @brief Clear out all groups
 * @param device pointer on the device
 */
void gxPLclearServiceGroups (gxPLDevice * device);


/**
 * @brief Clear out any/all installed filters
 * @param device pointer on the device
 */
void gxPLclearServiceFilters (gxPLDevice * device);


/**
 * @brief
 * @param device pointer on the device
 * @return
 */
bool gxPLIsServiceFiltered (gxPLDevice * device);

/**
 * @defgroup xPLServiceConfig Configurable devices
 * @{
 */

/* types ==================================================================== */

/**
 * @brief Changes to a devices configuration
 */
typedef void (* gxPLDeviceConfigChangedListener) (struct _gxPLDevice *, void *);

/* structures =============================================================== */
/**
 * @brief
 */
typedef struct _gxPLDeviceChangedListenerDef {

  gxPLDeviceConfigChangedListener changeListener;
  void * userValue;
} gxPLDeviceChangedListenerDef;

/**
 * @brief
 */
typedef struct  _gxPLDeviceConfigurable {

  char * name;
  gxPLConfigurableType itemType;
  int maxValueCount;

  int valueCount;
  int valueAllocCount;
  char **valueList;
} gxPLDeviceConfigurable;

/* internal public functions ================================================ */

/**
 * @brief Create a new device and prepare it for configuration
 *
 * Like other devices, this will still require being enabled to start.
 * Before it's started, you need to define and attach the configurable items
 * for the device.   When the device is enabled, if there is a non-null
 * configFile, it's values are read.  The devices instance value will be
 * created in a fairly unique method for devices that have not yet been
 * configured.
 *
 * @param vendor_id
 * @param device_id
 * @param localConfigFile
 * @return
 */
gxPLDevice * gxPLcreateConfigurableService (char * vendor_id,
    char * device_id,
    char * localConfigFile);

/**
 * @brief Return TRUE if this is a configured device
 * @param device
 * @return
 */
bool gxPLIsConfigurableService (gxPLDevice * device);

/**
 * @brief Return the installed config file, if any
 * @param device
 * @return
 */
char * gxPLgetServiceConfigFile (gxPLDevice * device);

/**
 * @brief Add a device config change listener
 * @param device
 * @param listener
 * @param userValue
 */
void gxPLaddServiceConfigChangedListener (gxPLDevice * device,
    gxPLDeviceConfigChangedListener listener,
    void * userValue) ;

/**
 * @brief Remove a config change listener
 * @param device
 * @param listener
 * @return
 */
bool gxPLremoveServiceConfigChangedListener (gxPLDevice * device,
    gxPLDeviceConfigChangedListener listener);

/**
 * @brief Add a new configurable
 *
 * If the item is added, TRUE is returned.  If the item already exists,
 * FALSE is returned and it's not added or altered
 * @param device
 * @param name
 * @param itemType
 * @param maxValues
 * @return
 */
bool gxPLaddServiceConfigurable (gxPLDevice * device, char * name,
                                 gxPLConfigurableType itemType, int maxValues);

/**
 * @brief Remove a configurable.
 * @param device
 * @param name
 * @return Return TRUE if item found and removed, FALSE if not found
 */
bool gxPLremoveServiceConfigurable (gxPLDevice * device, char * name);

/**
 * @brief Remove all configurables
 * @param device
 */
void gxPLremoveAllServiceConfigurables (gxPLDevice * device);

/**
 * @brief Search for a configurable and return it (or NULL)
 * @param device
 * @param name
 * @return
 */
gxPLDeviceConfigurable * gxPLfindServiceConfigurable (gxPLDevice * device,
    char * name);

/**
 * @brief Add a device item value
 *
 * If there are already values this is added to it, up to the limit defined for
 * the configurable.  If the item is "full", then the value is discarded.
 * @param device
 * @param name
 * @param value
 * @return
 */
bool gxPLaddServiceConfigValue (gxPLDevice * device,
                                char * name, char * value);


/**
 * @brief Simple form to set first/only value in an item
 *
 * @param device
 * @param name
 * @param value
 */
void gxPLsetServiceConfigValue (gxPLDevice * device,
                                char * name, char * value);

/**
 * @brief Set a item value at a given index.
 *
 * If that index is above the actual number of values, the value is appeneded
 * (i.e. may not be the same index as passed)
 * @param device
 * @param name
 * @param valueIndex
 * @param value
 */
void gxPLsetServiceConfigValueAt (gxPLDevice * device, char * name,
                                  int valueIndex, char * value);

/**
 * @brief Clear values for a given configurable
 * @param device
 * @param name
 */
void gxPLclearServiceConfigValues (gxPLDevice * device, char * name);

/**
 * @brief Return the value of the first/only index for an item
 * @param device
 * @param name
 * @return
 */
char * gxPLgetServiceConfigValue (gxPLDevice * device, char * name);

/**
 * @brief Return the value at the given index.
 * If the value is NULL of the index is out of range, NULL is returned
 * @param device
 * @param name
 * @param valueIndex
 * @return
 */
char * gxPLgetServiceConfigValueAt (gxPLDevice * device, char * name, int valueIndex);

/**
 * @brief Clear all configurable values out
 *
 * The configurable definitions remain intact
 * @param device
 */
void gxPLclearAllServiceConfigValues (gxPLDevice * device);

/**
 * @brief Return the number of values for a given configurable
 * @param device
 * @param name
 * @return
 */
int gxPLgetServiceConfigValueCount (gxPLDevice * device, char * name);

/**
 * @brief Return TRUE if this is a configured device and a configuration has been received
 * @param device
 * @return
 */
bool gxPLIsServiceConfigured (gxPLDevice * device);

/**
 * @}
 */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_SERVICE_HEADER_ defined */
