/*
 * lcd_test.c
 * LCD Test
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/lcd.h>

/* main ===================================================================== */
int
main (void) {
  int i;

  vLedInit();
  delay_ms (200);
  iLcdInit();
  ucLcdBacklightSet (32);
  ucLcdContrastSet (16);
  stdout = &xLcd;

  for (;;) {

    vLcdClear ();
    printf ("Hello World !\n");

    for (i=9; i >=0; i--) {

      printf ("%d", i);
      delay_ms (500);
    }

    vLcdGotoXY (13, 0);
    printf ("Go!");
    delay_ms (5000);
  }
  return 0;
}
/* ========================================================================== */
