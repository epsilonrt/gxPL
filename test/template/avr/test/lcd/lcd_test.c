/*
 * lcd_test.c
 * LCD Test
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <avrio/led.h>
#include <avrio/delay.h>
#include <avrio/lcd.h>
#include <avr/pgmspace.h>

/* internal public functions ================================================ */
void vTestBase (void);
void vTestBargraph (void);

/* main ===================================================================== */
int
main (void) {

  vLedInit();
  iLcdInit();
  ucLcdBacklightSet (8);
  ucLcdContrastSet (24);
  stdout = &xLcd;

  for (;;) {

    vTestBase();
    vTestBargraph();
    delay_ms (5000);
  }
  return 0;
}
/* internal public functions ================================================ */
/* -----------------------------------------------------------------------------
 | ------------------------------   Test de base ----------------------------- |
 * ---------------------------------------------------------------------------*/
void
vTestBase (void) {

  vLcdClear ();
  printf ("Hello World !\n");

  for (int i = 9; i >= 0; i--) {

    printf ("%d", i);
    delay_ms (500);
  }

  vLcdGotoXY (13, 0);
  printf ("Go!");
  delay_ms (500); // Pour voir le message complet...
}

/* -----------------------------------------------------------------------------
 | -----------------------------   Test Bargraph ----------------------------- |
 * ---------------------------------------------------------------------------*
 * Exemple d'utilisation des caractères personnalisés pour afficher un bargraph.
 * Les caractères personnalisés (motifs) sont passés à la fonction 
 * vBargraphInit() à l'aide d'un tableau d'octets qui doit être stocké en 
 * mémoire FLASH (PROGMEM), dans ce tableau:
 * - Chaque ligne correspond à un motif, un motif fait 8 pixels de haut sur
 *   5 pixels de large.
 * - Chaque octet de la ligne correspond donc à une ligne du motif, comme le
 *   motif fait 8 pixels de haut, il y a 8 octets par motif.
 * - Chaque bit de l'octet correspond à un pixel, la largeur du motif étant de
 *   5, les 3 bits de poids forts ne sont pas utilisés et toujours à zéro.
 */
static const PROGMEM uint8_t pucBarPatterns[] = {
  0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, /* Bloc 0% */
  0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, /* Bloc 20% */
  0x1F, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1F, /* Bloc 40% */
  0x1F, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1F, /* Bloc 60% */
  0x1F, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F, 0x1F, /* Bloc 80% */
  0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F  /* Bloc 100% */
};

// -----------------------------------------------------------------------------
void
vTestBargraph (void) {
  uint8_t ucValue, ucLen;

  for (ucLen = xLcdWidth(); ucLen >= 2; ucLen -= 2) {

    vLcdClear ();

    // Positionne le curseur afin de centrer le bargraph
    vLcdGotoXY ( (xLcdWidth() - ucLen) >> 1, 1);
    vLcdBargraphInit (pucBarPatterns);

    // Affiche Hello et les caractères blocs
    vLcdGotoXY (0, 0);
    fputs_P (PSTR ("Hello ! "), stdout);
    for (ucValue = 0; ucValue < 6; ucValue++) {
      putchar (ucValue);
    }

    for (ucValue = 0; ucValue < 255; ucValue++) {

      /* Valeur variant entre 0 et 255 sur une largeur de ucLen caractères */
      vLcdBargraph (ucValue, 255, ucLen);
      delay_ms (1); // Pour avoir le temps de voir la progression...
    }

    vLedToggle (LED_LED1);
    delay_ms (500); // Pour voir le bargraph complet...
  }
}

/* ========================================================================== */
