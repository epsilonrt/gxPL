/**
 * @file device_p.h
 * gxPLib internal include
 * 
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_SERVICE_PRIVATE_HEADER_
#define _GXPL_SERVICE_PRIVATE_HEADER_

#include <sysio/vector.h>
#include <gxPL/defs.h>

/* types ==================================================================== */

/* structures =============================================================== */

/*
 * @brief Describe a xPL service
 */
typedef struct _gxPLDevice {
  
  gxPLId id;
  char * version;
  gxPL * parent;
  xVector listener;
  
  int hbeat_interval; /**< heartbeat interval in seconds */
  long hbeat_last;
  gxPLMessage * hbeat_msg;

  union {
    unsigned int flag;
    struct {

      unsigned int ishubconfirmed: 1;
      unsigned int isenabled : 1;
      unsigned int nobroadcast : 1;
      unsigned int isreportownmsg: 1;
      unsigned int isconfigured: 1;
      unsigned int isconfigurable: 1;
    };
  };
} gxPLDevice;

/* ========================================================================== */
#endif /* _GXPL_SERVICE_PRIVATE_HEADER_ defined */
