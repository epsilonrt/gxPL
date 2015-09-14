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

/* constants ================================================================ */

/* macros =================================================================== */
/*
 * man 3 free:
 * ... If ptr is NULL, no operation is performed. 
 */
#define SAFE_FREE(x) if (x != NULL) { free(x); x = NULL; }
#define STR_FREE(x)  if (x != NULL) { xPL_FreeStr(x); x = NULL; }

/* internal public functions ================================================ */
int gxPLParseDatagram (gxPL * gxpl, char * data);

/* ========================================================================== */
#endif /* _GXPL_PRIVATE_HEADER_ defined */
