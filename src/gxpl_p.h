/**
 * @file gxpl_p.h
 * gxPLib internal include
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_PRIVATE_HEADER_
#define _GXPL_PRIVATE_HEADER_

#include <gxPL.h>
#include <sysio/vector.h>

/* structures =============================================================== */

/*
 * @brief
 */
typedef struct _gxPL {

  gxPLConfig * config;
  gxPLIo * io;  /**< abstract structure can not be used directly on top level */
  xVector msg_listener;
  xVector device;
  gxPLIoAddr net_info;
} gxPL;

/* constants ================================================================ */
/* macros =================================================================== */

/* ========================================================================== */
#endif /* _GXPL_PRIVATE_HEADER_ defined */
