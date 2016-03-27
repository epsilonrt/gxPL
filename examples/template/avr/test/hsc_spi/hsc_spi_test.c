/*
 * hsc_spi_test.c
 * HSC Pressure sensor Test: Takes measurements and display on the serial terminal, 
 * the values are displayed in a tabular order to be processed by a spreadsheet, 
 * here is a sample display:
 *  HSC SPI Demo
 *  P(hPa),T(oC)
 *  1010.25,23.2
 *  ...
 * Under Windows, the software serialchart (https://code.google.com/archive/p/serialchart) 
 * can be used to display measurements in graph form. The serialchart 
 * configuration file (tsl230.scc) should be amended to match the serial link 
 * connected to the PC (port = COM1).
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/tc.h>
#include <avrio/spi.h>
#include <avrio/hsc.h>

/* constants ================================================================ */
#define TERMINAL_BAUDRATE 38400

#define SPI_DIV SPI_DIV32 // Fsclk 800 KHz max.
// 1.6 Bar sensor
#define P_MAX 1600.0f
// 5 PSI sensor
//#define P_MAX (PSI_TO_PA(15)/100.0f)

/* internal public functions ================================================ */
void vSensorSelect (bool bEnable);

/* main ===================================================================== */
int
main (void) {
  int iError;
  xHscSensor xSensor;
  xHscRaw xRaw;
  xHscValue xValue;

  vLedInit();
  
  // Initialization of the serial port for display
  xSerialIos settings = SERIAL_SETTINGS (TERMINAL_BAUDRATE);
  FILE * tc = xFileOpen ("tty0", O_RDWR, &settings);
  stdout = tc;
  sei();
  
  // prints the measurement range and the header
  printf ("\nHSC SPI Demo\nP(hPa),T(oC)\n");
  
  // SPI Bus Init.
  vSpiSetSsAsOutput();      // Master Only.
  vSpiMasterInit (SPI_DIV); // Fsclk 800 KHz max.
  
  // Sensor Init
  iHscInitSpiSensor (&xSensor,  0, P_MAX, 0, vSensorSelect);

  for (;;) {

    // Reading raw values
    iError = iHscGetRaw (&xSensor, &xRaw);
    if (iError == 0) {

      // Convert from raw to physcal values
      vHscRawToValue (&xSensor, &xRaw, &xValue);
      
      // Prints pressure in hPa and temperature en oC
      printf ("%.2f,%.2f\n", xValue.dPress, xValue.dTemp);
    }
    else {
      
      printf ("Sensor Error: %d\n", iError);
    }
    // Waiting 100 ms and switches LED1
    delay_ms (100);
    vLedToggle (LED_LED1);
  }

  return 0;
}

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
// Fonction pour la sélection du capteur (broche /SS à 0)
void vSensorSelect (bool bEnable) {

  if (bEnable)
    vSpiSetSs();
  else
    vSpiClearSs();
}

/* ========================================================================== */
