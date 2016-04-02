/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_BOARD_LED_H_
#  define _AVRIO_BOARD_LED_H_
/* ========================================================================== */

/* LED ====================================================================== */
#  include <avrio/defs.h>
#  include <avr/io.h>

/* constants ================================================================ */
#  define LED_QUANTITY  8
#  define LED_LED0 _BV(0)
#  define LED_LED1 _BV(1)
#  define LED_LED2 _BV(2)
#  define LED_LED3 _BV(3)
#  define LED_LED4 _BV(4)
#  define LED_LED5 _BV(5)
#  define LED_LED6 _BV(6)
#  define LED_LED7 _BV(7)
#  define LED_ALL_LEDS (  LED_LED1 | LED_LED2 | \
                        LED_LED3 | LED_LED4 | \
                        LED_LED5 | LED_LED6 | \
                        LED_LED7 | LED_LED0 )
#  define LED_NO_LED (0)

/* types ==================================================================== */
typedef uint8_t xLedMask;

/* inline public functions ================================================== */
// ------------------------------------------------------------------------------
static inline void
vLedClear (xLedMask xMask) {

  PORTA &= ~(xMask & LED_ALL_LEDS);
}

// ------------------------------------------------------------------------------
static inline void
vLedInit (void) {

  DDRA |= LED_ALL_LEDS;
  vLedClear (LED_ALL_LEDS);
}

// ------------------------------------------------------------------------------
static inline void
vLedSet (xLedMask xMask) {

  PORTA |= (xMask & LED_ALL_LEDS);
}

// ------------------------------------------------------------------------------
static inline void
vLedToggle (xLedMask xMask) {

  PORTA ^= (xMask & LED_ALL_LEDS);
}

// ------------------------------------------------------------------------------
static inline void
vLedSetAll (xLedMask xMask) {

  PORTA = (xMask & LED_ALL_LEDS);
}

/* public variables ========================================================= */
#  if defined(LED_MASK_ARRAY_ENABLE)
#    define DECLARE_LED_MASK_ARRAY  \
  const xLedMask \
    xLedMaskArray [LED_QUANTITY] = { \
      LED_LED0, \
      LED_LED1, \
      LED_LED2, \
      LED_LED3, \
      LED_LED4, \
      LED_LED5, \
      LED_LED6, \
      LED_LED7  \
    }
#  else
#    define DECLARE_LED_MASK_ARRAY
#  endif

/* ========================================================================== */
#endif /* _AVRIO_BOARD_LED_H_ */
