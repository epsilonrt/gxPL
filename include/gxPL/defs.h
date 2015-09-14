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

/* forward struct defined */
typedef struct _gxPL gxPL;
typedef struct _gxPLIo gxPLIo;
typedef struct _xPL_Service xPL_Service;
typedef struct _xPL_Message gxPLMessage;
typedef struct _xPL_NameValueList xPL_NameValueList;
typedef struct _xPL_ServiceChangedListenerDef xPL_ServiceChangedListenerDef;
typedef struct _xPL_ServiceConfigurable xPL_ServiceConfigurable;

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
  xPLConnectStandAlone, /**< listen on xPL port */
  xPLConnectViaHub,     /**< listen on a client port */ 
  xPLConnectAuto
} gxPLConnectType;

/**
 * @brief
 */
typedef enum {
  xPLConfigOptional,
  xPLConfigMandatory,
  xPLConfigReconf
} gxPLConfigurableType;

/**
 * @brief Possible xPL message types
 */
typedef enum {
  xPLMessageAny,
  xPLMessageCommand,
  xPLMessageStatus,
  xPLMessageTrigger
} gxPLMessageType;


/**
 * @brief Possible xPL ioctl call
 */
typedef enum {
  xPLIoFuncPoll
} gxPLIoFunc;


/* types ==================================================================== */
/**
 * @brief Generic xPL object, void for now, but could change in the future
 */
typedef void xPL_Object;

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_DEFS_HEADER_ defined */
