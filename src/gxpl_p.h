/**
 * @file
 * gxPLib internal include
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_PRIVATE_HEADER_
#define _GXPL_PRIVATE_HEADER_

#include <gxPL.h>
#include "internal_p.h"

/* structures =============================================================== */

/*
 * @brief xPL Application
 */
typedef struct _gxPLApplication {

  gxPLSetting * setting;
  gxPLIo * io;  /**< abstract structure can not be used directly on top level */
  xVector msg_listener;
  xVector device;
  gxPLIoAddr net_info;
} gxPLApplication;


/* private api functions ==================================================== */
/*
 * @brief Gets the random seed for the application
 * @param app
 * @return 
 */
int gxPLRandomSeed (gxPLApplication * app);

/* ========================================================================== */
#endif /* _GXPL_PRIVATE_HEADER_ defined */
