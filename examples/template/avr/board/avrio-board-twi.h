/*
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _AVRIO_BOARD_TWI_H_
#  define _AVRIO_BOARD_TWI_H_
/* ========================================================================== */

/* TWI ====================================================================== */
#  include <avrio/defs.h>
#  include <avr/io.h>

/* constants ================================================================ */
/*
 * @ingroup twiboard_module
 * @def TWI_MASTER_ENABLE
 * @brief Valide le module \ref twimaster_module.
 */
// #define TWI_MASTER_ENABLE

/*
 * @ingroup twiboard_module
 * @def TWI_SLAVE_ENABLE
 * @brief Valide le module \ref twislave_module.
 */
// #define TWI_SLAVE_ENABLE



/* inline public functions ================================================== */
/*
 * @ingroup twiboard_module
 * @brief Valide les résistances de tirage à l'état haut des lignes SDA et SCL
 *
 * Doit être réimplémentée par l'utilisateur dans le cas d'une carte 
 * personnalisée.
 */
static inline void
vTwiEnablePullup (void) {

  PORTD |= (_BV (0) | _BV (1));
}

/*-----------------------------Mode Maître------------------------------------*/
/* constants ================================================================ */
/*
 * @ingroup twimasterboard_module
 * @def TWI_MASTER_RXBUFSIZE
 * @brief Taille du buffer de réception en octets.
 */
#  define TWI_MASTER_RXBUFSIZE 32

/*
 * @ingroup twimasterboard_module
 * @def TWI_MASTER_TXBUFSIZE
 * @brief Taille du buffer de transmission en octets.
 */
#  define TWI_MASTER_TXBUFSIZE 32

/*
 * @ingroup twimasterboard_module
 * @def TWI_MASTER_CTRLBUFSIZE
 * @brief Taille du buffer de contrôle en octets.
 *
 * Le buffer de contrôle permet de transmettre les messages de contrôle à la 
 * routine d'interruption du module.\n
 * Chaque message de contrôle est constitué d'un identifiant de trame
 * (xTwiId), d'une adresse circuit (xTwiDeviceAddr) et d'un nombre d'octets 
 * (xTwiLength).
 */
#  define TWI_MASTER_CTRLBUFSIZE 15

/*
 * @ingroup twimasterboard_module
 * @def TWI_MASTER_STATUSBUFSIZE
 * @brief Taille du buffer d'état en octets.
 *
 * Le buffer d'état permet de récupérer les messages d'état émis par la 
 * routine d'interruption du module.\n
 * Chaque message d'état est constitué d'un identifiant de trame
 * (\ref xTwiId), d'un état (xTwiStatus) et d'un nombre d'octets (xTwiLength).
 */
#  define TWI_MASTER_STATUSBUFSIZE 15

/*
 * @ingroup twimasterboard_module
 * @def TWI_MASTER_ADDR_NACK_RETRY
 * @brief Nombre d'essais avant de déclarer un esclave absent du bus.
 *
 *  Permet en mode maître, de renvoyer un certain nombre de demandes à un
 *  esclave qui n'a pas répondu la première fois (esclave lent à la détente !).
 */
#  define TWI_MASTER_ADDR_NACK_RETRY 1

/*----------------------------Mode Esclave------------------------------------*/
/* constants ================================================================ */
/*
 * @ingroup twislaveboard_module
 * @def TWI_SLAVE_RXBUFSIZE
 * @brief Taille du buffer de réception en octets.
 */
#  define TWI_SLAVE_RXBUFSIZE 32

/*
 * @ingroup twislaveboard_module
 * @def TWI_SLAVE_TXBUFSIZE
 * @brief Taille du buffer de transmission en octets.
 */
#  define TWI_SLAVE_TXBUFSIZE 32

/* ========================================================================== */
#endif /* _AVRIO_BOARD_TWI_H_ */
