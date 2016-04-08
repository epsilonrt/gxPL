/*
 * iaq_test.c
 * IAQ sensor test
 * 
 * Displays the values measured by an air quality sensor IAQ
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sysio/delay.h>
#include <sysio/i2c.h>

/* constants ================================================================ */
#define IAQ_OK    0x00
#define IAQ_BUSY  0x01
#define IAQ_RUNIN 0x10 // (module in warm up phase)
#define IAQ_ERROR 0x80

/* private variables ======================================================== */
static int fd;

/* public variables ========================================================= */

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  if (iI2cClose (fd) != 0) {
    perror ("iI2cClose");
    exit (EXIT_FAILURE);
  }
  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {
  uint8_t buf[9];
  uint16_t pred;
  uint8_t status;
  uint32_t resistance;
  uint16_t tvoc;

  fd = iI2cOpen ("/dev/i2c-1", 0x5A);
  if (fd < 0) {

    perror ("Failed to open i2c ! ");
  }

  // vSigIntHandler() intercepte le CTRL+C
  signal (SIGINT, vSigIntHandler);
  printf ("Press Ctrl+C to abort ...\n\n");
  printf ("Status,CO2(ppm),Resistance(ohm),Tvoc(ppb)\n");

  for (;;) {

    // Lecture d'un bloc de 9 octets
    if (iI2cReadBlock (fd, buf, 9) < 0) {

      perror ("Failed to read i2c ! ");
      continue;
    }

    // Extraction des données du bloc
    pred = (buf[0] << 8) + buf[1];
    status = buf[2];
    resistance = (buf[4] << 16) + (buf[5] << 8) + buf[6];
    tvoc = (buf[7] << 8) + buf[8];

    // Affichage
    switch (status) {
      case IAQ_OK:
        printf ("ok");
        break;
      case IAQ_BUSY:
        printf ("busy");
        break;
      case IAQ_RUNIN:
        printf ("warmup");
        break;
      case IAQ_ERROR:
        printf ("ERROR");
        break;
      default:
        printf ("Unknown status 0x%x", status);
        break;
    }
    printf (",%d,%d,%d\n", pred, resistance, tvoc);
    fflush (stdout);
    delay_ms (11000);
  }

  return 0;
}
/* ========================================================================== */
