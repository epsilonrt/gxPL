/**
 * @file
 * gxPLMessage test
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/version.h>
#include <gxPL.h>
#include "version-git.h"

/* constants ================================================================ */
#define TERM_PORT         "tty0"
#define TERM_BAUDRATE     500000
#define TERM_FLOW         SERIAL_FLOW_RTSCTS
//#define TERM_FLOW         SERIAL_FLOW_NONE


/* macros =================================================================== */
#define test(t) do { \
    if (!t) { \
      PERROR ("Test %d failed !", test_count); \
      exit (EXIT_FAILURE); \
    } \
    else { \
      PINFO ("Test %d success !", test_count); \
    } \
  } while (0)

/* private variables ======================================================== */
static int test_count;
static const gxPLId source = {
  .vendor = "epsirt",
  .device = "test",
  .instance = "message"
};

/* private functions ======================================================== */
static int iTermInit (void);
static void prvHeartbeatMessageNew (void);

/* main ===================================================================== */
int
main (int argc, char **argv) {
  static volatile unsigned long libc_version;
  static volatile int ret;
  gxPLMessage * m;
  char * str;
  const char * cstr;

  iTermInit();

  for (;;) {

    m = NULL;
    str = NULL;
    cstr = NULL;
    test_count = 0;

    printf_P (PSTR ("\ngxPLMessage test for AVR\n"
                    "gxPL Lib Version: %s\n"
                    "Libc Version: %s\n"
                    "Press any key to proceed...\n"),
              gxPLVersion(), __AVR_LIBC_VERSION_STRING__);
    getchar();

    printf_P (PSTR ("\n*** Libc Version Test ***\n"));
    test_count++;
    libc_version = __AVR_LIBC_VERSION__;
    test (libc_version >= 10800);

    printf_P (PSTR ("\n*** gxPLMessageNew Test ***\n"));
    test_count++;
    m = gxPLMessageNew (gxPLMessageTrigger);
    test (m);

    // Tests for type
    printf_P (PSTR ("\n*** gxPLMessageType Tests ***\n"));
    test_count++;
    ret = gxPLMessageTypeGet (m);
    test (ret == gxPLMessageTrigger);

    test_count++;
    ret = gxPLMessageTypeSet (m, gxPLMessageStatus);
    test (ret == 0);
    ret = gxPLMessageTypeGet (m);
    test (ret == gxPLMessageStatus);

    test_count++;
    ret = gxPLMessageTypeSet (m, gxPLMessageCommand);
    test (ret == 0);
    ret = gxPLMessageTypeGet (m);
    test (ret == gxPLMessageCommand);

    // Tests for hop
    printf_P (PSTR ("\n*** gxPLMessageHop Tests ***\n"));
    test_count++;
    ret = gxPLMessageHopGet (m);
    test (ret == 1);

    test_count++;
    ret = gxPLMessageHopInc (m);
    test (ret == 0);
    ret = gxPLMessageHopGet (m);
    test (ret == 2);

    test_count++;
    ret = gxPLMessageHopSet (m, 1);
    test (ret == 0);
    ret = gxPLMessageHopGet (m);
    test (ret == 1);


    const char large_text[] = "012345678901234567890123456789";
    gxPLId source = { .vendor = "xpl", .device = "xplhal", .instance = "myhouse" };
    gxPLId target = { .vendor = "acme", .device = "cm12", .instance = "server" };
    gxPLId empty_id  = { .vendor = "", .device = "", .instance = "" };
    const gxPLId * pid;

    // Tests for source
    printf_P (PSTR ("\n*** gxPLMessageSource Tests ***\n"));
    test_count++;
    pid = gxPLMessageSourceIdGet (m);
    test (pid);

    test_count++;
    ret = gxPLIdCmp (&empty_id, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageSourceIdSet (m, &source);
    test (ret == 0);

    test_count++;
    pid = gxPLMessageSourceIdGet (m);
    test (pid);

    test_count++;
    ret = gxPLIdCmp (&source, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageSourceVendorIdSet (m, large_text);
    test (ret == -1);
    ret = gxPLIdCmp (&source, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageSourceDeviceIdSet (m, large_text);
    test (ret == -1);
    ret = gxPLIdCmp (&source, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageSourceInstanceIdSet (m, large_text);
    test (ret == -1);
    ret = gxPLIdCmp (&source, pid);
    test (ret == 0);

    // Tests for target
    printf_P (PSTR ("\n*** gxPLMessageTarget Tests ***\n"));
    test_count++;
    pid = gxPLMessageTargetIdGet (m);
    test (pid);

    test_count++;
    ret = gxPLIdCmp (&empty_id, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageTargetIdSet (m, &target);
    test (ret == 0);

    test_count++;
    pid = gxPLMessageTargetIdGet (m);
    test (pid);

    test_count++;
    ret = gxPLIdCmp (&target, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageTargetVendorIdSet (m, large_text);
    test (ret == -1);
    ret = gxPLIdCmp (&target, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageTargetDeviceIdSet (m, large_text);
    test (ret == -1);
    ret = gxPLIdCmp (&target, pid);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageTargetInstanceIdSet (m, large_text);
    test (ret == -1);
    ret = gxPLIdCmp (&target, pid);
    test (ret == 0);

    // Tests for schema
    printf_P (PSTR ("\n*** gxPLMessageSchema Tests ***\n"));
    gxPLSchema schema = { .class = "x10", .type = "basic" };
    gxPLSchema empty_schema = { .class = "", .type = "" };
    const gxPLSchema * psch;

    test_count++;
    psch = gxPLMessageSchemaGet (m);
    test (psch);

    test_count++;
    ret = gxPLSchemaCmp (&empty_schema, psch);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageSchemaCopy (m, &schema);
    test (ret == 0);

    test_count++;
    psch = gxPLMessageSchemaGet (m);
    test (psch);

    test_count++;
    ret = gxPLSchemaCmp (&schema, psch);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageSchemaClassSet (m, large_text);
    test (ret == -1);
    ret = gxPLSchemaCmp (&schema, psch);
    test (ret == 0);

    test_count++;
    ret = gxPLMessageSchemaTypeSet (m, large_text);
    test (ret == -1);
    ret = gxPLSchemaCmp (&schema, psch);
    test (ret == 0);


    // Tests for body
    printf_P (PSTR ("\n*** gxPLMessageBody Tests ***\n"));
    test_count++;
    ret = gxPLMessageBodySize (m);
    test (ret == 0);

    test_count++;
    ret = gxPLMessagePairAdd (m, "command", "dim");
    test (ret == 0);

    test_count++;
    ret = gxPLMessageBodySize (m);
    test (ret == 1);

    printf_P (PSTR ("\n*** gxPLMessagePair Tests ***\n"));
    test_count++;
    ret = gxPLMessagePairExist (m, "command");
    test (ret == true);

    test_count++;
    ret = gxPLMessagePairExist (m, "none");
    test (ret == false);

    test_count++;
    cstr = gxPLMessagePairGet (m, "command");
    test (cstr);
    ret = strcmp (cstr, "dim");
    test (ret == 0);

    test_count++;
    ret = gxPLMessagePairSet (m, "command", "test");
    test (ret == 0);
    cstr = gxPLMessagePairGet (m, "command");
    test (cstr);
    ret = strcmp (cstr, "test");
    test (ret == 0);

    test_count++;
    char buf[64];
    sprintf (buf, "HelloWorld0x%X---%s", 1234, "Ok");
    ret = gxPLMessagePairSetFormat (m, "command", "HelloWorld0x%X---%s", 1234, "Ok");
    test (ret == 0);
    cstr = gxPLMessagePairGet (m, "command");
    test (cstr);
    printf_P (PSTR ("%s -> %s\n"), buf, cstr);
    ret = strcmp (cstr, buf);
    test (ret == 0);

    test_count++;
    ret = gxPLMessagePairSet (m, "command", "dim");
    test (ret == 0);
    cstr = gxPLMessagePairGet (m, "command");
    test (cstr);
    ret = strcmp (cstr, "dim");
    test (ret == 0);

    test_count++;
    ret = gxPLMessagePairAdd (m, "device", "a1");
    test (ret == 0);

    test_count++;
    ret = gxPLMessageBodySize (m);
    test (ret == 2);

    test_count++;
    ret = gxPLMessagePairAdd (m, "level", "75");
    test (ret == 0);

    test_count++;
    ret = gxPLMessageBodySize (m);
    test (ret == 3);

    // Test Received
    printf_P (PSTR ("\n*** gxPLMessageReceived Tests ***\n"));
    test_count++;
    ret = gxPLMessageIsReceived (m);
    test (ret == false);
    ret = gxPLMessageReceivedSet (m, true);
    test (ret == 0);
    ret = gxPLMessageIsReceived (m);
    test (ret == true);
    ret = gxPLMessageReceivedSet (m, false);
    test (ret == 0);
    ret = gxPLMessageIsReceived (m);
    test (ret == false);

    // Converts the message to a string and prints it
    printf_P (PSTR ("\n*** gxPLMessageToString Test ***\n"));
    test_count++;
    str = gxPLMessageToString (m);
    test (str);
    printf_P (PSTR ("unicast message:\n%s"), str);

    // Decode the message
    printf_P (PSTR ("\n*** gxPLMessageFromString Test ***\n"));
    test_count++;
    char * str1 = malloc (strlen (str) + 1);
    test (str1);
    strcpy (str1, str);
    gxPLMessage * rm = gxPLMessageFromString (NULL, str);
    test (rm);

    test_count++;
    char * str2 = gxPLMessageToString (m);
    test (str2);
    printf_P (PSTR ("received message:\n%s"), str2);

    test_count++;
    ret = strcmp (str1, str2);
    test (ret == 0);
    printf_P (PSTR ("the received message is the same as that which was sent\n\n"));

    gxPLMessageDelete (rm);
    free (str);
    free (str1);
    free (str2);

    printf_P (PSTR ("\n*** gxPLMessageBroadcast Tests ***\n"));
    test_count++;
    ret = gxPLMessageIsBroadcast (m);
    test (ret == false);
    ret = gxPLMessageBroadcastSet (m, true);
    test (ret == 0);
    ret = gxPLMessageIsBroadcast (m);
    test (ret == true);

    // Print the message
    test_count++;
    str = gxPLMessageToString (m);
    test (str);
    printf_P (PSTR ("broadcast message:\n%s"), str);

    // Decode the message
    test_count++;
    str1 = malloc (strlen (str) + 1);
    test (str1);
    strcpy (str1, str);
    rm = gxPLMessageFromString (NULL, str);
    test (rm);

    test_count++;
    str2 = gxPLMessageToString (m);
    test (str2);
    printf_P (PSTR ("received message:\n%s"), str2);

    test_count++;
    ret = strcmp (str1, str2);
    test (ret == 0);
    printf_P (PSTR ("the received message is the same as that which was sent\n\n"));

    gxPLMessageDelete (rm);
    free (str);
    free (str1);
    free (str2);

    printf_P (PSTR ("\n*** gxPLMessageBodyClear Test ***\n"));
    test_count++;
    ret = gxPLMessageBodyClear (m);
    test (ret == 0);
    ret = gxPLMessageBodySize (m);
    test (ret == 0);

    // Delete the message
    gxPLMessageDelete (m);

    prvHeartbeatMessageNew();

    printf_P (PSTR ("\n\n******************************************\n"));
    printf_P (PSTR ("**** All tests (%d) were successful ! ****\n"), test_count);
    printf_P (PSTR ("******************************************\n"));
    iFileFlush (stdout);
  }
  return 0;
}


// -----------------------------------------------------------------------------
// Create Heartbeat message
static void
prvHeartbeatMessageNew (void) {
  static volatile int ret;
  gxPLMessage * message;

  printf_P (PSTR ("\nTest %d: create heartbeat message...\n"), ++test_count);

  message = gxPLMessageNew (gxPLMessageStatus);
  assert (message);

  ret = gxPLMessageSourceIdSet (message, &source);
  assert (ret == 0);

  ret = gxPLMessageBroadcastSet (message, true);
  assert (ret == 0);

  ret = gxPLMessageSchemaClassSet (message, "hbeat");
  assert (ret == 0);

  ret = gxPLMessageSchemaTypeSet (message, "basic");

  ret = gxPLMessagePairAddFormat (message, "interval", "%d", 5);
  assert (ret == 0);

  ret = gxPLMessagePairAdd (message, "version", VERSION_SHORT);
  assert (ret == 0);

  char * mstr = gxPLMessageToString (message);
  assert (mstr);

  printf_P (PSTR ("%s"), mstr);
  free (mstr);
  gxPLMessageDelete (message);
}

// -----------------------------------------------------------------------------
static int
iTermInit (void) {
  xSerialIos term_setting = {
    .baud = TERM_BAUDRATE, .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE, .sbits = SERIAL_STOPBIT_ONE,
    .flow = TERM_FLOW, .eol = SERIAL_CRLF
  };

  FILE * tc = xFileOpen (TERM_PORT, O_RDWR, &term_setting);
  if (!tc) {
    return -1;
  }
  stdout = tc;
  stderr = tc;
  stdin = tc;
  sei();
  return 0;
}

/* ========================================================================== */
