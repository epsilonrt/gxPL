/**
 * @file xPL/defs.h
 * xPL4Linux Defs
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _XPL4LINUX_DEFS_HEADER_
#define _XPL4LINUX_DEFS_HEADER_

#ifndef __BEGIN_C_DECLS
# if defined(__cplusplus)
#   define __BEGIN_C_DECLS  extern "C" {
#   define __END_C_DECLS    }
# else
#   define __BEGIN_C_DECLS
#   define __END_C_DECLS
# endif
#endif

__BEGIN_C_DECLS
/* ========================================================================== */
#include <stdint.h>

#ifndef __DOXYGEN__
# ifndef __cplusplus
#   include <stdbool.h>
# endif
# ifndef TRUE
#   define TRUE true
# endif
# ifndef FALSE
#   define FALSE false
# endif
/* forward struct defined */
typedef struct _xPL_Service xPL_Service;
typedef struct _xPL_Message xPL_Message;
typedef struct _xPL_NameValueList xPL_NameValueList;
typedef struct _xPL_ServiceChangedListenerDef xPL_ServiceChangedListenerDef;
typedef struct _xPL_ServiceConfigurable xPL_ServiceConfigurable;
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
  xcStandAlone,
  xcViaHub,
  xcAuto
} xPL_ConnectType;

/**
 * @brief
 */
typedef enum {
  xPL_CONFIG_OPTIONAL,
  xPL_CONFIG_MANDATORY,
  xPL_CONFIG_RECONF
} xPL_ConfigurableType;

/**
 * @brief Possible xPL message types
 */
typedef enum {
  xPL_MESSAGE_ANY,
  xPL_MESSAGE_COMMAND,
  xPL_MESSAGE_STATUS,
  xPL_MESSAGE_TRIGGER
} xPL_MessageType;

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
#endif /* _XPL4LINUX_DEFS_HEADER_ defined */
