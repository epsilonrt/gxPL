/**
 * @file unix/gxpl.c
 * Top Layer of API, Unix code
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifdef  __unix__
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <gxPL/util.h>
#include "config.h"
#include "gxpl_p.h"

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
prvEncodeLong (unsigned long value, char * str, int size) {
  int i, len, str_len = strlen (str);
  static const char alphanum[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz";
  const int base = sizeof (alphanum) - 1;

  // Fill with zeros 
  for (i = str_len; i < size; i++) {
    
    str[i] = '0';
  }
  str[size] = '\0';
  len = strlen (str);

  // Handle the simple case
  if (value == 0) {
    return;
  }

  for (i = len - 1; i >= (len - str_len); i--) {
    
    str[i] = alphanum[value % base];
    if (value < base) {
      
      break;
    }
    value = value / base;
  }
}


/* api functions ============================================================ */

// -----------------------------------------------------------------------------
int
gxPLGenerateUniqueId (const gxPL * gxpl, char * s, int size) {
  int max, len = 0;

  if (gxpl->net_info.addrlen > 0) {

    for (int i = 0; (i < gxpl->net_info.addrlen) && (len < size); i++) {
      
      max = size - len + 1;
      len += snprintf (&s[len], max, "%02x", gxpl->net_info.addr[i]);
    }
    if (len > size) {

      len = size;
    }
  }
  if (len < size) {
    struct timeval tv;

    if (gettimeofday (&tv, NULL) == 0) {
      
      unsigned long ms = (tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL);
      // better if the function calls are very close but there is a risk of duplication
      // unsigned long ms = (tv.tv_sec * 1000000UL) + (tv.tv_usec);
      prvEncodeLong (ms, s, size);
    }
  }

  return strlen(s);
}

#endif /* __unix__ defined */
/* ========================================================================== */
