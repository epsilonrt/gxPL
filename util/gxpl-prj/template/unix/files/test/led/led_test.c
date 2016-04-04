/*
 * sysio_demo_led.c
 * Démo d'utilisation de doutput pour gérer des leds
 *
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sysio/delay.h>
#include <sysio/doutput.h>
#include <sysio/rpi.h>

/* constants ================================================================ */
/*
 * Description des broches de sortie reliées aux leds
 */
static const xDout xMyLeds[] = {
  { .num = GPIO17, .act = 1 }, /* broche GPIO17 (broche 11 GPIO), active à l'état haut... */
  { .num = GPIO18, .act = 1 },
  { .num = GPIO27, .act = 1 }
};

/* private variables ======================================================== */
/* Port pour gérer les leds */
static xDoutPort * xLedPort;


/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  iDoutClearAll (xLedPort);
  iDoutClose (xLedPort);
  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {

  xLedPort = xDoutOpen (xMyLeds, 3);
  
  if (xLedPort == NULL) {
    
    printf ("Failed to open port !\n");
    return -1;
  }
  iDoutClearAll (xLedPort);

  // vSigIntHandler() intercepte le CTRL+C
  signal (SIGINT, vSigIntHandler);

  printf ("Press Ctrl+C to abort ...\n");

  for (;;) {

    // Allume les leds les unes à la suite des autres
    for (unsigned led = 0; led < iDoutPortSize (xLedPort); led++) {

      iDoutSet (led, xLedPort);
      delay_ms (500);
    }
    delay_ms (1000);

    // Allume les leds les unes à la suite des autres
    for (unsigned led = 0; led < iDoutPortSize (xLedPort); led++) {

      iDoutClear (led, xLedPort);
      delay_ms (500);
    }
    delay_ms (1000);
  }

  return 0;
}
/* ========================================================================== */
