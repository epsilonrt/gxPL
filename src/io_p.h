/**
 * @file private.h
 * gxPLib internal include
 * 
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_IO_PRIVATE_HEADER_
#define _GXPL_IO_PRIVATE_HEADER_

#include <gxPL/io.h>
#include "utils_p.h"

/* public variables ========================================================= */
extern int xPLFD;   /* FD We are connecting on */
extern pid_t xPLPID;

/* internal public functions ================================================ */

/* io.c */
bool xPL_sendRawMessage(char *, int);
char * xPL_getFairlyUniqueIdent(void);

/* ========================================================================== */
#endif /* _GXPL_IO_PRIVATE_HEADER_ defined */
