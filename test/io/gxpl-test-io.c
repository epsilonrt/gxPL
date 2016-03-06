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
#include "version-git.h"

/* constants ================================================================ */
#ifdef __AVR__
#define AVR_IOLAYER_NAME      "xbeezb"
#define AVR_IOLAYER_PORT      "tty1"

#define AVR_TERM_PORT         "tty0"
#define AVR_TERM_BAUDRATE     500000
#define AVR_TERM_FLOW         SERIAL_FLOW_RTSCTS

#define AVR_XBEE_RESET_PORT   PORTB
#define AVR_XBEE_RESET_PIN    7
#else
#define PROGMEM
#endif

static const char success[] PROGMEM = "Success\n";
static const char hbeat_basic[] PROGMEM =
  "xpl-stat\n"
  "{\n"
  "hop=1\n"
  "source=epsirt-test.io\n"
  "target=*\n"
  "}\n"
  "hbeat.basic\n"
  "{\n"
  "interval=5\n"
  "version=" VERSION_SHORT "\n"
  "}\n";
static const char hbeat_basic_end[] PROGMEM =
  "xpl-stat\n"
  "{\n"
  "hop=1\n"
  "source=epsirt-test.io\n"
  "target=*\n"
  "}\n"
  "hbeat.end\n"
  "{\n"
  "interval=5\n"
  "version=" VERSION_SHORT "\n"
  "}\n";
static const char hbeat_app[] PROGMEM =
  "xpl-stat\n"
  "{\n"
  "hop=1\n"
  "source=epsirt-test.io\n"
  "target=*\n"
  "}\n"
  "hbeat.app\n"
  "{\n"
  "interval=5\n"
  "port=%d\n"
  "remote-ip=%s\n"
  "version=" VERSION_SHORT "\n"
  "}\n";
static const char hbeat_app_end[] PROGMEM =
  "xpl-stat\n"
  "{\n"
  "hop=1\n"
  "source=epsirt-test.io\n"
  "target=*\n"
  "}\n"
  "hbeat.end\n"
  "{\n"
  "interval=5\n"
  "port=%d\n"
  "remote-ip=%s\n"
  "version=" VERSION_SHORT "\n"
  "}\n";

/* macros =================================================================== */
#ifndef __AVR__
#define vTermInit()
#define PSUCCESS() printf(success)
#define PRINTF(fmt,...) printf(fmt,##__VA_ARGS__)
#define SPRINTF(str,fmt,...) sprintf(str,fmt,##__VA_ARGS__)
#define FFLUSH fflush
#else
static void vTermInit (void);
#define PSUCCESS() printf_P (success)
#define PRINTF(fmt,...) printf_P(PSTR(fmt),##__VA_ARGS__)
#define SPRINTF(str,fmt,...) sprintf_P(str,fmt,##__VA_ARGS__)
#define FFLUSH iFileFlush
#endif

/* private variables ======================================================== */
static int test_count;
static char rpkt[256];
static char hbeat[256];
static char hbeat_end[256];

/* api functions ============================================================ */
gxPLSetting * gxPLSettingNew (const char * iface, const char * iolayer, gxPLConnectType type);
gxPLSetting * gxPLSettingFromCommandArgs (int argc, char * argv[], gxPLConnectType type);

/* private functions ======================================================== */
static int prvIoCtl (gxPLIo * io, int c, ...);

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
  vTermInit();
  PRINTF ("\ngxPLIo test\n"
          "Press any key to proceed...\n");
  getchar();

  // retrieved the requested configuration
#ifndef __AVR__
  PRINTF ("\nTest %d: create new default setting > ",
          ++test_count);
  setting = gxPLSettingFromCommandArgs (argc, argv, gxPLConnectViaHub);
#else
  PRINTF ("\nTest %d: create new default setting for %s layer on %s > ",
          ++test_count, AVR_IOLAYER_NAME, AVR_IOLAYER_PORT);
  setting = gxPLSettingNew (AVR_IOLAYER_PORT, AVR_IOLAYER_NAME, gxPLConnectViaHub);
#endif
  assert (setting);
  PSUCCESS();

  // opens the io layer
  PRINTF ("\nTest %d: open io layer...\n", ++test_count);
  io = gxPLIoOpen (setting);
  assert (io);
  PSUCCESS();

  // Get network informations
  PRINTF ("\nTest %d: read local network address > ", ++test_count);
  ret = prvIoCtl (io, gxPLIoFuncGetNetInfo, &addr);
  assert (ret == 0);
  assert (addr.addrlen > 0);
  PSUCCESS();
  PRINTF ("\tlocal network address     : ");
  ret = prvIoCtl (io, gxPLIoFuncNetAddrToString, &addr, &str);
  assert (ret == 0);
  assert (str);
  PRINTF ("%s", str);
  if (addr.family & gxPLNetFamilyInet) {
    PRINTF (":%d\n", addr.port);
    SPRINTF (hbeat, hbeat_app, addr.port, str);
    SPRINTF (hbeat_end, hbeat_app_end, addr.port, str);
  }
  else {
    SPRINTF (hbeat, hbeat_basic);
    SPRINTF (hbeat_end, hbeat_basic_end);
    putchar ('\n');
  }

  PRINTF ("\nTest %d: convert previous string to network address > ", ++test_count);
  ret = prvIoCtl (io, gxPLIoFuncNetAddrFromString, &addr1, str);
  assert (ret == 0);
  ret = memcmp (&addr, &addr1, sizeof (gxPLIoAddr));
  assert (str);
  PSUCCESS();

  PRINTF ("\nTest %d: read broadcast network address > ", ++test_count);
  ret = prvIoCtl (io, gxPLIoFuncGetBcastAddr, &addr);
  assert (ret == 0);
  assert (addr.addrlen > 0);
  PSUCCESS();
  PRINTF ("\tbroadcast network address : ");
  ret = prvIoCtl (io, gxPLIoFuncNetAddrToString, &addr, &str);
  assert (ret == 0);
  assert (str);
  PRINTF ("%s\n", str);

  PRINTF ("\nPress any key to send heartbeat message...\n");
  getchar();

  PRINTF ("\nTest %d: send heartbeat message > ", ++test_count);
  ret = gxPLIoSend (io, hbeat, strlen (hbeat), NULL);
  assert (ret > 0);
  PSUCCESS();

  PRINTF ("\nTest %d: wait response from hub", ++test_count);
  available_bytes = 0;
  int count = 0;
  do {
    putchar ('.');
    ret = prvIoCtl (io, gxPLIoFuncPoll, &available_bytes, 1000);
    assert (ret == 0);
    delay_ms (100);
    count++;
    FFLUSH (stdout);
  }
  while ( (available_bytes == 0) && (count < 10));
  assert (available_bytes > 0);
  FFLUSH (stdout);
  PRINTF (" > %d bytes received\n", available_bytes);

  PRINTF ("\nTest %d: read received packet > ", ++test_count);
  assert (available_bytes < sizeof (rpkt));
  ret = gxPLIoRecv (io, rpkt, available_bytes, &addr);
  assert (ret > 0);
  PSUCCESS();
  PRINTF (">>>\n%s<<<\n", rpkt);

  PRINTF ("\nTest %d: send heartbeat-end message > ", ++test_count);
  ret = gxPLIoSend (io, hbeat_end, strlen (hbeat_end), NULL);
  assert (ret > 0);
  PSUCCESS();

  PRINTF ("\nTest %d: close io layer...\n", ++test_count);
  ret = gxPLIoClose (io);
  assert (ret == 0);
  PSUCCESS();

  if (setting->malloc) {

    free (setting);
    PRINTF ("setting memory was freed\n");
  }

  PRINTF ("\n\n******************************************\n");
  PRINTF ("**** All tests (%d) were successful ! ****\n", test_count);
  PRINTF ("******************************************\n");
  FFLUSH (stdout);
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

#ifdef __AVR__
// -----------------------------------------------------------------------------
static int
iTermInit (void) {
  xSerialIos term_setting = {
    .baud = AVR_TERM_BAUDRATE, .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE, .sbits = SERIAL_STOPBIT_ONE,
    .flow = AVR_TERM_FLOW, .eol = SERIAL_CRLF
  };

  vLedInit();
  FILE * tc = xFileOpen (AVR_TERM_PORT, O_RDWR, &term_setting);
  if (!tc) {
    return -1;
  }
  stdout = tc;
  stderr = tc;
  stdin = tc;
  sei();
  return 0;
}
#endif

/* ========================================================================== */
