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
#include <gxPL/message.h>
#include <gxPL/util.h>

#ifdef __AVR__
/* constants ================================================================ */
#define AVR_TERMINAL_PORT         "tty0"
#define AVR_TERMINAL_BAUDRATE     500000
#define AVR_TERMINAL_FLOW         SERIAL_FLOW_NONE
#endif

#define UTEST_COUNTER test_count
#include <gxPL/utest.h>

/* private variables ======================================================== */
static const gxPLId source = {
  .vendor = "epsirt",
  .device = "test",
  .instance = "message"
};

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret;
  gxPLMessage * m = NULL;
  char * str = NULL;
  const char * cstr = NULL;

  vLogSetMask (LOG_UPTO (LOG_DEBUG));
  gxPLStdIoOpen();
  gxPLPrintf ("\ngxPLMessage test (%s)\n", GXPL_TARGET_STR);
  UTEST_PMEM_BEFORE();
  gxPLPrintf ("Press any key to proceed...\n");
  gxPLWait();

  UTEST_NEW ("gxPLMessageNew() > ");
  m = gxPLMessageNew (gxPLMessageTrigger);
  assert (m);
  UTEST_SUCCESS();

// Tests for type
  UTEST_NEW ("gxPLMessageTypeGet() > ");
  ret = gxPLMessageTypeGet (m);
  assert (ret == gxPLMessageTrigger);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageTypeSet(m,gxPLMessageStatus) > ");
  ret = gxPLMessageTypeSet (m, gxPLMessageStatus);
  assert (ret == 0);
  ret = gxPLMessageTypeGet (m);
  assert (ret == gxPLMessageStatus);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageTypeSet(m,gxPLMessageCommand) > ");
  ret = gxPLMessageTypeSet (m, gxPLMessageCommand);
  assert (ret == 0);
  ret = gxPLMessageTypeGet (m);
  assert (ret == gxPLMessageCommand);
  UTEST_SUCCESS();

// Tests for hop
  UTEST_NEW ("gxPLMessageHopGet() > ");
  ret = gxPLMessageHopGet (m);
  assert (ret == 1);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageHopInc() > ");
  ret = gxPLMessageHopInc (m);
  assert (ret == 0);
  ret = gxPLMessageHopGet (m);
  assert (ret == 2);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageHopSet() > ");
  ret = gxPLMessageHopSet (m, 1);
  assert (ret == 0);
  ret = gxPLMessageHopGet (m);
  assert (ret == 1);
  UTEST_SUCCESS();

  const char large_text[] = "012345678901234567890123456789";
  gxPLId source = { .vendor = "xpl", .device = "xplhal", .instance = "myhouse" };
  gxPLId target = { .vendor = "acme", .device = "cm12", .instance = "server" };
  gxPLId empty_id  = { .vendor = "", .device = "", .instance = "" };
  const gxPLId * pid;

// Tests for source
  UTEST_NEW ("gxPLMessageSourceIdGet() > ");
  pid = gxPLMessageSourceIdGet (m);
  assert (pid);
  ret = gxPLIdCmp (&empty_id, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageSourceIdSet() > ");
  ret = gxPLMessageSourceIdSet (m, &source);
  assert (ret == 0);
  pid = gxPLMessageSourceIdGet (m);
  assert (pid);
  ret = gxPLIdCmp (&source, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageSourceVendorIdSet() > ");
  ret = gxPLMessageSourceVendorIdSet (m, large_text);
  assert (ret == -1);
  ret = gxPLIdCmp (&source, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageSourceDeviceIdSet() > ");
  ret = gxPLMessageSourceDeviceIdSet (m, large_text);
  assert (ret == -1);
  ret = gxPLIdCmp (&source, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageSourceInstanceIdSet() > ");
  ret = gxPLMessageSourceInstanceIdSet (m, large_text);
  assert (ret == -1);
  ret = gxPLIdCmp (&source, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

// Tests for target
  UTEST_NEW ("gxPLMessageTargetIdGet() > ");
  pid = gxPLMessageTargetIdGet (m);
  assert (pid);
  ret = gxPLIdCmp (&empty_id, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageTargetIdSet() > ");
  ret = gxPLMessageTargetIdSet (m, &target);
  assert (ret == 0);
  pid = gxPLMessageTargetIdGet (m);
  assert (pid);
  ret = gxPLIdCmp (&target, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageTargetVendorIdSet() > ");
  ret = gxPLMessageTargetVendorIdSet (m, large_text);
  assert (ret == -1);
  ret = gxPLIdCmp (&target, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageTargetDeviceIdSet() > ");
  ret = gxPLMessageTargetDeviceIdSet (m, large_text);
  assert (ret == -1);
  ret = gxPLIdCmp (&target, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageTargetInstanceIdSet() > ");
  ret = gxPLMessageTargetInstanceIdSet (m, large_text);
  assert (ret == -1);
  ret = gxPLIdCmp (&target, pid);
  assert (ret == 0);
  UTEST_SUCCESS();

// Tests for schema
  gxPLSchema schema = { .class = "x10", .type = "basic" };
  gxPLSchema empty_schema = { .class = "", .type = "" };
  const gxPLSchema * psch;


  UTEST_NEW ("gxPLMessageSchemaGet() > ");
  psch = gxPLMessageSchemaGet (m);
  assert (psch);
  ret = gxPLSchemaCmp (&empty_schema, psch);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageSchemaCopy() > ");
  ret = gxPLMessageSchemaCopy (m, &schema);
  assert (ret == 0);
  psch = gxPLMessageSchemaGet (m);
  assert (psch);
  ret = gxPLSchemaCmp (&schema, psch);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageSchemaClassSet() > ");
  ret = gxPLMessageSchemaClassSet (m, large_text);
  assert (ret == -1);
  ret = gxPLSchemaCmp (&schema, psch);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageSchemaTypeSet() > ");
  ret = gxPLMessageSchemaTypeSet (m, large_text);
  assert (ret == -1);
  ret = gxPLSchemaCmp (&schema, psch);
  assert (ret == 0);
  UTEST_SUCCESS();

// Tests for body
  UTEST_NEW ("gxPLMessageBodySize() > ");
  ret = gxPLMessageBodySize (m);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessagePairAdd() > ");
  ret = gxPLMessagePairAdd (m, "command", "dim");
  assert (ret == 0);
  ret = gxPLMessageBodySize (m);
  assert (ret == 1);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessagePairExist() > ");
  ret = gxPLMessagePairExist (m, "command");
  assert (ret == true);
  ret = gxPLMessagePairExist (m, "none");
  assert (ret == false);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessagePairGet() > ");
  cstr = gxPLMessagePairGet (m, "command");
  assert (cstr);
  ret = strcmp (cstr, "dim");
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessagePairSet() > ");
  ret = gxPLMessagePairSet (m, "command", "test");
  assert (ret == 0);
  cstr = gxPLMessagePairGet (m, "command");
  assert (cstr);
  ret = strcmp (cstr, "test");
  assert (ret == 0);
  UTEST_SUCCESS();

  char buf[64];
  sprintf (buf, "HelloWorld0x%X---%s", 1234, "Ok");

  UTEST_NEW ("gxPLMessagePairSetFormat() > ");
  ret = gxPLMessagePairSetFormat (m, "command", "HelloWorld0x%X---%s", 1234, "Ok");
  assert (ret == 0);
  cstr = gxPLMessagePairGet (m, "command");
  assert (cstr);
  gxPLPrintf ("%s -> %s\n", buf, cstr);
  ret = strcmp (cstr, buf);
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessagePairSet() > ");
  ret = gxPLMessagePairSet (m, "command", "dim");
  assert (ret == 0);
  cstr = gxPLMessagePairGet (m, "command");
  assert (cstr);
  ret = strcmp (cstr, "dim");
  assert (ret == 0);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessagePairAdd() > ");
  ret = gxPLMessagePairAdd (m, "device", "a1");
  assert (ret == 0);
  ret = gxPLMessageBodySize (m);
  assert (ret == 2);
  ret = gxPLMessagePairAdd (m, "level", "75");
  assert (ret == 0);
  ret = gxPLMessageBodySize (m);
  assert (ret == 3);
  UTEST_SUCCESS();

// Test Received
  UTEST_NEW ("gxPLMessageIsReceived() > ");
  ret = gxPLMessageIsReceived (m);
  assert (ret == false);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageReceivedSet() > ");
  ret = gxPLMessageReceivedSet (m, true);
  assert (ret == 0);
  ret = gxPLMessageIsReceived (m);
  assert (ret == true);
  ret = gxPLMessageReceivedSet (m, false);
  assert (ret == 0);
  ret = gxPLMessageIsReceived (m);
  assert (ret == false);
  UTEST_SUCCESS();

  // Converts the message to a string and prints it
  UTEST_NEW ("gxPLMessageToString() > ");
  str = gxPLMessageToString (m);
  assert (str);
  UTEST_SUCCESS();
  gxPLPrintf ("unicast message:\n%s", str);

  // Decode the message
  char * str1 = malloc (strlen (str) + 1);
  assert (str1);
  strcpy (str1, str);

  UTEST_NEW ("gxPLMessageFromString() > ");
  gxPLMessage * rm = gxPLMessageFromString (NULL, str);
  assert (rm);
  char * str2 = gxPLMessageToString (m);
  assert (str2);
  ret = strcmp (str1, str2);
  assert (ret == 0);
  UTEST_SUCCESS();

  gxPLMessageDelete (rm);
  free (str);
  free (str1);
  free (str2);

  UTEST_NEW ("gxPLMessageIsBroadcast() > ");
  ret = gxPLMessageIsBroadcast (m);
  assert (ret == false);
  UTEST_SUCCESS();

  UTEST_NEW ("gxPLMessageBroadcastSet() > ");
  ret = gxPLMessageBroadcastSet (m, true);
  assert (ret == 0);
  ret = gxPLMessageIsBroadcast (m);
  assert (ret == true);

  // Print the message
  str = gxPLMessageToString (m);
  assert (str);
  gxPLPrintf ("broadcast message:\n%s", str);

  // Decode the message
  str1 = malloc (strlen (str) + 1);
  assert (str1);
  strcpy (str1, str);
  rm = gxPLMessageFromString (NULL, str);
  assert (rm);
  str2 = gxPLMessageToString (m);
  assert (str2);
  gxPLPrintf ("received message:\n%s", str2);
  ret = strcmp (str1, str2);
  assert (ret == 0);
  UTEST_SUCCESS();

  gxPLMessageDelete (rm);
  free (str);
  free (str1);
  free (str2);

  UTEST_NEW ("gxPLMessageBodyClear() > ");
  ret = gxPLMessageBodyClear (m);
  assert (ret == 0);
  ret = gxPLMessageBodySize (m);
  assert (ret == 0);
  UTEST_SUCCESS();

  // Delete the message
  gxPLMessageDelete (m);

  gxPLPrintf ("\n\n******************************************\n");
  gxPLPrintf ("**** All tests (%d) were successful ! ****\n", test_count);
  gxPLPrintf ("******************************************\n");
  UTEST_PMEM_AFTER();
  gxPLFflush (stdout);
  gxPLStop();
  return 0;
}
/* ========================================================================== */
