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
 * @brief Service Listener Support
 */
typedef void (* gxPLDeviceListener) (gxPLDevice *, const gxPLMessage *, void *);

/* structures =============================================================== */


/* internal public functions ================================================ */
/**
 * @brief Return number of devices
 *
 * @return
 */
int gxPLDeviceCount (gxPL * gxpl);

/**
 * @brief Return a device at a given index.
 *
 * @param index
 * @return If the index is out of range, return NULL
 */
gxPLDevice * gxPLDeviceAt (gxPL * gxpl, int index);

/**
 * @brief Create a new xPL device
 */
gxPLDevice * gxPLDeviceNew (gxPL * gxpl,
                            const char * vendor_id,
                            const char * device_id,
                            const char * instance_id);

/**
 * @brief Release an xPL device
 */
void gxPLDeviceDelete (gxPLDevice * device);

const gxPLId * gxPLDeviceIdGet (const gxPLDevice * device);
int gxPLDeviceEnabledGet (const gxPLDevice * device);
int gxPLDeviceHeartbeatIntervalGet (const gxPLDevice * device);
const char * gxPLDeviceVersionGet (const gxPLDevice * device);
int gxPLRespondToBroadcastGet (const gxPLDevice * device);
int gxPLReportOwnMessagesGet (const gxPLDevice * device);

int gxPLDeviceIdSet (gxPLDevice * device,  const gxPLId * id);
int gxPLDeviceVendorIdSet (gxPLDevice * device, const char * vendor_id);
int gxPLDeviceDeviceIdSet (gxPLDevice * device, const char * device_id);
int gxPLDeviceInstanceIdSet (gxPLDevice * device, const char * instance_id);
int gxPLDeviceEnabledSet (gxPLDevice * device, bool enabled);
int gxPLDeviceHeartbeatIntervalSet (gxPLDevice * device, int interval);
int gxPLRespondToBroadcastSet (gxPLDevice * device, bool respond);
int gxPLReportOwnMessagesSet (gxPLDevice * device, bool reportmsg);

//------------------------------------------------------------------------------
/**
 * @brief Create a message suitable for broadcasting to all listeners
 * @param device
 * @param type
 * @return
 */
gxPLMessage * gxPLDeviceMessageNewBroadcast (gxPLDevice * device,
    gxPLMessageType type);

/**
 * @brief Create a message suitable for sending to a specific receiver
 * @param device
 * @param type
 * @param vendor_id
 * @param device
 * @param instance
 * @return
 */
gxPLMessage * gxPLDeviceMessageNewTargeted (gxPLDevice * device,
    gxPLMessageType type,
    char * target_vendor_id,
    char * target_device_id,
    char * target_instance_id);

/**
 * @brief
 * @param device
 * @param type
 * @param group
 * @return
 */
gxPLMessage * gxPLDeviceMessageNewGroupTargeted (gxPLDevice * device,
    gxPLMessageType type,
    char * group);

/**
 * @brief Send a message out from this device
 *
 * If the message has not had it's source set or the source does not match the
 * sending device, it is updated and the message sent
 * @param device
 * @param message
 * @return
 */
bool gxPLDeviceMessageSend (gxPLDevice * device, gxPLMessage * message);

/**
 * @brief Add a device listener
 */
int gxPLDeviceListenerAdd (gxPLDevice * device,
                           gxPLDeviceListener listener,
                           gxPLMessageType type,
                           char * schema_class, char * schema_type,
                           void * udata);

/**
 * @brief Remove a device listener
 * @param device
 * @param listener
 * @return
 */
int gxPLDeviceListenerRemove (gxPLDevice * device,
                              gxPLDeviceListener listener);


/**
 * @brief
 * @param device
 * @return
 */
bool gxPLdoesServiceHaveGroups (gxPLDevice * device);

/**
 * @brief Clear out all groups
 * @param device
 */
void gxPLclearServiceGroups (gxPLDevice * device);


/**
 * @brief Clear out any/all installed filters
 * @param device
 */
void gxPLclearServiceFilters (gxPLDevice * device);


/**
 * @brief
 * @param device
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
