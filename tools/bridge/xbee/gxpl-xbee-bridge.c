/**
 * @file
 * Implementation of an xPL Ethernet to XBee ZB (API without escape) Bridge using gxPLib
 *
 * Copyright 2015-2016 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysio/log.h>
#include <getopt.h>

#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define BRIDGE_MAX_RESTARTS 10000

#define BRIDGE_VERSION VERSION_SHORT
#define DEFAULT_VENDOR "epsirt"
#define DEFAULT_DEVICE "xbeebr"
#define DEFAULT_CONFIG_FILE "gxpl-xbee-bridge.xpl"

#define PANID_KEY    "panid"

/* structures =============================================================== */
typedef struct _prvBridgeSetting {
  gxPLSetting * in;
  gxPLSetting * out;
  const char * vendor_id;
  const char * device_id;
  const char * cfg_filename;
  int maxhop;
} prvBridgeSetting;

/* private variables ======================================================== */
static pid_t bridge_pid = 0;
static gxPLBridge * bridge;
static const char default_vendor_id[] = DEFAULT_VENDOR;
static const char default_device_id[] = DEFAULT_DEVICE;

/* private functions ======================================================== */
static void prvPrintUsage (void);
static void prvSupervisorSignalHandler (int sig);
static void prvBridgeSignalHandler (int sig);
static void prvSuperviseBridge (const prvBridgeSetting * setting);
static int prvRunBridge (const prvBridgeSetting * setting);
static void prvParseOptions (prvBridgeSetting * setting, int argc, char *argv[]);
static int prvPanIdFromString (uint64_t * panid, const char * str);

/* main ===================================================================== */
int
main (int argc, char * argv[]) {
prvBridgeSetting setting = {
  .in = NULL,
  .out = NULL,
  .vendor_id = default_vendor_id,
  .device_id = default_device_id,
  .maxhop = 1,
};

  vLogInit (LOG_UPTO (LOG_NOTICE));
  setting.cfg_filename = gxPLConfigPath (DEFAULT_CONFIG_FILE);

  setting.in = calloc (1, sizeof (gxPLSetting));
  assert (setting.in);
  setting.in->malloc = 1; // will be freed by gxPLBridgeClose()
  strcpy (setting.in->iolayer, "xbeezb");
  setting.in->connecttype = gxPLConnectStandAlone;

  setting.out = calloc (1, sizeof (gxPLSetting));
  assert (setting.out);
  setting.out->malloc = 1; // will be freed by gxPLBridgeClose()
  strcpy (setting.out->iolayer, "udp");
  setting.out->connecttype = gxPLConnectViaHub;

  prvParseOptions (&setting, argc, argv);
  if (setting.in->xbee.new_panid == 0) {

    xVector * config = gxPLConfigReadFile (setting.cfg_filename, NULL, NULL);
    if (config) {
      gxPLPair * p = pvVectorFindFirst (config, PANID_KEY);

      if (p) {

        prvPanIdFromString (&setting.in->xbee.panid, p->value);
      }
      vVectorDestroy (config);
    }
  }

  // Now we detach (daemonize ourself)
  if (setting.in->nodaemon == 0) {

    vLogDaemonize (true);

    // Fork ourselves
    switch (fork()) {
      case 0:            // child
        // Close standard I/O and become our own process group
        close (fileno (stdin));
        close (fileno (stdout));
        close (fileno (stderr));
        setpgrp();

        // Start he bridge and keep it running
        prvSuperviseBridge (&setting);
        break;

      default:           // parent
        break;

      case -1:           // error
        vLog (LOG_ERR, "unable to spawn gxpl-bridge supervisor, %s (%d)",
              strerror (errno), errno);
        exit (EXIT_FAILURE);
    }
  }
  else {

    // When running non-detached, just do the bridge work
    if (prvRunBridge (&setting) != 0) {

      exit (EXIT_FAILURE);
    }
  }
  return 0;
}

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static int
prvBaudrateFromString (long * baudrate, const char * str) {
  char * endptr;
  long b = strtol (str, &endptr, 10);

  if (*endptr != '\0') {

    vLog (LOG_ERR, "bad value for baudrate %s, was setting to default (%d)",
          str, GXPL_DEFAULT_BAUDRATE);
    return -1;
  }
  *baudrate = b;
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvPanIdFromString (uint64_t * panid, const char * str) {
  uint64_t ull;
  char * endptr;

  ull = strtoll (str, &endptr, 0);
  if (*endptr != '\0') {

    vLog (LOG_ERR, "bad value for PAN ID %s, was setting to default (0)", str);
    return -1;
  }
  *panid = ull;
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvConfigSetPanID (gxPLDevice * device, uint64_t panid) {
  char buffer[20];

  snprintf (buffer, 20, "0x%" PRIx64, panid);
  return gxPLDeviceConfigValueSet (device, PANID_KEY, buffer);
}

// -----------------------------------------------------------------------------
static void
prvParseOptions (prvBridgeSetting * setting, int argc, char *argv[]) {
  int c;
  int loglvl = LOG_NOTICE;

  static const char short_options[] = "i:o:f:m:B:p:bDdh" GXPL_GETOPT;
  static struct option long_options[] = {
    {"in",          required_argument,  NULL, 'i' },
    {"out",         required_argument,  NULL, 'o' },
    {"file",        required_argument,  NULL, 'f' },
    {"maxhop",      required_argument,  NULL, 'm' },
    {"baudrate",    required_argument,  NULL, 'B' },
    {"panid",       required_argument,  NULL, 'p' },
    {"broadcast",   no_argument,        NULL, 'b' },
    {"nodaemon",    no_argument,        NULL, 'D' },
    {"debug",       no_argument,        NULL, 'd' },
    {"help",        no_argument,        NULL, 'h' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

#ifdef DEBUG
  vLogSetMask (LOG_UPTO (LOG_DEBUG));
#endif

  do  {

    c = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (c) {

      case 'i':
        strcpy (setting->in->iface, optarg);
        PDEBUG ("set inside interface to %s", setting->in->iface);
        break;

      case 'o':
        strcpy (setting->out->iface, optarg);
        PDEBUG ("set outside interface to %s", setting->out->iface);
        break;

      case 'f':
        setting->cfg_filename = gxPLConfigPath (optarg);
        PDEBUG ("set configuration filename to %s", setting->cfg_filename);
        break;

      case 'B':
        if (prvBaudrateFromString (&setting->in->xbee.ios.baud, optarg) == 0) {

          setting->in->xbee.ios.dbits = SERIAL_DATABIT_8;
          setting->in->xbee.ios.parity = SERIAL_PARITY_NONE;
          setting->in->xbee.ios.sbits = SERIAL_STOPBIT_ONE;
          setting->in->xbee.ios.flow = GXPL_DEFAULT_FLOW;
          setting->in->xbee.ios.flag = 0;
          setting->in->iosflag = 1;
          PDEBUG ("set baudrate to %d", setting->in->xbee.ios.baud);
        }
        break;

      case 'p':
        if (prvPanIdFromString (&setting->in->xbee.panid, optarg) == 0) {

          setting->in->xbee.new_panid = 1;
          PDEBUG ("set PAN ID to 0x%" PRIx64, setting->in->xbee.panid);
        }
        break;

      case 'm': {
        char * endptr;

        setting->maxhop = strtol (optarg, &endptr, 10);
        if (*endptr != '\0') {

          vLog (LOG_ERR, "bad value for max hops %s, was setting to default (1)", optarg);
          setting->maxhop = 1;
        }
        PDEBUG ("set max hops to %d", setting->maxhop);
      }
      break;

      case 'b':
        setting->in->broadcast = 1;
        setting->out->broadcast = 1;
        PDEBUG ("enable broadcast");
        break;

      case 'd':
        loglvl++;
        PDEBUG ("enable log %d", loglvl);
        break;

      case 'D':
        setting->in->nodaemon = 1;
        setting->out->nodaemon = 1;
        PDEBUG ("set nodaemon flag");
        break;

      case 'h':
        prvPrintUsage();
        free (setting->in);
        free (setting->out);
        exit (EXIT_SUCCESS);
        break;

      default:
        break;
    }
  }
  while (c != -1);
  setting->in->log = MIN(LOG_DEBUG,loglvl);
  setting->out->log = MIN(LOG_DEBUG,loglvl);
}

// -----------------------------------------------------------------------------
// Print usage info
static void
prvPrintUsage (void) {
  printf ("%s - xPL XBee to Ethernet network bridge\n", __progname);
  printf ("Copyright (c) 2015, Pascal JEAN aka epsilonRT\n\n");
  //       01234567890123456789012345678901234567890123456789012345678901234567890123456789
  printf ("Usage: %s [-i interface] [-o interface] [-B baudrate] [-p panid] [-f filename]\n"
          "          [-m maxhop] [-b] [-d] [-h] [-D]\n", __progname);

  printf ("  -i interface - use interface named interface (i.e. /dev/ttyUSB0)\n"
          "                 as XBee interface\n");

  printf ("  -o interface - use interface named interface (i.e. eth0)\n"
          "                 as Ethernet interface\n");

  printf ("  -f filename  - file name where to save the device configuration\n"
          "                 (default: ~/.gxpl/" DEFAULT_CONFIG_FILE ")\n");

  printf ("  -B baudrate  - use this baudrate for the XBee interface\n"
          "                 (default: %d)\n", GXPL_DEFAULT_BAUDRATE);

  printf ("  -p panid     - use this PAN ID for the Zigbee network\n"
          "                 (default: 0)\n");

  printf ("  -m maxhop    - messages with hop count less than or equal to maxhop\n"
          "                 cross the bridge (default: 1)\n");

  printf ("  -b           - enable broadcast for inside network\n");
  printf ("  -D           - do not daemonize -- run from the console\n");
  printf ("  -d           - enable debugging messages\n");
  printf ("  -h           - print this message\n\n");
  //       01234567890123456789012345678901234567890123456789012345678901234567890123456789
  printf ("<NOTE>    PAN ID, max hop count and broadcast parameters may be configured\n"
          "          with a xPL configuration manager. If provided, these parameters will override\n"
          "          the existing ones in the device configuration file.\n");
  printf ("<WARNING> The XBee module (Series 2) must be configured as coordinator in\n"
          "          API mode (AP=1)\n");
}


// -----------------------------------------------------------------------------
static void
prvSetConfig (gxPLBridge * bridge, bool isremote) {
  uint64_t panid;
  gxPLDevice * device;
  gxPLSetting * insetting;

  PDEBUG ("Starting XBee bridge configuration");

  device = gxPLBridgeDevice (bridge);
  insetting = gxPLAppSetting (gxPLBridgeInApp (bridge));

  if (prvPanIdFromString (&panid,
                          gxPLDeviceConfigValueGet (device, PANID_KEY)) == 0) {

    if (insetting->xbee.panid != panid) {

      if (isremote == false) {
        // startup configuration

        if (insetting->xbee.new_panid) {

          // PAN ID provided from command line, save to config file
          prvConfigSetPanID (device, insetting->xbee.panid);
          gxPLDeviceConfigSave (device);
        }
      }
      else {
        // remote configuration, panid must be write to the module

        gxPLSetting * new_setting = malloc (sizeof (gxPLSetting));
        assert (new_setting);

        memcpy (new_setting, gxPLAppSetting (gxPLBridgeInApp (bridge)),
                sizeof (gxPLSetting));
        new_setting->malloc = 1;
        new_setting->xbee.panid = panid;
        new_setting->xbee.new_panid = 1;
        gxPLBridgeSetNewInSetting (bridge, new_setting);
      }
    }
  }
}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration */
static void
prvConfigChanged (gxPLDevice * device, void * udata) {

  // Read setting items for device and install */
  prvSetConfig (udata, true);
}

// -----------------------------------------------------------------------------
//  This is the bridge code itself.
static int
prvRunBridge (const prvBridgeSetting * setting) {
  int ret;
  gxPLDevice * device;

  bridge = gxPLBridgeOpen (setting->in, setting->out, setting->maxhop);
  if (bridge == NULL) {

    vLog (LOG_ERR, "Unable to start the bridge");
    return -1;
  }

  if (gxPLBridgeDeviceSet (bridge, setting->vendor_id, setting->device_id,
                           setting->cfg_filename, BRIDGE_VERSION) != 0) {

    vLog (LOG_ERR, "Unable to set device");
    return -1;
  }
  device = gxPLBridgeDevice (bridge);
  if (gxPLDeviceIsConfigured (device) == false) {

    gxPLDeviceConfigItemAdd (device, PANID_KEY, gxPLConfigReconf, 1);
    prvConfigSetPanID (device, setting->in->xbee.panid);
  }

  // Parse the device configurables into a form this program
  // can use (whether we read a config or not)
  prvSetConfig (bridge, false);

  // Add a device change listener we'll use to pick up changes
  gxPLDeviceConfigListenerAdd (device, prvConfigChanged, bridge);

  if (gxPLBridgeDeviceEnable (bridge, true) != 0) {

    vLog (LOG_ERR, "Unable to enable device");
    return -1;
  }

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvBridgeSignalHandler);
  signal (SIGINT, prvBridgeSignalHandler);

  if (setting->in->nodaemon) {
    // Gets the applications to read their informations
    gxPLApplication * in = gxPLBridgeInApp (bridge);
    gxPLApplication * out = gxPLBridgeOutApp (bridge);

    printf ("Starting xPL bridge\n");
    printf ("  listening on %s (inside) @ %s for %s\n",
            setting->in->iface,
            gxPLIoLocalAddrGet (in),
            setting->in->iolayer);
    printf ("  listening on %s (outside) @ %s:%d for %s\n",
            setting->out->iface,
            gxPLIoLocalAddrGet (out),
            gxPLIoInfoGet (out)->port,
            setting->out->iolayer);
    printf ("Press Ctrl+C to abort ...\n");
  }
  else {

    vLog (LOG_NOTICE, "xPL Bridge now running");
  }

  // Hand control over to gxPLib
  for (;;) {

    ret = gxPLBridgePoll (bridge, 100);
    if (ret != 0) {

      vLog (LOG_ERR, "Bridge failure, exiting !");
      return -1;
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------
//  Start the bridge and monitor it.  If it stops for any reason, restart it
static void
prvSuperviseBridge (const prvBridgeSetting * setting) {
  int bridge_restart_count = 0;
  int ret;

  // Begin bridge supervision

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSupervisorSignalHandler);
  signal (SIGINT, prvSupervisorSignalHandler);

  // To supervise the bridge, we repeatedly fork ourselves each time we
  // determine the bridge is not running.  We maintain a circuit breaker
  // to prevent an endless spawning if the bridge just can't run
  for (;;) {

    // See if we can still do this
    if (bridge_restart_count == BRIDGE_MAX_RESTARTS) {

      vLog (LOG_ERR, "gxpl-bridge has died %d times -- something may be wrong "
            "-- terminating supervisor", BRIDGE_MAX_RESTARTS);
      exit (EXIT_FAILURE);
    }

    // Up the restart count
    bridge_restart_count++;

    // Fork off the bridge
    switch (bridge_pid = fork()) {
      case 0:            // child
        // Close standard I/O and become our own process group
        close (fileno (stdin));
        close (fileno (stdout));
        close (fileno (stderr));
        setpgrp();

        // Run the bridge
        if (prvRunBridge (setting) != 0) {

          // If we come back, something bad likely happened
          exit (EXIT_FAILURE);
        }
        break;

      default:           // parent
        PDEBUG ("spawned gxpl-bridge process, pid=%d, spawn count=%d",
                bridge_pid, bridge_restart_count);
        break;

      case -1:           // error
        vLog (LOG_ERR, "unable to spawn gxpl-bridge supervisor, %s (%d)",
              strerror (errno), errno);
        exit (EXIT_FAILURE);
    }

    // Now we just wait for something bad to happen to our bridge
    waitpid (bridge_pid, &ret, 0);

    if (WIFEXITED (ret)) {
      vLog (LOG_NOTICE, "gxpl-bridge exited normally with status %d -- restarting...",
            WEXITSTATUS (ret));
      continue;
    }

    if (WIFSIGNALED (ret)) {
      vLog (LOG_NOTICE, "gxpl-bridge died from by receiving unexpected signal %d"
            " -- restarting...", WTERMSIG (ret));
      continue;
    }

    vLog (LOG_NOTICE, "gxpl-bridge died from unknown causes -- restarting...");
    continue;
  }
}

// -----------------------------------------------------------------------------
//  Shutdown gracefully if user hits ^C or received TERM signal
static void
prvBridgeSignalHandler (int sig) {
  int ret;
  switch (sig) {

    case SIGTERM:
    case SIGINT:
      ret = gxPLBridgeClose (bridge);
      if (ret != 0) {

        vLog (LOG_ERR, "unable to close bridge");
        exit (EXIT_FAILURE);
      }

      vLog (LOG_NOTICE, "everything was closed. Have a nice day !");
      exit (EXIT_SUCCESS);
      break;

    default:
      break;
  }
}

// -----------------------------------------------------------------------------
// signal handler
static void
prvSupervisorSignalHandler (int sig) {
  int ret;

  vLog (LOG_NOTICE, "received termination signal -- starting shutdown");

  switch (sig) {

    case SIGTERM:
    case SIGINT:
      // Stop the child and wait for it
      if ( (bridge_pid != 0) && (kill (bridge_pid, SIGTERM) == 0)) {

        waitpid (bridge_pid, &ret, 0);
        if (WIFEXITED (ret) == 0) {

          exit (EXIT_FAILURE);
        }
      }
      exit (EXIT_SUCCESS);
      break;

    default:
      break;
  }
}

/* ========================================================================== */
