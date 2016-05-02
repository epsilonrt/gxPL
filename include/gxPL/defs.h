/**
 * @file
 * gxPLApplication library definitions
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_DEFS_HEADER_
#define _GXPL_DEFS_HEADER_

#if defined(__unix__)
#include <limits.h>
#include <assert.h>
#include <sysio/defs.h>
#include <sysio/log.h>
#include <sysio/dlist.h>
#include <sysio/vector.h>
#include <sysio/serial.h>
#include <sysio/delay.h>
#if defined(ARCH_ARM_RASPBERRYPI)
#include <sysio/doutput.h>
#endif
#define GXPL_TARGET_STR "unix"
#ifndef PROGMEM
#define PROGMEM
#endif
#elif defined(__AVR__)
#include <avrio/defs.h>
#include <avrio/vector.h>
#include <avrio/dlist.h>
#include <avrio/tc.h>
#include <avrio/dpin.h>
#include <avrio/assert.h>
#include <avrio/log.h>
#include <avrio/delay.h>
#define NAME_MAX 16
#define GXPL_TARGET_STR "avr"
#else
#error This target platform is not supported.
#endif

__BEGIN_C_DECLS
/* ========================================================================== */
#include <errno.h>

/* forward struct defined */
typedef struct _gxPLApplication gxPLApplication;
typedef struct _gxPLIo gxPLIo;
typedef struct _gxPLMessage gxPLMessage;
typedef struct _gxPLDevice gxPLDevice;
typedef struct _gxPLDeviceConfig gxPLDeviceConfig;
typedef struct _gxPLHub gxPLHub;
typedef struct _gxPLBridge gxPLBridge;

#ifndef EINVAL
#define EINVAL          22      /* Invalid argument */
#endif

/**
 * @defgroup gxPLDefsDoc Definitions
 * @{
 */

/* constants ================================================================ */

/**
 * Communications between xPL applications on a Local Area Network (LAN) use
 * UDP on port 3865
 */
#define XPL_PORT 3865

/**
 * @brief Maximum number of characters allowed for vendor ID
 */
#define GXPL_VENDORID_MAX   8

/**
 * @brief Maximum number of characters allowed for device ID
 */
#define GXPL_DEVICEID_MAX   8

/**
 * @brief Maximum number of characters allowed for instance ID
 */
#define GXPL_INSTANCEID_MAX 16

/**
 * @brief Maximum number of characters allowed for schema class
 */
#define GXPL_CLASS_MAX      8

/**
 * @brief Maximum number of characters allowed for schema type
 */
#define GXPL_TYPE_MAX       8

/**
 * @brief Maximum number of characters allowed for a name of a name/value pair
 */
#define GXPL_NAME_MAX       16

/**
 * @brief Maximum number of hop count
 */
#define GXPL_HOP_MAX   9

/**
 * @brief getopt short options used by gxPLSettingFromCommandArgs()
 */
#define GXPL_GETOPT "i:n:B:W:dDr"

/**
 * @brief default baudrate for serial iolayer
 */
#define GXPL_DEFAULT_BAUDRATE             38400

/**
 * @brief default baudrate for serial iolayer
 */
#define GXPL_DEFAULT_FLOW                 SERIAL_FLOW_RTSCTS

/**
 * @brief xPL Connection mode
 */
typedef enum {
  gxPLConnectStandAlone, /**< listen on xPL port */
  gxPLConnectViaHub,     /**< listen on a client port */
  gxPLConnectAuto
} gxPLConnectType;


/**
 * @brief Possible xPL message types
 */
typedef enum {
  gxPLMessageAny,
  gxPLMessageCommand,
  gxPLMessageStatus,
  gxPLMessageTrigger,
  gxPLMessageUnknown = -1
} gxPLMessageType;


/**
 * @brief Possible xPL ioctl call
 */
typedef enum {
  gxPLIoFuncNone = 0,
  gxPLIoFuncPoll,
  gxPLIoFuncGetIface,
  gxPLIoFuncGetBcastAddr,
  gxPLIoFuncGetNetInfo,
  gxPLIoFuncGetLocalAddrList,
  gxPLIoFuncNetAddrToString,
  gxPLIoFuncNetAddrFromString,
  gxPLIoFuncError = -1
} gxPLIoFunc;

/**
 * @brief Net families
 */
typedef enum {
  gxPLNetFamilyUnknown  = 0,
  gxPLNetFamilyInet     = 2, /**< family & gxPLNetFamilyInet -> true for two revisions of IP (v4 and v6) */
  gxPLNetFamilyInet4    = gxPLNetFamilyInet,
  gxPLNetFamilyInet6    = gxPLNetFamilyInet | 1,
  gxPLNetFamilyZigbee   = 4,
  gxPLNetFamilyZigbee16 = gxPLNetFamilyZigbee,
  gxPLNetFamilyZigbee64 = gxPLNetFamilyZigbee | 1
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

/**
 * @brief Heartbeat type
 */
typedef enum {
  gxPLHeartbeatHello    = 0,
  gxPLHeartbeatGoodbye  = 1,
} gxPLHeartbeatType;

/**
 * @brief xPL Configurable Type
 */
typedef enum {
  gxPLConfigOptional,
  gxPLConfigMandatory,
  gxPLConfigReconf
} gxPLConfigurableType;

/* types ==================================================================== */

/* structures =============================================================== */
/**
 * @brief Describe a XBee configuration
 */
typedef struct _gxPLIoXBeeSetting {
  xSerialIos ios; /**< serial port configuration */
  uint64_t panid; /**< Zigbee PAN ID, host order */
  union {
    uint8_t flag;
    struct {
      uint8_t coordinator: 1;
      uint8_t new_panid: 1;
      uint8_t reset_sw: 1;
      uint8_t reset_hw: 1;
    };
  };
#if defined(__AVR__)
#define GXPL_XBEEZB_HAS_HWRESET 1
  xDPin reset_pin;
#elif defined(ARCH_ARM_RASPBERRYPI)
#define GXPL_XBEEZB_HAS_HWRESET 1
  xDout reset_pin;
#endif
} gxPLIoXBeeSetting;

/**
 * @brief Describe a gxPLApplication configuration
 */
typedef struct _gxPLSetting {

  char iface[NAME_MAX]; /**< interface name */
  char iolayer[NAME_MAX]; /**< io layer name */
  gxPLConnectType connecttype;
  union {
    uint16_t flag;
    struct {
      uint16_t log: 3;      /**< log level 0 - 7 */
      uint16_t malloc: 1;     /**< this configuration has been allocated on the heap and should be released. */
      uint16_t nodaemon: 1;   /**< do not daemonize */
      uint16_t iosflag: 1;    /**< true if io setting was configured */
      uint16_t broadcast: 1;  /**< all broadcasts messages will be rebroadcasted by the bridge */
    };
  };
  unsigned iotimeout; /**< timeout at the opening of the io layer */
  union {

    gxPLIoXBeeSetting xbee;
  };
} gxPLSetting;

/**
 * @brief Describe a network address
 */
typedef struct _gxPLIoAddr {
  gxPLNetFamily family; /**< network family */
  uint8_t addrlen;  /**< number of bytes of the address */
  uint8_t addr[16]; /**< address in network order */
  int port;  /**< port in host order, -1 if not use */
  union {
    uint16_t flag;
    struct {
      uint16_t isbroadcast: 1;
    };
  };
} gxPLIoAddr;

/**
 * @brief Describe a source or destination xPL identifier
 */
typedef struct _gxPLId {
  char vendor[GXPL_VENDORID_MAX + 1]; /**< vendor id */
  char device[GXPL_DEVICEID_MAX + 1]; /**< devide id */
  char instance[GXPL_INSTANCEID_MAX + 1]; /**< instance id */
} gxPLId;

/**
 * @brief Describe a xPL schema
 */
typedef struct _gxPLSchema {
  char class[GXPL_CLASS_MAX + 1];
  char type[GXPL_TYPE_MAX + 1];
} gxPLSchema;

/**
 * @brief Describe a xPL filter
 */
typedef struct _gxPLFilter {

  gxPLMessageType type;
  gxPLId source;
  gxPLSchema schema;
} gxPLFilter;

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_DEFS_HEADER_ defined */
