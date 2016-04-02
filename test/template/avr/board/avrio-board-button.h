/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_BOARD_BUTTON_H_
#  define _AVRIO_BOARD_BUTTON_H_
/* ========================================================================== */

/* BUTTON==================================================================== */
#  include <avrio/defs.h>
#  include <avr/io.h>

/* constants ================================================================ */
#  define BUTTON_QUANTITY  5
#  define BUTTON_BUTTON1 _BV(2)
#  define BUTTON_BUTTON2 _BV(4)
#  define BUTTON_BUTTON3 _BV(5)
#  define BUTTON_BUTTON4 _BV(6)
#  define BUTTON_BUTTON5 _BV(7)
#  define BUTTON_ALL_BUTTONS (  BUTTON_BUTTON1 | BUTTON_BUTTON2 | \
                              BUTTON_BUTTON3 | BUTTON_BUTTON4 | \
                              BUTTON_BUTTON5 )
#  define BUTTON_NO_BUTTON (0)

#  if !defined(PCICR) && defined(GIMSK)
#   define PCICR GIMSK
#  endif
/* types ==================================================================== */
typedef uint8_t xButMask;

/* inline public functions ================================================== */
// ------------------------------------------------------------------------------
static inline void
vButHardwareInit (void) {

  PORTE |= BUTTON_ALL_BUTTONS;
  DDRE &= ~BUTTON_ALL_BUTTONS;
}

// ------------------------------------------------------------------------------
static inline xButMask
xButHardwareGet (xButMask xMask) {

  return (PINE ^ BUTTON_ALL_BUTTONS) & xMask;
}

/* public variables ========================================================= */
#  if defined(BUTTON_MASK_ARRAY_ENABLE)
#    define DECLARE_BUTTON_MASK_ARRAY  \
  const xButMask \
    xButMaskArray [BUTTON_QUANTITY] = { \
      BUTTON_BUTTON1, \
      BUTTON_BUTTON2, \
      BUTTON_BUTTON3, \
      BUTTON_BUTTON4, \
      BUTTON_BUTTON5  \
    }
#  else
#    define DECLARE_BUTTON_MASK_ARRAY
#  endif

/* ========================================================================== */
#endif /* _AVRIO_BOARD_BUTTON_H_ */
