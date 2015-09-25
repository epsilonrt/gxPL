/**
 * @file service_p.h
 * gxPLib internal include
 * 
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_SERVICE_PRIVATE_HEADER_
#define _GXPL_SERVICE_PRIVATE_HEADER_

#include <time.h>
#include <sysio/vector.h>
#include <gxPL/defs.h>

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
  time_t hbeat_last;
  gxPLMessage * hbeat_msg;

  union {
    unsigned int flag;
    struct {

      unsigned int isenabled : 1;
      unsigned int nobroadcast : 1;
      unsigned int reportmsg: 1;
    };
  };
} gxPLDevice;

#if 0
int groupCount;
int groupAllocCount;
char **groupList;

bool configurableService;
bool serviceConfigured;
char * configFileName;
int configChangedCount;
int configChangedAllocCount;
gxPLDeviceChangedListenerDef * changedListenerList;

int configCount;
int configAllocCount;
gxPLDeviceConfigurable * configList;

int filterCount;
int filterAllocCount;
gxPLDeviceFilter * messageFilterList;
#endif

/* ========================================================================== */
#endif /* _GXPL_SERVICE_PRIVATE_HEADER_ defined */
