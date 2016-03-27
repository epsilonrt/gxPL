/*
 * tsl230_test.c
 * TSL230 Test: Takes measurements and display on the serial terminal, 
 * the values are displayed in a tabular order to be processed by a spreadsheet, 
 * here is a sample display:
 * Tsl230 Test
 * Range:2000 W/m2
 * fo(kHz),Ee(W/m2)
 * 589.453,14.923
 * 579.200,14.663
 * 588.573,14.901
 *  ...
 * Under Windows, the software serialchart (https://code.google.com/archive/p/serialchart) 
 * can be used to display measurements in graph form. The serialchart 
 * configuration file (tsl230.scc) should be amended to match the serial link 
 * connected to the PC (port = COM1) and Max irradiance value 
 * (max parameter in the [_default_].
 * This module uses a hardware timer 16-bit, configuration is made in 
 * the avrio-board-tsl230.h file.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/tc.h>
#include <avrio/tsl230.h>

/* constants ================================================================ */
#define TERMINAL_BAUDRATE 38400

/* private variables ======================================================== */
static double dFreq, dIrradiance;

/* main ===================================================================== */
int
main (void) {
  uint16_t usRange;

  vLedInit ();
  
  // Initialization of the serial port for display
  xSerialIos settings = SERIAL_SETTINGS (TERMINAL_BAUDRATE);
  FILE * tc = xFileOpen ("tty0", O_RDWR, &settings);
  stdout = tc;
  
  vTsl230Init();
  sei(); // Enables interruptions (used by tsl230 and tc modules)

  // Reading initial measurement range
  usRange = usTsl230Range();
  // Performs autorange if uncommented
  //usRange = usTsl230AutoRange();

  // prints the measurement range and the header
  printf ("\nTsl230 Test\nRange:%u W/m2\nfo(kHz),Ee(W/m2)\n", usRange);

  for (;;) {

    /*
     * triggering measurement and reading frequency, set true instead of false 
     * to enable autorange ...
     */
    dFreq = dTsl230ReadFreq (true);
    // Convert from frequency to irradiance µW/cm²
    dIrradiance = dTsl230FreqToIrradiance (dFreq);
    // Prints the frequency in kHz, and irradiance in W/m²
    printf ("%.3f,%.3f\n", dFreq / 1000., dIrradiance / 100.);
    // Waiting 100 ms and switches LED1
    delay_ms (100);
    vLedToggle (LED_LED1);
  }
  return 0;
}
/* ========================================================================== */
