/**
 * @file utils_p.h
 * gxPLib internal include
 * 
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_UTILS_PRIVATE_HEADER_
#define _GXPL_UTILS_PRIVATE_HEADER_

#include <gxPL/utils.h>
#include "xpl_p.h"

/* types ==================================================================== */
typedef void * cachedItem;
typedef char * itemCacheList;

/* structures =============================================================== */
/* Structured cache info */
typedef struct {
  itemCacheList *cachedItems;
  int cacheAllocCount;
  int cacheCount;
} itemCache;

/* public variables ========================================================= */
extern bool xPL_DebugMode;

/* internal public functions ================================================ */

/* utils.c */
xPL_NameValueList * xPL_newNamedValueList(void);
void xPL_freeNamedValueList(xPL_NameValueList *);
xPL_NameValuePair * xPL_newNamedValuePair(xPL_NameValueList *, char *);
void xPL_freeNamedValuePair(xPL_NameValuePair *);

/* alloc.c */
itemCache * xPL_AllocItemCache (void);
cachedItem xPL_AllocItem (itemCache * theCache);
void xPL_ReleaseItem (cachedItem theItem, itemCache * theCache);

void xPL_FreeNVPair(xPL_NameValuePair *);
xPL_NameValuePair * xPL_AllocNVPair(void);

void xPL_FreeNVList(xPL_NameValueList *);
xPL_NameValueList * xPL_AllocNVList(void);

char * xPL_StrAlloc(int);
void xPL_FreeStr(char *);
char * xPL_StrDup(char *);
char * xPL_StrNDup(char *, int);

/* ========================================================================== */
#endif /* _GXPL_UTILS_PRIVATE_HEADER_ defined */
