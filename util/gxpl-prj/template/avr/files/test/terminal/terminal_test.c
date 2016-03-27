/*
 * terminal_test.c
 * Serial Terminal Test: Prints 32 times the alphabet and then wait for the 
 * user to transmit characters over the serial link and refers to the receipt 
 * of a carriage return, then resumes. On error, the LED1 flashes quickly, 
 * otherwise it switches to each correct action.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/tc.h>
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avr/version.h>

/* constants ================================================================ */
#define PORT         "tty0"
#define BAUDRATE     38400

/* internal public functions ================================================ */
void vAlphabetTest (void);
void vTerminalTest (void);
void vLedAssert (int i);

/* main ===================================================================== */
int
main (void) {

  vLedInit();
  xSerialIos settings = SERIAL_SETTINGS (BAUDRATE);
  FILE * tc = xFileOpen (PORT, O_RDWR | O_NONBLOCK, &settings);
  stdout = tc;
  stdin = tc;
  sei();

  for (;;) {

    vAlphabetTest ();
    vTerminalTest ();
  }
  return 0;
}

/* internal public functions ================================================ */
static int iErr;
static int c;

/* -----------------------------------------------------------------------------
 * Prints 32 times the alphabet (A-Z)
 */
void
vAlphabetTest (void) {
  uint8_t ucCount = 32;

  vLedSet (LED_LED1);
  while (ucCount--) {

    c = 'A';
    do {

      iErr = putchar (c);
      vLedAssert (iErr == c);
    }
    while (c++ < 'Z');

    c = '\n';
    iErr = putchar (c);
    vLedAssert (iErr == c);
  }
  vLedClear (LED_LED1);
}

/* -----------------------------------------------------------------------------
 * Wait for the user to transmit characters over the serial link and refers to 
 * the receipt of a carriage return
 */
void
vTerminalTest (void) {
  uint16_t usCount = 0;
  bool isWait = true;

  printf("\nTerminal Test (%s version)\nPress any key (ENTER to quit)...",
            (AVRIO_TC_FLAVOUR == TC_FLAVOUR_POLL) ? "poll" : "irq");
  do {

    c = getchar ();
    if (c != EOF) {

      if (isWait) {
        iErr = putchar ('\n');
        vLedAssert (iErr == '\n');
        isWait = false;
      }
      iErr = putchar (c);
      vLedAssert (iErr == c);
      vLedToggle (LED_LED1);
    }
    else {
      if ( (isWait) && ( (usCount++ % 32768) == 0)) {

        iErr = putchar ('.');
        vLedAssert (iErr == '.');
      }
    }
  }
  while (c != '\r');       /* Return pour terminer */
  iErr = putchar ('\n');
  vLedAssert (iErr == '\n');
}

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
