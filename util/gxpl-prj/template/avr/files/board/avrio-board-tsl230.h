/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_BOARD_TSL230_H_
#define _AVRIO_BOARD_TSL230_H_
/* ========================================================================== */
#include <avr/io.h>

/* TSL230 Sensor ============================================================ */

/* configuration ============================================================ */
#define TSL230_DEFAULT_SENSITIVITY eTsl230Sensitivity1
#define TSL230_DEFAULT_SCALE eTsl230ScaleDiv2
#define TSL230_DEFAULT_WINDOW 75
#define TSL230_DEFAULT_DARK_FREQ 0.4
#define TSL230_DEFAULT_RESPONSITIVITY 790.

#define TSL230_S0 0
#define TSL230_S1 1
#define TSL230_SENS_PORT  PORTB
#define TSL230_SENS_DDR   DDRB

#define TSL230_S2 2
#define TSL230_S3 3
#define TSL230_SCAL_PORT  PORTB
#define TSL230_SCAL_DDR   DDRB

#define TSL230_OE 4
#define TSL230_OE_PORT  PORTB
#define TSL230_OE_DDR   DDRB

/* inline public functions ================================================== */
#if defined(TSL230_OE) && defined(TSL230_OE_PORT) && defined(TSL230_OE_DDR)
#define TSL230_OE_ENABLE
#endif

// -----------------------------------------------------------------------------
INLINE void
vTsl230PinSetSensitivity (eTsl230Sensitivity eSensitivity) {

  if (eSensitivity & _BV (0)) {

    TSL230_SENS_PORT |= _BV (TSL230_S0);
  }
  else {

    TSL230_SENS_PORT &= ~_BV (TSL230_S0);
  }

  if (eSensitivity & _BV (1)) {

    TSL230_SENS_PORT |= _BV (TSL230_S1);
  }
  else {

    TSL230_SENS_PORT &= ~_BV (TSL230_S1);
  }
}

// -----------------------------------------------------------------------------
INLINE void
vTsl230PinSetScale (eTsl230Scale eScale) {

  if (eScale & _BV (0)) {

    TSL230_SCAL_PORT |= _BV (TSL230_S2);
  }
  else {

    TSL230_SCAL_PORT &= ~_BV (TSL230_S2);
  }
  if (eScale & _BV (1)) {

    TSL230_SCAL_PORT |= _BV (TSL230_S3);
  }
  else {

    TSL230_SCAL_PORT &= ~_BV (TSL230_S3);
  }
}

// -----------------------------------------------------------------------------
INLINE void
vTsl230Disable (void) {
#ifdef TSL230_OE_ENABLE
  TSL230_OE_PORT |= _BV (TSL230_OE);
#endif
}

// -----------------------------------------------------------------------------
INLINE void
vTsl230Enable (void) {
#ifdef TSL230_OE_ENABLE
  TSL230_OE_PORT &= ~_BV (TSL230_OE);
#endif
}

// -----------------------------------------------------------------------------
INLINE void
vTsl230PinInit (void) {

  TSL230_SENS_DDR |= _BV (TSL230_S1) | _BV (TSL230_S0);
  TSL230_SCAL_DDR |= _BV (TSL230_S3) | _BV (TSL230_S2);
#ifdef TSL230_OE_ENABLE
  TSL230_OE_DDR |= _BV (TSL230_OE);
#endif
}

// -----------------------------------------------------------------------------
INLINE void 
vTsl230CounterInit(void) {

  TCCR1A = 0;
  TCCR1B = 0;
}

// -----------------------------------------------------------------------------
INLINE void 
vTsl230CounterClear(void) {
  
  TCNT1 = 0;
}

// -----------------------------------------------------------------------------
INLINE void 
vTsl230CounterEnable(bool En) {
  
  if (En) {
    
    TCCR1B = 0x07; // Horloge T1 sur front montant
  }
  else {
    
    TCCR1B = 0; // Stop
  }
}

// -----------------------------------------------------------------------------
INLINE uint16_t 
usTsl230CounterRead(void) {
  
  return TCNT1;
}

/* ========================================================================== */
#endif /* _AVRIO_BOARD_TSL230_H_ */
