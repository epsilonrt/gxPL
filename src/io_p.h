/**
 * @file xPL-private.h
 * xPLLib internal include
 * 
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _XPL4LINUX_IO_PRIVATE_HEADER_
#define _XPL4LINUX_IO_PRIVATE_HEADER_

#include <xPL/io.h>
#include "utils_p.h"

/* public variables ========================================================= */
extern int xPLFD;   /* FD We are connecting on */
extern pid_t xPLPID;

/* internal public functions ================================================ */

/* io.c */
bool xPL_sendRawMessage(char *, int);
char * xPL_getFairlyUniqueIdent(void);

/* ========================================================================== */
#endif /* _XPL4LINUX_IO_PRIVATE_HEADER_ defined */
