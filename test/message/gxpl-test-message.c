/*
 * gxpl-test-message.c
 * @brief Description de votre programme
 *
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <gxPL/message.h>

/* macros =================================================================== */
#define test(t) do { \
    if (!t) { \
      fprintf (stderr, "line %d in %s: test %d failed !\n",  __LINE__, __FUNCTION__, test_count); \
      exit (EXIT_FAILURE); \
    } \
  } while (0)

/* private variables ======================================================== */
static int test_count;

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret;
  gxPLMessage * m = NULL;
  char * str = NULL;

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
  gxPLMessageId source = { .vendor = "xpl", .device = "xplhal", .instance = "myhouse" };
  gxPLMessageId target = { .vendor = "acme", .device = "cm12", .instance = "server" };
  gxPLMessageId empty_id  = { .vendor = "", .device = "", .instance = "" };
  const gxPLMessageId * pid;

  // Tests for source
  test_count++;
  pid = gxPLMessageSourceIdGet (m);
  test (pid);
  
  test_count++;
  ret = gxPLMessageIdCmp (&empty_id, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSourceIdSet (m, &source);
  test (ret == 0);
  
  test_count++;
  pid = gxPLMessageSourceIdGet (m);
  test (pid);
  
  test_count++;
  ret = gxPLMessageIdCmp (&source, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSourceVendorIdSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageIdCmp (&source, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSourceDeviceIdSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageIdCmp (&source, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSourceInstanceIdSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageIdCmp (&source, pid);
  test (ret == 0);

  // Tests for target
  test_count++;
  pid = gxPLMessageTargetIdGet (m);
  test (pid);
  
  test_count++;
  ret = gxPLMessageIdCmp (&empty_id, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageTargetIdSet (m, &target);
  test (ret == 0);
  
  test_count++;
  pid = gxPLMessageTargetIdGet (m);
  test (pid);
  
  test_count++;
  ret = gxPLMessageIdCmp (&target, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageTargetVendorIdSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageIdCmp (&target, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageTargetDeviceIdSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageIdCmp (&target, pid);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageTargetInstanceIdSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageIdCmp (&target, pid);
  test (ret == 0);

  // Tests for schema
  gxPLMessageSchema schema = { .class = "x10", .type = "basic" };
  gxPLMessageSchema empty_schema = { .class = "", .type = "" };
  const gxPLMessageSchema * psch;
  
  test_count++;
  psch = gxPLMessageSchemaGet (m);
  test (psch);
  
  test_count++;
  ret = gxPLMessageSchemaCmp (&empty_schema, psch);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSchemaSet (m, &schema);
  test (ret == 0);
  
  test_count++;
  psch = gxPLMessageSchemaGet (m);
  test (psch);
  
  test_count++;
  ret = gxPLMessageSchemaCmp (&schema, psch);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSchemaClassSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageSchemaCmp (&schema, psch);
  test (ret == 0);

  test_count++;
  ret = gxPLMessageSchemaTypeSet (m, large_text);
  test (ret == -1);
  ret = gxPLMessageSchemaCmp (&schema, psch);
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
  str = gxPLMessagePairValueGet(m, "command");
  test (str);
  ret = strcmp (str, "dim");
  test (ret == 0);

  test_count++;
  ret = gxPLMessagePairValueSet(m, "command", "test");
  test (ret == 0);
  str = gxPLMessagePairValueGet(m, "command");
  test (str);
  ret = strcmp (str, "test");
  test (ret == 0);
  
  test_count++;
  char buf[64];
  sprintf (buf, "HelloWorld0x%X---%s", 1234, "Ok");
  ret = gxPLMessagePairValuePrintf (m, "command", "HelloWorld0x%X---%s", 1234, "Ok");
  test (ret == 0);
  str = gxPLMessagePairValueGet(m, "command");
  test (str);
  printf ("%s -> %s\n", buf, str);
  ret = strcmp (str, buf);
  test (ret == 0);

  test_count++;
  ret = gxPLMessagePairValueSet(m, "command", "dim");
  test (ret == 0);
  str = gxPLMessagePairValueGet(m, "command");
  test (str);
  ret = strcmp (str, "dim");
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

  // Print the message
  test_count++;
  str = gxPLMessageToString (m);
  test (str);
  printf ("unicast message:\n%s\n", str);
  free(str);
  
  // Test Received
  test_count++;
  ret = gxPLMessageReceivedGet(m);
  test(ret == false);
  ret = gxPLMessageReceivedSet(m, true);
  test(ret == 0);
  ret = gxPLMessageReceivedGet(m);
  test(ret == true);
  ret = gxPLMessageReceivedSet(m, false);
  test(ret == 0);
  ret = gxPLMessageReceivedGet(m);
  test(ret == false);

  // Test Broadcast
  test_count++;
  ret = gxPLMessageBroadcastGet(m);
  test(ret == false);
  ret = gxPLMessageBroadcastSet(m, true);
  test(ret == 0);
  ret = gxPLMessageBroadcastGet(m);
  test(ret == true);

  
  // Print the message
  test_count++;
  str = gxPLMessageToString (m);
  test (str);
  printf ("broadcast message:\n%s\n", str);
  free(str);

  // Clear body
  test_count++;
  ret = gxPLMessageBodyClear(m);
  test(ret == 0);
  ret = gxPLMessageBodySize (m);
  test (ret == 0);
  
  // Delete the message
  test_count++;
  ret = gxPLMessageDelete (m);
  test (ret == 0);

  printf ("%d tests were successful !\n", test_count);
  return 0;
}
/* ========================================================================== */
