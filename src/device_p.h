/**
 * @file src/device_p.h
 * High level interface to manage xPL devices (private header)
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
 * @brief Describe a device configuration
 */
typedef struct _gxPLDeviceConfig {
  
  char * filename;
  xVector items; /**< vector of gxPLDeviceConfigItem */
  xVector listener; /**< vector of listener_elmt (config changed) */
} gxPLDeviceConfig;

/*
 * @brief Describe a device
 */
typedef struct _gxPLDevice {
  
  gxPLId id;
  char * version;
  gxPLApplication * parent;
  xVector listener; /**< vector of listener_elmt (message received) */
  
  int hbeat_interval; /**< heartbeat interval in seconds */
  long hbeat_last;
  gxPLMessage * hbeat_msg;
    
  // Optionnal fields
#if CONFIG_DEVICE_CONFIGURABLE
  gxPLDeviceConfig * config;
#endif /* CONFIG_DEVICE_CONFIGURABLE true */

#if CONFIG_DEVICE_GROUP
  xVector group; /**< vector of string */
  uint8_t group_max;
#endif /* CONFIG_DEVICE_GROUP true */

#if CONFIG_DEVICE_FILTER
  xVector filter;  /**< vector of gxPLFilter* */
  uint8_t filter_max;
#endif /* CONFIG_DEVICE_FILTER true */

  union {
    unsigned int flag;
    struct {

      unsigned int isenabled : 1;
      unsigned int nobroadcast : 1;
      unsigned int isreportownmsg: 1;
      unsigned int ishubconfirmed: 1;
      unsigned int isconfigured: 1;
      unsigned int isconfigurable: 1;
      unsigned int havegroup: 1;
      unsigned int havefilter: 1;
    };
  };
} gxPLDevice;

/* internal public functions ================================================ */

/**
 * -----------------------------------------------------------------------------
 * @addtogroup xPLDeviceConfig
 * @{
 */

/**
 * @brief Load config from file
 *
 * Clear out existing configuration data and attempt to load it from the
 * currently installed config file.  If there is no installed config file,
 * nothing happens.  If there is a file specified but it does not exist, any
 * previous config data is lost, but no error is thrown (it may be this is the
 * first use of this file).
 * @param device
 * @return a vector of gxPLPair, NULL if error occurs
 */
xVector * gxPLDeviceConfigLoad (gxPLDevice * device);

/**
 * @brief Save out the current configuration
 * @param device
 * @return 0, -1 if error occurs
 */
int gxPLDeviceConfigSave (const gxPLDevice * device);

/**
 * @brief 
 * @param device
 */
void gxPLDeviceConfigDelete (gxPLDevice * device);

/**
 * @}
 * -----------------------------------------------------------------------------
 */

/**
 * -----------------------------------------------------------------------------
 * @addtogroup xPLDeviceGroup
 * @{
 */
/**
 * @brief 
 * @param device
 * @return 
 */
int gxPLDeviceGroupInit (gxPLDevice * device);

/**
 * @brief 
 * @param device
 */
void gxPLDeviceGroupDelete (gxPLDevice * device);

/**
 * @brief 
 * @param device
 * @param message
 * @return 
 */
int  gxPLDeviceGroupAddListOfItems (gxPLDevice * device, gxPLMessage * message);

/**
 * @brief 
 * @param device
 * @param message
 * @return 
 */
int  gxPLDeviceGroupAddCurrentValues (gxPLDevice * device, gxPLMessage * message);

/**
 * @}
 * -----------------------------------------------------------------------------
 */

/**
 * -----------------------------------------------------------------------------
 * @addtogroup xPLDeviceFilter
 * @{
 */

/**
 * @brief 
 * @param device
 * @return 
 */
int gxPLDeviceFilterInit (gxPLDevice * device);

/**
 * @brief 
 * @param device
 */
void gxPLDeviceFilterDelete (gxPLDevice * device);

/**
 * @brief 
 * @param device
 * @param message
 * @return 
 */
int  gxPLDeviceFilterAddListOfItems (gxPLDevice * device, gxPLMessage * message);

/**
 * @brief 
 * @param device
 * @param message
 * @return 
 */
int  gxPLDeviceFilterAddCurrentValues (gxPLDevice * device, gxPLMessage * message);

/**
 * @}
 * -----------------------------------------------------------------------------
 */

/* ========================================================================== */
#endif /* _GXPL_SERVICE_PRIVATE_HEADER_ defined */
