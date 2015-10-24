/**
 * @file
 * Utilities, (unix source code)
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifdef  __unix__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sysio/delay.h>
#include <gxPL/util.h>

/* private functions ======================================================== */


/* api functions ============================================================ */

// -----------------------------------------------------------------------------
long
gxPLTime (void) {

  return time (NULL);
}

// -----------------------------------------------------------------------------
int
gxPLTimeMs (unsigned long * ms) {
  int ret;
  struct timeval tv;

  if ( (ret = gettimeofday (&tv, NULL)) == 0) {

    *ms = (tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL);
    return 0;
  }
  return ret;
}

// -----------------------------------------------------------------------------
char *
gxPLTimeStr (unsigned long time) {
  static char buf[41];

  strftime (buf, sizeof (buf) - 1, "%y/%m/%d %H:%M:%S", localtime ( (time_t *) &time));

  return buf;
}

// -----------------------------------------------------------------------------
int
gxPLTimeDelayMs (unsigned long ms) {
  return delay_ms (ms);
}


// -----------------------------------------------------------------------------
const char *
gxPLConfigPath (const char * filename) {
  static char path[NAME_MAX + 1];

  strcpy (path, filename);
  if (strcmp (basename (path), filename) == 0) {
    const char * homedir = NULL;
    struct passwd * pwuid = getpwuid (getuid());
    struct stat st;

    if (pwuid->pw_uid == 0) {
      // root user

      snprintf (path, NAME_MAX, "%s/", DEFAULT_CONFIG_SYS_DIRECTORY);

      if (stat (path, &st) != 0) {

        if (errno == ENOENT) {

          // /etc/gxpl does not exist
          mkdir (path, 0755);
        }
      }

      if (stat (path, &st) == 0) {
        if ( (st.st_mode & S_IFMT) == S_IFDIR) {

          int max;
          // /etc/gxpl exists and is a directory.
          max = NAME_MAX - strlen (path) - 1;
          strncat (path, filename, max);
          return path;
        }
      }
    }
    else {

      if ( (homedir = getenv ("HOME")) == NULL) {

        homedir = pwuid->pw_dir;
      }

      if (homedir) {

        snprintf (path, NAME_MAX, "%s/%s/", homedir, DEFAULT_CONFIG_HOME_DIRECTORY);
        if (stat (path, &st) != 0) {

          if (errno == ENOENT) {

            // ~/.gxpl does not exist
            mkdir (path, 0755);
          }
        }

        if (stat (path, &st) == 0) {
          if ( (st.st_mode & S_IFMT) == S_IFDIR) {

            int max;
            // ~/.gxpl exists and is a directory.
            max = NAME_MAX - strlen (path) - 1;
            strncat (path, filename, max);
            return path;
          }
        }
      }
    }
  }
  return filename;
}

#endif /* __unix__ defined */
/* ========================================================================== */
