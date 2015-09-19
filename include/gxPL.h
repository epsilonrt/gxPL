/**
 * @file gxPL.h
 * gxPL API
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
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
 * @defgroup gxPLib Library Context
 * @{
 */

/* structures =============================================================== */
/**
 * @brief
 */
typedef struct _gxPLConfig {

  char iface[NAME_MAX];
  char iolayer[NAME_MAX];
  gxPLConnectType connecttype;
  union {
    unsigned int flag;
    struct {
      unsigned int debug: 1;
      unsigned int malloc: 1;
    };
  };
} gxPLConfig;

/**
 * @brief
 */
typedef struct _gxPL {

  gxPLConfig * config;
  gxPLIo * io;  /**< abstract structure can not be used by top user. */
} gxPL;

/* api functions ============================================================ */
/**
 * @brief
 * @param iface
 * @param iolayer
 * @param type
 * @return
 */
gxPLConfig * gxPLNewConfig (const char * iface, const char * iolayer, gxPLConnectType type);

/**
 * @brief
 * @param argc
 * @param argv
 * @param type
 * @return
 */
gxPLConfig * gxPLNewConfigFromCommandArgs (int argc, char * argv[], gxPLConnectType type);

/**
 * @brief
 * @param argc
 * @param argv
 * @param type
 * @return
 */
gxPL * gxPLOpen (gxPLConfig * config);

/**
 * @brief
 * @param gxpl pointer to a gxPL structure
 * @return
 */
int gxPLClose (gxPL * gxpl);

/**
 * @brief
 * @param gxpl pointer to a gxPL structure
 * @param timeout_ms
 * @return
 */
int gxPLPoll (gxPL * gxpl, int timeout_ms);

/**
 * @brief Send an xPL message
 * If the message is valid and is successfully sent, TRUE is returned
 * @param message
 * @return
 */
int gxPLSendMessage (gxPL * gxpl, gxPLMessage * message);

/**
 * @brief System call for device-specific input/output operations
 *
 * system call for device-specific input/output operations and other operations
 * which cannot be expressed by regular system calls. \n
 * It takes a parameter specifying a request code; the effect of a call depends
 * completely on the request code. Request codes are often device-specific.
 *
 * -  \b gxPLIoFuncGetBcastAddr
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetBcastAddr, gxPLAddress * bcast_addr)
 *    Broadcast address on the network.
 * -  \b gxPLIoFuncGetLocalAddr
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetLocalAddr, gxPLAddress * local_addr)
 *    Local address associated with this machine on the network.
 * -  \b gxPLIoFuncNetAddrToString
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncNetAddrToString, gxPLAddress * net_addr, char ** str_addr)
 *    Converts a network address to a corresponding character string
 * -  \b gxPLIoFuncGetInetPort
 *    \code int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetInetPort, int * iport)
 *    IP port in host order, only if the interface is in the INET/INET6 family.
 * .
 *
 * @param gxpl pointer to a gxPL structure
 * @param req 
 * @return
 */
int gxPLIoCtl (gxPL * gxpl, int req, ...);

/**
 * @brief Local network address as a string
 */
char * gxPLLocalAddressString (gxPL * gxpl);

/**
 * @brief Broadcast network address as a string
 */
char * gxPLBroadcastAddressString (gxPL * gxpl);

/**
 * @brief IP port in host order
 * 
 * only if the interface is in the INET/INET6 family !
 */
int gxPLInetPort (gxPL * gxpl) ;

/**
 * @brief Set Debugging Mode
 * @param isDebugging
 */
void xPL_setDebugging (bool isDebugging);

/**
 * @brief  Return if debug mode in use
 * @return 
 */
bool gxPLIsDebugging (void);

# ifdef __DOXYGEN__
/*
 * __DOXYGEN__ defined
 * =============================================================================
 */

/**
 * @brief
 * @param gxpl
 * @return
 */
static inline gxPLConnectType gxPLGetConnectionType (gxPL * gxpl);

/**
 * @brief
 * @param gxpl
 * @return
 */
static inline const char * gxPLGetInterfaceName (gxPL * gxpl);

/**
 * @brief
 * @param gxpl
 * @return
 */
static inline const char * gxPLGetIoName (gxPL * gxpl);

# else
/*
 * __DOXYGEN__ not defined
 * =============================================================================
 */
// -----------------------------------------------------------------------------
INLINE gxPLConnectType
gxPLGetConnectionType (gxPL * gxpl) {
  return gxpl->config->connecttype;
}

// -----------------------------------------------------------------------------
INLINE const char *
gxPLGetInterfaceName (gxPL * gxpl) {
  return gxpl->config->iface;
}

// -----------------------------------------------------------------------------
INLINE const char *
gxPLGetIoName (gxPL * gxpl) {
  return gxpl->config->iolayer;
}
# endif /* __DOXYGEN__ not defined */

/**
 * @defgroup xPLMessageListener Listeners
 * Message Listener SupportService Listener Support
 * @{
 */

/* types ==================================================================== */
/**
 * @brief
 */
typedef void (* xPL_messageListener) (gxPLMessage *, xPL_Object *);

/* internal public functions ================================================ */

/**
 * @brief Add a message listener
 * @param theHandler
 * @param userValue
 */
void gxPLMessageAddListener (xPL_messageListener theHandler, xPL_Object * userValue);

/**
 * @brief
 * @param theHandler
 * @return
 */
int gxPLMessageRemoveListener (xPL_messageListener theHandler);


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

/* ========================================================================== */
__END_C_DECLS

#if 0
#include <gxPL/utils.h>
#include <gxPL/service.h>
#include <gxPL/message.h>
#include <gxPL/io.h>
#include <gxPL/hub.h>
#include <gxPL/compat.h>
#endif

#endif /* _GXPL_HEADER_ defined */
