/**
 * @file xpl_p.h
 * xPLLib internal include
 * 
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _XPL4LINUX_PRIVATE_HEADER_
#define _XPL4LINUX_PRIVATE_HEADER_

#include "version.h"
#include "config.h"

/* constants ================================================================ */
#ifndef INADDR_NONE
# define INADDR_NONE 0xffffffff
#endif
#define SYSCALL(call) while(((call) == -1) && (errno == EINTR))

/* macros =================================================================== */
/*
 * man 3 free:
 * ... If ptr is NULL, no operation is performed. 
 */
#define SAFE_FREE(x) if (x != NULL) { free(x); x = NULL; }
#define STR_FREE(x) if (x != NULL) { xPL_FreeStr(x); x = NULL; }

/* ========================================================================== */
#endif /* _XPL4LINUX_PRIVATE_HEADER_ defined */
