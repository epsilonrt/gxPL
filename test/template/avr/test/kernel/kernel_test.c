/*
 * kernel_test.c
 * Scheduled tasks Test: micro kernel test that runs scheduled tasks.
 * This kernel uses a hardware timer that is set in the avrio-board-kernel.h file.
 * It generates a periodic interrupt (every millisecond mostly) and allows to 
 * perform shared tasks with a round-robin organization.
 * Here the task toggle the state of an LED every 50 milliseconds, a global 
 * variable is used to disable the task every second.
 * This program checks the setting of the  hardware timer.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/task.h>

/* private variables ======================================================== */
static volatile bool bTaskEnabled = true;

/* private functions ======================================================== */
/*
 * LEd Task, Performed under interrupt
 */
static void
vTaskLed (xTaskHandle xTaskLed) {

  if (bTaskEnabled) {

    vLedToggle (LED_LED1);  /* Toggle the status of the LED */
  } 
  else {

    vLedClear (LED_LED1); /* LED off */
  }
  vTaskStart (xTaskLed);  /* restart the countdown for 50 ms */
}

/* internal public functions ================================================ */
int
main (void) {
  xTaskHandle xTaskLed;

  vLedInit ();
  xTaskLed = xTaskCreate (xTaskConvertMs (50), vTaskLed);
  vTaskStart (xTaskLed);

  for (;;) {

    /* Toggle the status of task every second */
    delay_ms (1000);
    bTaskEnabled = !bTaskEnabled;
  }
  return 0;
}

/* ========================================================================== */
