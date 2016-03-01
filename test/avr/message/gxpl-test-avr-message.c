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
#include <avr/version.h>
#include <gxPL/message.h>
#include <gxPL/util.h>

/* macros =================================================================== */
#define test(t) do { \
    if (!t) { \
      fprintf_P (stderr, PSTR("line %d in %s: %d failed !\n"),  __LINE__, __FUNCTION__, test_count); \
      exit (-1); \
    } \
  } while (0)

/* private variables ======================================================== */
static int test_count;

/* main ===================================================================== */
int
main (int argc, char **argv) {
  static volatile unsigned long libc_version;
  static volatile int ret;
  gxPLMessage * m = NULL;
  char * str = NULL;
  const char * cstr = NULL;

  libc_version = __AVR_LIBC_VERSION__;
  test (libc_version >= 10800);

  test_count++;
  m = gxPLMessageNew (gxPLMessageTrigger);
  test (m);

  // Tests for type
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
  test_count++;
  ret = gxPLMessageBodySize(m);
  test (ret == 0);
  
  test_count++;
  ret = gxPLMessagePairAdd(m, "command", "dim");
  test (ret == 0);

  test_count++;
  ret = gxPLMessageBodySize (m);
  test (ret == 1);
  
  test_count++;
  ret = gxPLMessagePairExist(m, "command");
  test (ret == true);
  
  test_count++;
  ret = gxPLMessagePairExist(m, "none");
  test (ret == false);
  
  test_count++;
  cstr = gxPLMessagePairGet(m, "command");
  test (cstr);
  ret = strcmp (cstr, "dim");
  test (ret == 0);

  test_count++;
  ret = gxPLMessagePairSet(m, "command", "test");
  test (ret == 0);
  cstr = gxPLMessagePairGet(m, "command");
  test (cstr);
  ret = strcmp (cstr, "test");
  test (ret == 0);
  
  test_count++;
  char buf[64];
  sprintf (buf, "HelloWorld0x%X---%s", 1234, "Ok");
  ret = gxPLMessagePairAddFormat (m, "command", "HelloWorld0x%X---%s", 1234, "Ok");
  test (ret == 0);
  cstr = gxPLMessagePairGet(m, "command");
  test (cstr);
  printf (PSTR("%s -> %s\n"), buf, cstr);
  ret = strcmp (cstr, buf);
  test (ret == 0);

  test_count++;
  ret = gxPLMessagePairSet(m, "command", "dim");
  test (ret == 0);
  cstr = gxPLMessagePairGet(m, "command");
  test (cstr);
  ret = strcmp (cstr, "dim");
  test (ret == 0);
  
  test_count++;
  ret = gxPLMessagePairAdd(m, "device", "a1");
  test (ret == 0);

  test_count++;
  ret = gxPLMessageBodySize (m);
  test (ret == 2);
  
  test_count++;
  ret = gxPLMessagePairAdd(m, "level", "75");
  test (ret == 0);

  test_count++;
  ret = gxPLMessageBodySize (m);
  test (ret == 3);

  // Test Received
  test_count++;
  ret = gxPLMessageIsReceived(m);
  test(ret == false);
  ret = gxPLMessageReceivedSet(m, true);
  test(ret == 0);
  ret = gxPLMessageIsReceived(m);
  test(ret == true);
  ret = gxPLMessageReceivedSet(m, false);
  test(ret == 0);
  ret = gxPLMessageIsReceived(m);
  test(ret == false);

  // Converts the message to a string and prints it
  test_count++;
  str = gxPLMessageToString (m);
  test (str);
  printf (PSTR("unicast message:\n%s"), str);
  
  // Decode the message
  test_count++;
  char * str1 = malloc (strlen(str) + 1);
  test(str1);
  strcpy (str1, str);
  gxPLMessage * rm = gxPLMessageFromString(NULL, str);
  test (rm);

  test_count++;
  char * str2 = gxPLMessageToString (m);
  test (str2);
  printf (PSTR("received message:\n%s"), str2);

  test_count++;
  ret = strcmp (str1, str2);
  test(ret == 0);
  printf("the received message is the same as that which was sent\n\n");

  gxPLMessageDelete(rm);
  free(str);
  free(str1);
  free(str2);
  
  // Converts the message to a string and prints it
  test_count++;
  ret = gxPLMessageIsBroadcast(m);
  test(ret == false);
  ret = gxPLMessageBroadcastSet(m, true);
  test(ret == 0);
  ret = gxPLMessageIsBroadcast(m);
  test(ret == true);
  
  // Print the message
  test_count++;
  str = gxPLMessageToString (m);
  test (str);
  printf (PSTR("broadcast message:\n%s"), str);
  
  // Decode the message
  test_count++;
  str1 = malloc (strlen(str) + 1);
  test(str1);
  strcpy (str1, str);
  rm = gxPLMessageFromString(NULL, str);
  test (rm);

  test_count++;
  str2 = gxPLMessageToString (m);
  test (str2);
  printf (PSTR("received message:\n%s"), str2);

  test_count++;
  ret = strcmp (str1, str2);
  test(ret == 0);
  printf("the received message is the same as that which was sent\n\n");

  gxPLMessageDelete(rm);
  free(str);
  free(str1);
  free(str2);

  // Clear body
  test_count++;
  ret = gxPLMessageBodyClear(m);
  test(ret == 0);
  ret = gxPLMessageBodySize (m);
  test (ret == 0);
  
  // Delete the message
  gxPLMessageDelete (m);

  printf (PSTR("All tests (%d) were successful !\n"), test_count);
  return 0;
}
/* ========================================================================== */
