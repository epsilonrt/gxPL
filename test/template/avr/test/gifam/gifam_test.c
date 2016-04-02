/**
 * gifam-test.c
 * GIFAM control from a serial terminal:
 * - 0: Comfort (No signal)
 * - 1: Off (+)
 * - 2: AboveFreezing (-)
 * - 3: Economy (full)
 * - 4: Comfort-1 (3"/4'57)
 * - 5: Comfort-2 (7"/4'53)
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <avrio/twi.h>
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/tc.h>

/* constants ================================================================ */
#define TERMINAL_BAUDRATE 115200
#define TERMINAL_PORT     "tty0"

#define GIFAM_SLAVE 0x20

/* private functions ======================================================== */
void vLedAssert (int i);
void vSetGifam (uint8_t mode);

/* main ===================================================================== */
int
main (void) {
  int ret;

  vLedInit();

  // Initialization of the serial port for display
  xSerialIos xTermIos = SERIAL_SETTINGS (TERMINAL_BAUDRATE);
  FILE * tc = xFileOpen (TERMINAL_PORT, O_RDWR | O_NONBLOCK, &xTermIos);
  vLedAssert (tc != NULL);
  stdout = tc;
  stdin = tc;
  sei();

  printf ("\nGIFAM Test\nPress Key 0-5 to set GIFAM mode...\n");
  printf ("- 0: Comfort (No signal)\n");
  printf ("- 1: Off (+)\n");
  printf ("- 2: AboveFreezing (-)\n");
  printf ("- 3: Economy (full)\n");
  printf ("- 4: Comfort-1 (3\"/4'57)\n");
  printf ("- 5: Comfort-2 (7\"/4'53) \n\n");

  // I²C bus initialization as master at 400 kHz and no error checking
  vTwiInit ();

  ret = eTwiSetSpeed (400);
  if (ret != 0) {

    printf ("eTwiSetSpeed failed, error = % d\n", ret);
    for (;;);
  }

  vSetGifam (0);

  for (;;) {

    ret = getchar();
    if ( (ret >= '0') && (ret <= '5')) {
      uint8_t req_mode = ret - '0'; // Convert ASCII -> Bin

      vSetGifam (req_mode);
    }
  }
}

// -----------------------------------------------------------------------------
void
vSetGifam (uint8_t req_mode) {
  int ret;
  uint8_t set_mode;

  printf ("Set mode % d > ", req_mode);
  ret = eTwiWrite (GIFAM_SLAVE, req_mode);
  if (ret == 0) {

    ret = eTwiRead (GIFAM_SLAVE, &set_mode);
    if (ret == 0) {

      printf ("Success\n");
    }
    else {

      printf ("Read failed, error = % d\n", ret);
    }
  }
  else {

    printf ("Write failed, error = % d\n", ret);
  }
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
