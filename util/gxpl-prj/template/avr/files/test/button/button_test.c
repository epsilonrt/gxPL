/*
 * button_test.c
 * Push buttons Test: The LED flashes 1 times to button 1, 2 times for 2 ....
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/button.h>
#include <avrio/tc.h>
#include <avrio/delay.h>

/* constants ================================================================ */
#define PORT         "tty1"
#define BAUDRATE     38400

/* main ===================================================================== */
int
main (void) {
  static uint16_t usCount;
  xButMask xCurrent;
  static xButMask xPrevious;

  vLedInit ();
  vButInit ();
  xSerialIos settings = SERIAL_SETTINGS (BAUDRATE);
  FILE * tc = xFileOpen (PORT, O_WR | O_NONBLOCK, &settings);
  stdout = tc;
  sei();
  
  printf ("Button test\n");
  for (;;) {

    xCurrent = xButGet (BUTTON_ALL_BUTTONS);

    if (xCurrent != xPrevious) {

      usCount++;
      xButMask xDiff = xCurrent ^ xPrevious;
      for (uint8_t ucBit = 0; ucBit < BUTTON_QUANTITY; ucBit++) {
        if (xDiff & xButGetMask (ucBit)) {
          
          printf ("%03d - %c: %u\n", usCount % 1000, (xCurrent & xButGetMask (ucBit)) ? 'P' : 'R', ucBit);
        }
      }
      xPrevious = xCurrent;
    }
  }
  return 0;
}
