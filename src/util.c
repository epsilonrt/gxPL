/**
 * @file util.c
 * Misc support for gxPLib
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <string.h>
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
int
gxPLStrCpy (char * dst, const char * src) {
  int c;
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
    }
    else {

      // c is not a ASCII character
      errno = EINVAL;
      return -1;
    }
  }
  return 0;
}

/* ========================================================================== */
