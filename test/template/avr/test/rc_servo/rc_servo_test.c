/*
 * rc_servo_test.c
 * RC Servos Test: At startup, all servos receive a periodic signal with a pulse 
 * duration 1500us (0°)
 * One can vary the pulse width between 800us and  2200us by step of 20us.
 * Each press of the button increases the duration of 20us, after the maximum 
 * occured, the duration is reduced to the minimum.
 * This module uses a hardware timer 16-bit , configuration is made in 
 * the avrio-board-servo.h file.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/button.h>
#include <avrio/servo.h>

/* constants ================================================================ */
#define DEFAULT_PULSE 1500
#define MIN_PULSE     800
#define MAX_PULSE     2200
#define STEP_PULSE    20

/* main ===================================================================== */
int
main (void) {
  uint16_t usPulse = DEFAULT_PULSE;

  vLedInit ();
  vButInit ();
  vServoInit (); // All channels are enabled

  // Trimming the default pulse width
  vLedSet (LED_LED1);
  for (uint8_t ucServo = 0; ucServo < ucServoChannels(); ucServo++) {

    vServoSetPulse (ucServo, DEFAULT_PULSE);
  }

  for (;;) {

    if (xButGet (BUTTON_BUTTON1)) {
      // The push button has been pressed.
      while (xButGet (BUTTON_BUTTON1))
        ; // Wait for the release of the button1
      usPulse += STEP_PULSE; // Increases the width of a step.
      // If the width is too large, it amounts to min
      if (usPulse > MAX_PULSE) {
        usPulse = MIN_PULSE;
      }
      // We send the new setting to the servos
      for (uint8_t ucServo = 0; ucServo < ucServoChannels(); ucServo++) {

        vServoSetPulse (ucServo, usPulse);
      }
      // We switches the state of LED1
      vLedToggle (LED_LED1);
    }
  }
  return 0;
}

/* ========================================================================== */
