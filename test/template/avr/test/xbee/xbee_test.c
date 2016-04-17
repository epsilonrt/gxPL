/*
 * xbee_test.c
 * Test XBee Router / End Device XBee Test
 * - Displays content of data packets received on serial port (and toggles LED1)
 * - Send test data packet to each key press.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#define __ASSERT_USE_STDERR
#include <avrio/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <avrio/xbee.h>
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/tc.h>

/* constants ================================================================ */
#define XBEE_BAUDRATE   38400
#define XBEE_PORT       "tty0"
#define XBEE_RESET_PORT PORTD
#define XBEE_RESET_PIN  4

#define TERMINAL_BAUDRATE 115200
#define TERMINAL_PORT     "tty1"

/* private variables ======================================================== */
static xXBee * xbee;
volatile uint8_t ucModemStatus;
volatile int iDataFrameId = 0;
volatile int iAtLocalFrameId = 0;
volatile int iAtLocalStatus = XBEE_PKT_STATUS_UNKNOWN;
static xXBeePkt * xAtLocalPkt;
static char sMyNID[21];
static xDPin xResetPin = { .port = &XBEE_RESET_PORT, .pin = XBEE_RESET_PIN };

/* private functions ======================================================== */
int iInit (xSerialIos * xXBeeIos);
void vWaitToJoinNetwork (void);
int iSendAtCmd (const char * sCmd, uint8_t * pParams, uint8_t ucParamsLen);
void vLedAssert (int i);

int iDataCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len);
int iTxStatusCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len);
int iAtLocalCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len);
int iModemStatusCB (xXBee * xbee, xXBeePkt * pkt, uint8_t len);

/* internal public functions ================================================ */
int
main (void) {
  static int ret;
  xSerialIos xXBeeIos = {
    .baud = XBEE_BAUDRATE, .dbits = SERIAL_DATABIT_8,
    .parity = SERIAL_PARITY_NONE, .sbits = SERIAL_STOPBIT_ONE,
    .flow = SERIAL_FLOW_RTSCTS, .eol = SERIAL_BINARY
  };

  if (iInit (&xXBeeIos) != 0) {

    return -1;
  }

  vWaitToJoinNetwork();

  for (;;) {

    // Polling receiving packets
    ret = iXBeePoll (xbee, 0);
    assert (ret == 0);

    if (getchar() != EOF) {
      char message[33];
      static int iCount = 1;

      snprintf (message, 32, "%s #%d", sMyNID, iCount++);

      iDataFrameId = iXBeeZbSendToCoordinator (xbee, message, strlen (message));
      assert (iDataFrameId >= 0);

      printf ("Tx%d>%s\n", iDataFrameId, message);
    }
  }

  ret = iXBeeClose (xbee);
  return 0;
}


// -----------------------------------------------------------------------------
int
iInit (xSerialIos * xXBeeIos) {

  /*
   * LED used to indicate the reception of a packet or to report an error
   * when opening the serial terminal.
   */
  vLedInit();

  // Initialization of the serial port for display
  xSerialIos xTermIos = SERIAL_SETTINGS (TERMINAL_BAUDRATE);
  FILE * tc = xFileOpen (TERMINAL_PORT, O_RDWR | O_NONBLOCK, &xTermIos);
  vLedAssert (tc != NULL);
  stdout = tc;
  stderr = tc;
  stdin = tc;
  sei();

  printf ("\n**  XBee Test **\nPress any key to send msg\nInit... ");
  xbee = xXBeeNew (XBEE_SERIES_S2, &xResetPin);
  assert (xbee);

  vXBeeSetCB (xbee, XBEE_CB_AT_LOCAL, iAtLocalCB);
  vXBeeSetCB (xbee, XBEE_CB_MODEM_STATUS, iModemStatusCB);

  if (iXBeeOpen (xbee, XBEE_PORT, xXBeeIos) != 0) {

    printf ("iXBeeOpen() failed !\n");
    return -1;
  }

  printf ("Success\n");
  delay_ms (500);
  return 0;
}

// -----------------------------------------------------------------------------
int
iSendAtCmd (const char * sCmd, uint8_t * pParams, uint8_t ucParamsLen) {

  iAtLocalStatus = XBEE_PKT_STATUS_UNKNOWN;
  iAtLocalFrameId = iXBeeSendAt (xbee, sCmd, pParams, ucParamsLen);
  assert (iAtLocalFrameId > 0);

  while (iAtLocalStatus == XBEE_PKT_STATUS_UNKNOWN) {

    iXBeePoll (xbee, 0);
  }
  return iAtLocalStatus;
}

// -----------------------------------------------------------------------------
void
vWaitToJoinNetwork (void) {
  int ret, i;

  if (iSendAtCmd (XBEE_CMD_PAN_ID, NULL, 0) == XBEE_PKT_STATUS_OK) {

    ret = iXBeePktParamLen (xAtLocalPkt);

    if (ret == 8) {
      uint32_t ulMsb, ulLsb;

      if (iXBeePktParamGetULong (&ulMsb, xAtLocalPkt, 0) == 0) {
        if (iXBeePktParamGetULong (&ulLsb, xAtLocalPkt, 4) == 0) {

          printf ("Requested PAN Id> 0x%lx%08lx\n", ulMsb, ulLsb);
        }
      }
    }
    vXBeeFreePkt (xbee, xAtLocalPkt);
  }

  i = printf ("Waiting to join");

  while (ucModemStatus != XBEE_PKT_MODEM_JOINED_NETWORK) {

    if (i++ < 80) {

      putchar ('.');
    }
    else {

      putchar ('\n');
      i = 0;
    }

    iXBeePoll (xbee, 0);
    delay_ms (1000);
  }

  // Connected to the network
  if (iSendAtCmd (XBEE_CMD_OPERATING_PAN_ID, NULL, 0) == XBEE_PKT_STATUS_OK) {

    ret = iXBeePktParamLen (xAtLocalPkt);

    if (ret == 8) {
      uint32_t ulMsb, ulLsb;

      if (iXBeePktParamGetULong (&ulMsb, xAtLocalPkt, 0) == 0) {
        if (iXBeePktParamGetULong (&ulLsb, xAtLocalPkt, 4) == 0) {

          printf ("\nOperating PAN Id> 0x%lx%08lx\n", ulMsb, ulLsb);
        }
      }
    }
    vXBeeFreePkt (xbee, xAtLocalPkt);
  }
  delay_ms (2000);

  if (iSendAtCmd (XBEE_CMD_NODE_ID, NULL, 0) == XBEE_PKT_STATUS_OK) {

    ret = iXBeePktParamGetStr (sMyNID, xAtLocalPkt, sizeof (sMyNID));

    if (ret > 0) {

      printf ("NID>%s\n", sMyNID);
    }
    vXBeeFreePkt (xbee, xAtLocalPkt);
  }

  vXBeeSetCB (xbee, XBEE_CB_TX_STATUS, iTxStatusCB);
  vXBeeSetCB (xbee, XBEE_CB_DATA, iDataCB);
}


// -----------------------------------------------------------------------------
int
iTxStatusCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {

  if (iXBeePktFrameId (pkt) == iDataFrameId) {
    int status = iXBeePktStatus (pkt);

    if (status) {

      printf ("Tx%d Err. %d\n", iDataFrameId, status);
    }
    else {

      printf ("Tx%d Ok\n", iDataFrameId);
    }
    iDataFrameId = 0;
  }
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
int
iDataCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  int size;

  size = iXBeePktDataLen (pkt);
  if (size > 0) {
    volatile char * p;
    uint8_t * src64 = pucXBeePktAddrSrc64 (pkt);

    printf ("...");

    for (int i = 4; i < 8; i++) {

      printf ("%02X", src64[i]);
    }

    putchar ('>');

    if (iXBeePktIsBroadcast (pkt)) {
      putchar ('*');
    }

    p = (char *) pucXBeePktData (pkt);
    p[size] = 0;
    printf ("\n%s\n", p);
    vLedToggle (LED_LED1);
  }


  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
int
iModemStatusCB (xXBee * xbee, xXBeePkt * pkt, uint8_t len) {
  int i = iXBeePktStatus (pkt);

  if (i != -1) {

    ucModemStatus = (unsigned) i;
  }
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
int
iAtLocalCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {

  if (iXBeePktFrameId (pkt) == iAtLocalFrameId) {

    iAtLocalStatus = iXBeePktStatus (pkt);
    iAtLocalFrameId = 0;
    if (iAtLocalStatus == XBEE_PKT_STATUS_OK) {

      xAtLocalPkt = pkt;
    }
    else {

      vXBeeFreePkt (xbee, pkt);
    }
  }
  return 0;
}

/* -----------------------------------------------------------------------------
 * Checks that the condition is true, otherwise the LED flashes quickly always
 */
void
vLedAssert (int i) {

  if (!i) {

    for (;;) {

      vLedSet (LED_LED1);
      delay_ms (5);
      vLedClear (LED_LED1);
      delay_ms (25);
    }
  }
}

/* ========================================================================== */
