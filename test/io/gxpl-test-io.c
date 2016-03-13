/**
 * @file
 * gxPL Io test
 *
 * Copyright 2016 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GXPL_INTERNALS
#include <gxPL/io.h>
#include <gxPL/util.h>
#include "version-git.h"
#include "config.h"

#ifdef __AVR__
/* constants ================================================================ */
#define AVR_IOLAYER_NAME      "xbeezb"
#define AVR_IOLAYER_PORT      "tty1"

#define AVR_TERMINAL_PORT         "tty0"
#define AVR_TERMINAL_BAUDRATE     500000
#define AVR_TERMINAL_FLOW         SERIAL_FLOW_NONE

#endif

#define UTEST_COUNTER test_count
#include <gxPL/utest.h>

/* private variables ======================================================== */
static char rpkt[256];
static char hbeat[200];
static char hbeat_end[200];

/* api functions ============================================================ */
gxPLSetting * gxPLSettingNew (const char * iface, const char * iolayer, gxPLConnectType type);
gxPLSetting * gxPLSettingFromCommandArgs (int argc, char * argv[], gxPLConnectType type);

/* private functions ======================================================== */
static int prvIoCtl (gxPLIo * io, int c, ...);
static void prvCopyAppHbeat (const char * addr, int port);
static void prvCopyBasicHbeat (const char * addr);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  static volatile int ret;
  int available_bytes;
  char * str;
  gxPLIoAddr addr, addr1;
  gxPLSetting * setting;
  gxPLIo * io;

  vLogSetMask (LOG_UPTO (LOG_DEBUG));
  gxPLStdIoOpen();
  gxPLPrintf ("\ngxPLIo test (%s)\n", GXPL_TARGET_STR);
  UTEST_PMEM_BEFORE();
  gxPLPrintf ("Press any key to proceed...\n");
  gxPLWait();

  // retrieved the requested configuration
#ifndef __AVR__
  // TEST 1
  UTEST_NEW ("create new default setting from command line args > ");
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
  assert (setting);
#else
  UTEST_NEW ("create new default setting for %s layer on %s > ",
             AVR_IOLAYER_NAME, AVR_IOLAYER_PORT);
  setting = gxPLSettingNew (AVR_IOLAYER_PORT, AVR_IOLAYER_NAME, gxPLConnectViaHub);
  assert (setting);
  setting->log = LOG_DEBUG;
#endif
#ifndef GXPL_XBEEZB_HAS_HWRESET
  setting->xbee.reset_sw = 1;
#endif
  UTEST_SUCCESS();

  // TEST 2
  // opens the io layer
  UTEST_NEW ("open io layer...\n");
  io = gxPLIoOpen (setting);
  assert (io);
  UTEST_SUCCESS();

  // TEST 3
  // Get network informations
  UTEST_NEW ("read local network address > ");
  ret = prvIoCtl (io, gxPLIoFuncGetNetInfo, &addr);
  assert (ret == 0);
  assert (addr.addrlen > 0);
  UTEST_SUCCESS();
  gxPLPrintf ("\tlocal network address     : ");
  ret = prvIoCtl (io, gxPLIoFuncNetAddrToString, &addr, &str);
  assert (ret == 0);
  assert (str);
  gxPLPrintf ("%s", str);

  // Create heartbeat messages
  if (addr.family & gxPLNetFamilyInet) {
    gxPLPrintf (":%d\n", addr.port);
    prvCopyAppHbeat (str, addr.port);
  }
  else {
    gxPLPutchar ('\n');
    prvCopyBasicHbeat (str);
  }

  // TEST 4
  UTEST_NEW ("convert previous string to network address > ");
  ret = prvIoCtl (io, gxPLIoFuncNetAddrFromString, &addr1, str);
  assert (ret == 0);
  ret = memcmp (&addr, &addr1, sizeof (gxPLIoAddr));
  assert (str);
  UTEST_SUCCESS();

  // TEST 5
  UTEST_NEW ("read broadcast network address > ");
  ret = prvIoCtl (io, gxPLIoFuncGetBcastAddr, &addr);
  assert (ret == 0);
  assert (addr.addrlen > 0);
  UTEST_SUCCESS();
  gxPLPrintf ("\tbroadcast network address : ");
  ret = prvIoCtl (io, gxPLIoFuncNetAddrToString, &addr, &str);
  assert (ret == 0);
  assert (str);
  gxPLPrintf ("%s\n", str);

  gxPLPrintf ("\nPress any key to send heartbeat message...\n");
  gxPLWait();

  // TEST 6
  UTEST_NEW ("send heartbeat message > ");
  ret = gxPLIoSend (io, hbeat, strlen (hbeat), NULL);
  assert (ret > 0);
  UTEST_SUCCESS();

  // TEST 7
  UTEST_NEW ("wait response from hub");
  available_bytes = 0;
  int count = 0;
  do {
    gxPLPutchar ('.');
    ret = prvIoCtl (io, gxPLIoFuncPoll, &available_bytes, 1000);
    assert (ret == 0);
    count++;
    gxPLFflush (stdout);
  }
  while ( (available_bytes == 0) && (count < 10));
  assert (available_bytes > 0);
  gxPLFflush (stdout);
  gxPLPrintf (" > %d bytes received\n", available_bytes);

  // TEST 8
  UTEST_NEW ("read received packet > ");
  assert (available_bytes < sizeof (rpkt));
  ret = gxPLIoRecv (io, rpkt, available_bytes, &addr);
  assert (ret > 0);
  UTEST_SUCCESS();
  gxPLPrintf (">>>\n%s<<<\n", rpkt);

  // TEST 9
  UTEST_NEW ("send heartbeat-end message > ");
  ret = gxPLIoSend (io, hbeat_end, strlen (hbeat_end), NULL);
  assert (ret > 0);
  UTEST_SUCCESS();

  // TEST 10
  gxPLPrintf ("\nPress any key to close...\n");
  gxPLWait();

  UTEST_NEW ("close io layer...\n");
  ret = gxPLIoClose (io);
  assert (ret == 0);
  UTEST_SUCCESS();

  if (setting->malloc) {

    free (setting);
    gxPLPrintf ("setting memory was freed\n");
  }

  gxPLPrintf ("\n\n******************************************\n");
  gxPLPrintf ("**** All tests (%d) were successful ! ****\n", test_count);
  gxPLPrintf ("******************************************\n");
  UTEST_PMEM_AFTER();
  gxPLFflush (stdout);
  gxPLExit (EXIT_SUCCESS);
  return 0;
}

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
// draft of the future gxPLIoCtl() function
static int
prvIoCtl (gxPLIo * io, int c, ...) {
  int ret = 0;
  va_list ap;

  va_start (ap, c);
  ret = gxPLIoIoCtl (io, c, ap);
  va_end (ap);

  return ret;
}

// -----------------------------------------------------------------------------
static const char hbeat_basic[] PROGMEM =
  "xpl-stat\n"
  "{\n"
  "hop=1\n"
  "source=epsirt-test.io\n"
  "target=*\n"
  "}\n"
  "hbeat.%s\n"
  "{\n"
  "interval=5\n"
  "version=" VERSION_SHORT "\n"
#if CONFIG_HBEAT_BASIC_EXTENSION
  "remote-addr=%s\n"
#endif
  "}\n";

static const char hbeat_app[] PROGMEM =
  "xpl-stat\n"
  "{\n"
  "hop=1\n"
  "source=epsirt-test.io\n"
  "target=*\n"
  "}\n"
  "hbeat.%s\n"
  "{\n"
  "interval=5\n"
  "port=%d\n"
  "remote-ip=%s\n"
  "version=" VERSION_SHORT "\n"
  "}\n";

// -----------------------------------------------------------------------------
static void
prvCopyAppHbeat (const char * addr, int port) {

  gxPLSprintf (hbeat, hbeat_app, "app", port, addr);
  gxPLSprintf (hbeat_end, hbeat_app, "end", port, addr);
}
// -----------------------------------------------------------------------------
static void
prvCopyBasicHbeat (const char * addr) {

#if CONFIG_HBEAT_BASIC_EXTENSION
  gxPLSprintf (hbeat, hbeat_basic, "basic", addr);
  gxPLSprintf (hbeat_end, hbeat_basic, "end", addr);
#else
  gxPLSprintf (hbeat, hbeat_basic, "basic");
  gxPLSprintf (hbeat_end, hbeat_basic, "end");
#endif
}

/* ========================================================================== */
