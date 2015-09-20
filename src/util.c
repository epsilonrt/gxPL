/**
 * @file util.c
 * Misc support for gxPLib
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <gxPL/util.h>

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */
/* types ==================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
/* public variables ========================================================= */
/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
// name=value\0
gxPLPair * 
gxPLPairFromString (char * str) {
  if (str) {
    char * name;

    char * value = str;

    name = strsep (&value, "=");
    if (value) {
      gxPLPair * p = malloc (sizeof(gxPLPair));
      assert (p);
      p->name = malloc (strlen (name) + 1);
      p->value = malloc (strlen (value) + 1);
      strcpy (p->name, name);
      strcpy (p->value, value);
      return p;
    }
  }
  return NULL;
}

// -----------------------------------------------------------------------------
int
gxPLStrCpy (char * dst, const char * src) {
  int c;
  int count = 0;
  char * p = dst;

  while ( (c = *src) != 0) {

    if (isascii (c)) {

      if ( (!isalnum (c)) && (c != '-')) {

        // c is not a letter, number or hyphen/dash
        errno = EINVAL;
        return -1;
      }

      if (isupper (c)) {

        // convert to lowercase
        *p = tolower (c);
      }
      else {

        // raw copy
        *p = c;
      }

      p++;
      src++;
      count++;
    }
    else {

      // c is not a ASCII character
      errno = EINVAL;
      return -1;
    }
  }
  return count;
}

/* ========================================================================== */
