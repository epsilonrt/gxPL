/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_BOARD_SERVO_H_
#define _AVRIO_BOARD_SERVO_H_
/* ========================================================================== */
#include <avrio/defs.h>
#include <avr/io.h>

/* constants ================================================================ */
#define SERVO_PERIOD_US 20000UL
#define SERVO_CHANNELS 1

/* inline public functions ================================================== */
/**
 * @brief Initialise la ressource matérielle (timer)
 * A modifier en fonction du timer et de la sortie utilisée.
 * @param  Fréquence en Hertz du signal PWM
 * @return Valeur TOP correspondant à la fréquence
 */
static inline uint16_t
usServoTimerInit (void) {

  /* Broche OC1A (PB5)en sortie */
  DDRB |= _BV (5);

  /*
   * Réglage du timer utilisé pour générer un signal périodique de période max
   * 20 ms avec la meilleur résolution possible pour la durée:
   *
   * TIMER1 -> Mode 8 PWM Phase et fréquence correcte, TOP ICR1
   *  Period = 2 * N * TOP / F_CPU
   *  TOP = Period * F_CPU / (2 * N)
   * -- Calcul
   * F_CPU: 16 MHz
   * Si N = 1, TopMax = 65535, PeriodMax = 1 * 2 * 65535 / 16E6 = 8191 us
   * Si N = 8, TopMax = 65535, PeriodMax = 8 * 2 * 65535 / 16E6 = 65535 us
   * On prend donc N = 8, on règle  SERVO_PERIOD_US à 20000 us par exemple
   */
  uint16_t usTop = (SERVO_PERIOD_US * (F_CPU / 1000UL)) / (2000UL * 8);
  TCCR1B = 0;
  TCCR1A = 0x80;
  TCNT1 = 0;
  ICR1 = usTop;
  TCCR1B = 0x12;  /* N = 8 */

  return ICR1;
}

/**
 * @brief Réglage
 * @param ucChannel Canal du servo
 * @param usSetting Réglage  (min. = 0 pour r = 0 / max. = TOP pour r = 1)
 */
static inline void
vServoTimerSet (uint8_t ucChannel, uint16_t usSetting) {

  OCR1A = usSetting;
}

/**
 * @brief Lecture réglage servo
 * @param ucChannel Canal du servo
 */
static inline uint16_t
usServoTimerGet (uint8_t ucChannel) {

  return OCR1A;
}

/**
 * @brief Validation sortie
 * @param ucChannel Canal du servo
 */
static inline void
vServoTimerEnable (uint8_t ucChannel) {

  TCCR1A |= _BV (COM1A1); /* Clear OC1A on Compare Match */
}

/**
 * @brief Invalidation sortie
 * @param ucChannel Canal du servo
 */
static inline void
vServoTimerDisable (uint8_t ucChannel) {

  TCCR1A &= ~_BV (COM1A1); /* Disconnect OC1A */
}

/* ========================================================================== */
#endif /* _AVRIO_BOARD_SERVO_H_ */
