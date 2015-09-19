/**
 * @file gxPL/defs.h
 * gxPL Defs
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_DEFS_HEADER_
#define _GXPL_DEFS_HEADER_

#include <sysio/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */
#include <errno.h>

/* forward struct defined */
typedef struct _gxPL gxPL;
typedef struct _gxPLIo gxPLIo;
typedef struct _gxPLService gxPLService;
typedef struct _gxPLMessage gxPLMessage;
typedef struct _gxPLNameValueList gxPLNameValueList;
typedef struct _gxPLServiceChangedListenerDef gxPLServiceChangedListenerDef;
typedef struct _gxPLServiceConfigurable gxPLServiceConfigurable;

#if defined(SYSIO_OS_UNIX)
#include <limits.h>
#else
#define NAME_MAX 255
#endif

#ifndef EINVAL
#define EINVAL          22      /* Invalid argument */
#endif

/**
 * @defgroup xPLDefs Definitions
 * @{
 */

/* constants ================================================================ */

/**
 * Communications between xPL applications on a Local Area Network (LAN) use UDP on port 3865
 */
#define XPL_PORT 3865

/**
 * @brief xPL Connection mode
 */
typedef enum {
  gxPLConnectStandAlone, /**< listen on xPL port */
  gxPLConnectViaHub,     /**< listen on a client port */ 
  gxPLConnectAuto
} gxPLConnectType;

/**
 * @brief
 */
typedef enum {
  gxPLConfigOptional,
  gxPLConfigMandatory,
  gxPLConfigReconf
} gxPLConfigurableType;

/**
 * @brief Possible xPL message types
 */
typedef enum {
  gxPLMessageAny,
  gxPLMessageCommand,
  gxPLMessageStatus,
  gxPLMessageTrigger
} gxPLMessageType;


/**
 * @brief Possible xPL ioctl call
 */
typedef enum {
  gxPLIoFuncPoll,
  gxPLIoFuncGetIface,
  gxPLIoFuncGetBcastAddr,
  gxPLIoFuncGetLocalAddr,
  gxPLIoFuncNetAddrToString,
  gxPLIoFuncGetInetPort,
} gxPLIoFunc;

/**
 * @brief Io families
 */
typedef enum {
  gxPLIoFamilyInet4,
  gxPLIoFamilyInet6,
  gxPLIoFamilyZigbee16,
  gxPLIoFamilyZigbee64,
} gxPLIoFamily;
  
/* types ==================================================================== */
/**
 * @brief Generic xPL object, void for now, but could change in the future
 */
typedef void xPL_Object;

/**
 * @brief Network address
 */
typedef struct _gxPLAddress {
  gxPLIoFamily family;
  uint8_t n_addr[16]; /**< address in network order */
} gxPLAddress;

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_DEFS_HEADER_ defined */
