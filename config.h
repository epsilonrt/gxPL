/**
 * @file
 * gxPLib Configuration file
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif

/* constants ================================================================ */

/* default values =========================================================== */
#define DEFAULT_UNIX_IO_LAYER             "udp"
#define DEFAULT_AVR8_IO_LAYER             "xbeezb"
#define DEFAULT_AVR8_EEFILE_MAX           512
#define DEFAULT_CONNECT_TYPE              gxPLConnectViaHub
#define DEFAULT_HEARTBEAT_INTERVAL        300
#define DEFAULT_CONFIG_HEARTBEAT_INTERVAL 60
#define DEFAULT_CONFIG_HOME_DIRECTORY     ".gxpl"
#define DEFAULT_CONFIG_SYS_DIRECTORY      "/etc/gxpl"
#define DEFAULT_HUB_DISCOVERY_INTERVAL    3
#define DEFAULT_ALLOC_STR_GROW            256
#define DEFAULT_LINE_BUFSIZE              256
#define DEFAULT_MAX_DEVICE_GROUP          4
#define DEFAULT_MAX_DEVICE_FILTER         4
#define DEFAULT_XBEE_BAUDRATE             38400
#define DEFAULT_XBEE_PORT                 "/dev/ttyUSB0"
#define DEFAULT_XBEE_FLOW                 SERIAL_FLOW_RTSCTS

/* build options ============================================================ */
#define CONFIG_DEVICE_CONFIGURABLE    1
#define CONFIG_DEVICE_GROUP           1
#define CONFIG_DEVICE_FILTER          1
// add the "remote-addr" field in hbeat.basic
#define CONFIG_HBEAT_BASIC_EXTENSION  1

/* conditionals options ====================================================== */
#ifndef NDEBUG
#define GXPL_LOG_DEBUG_LEVEL LOG_DEBUG
#else
#define GXPL_LOG_DEBUG_LEVEL LOG_INFO
#endif

#if defined(__unix__)
#define DEFAULT_IO_LAYER DEFAULT_UNIX_IO_LAYER
#elif defined(__AVR__)
#define DEFAULT_IO_LAYER DEFAULT_AVR8_IO_LAYER
#else
#error this platform is not supported yet
#endif
/* ========================================================================== */
#ifdef __cplusplus
}
#endif
#endif /* _CONFIG_H_ defined */
