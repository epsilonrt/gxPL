/**
 * @file gxPL.h
 * gxPL API
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_HEADER_
#define _GXPL_HEADER_

#include <gxPL/defs.h>
#include <gxPL/message.h>

__BEGIN_C_DECLS
/* ========================================================================== */
/**
 * @defgroup gxPLApi Top Level API
 * @{
 */

/* api functions ============================================================ */
/**
 * @brief Returns a new gxPL config from parameters
 * 
 * @param iface network interface name o, the system
 * @param iolayer network access layer name
 * @param type network connection type
 * @return the config or NULL if error occurs
 */
gxPLConfig * gxPLNewConfig (const char * iface, const char * iolayer, gxPLConnectType type);

/**
 * @brief Returns a new gxPL config from command line parameters
 * @param argc number of parameters from main
 * @param argv list of parameters from main
 * @param type network connection type
 * @return the config or NULL if error occurs
 */
gxPLConfig * gxPLNewConfigFromCommandArgs (int argc, char * argv[], gxPLConnectType type);

/**
 * @brief Opens a new gxPL object
 * @param config pointer to a configuration, this configuration can be modified 
 * by the function to return the actual configuration.
 * @return the object or NULL if error occurs
 */
gxPL * gxPLOpen (gxPLConfig * config);

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
 * @return 0, -1 if error occurs
 */
int gxPLSendMessage (gxPL * gxpl, gxPLMessage * message);

/**
 * @brief Check if a message is echo hub
 * @param gxpl pointer to a gxPL object
 * @param message pointer to the message
 * @param my_id identifier of the request source. Necessary if the underlying 
 * network is not udp, may be NULL otherwise.
 * @return
 */
int gxPLMessageIsHubEcho (const gxPL * gxpl, const gxPLMessage * message, const gxPLMessageId * my_id);

/**
 * @brief System call for device-specific input/output operations
 *
 * system call for device-specific input/output operations and other operations
 * which cannot be expressed by regular system calls. \n
 * It takes a parameter specifying a request code; the effect of a call depends
 * completely on the request code. Request codes are often device-specific.
 *
 * -  \b gxPLIoFuncGetBcastAddr
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetBcastAddr, gxPLNetAddress * bcast_addr)
 *    Broadcast address on the network.
 * -  \b gxPLIoFuncGetLocalAddr
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetLocalAddr, gxPLNetAddress * local_addr)
 *    Local address associated with this machine on the network.
 * -  \b gxPLIoFuncNetAddrToString
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncNetAddrToString, gxPLNetAddress * net_addr, char ** str_addr)
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
 * @brief Local network address as a string
 * 
 * @param gxpl pointer to a gxPL object
 * @return the address as a string, NULL if an error occurs
 */
const char * gxPLLocalAddressString (const gxPL * gxpl);

/**
 * @brief Broadcast network address as a string
 * 
 * @param gxpl pointer to a gxPL object
 * @return the address as a string, NULL if an error occurs
 */
const char * gxPLBroadcastAddressString (const gxPL * gxpl);

/**
 * @brief Local Network informations
 * 
 * @param gxpl pointer to a gxPL object
 * @return network infos, NULL if an error occurs
 */
const gxPLNetAddress * gxPLNetInfo (const gxPL * gxpl);

/**
 * @brief Connection type
 * 
 * @param gxpl pointer to a gxPL object
 * @return the type
 */
gxPLConnectType gxPLGetConnectionType (const gxPL * gxpl);

/**
 * @brief Name of the network interface on the system
 * 
 * @param gxpl pointer to a gxPL object
 * @return the name, NULL if an error occurs
 */
const char * gxPLGetInterfaceName (const gxPL * gxpl);

/**
 * @brief Name of the underlying layer of the network
 * 
 * @param gxpl pointer to a gxPL object
 * @return the name, NULL if an error occurs
 */
const char * gxPLGetIoLayerName (const gxPL * gxpl);

/**
 * @defgroup xPLMessageListener Message Listeners
 * Message Listeners 
 * @{
 */

/* types ==================================================================== */
/**
 * @brief Function that will be called each valid message reception
 */
typedef int (* gxPLMessageListener) (const gxPLMessage *, const gxPLNetAddress *, void *);

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


# ifndef __DOXYGEN__
/*
 * __DOXYGEN__ not defined
 * =============================================================================
 */
# endif /* __DOXYGEN__ not defined */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_HEADER_ defined */
