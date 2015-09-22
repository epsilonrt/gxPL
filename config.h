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
#define DEFAULT_IO_LAYER "udp"
#define DEFAULT_CONNECT_TYPE gxPLConnectViaHub
#define DEFAULT_HEARTBEAT_INTERVAL 300

/* build options ============================================================ */
#define CONFIG_ALLOC_STR_GROW  256
#define CONFIG_HEARTBEAT_INTERVAL 60
#define CONFIG_HUB_DISCOVERY_INTERVAL 3

#ifndef NDEBUG
#define GXPL_LOG_DEBUG_LEVEL LOG_DEBUG
#else
#define GXPL_LOG_DEBUG_LEVEL LOG_INFO
#endif

/* ========================================================================== */
#ifdef __cplusplus
}
#endif
#endif /* _CONFIG_H_ defined */
