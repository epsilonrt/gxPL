/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_CONFIG_H_
#define _AVRIO_CONFIG_H_
/* ========================================================================== */

/* 
 * Validation des modules
 * Pour dévalider un module, mettre le define correspondant en commentaire.
 * Si le module 'moduleX' est validé, un fichier avrio-board-moduleX.h doit être
 * présent dans le même répertoire que ce fichier de configuration.
 */
//#define AVRIO_CPU_FREQ F_CPU
//#define AVRIO_TASK_ENABLE
#define AVRIO_ADC_ENABLE
#define AVRIO_LED_ENABLE
#define AVRIO_BUTTON_ENABLE
#define AVRIO_LCD_ENABLE
//#define AVRIO_LCD_BACKLIGHT_ENABLE
#define AVRIO_KEYB_ENABLE
#define AVRIO_MELODY_ENABLE
#define AVRIO_SERIAL_ENABLE
//#define AVRIO_SERIAL_FLAVOUR SERIAL_FLAVOUR_IRQ
#define AVRIO_SERIAL_FLAVOUR SERIAL_FLAVOUR_POLL
//#define AVRIO_MODBUS_ENABLE
//#define AVRIO_PHONE_ENABLE
//#define AVRIO_BDCM_ENABLE
//#define AVRIO_ENCODER_ENABLE
#define AVRIO_CAN_ENABLE
#define AVRIO_TWI_ENABLE
#define AVRIO_SPI_ENABLE
#define AVRIO_WUSB_ENABLE
//#define AVRIO_TSL230_ENABLE
#ifndef AVRIO_SERVO_ENABLE
#define AVRIO_SERVO_ENABLE
#endif
//#define AVRIO_BATTERY_ENABLE
/* ========================================================================== */
#endif /* _AVRIO_CONFIG_H_ */
