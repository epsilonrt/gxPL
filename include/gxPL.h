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

__BEGIN_C_DECLS
/* ========================================================================== */
#if defined(SYSIO_OS_UNIX)
#include <limits.h>
#else
#define NAME_MAX 255
#endif

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
  gxPLIo * io;
} gxPL;

/* api functions ============================================================ */
/**
 * @brief
 * @param iface
 * @param iolayer
 * @param type
 * @return
 */
gxPL * gxPLOpen (const char * iface, const char * iolayer, gxPLConnectType type);

/**
 * @brief 
 * @param argc
 * @param argv
 * @param type
 * @return 
 */
gxPL * gxPLOpenWithArgs (int argc, char * argv[], gxPLConnectType type);

/**
 * @brief 
 * @param argc
 * @param argv
 * @param type
 * @return 
 */
gxPL * gxPLOpenWithConfig (gxPLConfig * config);

/**
 * @brief
 * @param xpl
 * @return
 */
int gxPLClose (gxPL * xpl);

/**
 * @brief 
 * @param gxpl
 * @param timeout_ms
 * @return 
 */
int gxPLPoll (gxPL * gxpl, int timeout_ms);

# ifdef __DOXYGEN__
/*
 * __DOXYGEN__ defined
 * =============================================================================
 */

/**
 * @brief
 * @param xpl
 * @return
 */
static inline gxPLConnectType gxPLGetConnectionType (gxPL * xpl);

/**
 * @brief
 * @param xpl
 * @return
 */
static inline const char * gxPLGetDeviceName (gxPL * xpl);

/**
 * @brief
 * @param xpl
 * @return
 */
static inline const char * gxPLGetIoName (gxPL * xpl);

# else
/*
 * __DOXYGEN__ not defined
 * =============================================================================
 */
// -----------------------------------------------------------------------------
INLINE gxPLConnectType
gxPLGetConnectionType (gxPL * xpl) {
  return xpl->config->connecttype;
}

// -----------------------------------------------------------------------------
INLINE const char *
gxPLGetDeviceName (gxPL * xpl) {
  return xpl->config->iface;
}

// -----------------------------------------------------------------------------
INLINE const char *
gxPLGetIoName (gxPL * xpl) {
  return xpl->config->iolayer;
}

# endif /* __DOXYGEN__ not defined */

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
