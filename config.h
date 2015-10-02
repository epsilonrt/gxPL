/**
 * @file config.h
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
#define DEFAULT_CONNECT_TYPE              gxPLConnectViaHub
#define DEFAULT_HEARTBEAT_INTERVAL        300
#define DEFAULT_CONFIG_HEARTBEAT_INTERVAL 60
#define DEFAULT_HUB_DISCOVERY_INTERVAL    3
#define DEFAULT_ALLOC_STR_GROW            256
#define DEFAULT_LINE_BUFSIZE              256
#define DEFAULT_MAX_DEVICE_GROUP          4
#define DEFAULT_MAX_DEVICE_FILTER         4

/* build options ============================================================ */
#define CONFIG_DEVICE_CONFIGURABLE    1
#define CONFIG_DEVICE_GROUP           1
#define CONFIG_DEVICE_FILTER          1

/* conditionals options ====================================================== */
#ifndef NDEBUG
#define GXPL_LOG_DEBUG_LEVEL LOG_DEBUG
#else
#define GXPL_LOG_DEBUG_LEVEL LOG_INFO
#endif

#ifdef  __unix__
#define DEFAULT_IO_LAYER DEFAULT_UNIX_IO_LAYER
#else
#error this platform is not supported yet
#endif
/* ========================================================================== */
#ifdef __cplusplus
}
#endif
#endif /* _CONFIG_H_ defined */
