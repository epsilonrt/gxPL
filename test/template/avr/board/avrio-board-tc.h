/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_BOARD_TC_H_
#define _AVRIO_BOARD_TC_H_
/* ========================================================================== */
#include <avrio/defs.h>
#include <avr/io.h>

/* constants ================================================================ */
#define TC_EOL SERIAL_CR
#define TC_RXBUFSIZE 64
#define TC_TXBUFSIZE 32
#define TC_RTSCTS_ENABLE
#define TC_RXTX_PULLUP_ENABLE
#define TC_NUMOF_PORT 2
//#define AVRIO_TC_FLAVOUR TC_FLAVOUR_IRQ

#define TC0_IO { \
  .dr   = &UDR0,   \
  .csra = &UCSR0A, \
  .csrb = &UCSR0B, \
  .csrc = &UCSR0C, \
  .brrl = &UBRR0L, \
  .brrh = &UBRR0H, \
  .rxd = { .port = &PORTE, .pin = 0 }, \
  .txd = { .port = &PORTE, .pin = 1 }, \
  .rts = { .port = &PORTG, .pin = 0 }, \
  .cts = { .port = &PORTG, .pin = 1 }, \
 }

#define TC1_IO { \
  .dr   = &UDR1,   \
  .csra = &UCSR1A, \
  .csrb = &UCSR1B, \
  .csrc = &UCSR1C, \
  .brrl = &UBRR1L, \
  .brrh = &UBRR1H, \
  .rxd = { .port = &PORTD, .pin = 2 }, \
  .txd = { .port = &PORTD, .pin = 3 }, \
  .rts = { .port = &PORTD, .pin = 4 }, \
  .cts = { .port = &PORTD, .pin = 5 }, \
  }

/* ========================================================================== */
#endif /* _AVRIO_BOARD_TC_H_ */
