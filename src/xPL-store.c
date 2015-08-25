/**
 * @file xPL-store.c
 * Memory management for xPLLib
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "xPL-private.h"


#define STRING_CACHE_MAX 256
#define GROW_CACHE_BY 16

#define CONFIRM_CACHE_OK if (NameValueCache == NULL) initCaches();

/* Structured cache info */
typedef void * cachedItem;
typedef char * itemCacheList;
typedef struct {
  itemCacheList *cachedItems;
  int cacheAllocCount;
  int cacheCount;
} itemCache;

/* Caches */
static itemCache * NameValueCache = NULL;
static itemCache * NameValueListCache = NULL;
static itemCache * MessageCache = NULL;
static itemCache * ServiceCache = NULL;

static int totalNameValueAlloc = 0;
static int totalNameValueListAlloc = 0;
static int totalMessageAlloc = 0;
static int totalServiceAlloc = 0;

/* String cache */
static itemCache * StringCache[STRING_CACHE_MAX];

/* Allocate a new cache pointer */
static itemCache * allocItemCache (void) {
  itemCache * theCache = malloc (sizeof (itemCache));
  bzero (theCache, sizeof (itemCache));
  return theCache;
}

/* Put an item into the cache */
static void releaseItem (cachedItem theItem, itemCache * theCache) {
  /* See if the cache needs more room */
  if (theCache->cacheCount == theCache->cacheAllocCount) {
    theCache->cacheAllocCount += GROW_CACHE_BY;
    theCache->cachedItems = realloc (theCache->cachedItems, sizeof (itemCacheList) * theCache->cacheAllocCount);
  }
  theCache->cachedItems[theCache->cacheCount++] = theItem;
}

/* Get an item from the cache */
static cachedItem allocItem (itemCache * theCache) {
  if (theCache->cacheCount == 0) {
    return NULL;
  }
  return theCache->cachedItems[-- (theCache->cacheCount)];
}

/* Initialize the caches */
static void initCaches (void) {
  NameValueCache = allocItemCache();
  NameValueListCache = allocItemCache();
  MessageCache = allocItemCache();
  ServiceCache = allocItemCache();

  bzero (StringCache, sizeof (StringCache));
}

void xPL_FreeNVPair (xPL_NameValuePair * thePair) {
  CONFIRM_CACHE_OK;
  xPL_Debug ("STORE:: Releasing NameValuePair @ %p back to cache pool", thePair);
  releaseItem (thePair, NameValueCache);
}

xPL_NameValuePair * xPL_AllocNVPair (void) {
  xPL_NameValuePair * thePair;
  CONFIRM_CACHE_OK;

  /* Try for a cached item */
  if ( (thePair = (xPL_NameValuePair *) allocItem (NameValueCache)) == NULL) {
    thePair = malloc (sizeof (xPL_NameValuePair));
    xPL_Debug ("STORE:: Allocating new xPL_NameValuePair @ %p, now %d pairs allocated", thePair, ++totalNameValueAlloc);
  }
  else {
    xPL_Debug ("STORE:: Reused NameValuePair @ %p from cache", thePair);
  }

  bzero (thePair, sizeof (xPL_NameValuePair));
  return thePair;
}

void xPL_FreeNVList (xPL_NameValueList * theList) {
  CONFIRM_CACHE_OK;
  xPL_Debug ("STORE:: Releasing NameValueList @ %p back to cache pool", theList);
  releaseItem (theList, NameValueListCache);
}

xPL_NameValueList * xPL_AllocNVList (void) {
  xPL_NameValueList * theList;
  CONFIRM_CACHE_OK;

  /* Try for a cached item */
  if ( (theList = (xPL_NameValueList *) allocItem (NameValueListCache)) == NULL) {
    theList = malloc (sizeof (xPL_NameValueList));
    bzero (theList, sizeof (xPL_NameValueList));
    xPL_Debug ("STORE:: Allocating new xPL_NameValueList @ %p, now %d lists allocted", theList, ++totalNameValueListAlloc);
  }
  else {
    theList->namedValueCount = 0;
    xPL_Debug ("STORE:: Reused NameValueList @ %p from cache", theList);
  }

  return theList;
}

void xPL_FreeMessage (xPL_Message * theMessage) {
  CONFIRM_CACHE_OK;
  xPL_Debug ("STORE:: Relesing xPL_Message @ %p back to cache pool", theMessage);
  releaseItem (theMessage, MessageCache);
}

xPL_Message * xPL_AllocMessage (void) {
  xPL_Message * theMessage;

  CONFIRM_CACHE_OK;
  if ( (theMessage = (xPL_Message *) allocItem (MessageCache)) == NULL) {
    theMessage = malloc (sizeof (xPL_Message));
    bzero (theMessage, sizeof (xPL_Message));
    xPL_Debug ("STORE:: Allocated new xPL_Message, now %d messages allocated", ++totalMessageAlloc);
  }
  else {
    xPL_Debug ("STORE:: Reused xPL_Message @ %p from cache", theMessage);
  }

  return theMessage;
}

void xPL_FreeService (xPL_Service * theService) {
  CONFIRM_CACHE_OK;
  xPL_Debug ("STORE:: Releasing xPL_Service @ %p back to cache pool", theService);
  releaseItem (theService, ServiceCache);
}

xPL_Service * xPL_AllocService (void) {
  xPL_Service * theService;

  CONFIRM_CACHE_OK;
  if ( (theService = (xPL_Service *) allocItem (ServiceCache)) == NULL) {
    theService = malloc (sizeof (xPL_Service));
    xPL_Debug ("STORE:: Allocated new xPL_Service, now %d services allocated", ++totalServiceAlloc);
  }
  else {
    xPL_Debug ("STORE:: Reused xPL_Service @ %p from cache", theService);
  }

  bzero (theService, sizeof (xPL_Service));
  return theService;
}

void xPL_FreeStr (char * theString) {
  int theLength;
  itemCache * theCache;

  CONFIRM_CACHE_OK;

  /* Get the length of a valid string.  If bigger than */
  /* the strings we are caching, dump it               */
  if (theString == NULL) {
    return;
  }
  if ( (theLength = strlen (theString)) > STRING_CACHE_MAX) {
    xPL_Debug ("STORE:: Permanently releasing large string @ %p, [%s]", theString, theString);
    free (theString);
    return;
  }

  /* Get the cache and init it if needed */
  if ( (theCache = StringCache[theLength]) == NULL) {
    xPL_Debug ("STORE:: Allocating new cache for strings %d characters long", theLength);
    theCache = allocItemCache();
    StringCache[theLength] = theCache;
  }

  /* Release the item into the cache */
  xPL_Debug ("STORE:: Releasing string @ %p to cache -- [%s] (LEN=%d, CNT=%d)", theString, theString, theLength, theCache->cacheCount);
  releaseItem (theString, theCache);
}

char * xPL_StrDup (char * theOrigString) {
  char * theString;

  /* Handle attempt to copy null */
  if (theOrigString == NULL) {
    return NULL;
  }

  /* Allocate a string and copy */
  theString = xPL_StrAlloc (strlen (theOrigString));
  strcpy (theString, theOrigString);
  xPL_Debug ("STORE:: Duped string [%s] (%d bytes)", theOrigString, strlen (theOrigString));
  return theString;
}

/* Copy at MOST the passed number of characters.  If the */
/* string is shorted, we'll copy less.                   */
char * xPL_StrNDup (char * theOrigString, int maxChars) {
  char * theString;
  int theLength;

  /* Get the length of a valid string.  If bigger than */
  /* the strings we are caching, dump it               */
  if (theOrigString == NULL) {
    return NULL;
  }

  /* Get the string length, and truncate it to our max */
  if ( (theLength = strlen (theOrigString)) > maxChars) {
    theLength = maxChars;
  }

  /* Allocate a string and copy */
  theString = xPL_StrAlloc (theLength);
  strncpy (theString, theOrigString, theLength);
  theString[theLength] = '\0';
  xPL_Debug ("STORE:: Duped string [%s] (%d bytes copied, %d bytes max)", theOrigString, strlen (theOrigString), maxChars);
  return theString;
}

char * xPL_StrAlloc (int theLength) {
  char * theString;
  itemCache * theCache;

  CONFIRM_CACHE_OK;

  /* Skip bad strings */
  if (theLength < 0) {
    return NULL;
  }

  /* If bigger than  the strings we are caching, dump it */
  if (theLength > STRING_CACHE_MAX) {
    theString = (char *) malloc (theLength + 1);
    theString[0] = '\0';
    xPL_Debug ("STORE:: Allocating entirely new LARGE string @ %p (LEN=%d)", theString, theLength);
    return theString;
  }

  /* Get the cache.  If no cache, just malloc() a new one */
  if ( (theCache = StringCache[theLength]) == NULL) {
    theString = (char *) malloc (theLength + 1);
    theString[0] = '\0';
    xPL_Debug ("STORE:: Cache Empty, Allocating entirely new string @ %p (LEN=%d)", theString, theLength);
    return theString;
  }

  /* Get an item from the cache.  If none is found/available, malloc() a new one */
  if ( (theString = allocItem (theCache)) == NULL) {
    theString = (char *) malloc (theLength + 1);
    theString[0] = '\0';
    xPL_Debug ("STORE:: No Cache Entry found, Allocating entirely new string @ %p (LEN=%d)", theString, theLength);
    return theString;
  }

  xPL_Debug ("STORE:: Reusing cache entry for String @ %p (LEN=%d)", theString, theLength);
  theString[0] = '\0';
  return theString;
}
