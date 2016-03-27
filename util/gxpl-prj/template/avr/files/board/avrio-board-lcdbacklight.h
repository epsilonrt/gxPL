/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_BOARD_LCDBACKLIGHT_H_
#  define _AVRIO_BOARD_LCDBACKLIGHT_H_
/* ========================================================================== */

/* LCD_BACKLIGHT ============================================================ */
#  include <avrio/defs.h>
#  include <avr/io.h>

/* inline public functions ================================================== */
/*
 * Initialise la ressource matérielle (timer PWM de préférence) utilisée par
 * le module lcd (rétroéclairage).
 * A modifier en fonction du timer et de la sortie utilisée.
 */
static inline void
vLcdDriverBacklightInit (void) {

  DDRB |= _BV (7);  /* PB7 (OC0A) en sortie */
  /*
   * Mode PWM Phase correcte (1),
   * Clear OC on compare match
   * FCLK / 8
   * F = FCPU / (8 * 510) = 3.8 KHz pour 16 MHz
   */
  TCCR0A = 0b01100010;
}

/*
 * Modifie le niveau de rétroéclairage du LCD
 * @param ucValue niveau entre 0 et 63
 */
static inline void
vLcdDriverBacklightSet (uint8_t ucValue) {
  uint16_t usOcr = ucValue << 2; // x4

  OCR0A = usOcr;
}

/* ========================================================================== */
#endif /* _AVRIO_BOARD_LCDBACKLIGHT_H_ */
