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
  gxPLIoFuncNone = 0,
  gxPLIoFuncPoll,
  gxPLIoFuncGetIface,
  gxPLIoFuncGetBcastAddr,
  gxPLIoFuncGetLocalAddr,
  gxPLIoFuncNetAddrToString,
  gxPLIoFuncError = -1
} gxPLIoFunc;

/**
 * @brief Io families
 */
typedef enum {
  gxPLNetFamilyInet4    = 2,
  gxPLNetFamilyInet6    = 3, // gxPLNetFamilyInet4 | 1
  gxPLNetFamilyZigbee16 = 4,
  gxPLNetFamilyZigbee64 = 5, // gxPLNetFamilyZigbee16 | 1
} gxPLNetFamily;

/**
 * @brief Decoding states of a message
 */
typedef enum {
  gxPLMessageStateInit = 0,
  gxPLMessageStateHeader,
  gxPLMessageStateHeaderHop,
  gxPLMessageStateHeaderSource,
  gxPLMessageStateHeaderTarget,
  gxPLMessageStateHeaderEnd,
  gxPLMessageStateSchema,
  gxPLMessageStateBodyBegin,
  gxPLMessageStateBody,
  gxPLMessageStateBodyEnd,
  gxPLMessageStateEnd,
  gxPLMessageStateError = -1
} gxPLMessageState;
  
/* types ==================================================================== */

/* structures =============================================================== */
/**
 * @brief Describe a gxPL configuration
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
 * @brief Describe a network address
 */
typedef struct _gxPLNetAddress {
  gxPLNetFamily family;
  uint8_t size;
  uint16_t port;  /**< port in host order */
  union {
    uint16_t flag;
    struct {
      uint16_t isbroadcast: 1;
    };
  };
  uint8_t n_addr[16]; /**< address in network order */
} gxPLNetAddress;

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_DEFS_HEADER_ defined */
