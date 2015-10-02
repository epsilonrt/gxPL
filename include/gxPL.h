/**
 * @file include/gxPL.h
 * Top level API (public header)
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

__BEGIN_C_DECLS
/* ========================================================================== */
/**
 * @defgroup gxPLApi Top Level API
 * @{
 */

/* api functions ============================================================ */
/**
 * @brief Returns a new gxPL setting from parameters
 *
 * @param iface network interface name o, the system
 * @param iolayer network access layer name
 * @param type network connection type
 * @return the setting or NULL if error occurs
 */
gxPLSetting * gxPLSettingNew (const char * iface, const char * iolayer, gxPLConnectType type);

/**
 * @brief Returns a new gxPL setting from command line parameters
 *
 * This will parse the passed command array for options and parameters
 * It supports the following options:
 *    -i / --interface xxx : interface or device used to access the network
 *    -h / --hal       xxx : hardware abstraction layer to access the network
 *    -d / --debug         : enable debugging
 *
 * @param argc number of parameters from main
 * @param argv list of parameters from main
 * @param type network connection type
 * @return the setting or NULL if error occurs
 */
gxPLSetting * gxPLSettingNewFromCommandArgs (int argc, char * argv[], gxPLConnectType type);

/**
 * @brief Opens a new gxPL object
 * @param setting pointer to a configuration, this configuration can be modified
 * by the function to return the actual configuration.
 * @return the object or NULL if error occurs
 */
gxPL * gxPLOpen (gxPLSetting * setting);

/**
 * @brief Close a gxPL object and release all ressources
 * @param gxpl pointer to a gxPL object
 * @return 0, -1 if an error occurs
 */
int gxPLClose (gxPL * gxpl);

/**
 * @brief
 * @param gxpl pointer to a gxPL object
 * @param timeout_ms
 * @return 0, -1 if an error occurs
 */
int gxPLPoll (gxPL * gxpl, int timeout_ms);

/**
 * @brief Send an xPL message
 *
 * @param gxpl pointer to a gxPL object
 * @param message pointer to the message
 * @return number of bytes send, -1 if error occurs
 */
int gxPLMessageSend (gxPL * gxpl, gxPLMessage * message);

/**
 * @brief Check if a message is an echo hub
 *
 * @param gxpl pointer to a gxPL object
 * @param message pointer to the message
 * @param my_id identifier of the request source. Necessary if the underlying
 * network is not udp (hbeat.basic), may be NULL otherwise (hbeat.app).
 * @return true, false, -1 if an error occurs
 */
int gxPLMessageIsHubEcho (const gxPL * gxpl, const gxPLMessage * message, const gxPLId * my_id);

/**
 * @brief Connection type
 *
 * @param gxpl pointer to a gxPL object
 * @return the type
 */
gxPLConnectType gxPLConnectionTypeGet (const gxPL * gxpl);

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
 * @brief Return index for a given device
 *
 * @param index
 * @return the index, -1 if not found
 */
int gxPLDeviceIndex (gxPL * gxpl, const gxPLDevice * device);

/**
 * @brief Stop all devices
 * 
 * Usually in preparation for shutdown, but that isn't the only possible reason.
 * It is not necessary to call this function before calling gxPLClose()
 * @param gxpl
 * @return 
 */
int gxPLDeviceDisableAll (gxPL * gxpl);

/**
 * @brief Adds a new device on the network
 * @param gxpl pointer to a gxPL object
 * @param vendor_id pointer to the vendor id
 * @param device_id pointer to the device id
 * @param instance_id pointer to the instance id
 * @return pointer on the device, NULL if error occurs
 */
gxPLDevice * gxPLDeviceAdd (gxPL * gxpl, const char * vendor_id,
                            const char * device_id, const char * instance_id);

/**
 * @brief Adds a new configurabledevice on the network
 * @param gxpl pointer to a gxPL object
 * @param vendor_id pointer to the vendor id
 * @param device_id pointer to the device id
 * @param filename pointer to the config filename
 * @return pointer on the device, NULL if error occurs
 */
gxPLDevice * gxPLDeviceConfigAdd (gxPL * gxpl, const char * vendor_id,
                            const char * device_id, const char * filename);
                            
/**
 * @brief 
 * @param gxpl
 * @param device
 * @return 
 */
int gxPLDeviceRemove (gxPL * gxpl, gxPLDevice * device);

/**
 * @addtogroup xPLUtil
 * @{
 */

/**
 * @brief Generates a fairly unique identifier
 *
 * The identifier consists only of valid characters in xPL (0-9,a-z).
 * The algorithm uses the hardware address of the host and the time of day.
 * Two spaced successive calls of less than one millisecond give the same result.
 *
 * @param gxpl pointer to a gxPL object
 * @param id the generated id string
 * @param len length of the generated string not including the terminating null character
 * @return length of the generated string, -1 if error occurs
 */
int gxPLGenerateUniqueId (const gxPL * gxpl, char * id, int len);

/**
 * @}
 */

/**
 * @defgroup xPLMessageListener Message Listeners
 * Message Listeners
 * @{
 */

/* types ==================================================================== */
/**
 * @brief Function that will be called each valid message reception
 */
typedef void (* gxPLMessageListener) (gxPL * gxpl, gxPLMessage *, void *);

/* internal public functions ================================================ */

/**
 * @brief Add a message listener
 *
 * @param gxpl pointer to a gxPL object
 * @param listener function that will be called each message reception.
 * @param udata pointer to the data passed to the listener
 * @return 0, -1 if an error occurs
 */
int gxPLMessageListenerAdd (gxPL * gxpl, gxPLMessageListener listener, void * udata);

/**
 * @brief Remove a message listener
 * @param gxpl pointer to a gxPL object
 * @param listener the listener to remove
 * @return 0, -1 if an error occurs
 */
int gxPLMessageListenerRemove (gxPL * gxpl, gxPLMessageListener listener);

/**
 * @}
 */

/**
 * @defgroup gxPLibVersion Version
 * @{
 */

/**
 * @brief
 * @return
 */
const char * gxPLVersion (void);

/**
 * @brief
 * @return
 */
int gxPLVersionMajor (void);

/**
 * @brief
 * @return
 */
int gxPLVersionMinor (void);

/**
 * @brief
 * @return
 */
int gxPLVersionPatch (void);

/**
 * @brief
 * @return
 */
int gxPLVersionSha1 (void);
/**
 *  @}
 * @}
 */


/**
 * @defgroup xPLIoApi Low-level API
 * Allows you to read information and to control the IO layer.
 * @{
 */

/* internal public functions ================================================ */
/**
 * @brief Returns a list of io layers available on the system
 *
 * @return returned list of io layers as a pointeur on an vector of const string,
 * NULL if error occurs
 *
 * @warnig the list returned by the pointer must be freed with vVectorDestroy()
 * after use.
 */
xVector * gxPLIoLayerList (void);

/**
 * @brief Local network address as a string
 *
 * @param gxpl pointer to a gxPL object
 * @return the address as a string, NULL if an error occurs
 */
const char * gxPLIoLocalAddrGet (const gxPL * gxpl);

/**
 * @brief Broadcast network address as a string
 *
 * @param gxpl pointer to a gxPL object
 * @return the address as a string, NULL if an error occurs
 */
const char * gxPLIoBcastAddrGet (const gxPL * gxpl);

/**
 * @brief Local Network informations
 *
 * @param gxpl pointer to a gxPL object
 * @return network infos, NULL if an error occurs
 */
const gxPLIoAddr * gxPLIoInfoGet (const gxPL * gxpl);

/**
 * @brief Name of the network interface on the system
 *
 * @param gxpl pointer to a gxPL object
 * @return the name, NULL if an error occurs
 */
const char * gxPLIoInterfaceGet (const gxPL * gxpl);

/**
 * @brief Name of the underlying layer of the network
 *
 * @param gxpl pointer to a gxPL object
 * @return the name, NULL if an error occurs
 */
const char * gxPLIoLayerGet (const gxPL * gxpl);

/**
 * @brief System call for device-specific input/output operations
 *
 * system call for device-specific input/output operations and other operations
 * which cannot be expressed by regular system calls. \n
 * It takes a parameter specifying a request code; the effect of a call depends
 * completely on the request code. Request codes are often device-specific.
 *
 * -  \b gxPLIoFuncGetBcastAddr
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetBcastAddr, gxPLIoAddr * bcast_addr)
 *    Broadcast address on the network.
 * -  \b gxPLIoFuncGetLocalAddr
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetLocalAddr, gxPLIoAddr * local_addr)
 *    Local address associated with this machine on the network.
 * -  \b gxPLIoFuncNetAddrToString
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncNetAddrToString, gxPLIoAddr * net_addr, char ** str_addr)
 *    Converts a network address to a corresponding character string
 * .
 *
 * @param gxpl pointer to a gxPL object
 * @param req request code
 * @param ... optional parameters
 * @return 0, -1 if an error occurs
 */
int gxPLIoCtl (gxPL * gxpl, int req, ...);

/**
 * @}
 */


# ifndef __DOXYGEN__
// -----------------------------------------------------------------------------

/*
 * @brief Create a new xPL device
 * @param gxpl
 * @param vendor_id
 * @param device_id
 * @param instance_id
 * @return 
 */
gxPLDevice * gxPLDeviceNew (gxPL * gxpl,
                            const char * vendor_id,
                            const char * device_id,
                            const char * instance_id);
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
 * @param filename
 * @return
 */
gxPLDevice * gxPLDeviceConfigNew (gxPL * gxpl, const char * vendor_id,
    const char * device_id,
    const char * filename);
                            
/*
 * @brief Release an xPL device
 * @param device
 */
void gxPLDeviceDelete (gxPLDevice * device);

/*
 * @brief Messages handler
 * @param device
 * @param message
 * @param udata
 */
void gxPLDeviceMessageHandler (gxPLDevice * device, gxPLMessage * message,
                               void * udata);

/*
 * @brief Sends an heartbeat immediately
 * @param device pointer on the device
 * @param type
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceHeartbeatSend (gxPLDevice * device, gxPLHeartbeatType type);

/*
 * @brief 
 * @param config
 * @param argc
 * @param argv
 */
void gxPLParseCommonArgs (gxPLSetting * config, int argc, char *argv[]);

// -----------------------------------------------------------------------------
# endif /* __DOXYGEN__ not defined */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_HEADER_ defined */
