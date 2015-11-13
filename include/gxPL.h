/**
 * @file
 * End-user API (public header)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_HEADER_
#define _GXPL_HEADER_

#include <gxPL/defs.h>
#include <gxPL/message.h>
#include <gxPL/util.h>
#include <gxPL/device.h>
#include <gxPL/hub.h>
#include <gxPL/bridge.h>

__BEGIN_C_DECLS
/* ========================================================================== */

/* api functions ============================================================ */
/**
 * @defgroup gxPLSettingDoc Settings
 * gxPLSetting is used to pass settings to top-level classes. This class can be
 * instantiated directly or through the parameters of the command line.
 * @{
 */

/**
 * @brief Returns a new gxPLApplication setting from parameters
 *
 * @param iface network interface name o, the system
 * @param iolayer network access layer name
 * @param type network connection type
 * @return the setting or NULL if error occurs
 */
gxPLSetting * gxPLSettingNew (const char * iface, const char * iolayer, gxPLConnectType type);

/**
 * @brief Returns a new gxPLApplication setting from command line parameters
 *
 * This will parse the passed command array for options and parameters
 * It supports the following options:
 *    -i / --interface xxx : interface or device used to access the network
 *    -n / --net       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 *    -D / --nodaemon      : do not daemonize
 *
 * @param argc number of parameters from main
 * @param argv list of parameters from main
 * @param type network connection type
 * @return the setting or NULL if error occurs
 */
gxPLSetting * gxPLSettingFromCommandArgs (int argc, char * argv[], gxPLConnectType type);

/**
 * @}
 */

/**
 * @defgroup gxPLApplicationDoc Applications
 * gxPLApplication is the central element of a xPL application.
 * This class performs all operations to open and close the xPL network,
 * send and receive messages. An application is needed to create devices.
 * @{
 */

/**
 * @brief Opens a new gxPLApplication object
 * @param setting pointer to a configuration, this configuration can be modified
 * by the function to return the actual configuration.
 * @return the object or NULL if error occurs
 */
gxPLApplication * gxPLAppOpen (gxPLSetting * setting);

/**
 * @brief Close a gxPLApplication object and release all ressources
 * @param app pointer to a gxPLApplication object
 * @return 0, -1 if an error occurs
 */
int gxPLAppClose (gxPLApplication * app);

/**
 * @brief Polling event of an application
 * @param app pointer to a gxPLApplication object
 * @param timeout_ms waiting period in ms before output if no event occurs
 * @return 0, -1 if an error occurs
 */
int gxPLAppPoll (gxPLApplication * app, int timeout_ms);

/**
 * @brief Connection type
 *
 * @param app pointer to a gxPLApplication object
 * @return the type
 */
gxPLConnectType gxPLAppConnectionType (const gxPLApplication * app);

/**
 * @brief Returns application setting
 *
 * @param app pointer to a gxPLApplication object
 * @return the setting or NULL if error occurs
 */
gxPLSetting * gxPLAppSetting (gxPLApplication * app);

/**
 * @}
 */

/**
 * @addtogroup gxPLMessageDoc
 * @{
 */

/**
 * @brief Broadcast a message
 *
 * @param app pointer to a gxPLApplication object
 * @param message pointer to the message
 * @return number of bytes send, -1 if error occurs
 */
int gxPLAppBroadcastMessage (gxPLApplication * app, const gxPLMessage * message);

/**
 * @brief Send a targeted xPL message
 *
 * @param app pointer to a gxPLApplication object
 * @param message pointer to the message
 * @param target io adress of the target
 * @return number of bytes send, -1 if error occurs
 */
int gxPLAppSendMessage (gxPLApplication * app, const gxPLMessage * message,
                        const gxPLIoAddr * target);
/**
 * @brief Check if a message is an echo hub
 *
 * @param app pointer to a gxPLApplication object
 * @param message pointer to the message
 * @param my_id identifier of the request source. Necessary if the underlying
 * network is not udp (hbeat.basic), may be NULL otherwise (hbeat.app).
 * @return true, false, -1 if an error occurs
 */
int gxPLAppIsHubEchoMessage (const gxPLApplication * app,
                             const gxPLMessage * message, const gxPLId * my_id);

/**
 * @}
 */

/**
 * @addtogroup gxPLDeviceDoc
 * @{
 */

/**
 * @brief Adds a new device to an application
 * 
 * @param app pointer to a gxPLApplication object
 * @param vendor_id pointer to the vendor id
 * @param device_id pointer to the device id
 * @param instance_id pointer to the instance id
 * @return pointer on the device, NULL if error occurs
 */
gxPLDevice * gxPLAppAddDevice (gxPLApplication * app, const char * vendor_id,
                               const char * device_id, const char * instance_id);

/**
 * @brief Return number of devices
 */
int gxPLAppDeviceCount (gxPLApplication * app);

/**
 * @brief Return a device at a given index
 *
 * @param app pointer to a gxPLApplication object
 * @param index of the device
 * @return If the index is out of range, return NULL
 */
gxPLDevice * gxPLAppDeviceAt (gxPLApplication * app, int index);

/**
 * @brief Return index for a given device
 *
 * @param app pointer to a gxPLApplication object
 * @param device pointer on the device
 * @return the index, -1 if not found
 */
int gxPLAppDeviceIndex (gxPLApplication * app, const gxPLDevice * device);

/**
 * @brief Removes a device
 * @param app pointer to a gxPLApplication object
 * @param device pointer on the device
 * @return 0, -1 if an error occurs
 */
int gxPLAppRemoveDevice (gxPLApplication * app, gxPLDevice * device);

/**
 * @addtogroup gxPLDeviceConfigDoc
 * @{
 */

/**
 * @brief Adds a new configurable device
 *
 * @param app pointer to a gxPLApplication object
 * @param vendor_id pointer to the vendor id
 * @param device_id pointer to the device id
 * @param filename pointer to the config filename
 * @return pointer on the configurable device, NULL if error occurs
 */
gxPLDevice * gxPLAppAddConfigurableDevice (gxPLApplication * app,
    const char * vendor_id, const char * device_id,
    const char * filename);
/**
 *  @}
 * @}
 */

/**
 * @addtogroup gxPLUtilIdDoc
 * @{
 */

/**
 * @brief Generates a fairly unique identifier
 *
 * The identifier consists only of valid characters in xPL (0-9,a-z).
 * The algorithm uses the hardware address of the host and the time of day.
 * Two spaced successive calls of less than one millisecond give the same result.
 *
 * @param app pointer to a gxPLApplication object
 * @param id the generated id string
 * @param len length of the generated string not including the terminating null character
 * @return length of the generated string, -1 if error occurs
 */
int gxPLGenerateUniqueId (const gxPLApplication * app, char * id, int len);

/**
 * @}
 */

/**
 * @addtogroup gxPLMessageDoc
 * @{
 * @defgroup gxPLMessageListenerDoc Message Listeners
 * Provides functions to intercept messages received if the devices are not used.
 * @{
 */

/* types ==================================================================== */
/**
 * @brief Function that will be called each valid message reception
 */
typedef void (* gxPLMessageListener) (gxPLApplication * app, gxPLMessage *, void *);

/* internal public functions ================================================ */

/**
 * @brief Add a message listener
 *
 * @param app pointer to a gxPLApplication object
 * @param listener function that will be called each message reception.
 * @param udata pointer to the data passed to the listener
 * @return 0, -1 if an error occurs
 */
int gxPLMessageListenerAdd (gxPLApplication * app, gxPLMessageListener listener, void * udata);

/**
 * @brief Remove a message listener
 * @param app pointer to a gxPLApplication object
 * @param listener the listener to remove
 * @return 0, -1 if an error occurs
 */
int gxPLMessageListenerRemove (gxPLApplication * app, gxPLMessageListener listener);

/**
 *  @}
 * @}
 */

/**
 * @defgroup gxPLVersionDoc Version
 * Provides information on the version of the library.
 * @{
 */

/**
 * @brief Current version as a static string buffer
 */
const char * gxPLVersion (void);

/**
 * @brief Major number of the current version
 */
int gxPLVersionMajor (void);

/**
 * @brief  Minor number of the current version
 */
int gxPLVersionMinor (void);

/**
 * @brief Patch number of the current version
 */
int gxPLVersionPatch (void);

/**
 * @brief SHA1 signature of the current version
 */
unsigned long gxPLVersionSha1 (void);

/**
 * @}
 */

/**
 * @addtogroup gxPLIoDoc
 * @{
 */

/* internal public functions ================================================ */
/**
 * @brief Returns a list of io layers available on the system
 *
 * @return returned list of io layers as a pointeur on an vector of const string,
 * NULL if error occurs
 *
 * @warning the list returned by the pointer must be freed with vVectorDestroy()
 * after use.
 */
xVector * gxPLIoLayerList (void);

/**
 * @brief Local network address as a string
 *
 * @param app pointer to a gxPLApplication object
 * @return network address as a string, NULL if an error occurs
 */
const char * gxPLIoLocalAddrGet (const gxPLApplication * app);

/**
 * @brief Local network address list as a vector of strings
 *
 * @param app pointer to a gxPLApplication object
 * @return the vector of strings, NULL if an error occurs
 */
const xVector * gxPLIoLocalAddrList (const gxPLApplication * app);

/**
 * @brief Broadcast network address as a string
 *
 * @param app pointer to a gxPLApplication object
 * @return the address as a string, NULL if an error occurs
 */
const char * gxPLIoBcastAddrGet (const gxPLApplication * app);

/**
 * @brief Local Network informations
 *
 * @param app pointer to a gxPLApplication object
 * @return network infos, NULL if an error occurs
 */
const gxPLIoAddr * gxPLIoInfoGet (const gxPLApplication * app);

/**
 * @brief Name of the network interface on the system
 *
 * @param app pointer to a gxPLApplication object
 * @return the name, NULL if an error occurs
 */
const char * gxPLIoInterfaceGet (const gxPLApplication * app);

/**
 * @brief Name of the underlying layer of the network
 *
 * @param app pointer to a gxPLApplication object
 * @return the name, NULL if an error occurs
 */
const char * gxPLIoLayerGet (const gxPLApplication * app);

/**
 * @brief System call for device-specific input/output operations
 *
 * system call for device-specific input/output operations and other operations
 * which cannot be expressed by regular system calls. \n
 * It takes a parameter specifying a request code; the effect of a call depends
 * completely on the request code. Request codes are often device-specific.
 *
 * -  \b gxPLIoFuncGetBcastAddr
 *    \code int gxPLIoCtl (gxPLApplication * app, gxPLIoFuncGetBcastAddr, gxPLIoAddr * bcast_addr)
 *    returns broadcast address used
 * -  \b gxPLIoFuncGetNetInfo
 *    \code int gxPLIoCtl (gxPLApplication * app, gxPLIoFuncGetNetInfo, gxPLIoAddr * local_addr)
 *    returns network informations
 * -  \b gxPLIoFuncNetAddrToString
 *    \code int gxPLIoCtl (gxPLApplication * app, gxPLIoFuncNetAddrToString, gxPLIoAddr * net_addr, char ** str_addr)
 *    converts a network address in a gxPLIoAddr to a dots-and-numbers format string
 * -  \b gxPLIoFuncNetAddrFromString
 *    \code int gxPLIoCtl (gxPLIo * io, gxPLIoFuncNetAddrFromString, gxPLIoAddr * net_addr, const char * str_addr)
 *    converts from a dots-and-numbers string into a gxPLIoAddr
 * -  \b gxPLIoFuncGetLocalAddrList
 *    \code int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetLocalAddrList, const xVector ** addr_list)
 *    returns binded adresses list
 * .
 *
 * @param app pointer to a gxPLApplication object
 * @param req request code
 * @param ... optional parameters
 * @return 0, -1 if an error occurs
 */
int gxPLIoCtl (gxPLApplication * app, int req, ...);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_HEADER_ defined */
