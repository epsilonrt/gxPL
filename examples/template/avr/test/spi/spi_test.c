/*
 * spi_test.c
 * SPI Bus Test: This test writes a byte to the SPI bus and reads this byte 
 * checking that the reading is the same as written.
 * If it works, LED1 flashes regularly, otherwise it flashes quickly.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/spi.h>
#include <avrio/delay.h>

/* constants ================================================================ */
#define TEST_BYTE 0x55

/* internal public functions ================================================ */
void vLedAssert (int i);

/* main ===================================================================== */
int
main (void) {
  uint8_t ucByte;

  vLedInit();
  vSpiSetSsAsOutput();
  vSpiMasterInit (SPI_DIV2);

  for (;;) {

    vSpiSetSs();
    vSpiMasterWrite (TEST_BYTE);
    ucByte = ucSpiMasterRead();
    vSpiClearSs();

    vLedAssert (ucByte == TEST_BYTE);
    vLedToggle (LED_LED1);
    delay_ms (500);
  }

  return 0;
}

/* internal public functions ================================================ */

/* -----------------------------------------------------------------------------
 * Checks that the condition is true, otherwise the LED flashes quickly
 */
void
vLedAssert (int i) {

  if (!i) {

    for (;;) {

      vLedSet (LED_LED1);
      delay_ms (50);
      vLedClear (LED_LED1);
      delay_ms (100);
    }
  }
}

/* ========================================================================== */
