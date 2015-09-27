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
  union {
    unsigned int flag;
    struct {
    };
  };
} gxPL;

/* constants ================================================================ */
/* macros =================================================================== */

/* private api functions ==================================================== */

/*
 * @brief Create a new xPL device
 * @param gxpl
 * @param vendor_id
 * @param device_id
 * @param instance_id
 * @return 
 */
gxPLDevice * gxPLDeviceNew (gxPL * gxpl,
                            const char * vendor_id,
                            const char * device_id,
                            const char * instance_id);
/*
 * @brief Release an xPL device
 * @param device
 */
void gxPLDeviceDelete (gxPLDevice * device);

/*
 * @brief Messages handler
 * @param device
 * @param message
 * @param udata
 */
void gxPLDeviceMessageHandler (gxPLDevice * device, const gxPLMessage * message,
                               void * udata);

/*
 * @brief Sends an heartbeat immediately
 * @param device pointer on the device
 * @param type
 * @return 0, -1 if an error occurs
 */
int gxPLDeviceHeartbeatSend (gxPLDevice * device, gxPLHeartbeatType type);

/*
 * @brief 
 * @param config
 * @param argc
 * @param argv
 */
void gxPLParseCommonArgs (gxPLConfig * config, int argc, char *argv[]);
/* ========================================================================== */
#endif /* _GXPL_PRIVATE_HEADER_ defined */
