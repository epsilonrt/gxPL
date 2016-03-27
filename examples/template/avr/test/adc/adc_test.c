/*
 * adc_test.c
 * ADC Test: Print the raw and measure value for each ADC channel, ie:
 * R00,M00,R01,M01,R02,M02,R03,M03
 * 515,1.66,538,1.73,34,0.55,0,0.00
 * 514,1.66,476,1.53,33,0.53,0,0.00
 * 515,1.66,543,1.75,34,0.55,0,0.00
 * 514,1.66,469,1.51,33,0.53,0,0.00
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/tc.h>
#include <avrio/adc.h>

/* constants ================================================================ */
#define TERMINAL_BAUDRATE 38400

static const uint8_t ucAdcChan[ADC_CHAN_QUANTITY] = ADC_CHAN_LIST;
static const double dAdcFullScale [ADC_CHAN_QUANTITY] = ADC_FULLSCALE_LIST;

/* main ===================================================================== */
int
main (void) {
  uint16_t usAdc;
  uint8_t ucIndex;
  
  xSerialIos settings = SERIAL_SETTINGS (TERMINAL_BAUDRATE);
  FILE * tc = xFileOpen ("tty0", O_RDWR, &settings);
  stdout = tc;
  stdin = tc;
  sei();
  
  vLedInit();
  vAdcInit();

  // Print header line
  putchar  ('\n');
  for (ucIndex = 0; ucIndex < ADC_CHAN_QUANTITY; ucIndex++) {

    printf("R%02d,M%02d", ucIndex, ucIndex);
    if (ucIndex < (ADC_CHAN_QUANTITY -1)) {
      putchar (',');
    }
  }
  putchar  ('\n');

  for (;;) {

    for (ucIndex = 0; ucIndex < ADC_CHAN_QUANTITY; ucIndex++) {

      usAdc  = usAdcReadAverage (ucAdcChan[ucIndex], 8);
      printf("%d,%.2f", usAdc, ADC_MEASUREMENT(usAdc, dAdcFullScale[ucIndex]));
      if (ucIndex < (ADC_CHAN_QUANTITY -1)) {
        putchar (',');
      }
    }
    putchar  ('\n');
    vLedToggle (LED_LED1);
    delay_ms (1000);
  }
  return 0;
}
/* ========================================================================== */
