/*
 * button_test.c
 * Push buttons Test: The LED flashes 1 times to button 1, 2 times for 2 ....
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/button.h>
#include <avrio/delay.h>

int
main (void) {
  uint8_t ucBit;

  vLedInit ();
  vButInit ();

  for (;;) {

    for (ucBit = 0; ucBit < BUTTON_QUANTITY; ucBit++) {

      if (xButGet (xButGetMask (ucBit))) {
        uint8_t ucCount = (ucBit + 1) * 2;
        
        while (ucCount--) {
          // The LED flashes 1 times to button 1, 2 times for 2 ....
          vLedToggle (LED_LED1);
          delay_ms (200);
        }
      }
    }
  }
  return 0;
}
