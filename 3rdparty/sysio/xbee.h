/**
 * @file xbee.h
 * @brief Maxstream XBee module Interface Header
 * 
 * Copyright © 2006-2008 Tymm Twillman <tymm@booyaka.com>
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#ifndef _SYSIO_XBEE_H_
#define _SYSIO_XBEE_H_

#include <sysio/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */
#include <errno.h>
#include <stdio.h>

/**
 *  @defgroup sysio_xbee XBee
 *  Ce module permet d'utiliser des modules XBee Série 1 et 2
 *
 *  Copyright © 2006-2008 Tymm Twillman <tymm@booyaka.com>
 *  @{
 */

/* constants ================================================================ */
/**
 * @defgroup xbee_commands Commandes XBee
 *
 * @{
 */

/*
 * Basic communication parameters/values
 */
#define XBEE_CMD_CHANNEL                "CH"
#define XBEE_CMD_PAN_ID                 "ID"
#define XBEE_CMD_DEST_ADDR64_HI         "DH"
#define XBEE_CMD_DEST_ADDR64_LO         "DL"
#define XBEE_CMD_SRC_ADDR16             "MY"
#define XBEE_CMD_SER_HI                 "SH"
#define XBEE_CMD_SER_LO                 "SL"
#define XBEE_CMD_RAND_DLY_SLOTS         "RN"
#define XBEE_CMD_MAC_MODE               "MM"
#define XBEE_CMD_COORD_ENA              "CE"
#define XBEE_CMD_SCAN                   "SC"
#define XBEE_CMD_SCAN_DURATION          "SD"
#define XBEE_CMD_ASSOC_END              "A1"
#define XBEE_CMD_ASSOC_COORD            "A2"
#define XBEE_CMD_ASSOC_STATUS           "AI"
#define XBEE_CMD_RSSI                   "DB"

/*
 * Transceiver Control
 */
#define XBEE_CMD_PWR_LEVEL              "PL"
#define XBEE_CMD_CCA_THRESH             "CA"

/*
 * Sleep Parameters
 */
#define XBEE_CMD_SLEEP_MODE             "SM"
#define XBEE_CMD_SLEEP_TIMEOUT          "ST"
#define XBEE_CMD_SLEEP_PERIOD           "SP"
#define XBEE_CMD_SLEEP_PERIOD_DISASSOC  "DP"

/*
 * Interface parameters
 */
#define XBEE_CMD_DATA_RATE              "BD"
#define XBEE_CMD_PACKETIZATION_TIMEOUT  "RO"
#define XBEE_CMD_DIO7_CONFIG            "D7"
#define XBEE_CMD_DIO6_CONFIG            "D6"
#define XBEE_CMD_DIO5_CONFIG            "D5"
#define XBEE_CMD_DIO4_CONFIG            "D4"
#define XBEE_CMD_DIO3_CONFIG            "D3"
#define XBEE_CMD_DIO2_CONFIG            "D2"
#define XBEE_CMD_DIO1_CONFIG            "D1"
#define XBEE_CMD_DIO0_CONFIG            "D0"
#define XBEE_CMD_PWM0_CONFIG            "PO"
#define XBEE_CMD_API_ENA                "AP"
#define XBEE_CMD_PULLUP_ENA             "PR"

/*
 * Version Info
 */
#define XBEE_CMD_VERS_FIRMWARE          "VR"
#define XBEE_CMD_VERS_HARDWARE          "HV"
#define XBEE_CMD_VERS_FIRM_VERBOSE      "VL"

/*
 * Received Signal Strength
 */
#define XBEE_CMD_RSSI_PWM_TIMER         "RP"
#define XBEE_CMD_RSS                    "DB"

/*
 * Error counters
 */
#define XBEE_CMD_CCA_FAILS              "EC"
#define XBEE_CMD_ACK_FAILS              "EA"

/*
 * AT Command Params
 */
#define XBEE_CMD_AT_MODE_TIMEOUT        "CT"
#define XBEE_CMD_AT_GUARD_TIME          "GT"
#define XBEE_CMD_AT_CMD_CHAR            "CC"
#define XBEE_CMD_AT_EXIT                "CN"

/*
 * XBEE specific routing
 */
#define XBEE_CMD_NODE_FIND_DEST         "DN"
#define XBEE_CMD_NODE_DISCOVER          "ND"
#define XBEE_CMD_NODE_ID                "NI"
#define XBEE_CMD_ACTIVE_SCAN            "AS"
#define XBEE_CMD_FORCE_DISASSOC         "DA"
#define XBEE_CMD_ENERGY_SCAN            "ED"
#define XBEE_CMD_FORCE_POLL             "FP"

/*
 * IO Line Passing / Sensor Interfacing
 */
#define XBEE_CMD_SAMPLE_RATE            "IR"
#define XBEE_CMD_SAMPLES_BEFORE_TX      "IT"

/*
 * Misc
 */
#define XBEE_CMD_WRITE_PARAMS           "WR"
#define XBEE_CMD_RESET_SOFT             "FR"
#define XBEE_CMD_APPLY_CHANGES          "AC"
#define XBEE_CMD_RESTORE_DEFAULTS       "RE"
#define XBEE_CMD_MAX_PAYLOAD            "NP"

/**
 * @}
 */

/* macros =================================================================== */
#ifndef __DOXYGEN__
struct xXBeePkt;
struct xXBee;
#define pvXBeeUserContext(xbee) ((xbee).user_context)

// ntohs, htons, ntohl, htonl, ntohll, htonll 
#include <arpa/inet.h>
// mutex
#include <pthread.h>

#ifndef htonll
/* htonll not defined ------------------------------------------------------- */
#if defined(SYSIO_OS_LINUX)
#include <byteswap.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define be64_to_cpu(val) bswap_64(val)
#elif BYTE_ORDER == BIG_ENDIAN
#define be64_to_cpu(val) (val)
#else
#error unknwon BYTE_ORDER
#endif

#elif defined(SYSIO_OS_APPLE)
#warning TODO:  be64_to_cpu(val) for darwin

#elif defined(SYSIO_OS_WIN32)
#warning TODO:  be64_to_cpu(val) for win32

#endif /* SYSIO_OS_WIN32 defined */

#define ntohll(x) be64_to_cpu(x)
#define htonll(x) ntohll(x)
/* htonll not defined ------------------------------------------------------- */
#endif

#else
/**
 * @brief Renvoie le pointeur contexte utilisateur
 *
 * Le contexte xbee dispose d'un pointeur void * permettant à l'utilisateur
 * d'y attacher des donnnées spécifiques. \n
 * Cette macro renvoie ce pointeur.
 *
 * @param xbee pointeur sur le contexte xbee initialisé
 * @return pointeur sur le contexte utilisateur
 */
#define pvXBeeUserContext(xbee)

#endif

/* types ==================================================================== */
/**
 * @brief Prototype d'un gestionnaire de réception
 *
 * @param xbee pointeur sur le contexte du module xbee initialisé
 * @param pkt pointeur sur le paquet reçu. Après utilisation et avant de sortir,
 *            le gestionnaire installé doit libérer la mémoire de ce paquet
 *            à l'aide de \ref vXBeeFreePkt
 * @param len taille en octets de pkt
 * @return Si il réussit un gestionnaire doit renvoyer 0, sinon c'est une erreur
 */
typedef int (*iXBeeRxCB) (struct xXBee *xbee, struct xXBeePkt *pkt, uint8_t len);

/**
 * @brief Type de gestionnaire de réception
 */
typedef enum {
  XBEE_CB_AT_LOCAL      = 0, /**< Réception d'une réponse à une commande AT locale */
  XBEE_CB_AT_REMOTE     = 1, /**< Réception d'une réponse à une commande AT distante */
  XBEE_CB_DATA          = 2, /**< Réception d'un paquet de données venant d'un autre noeud */
  XBEE_CB_IO            = 3, /**< Réception d'un paquet de IO venant d'un autre noeud */
  XBEE_CB_TX_STATUS     = 4, /**< Réception d'une réponse suite à une transmission */
  XBEE_CB_MODEM_STATUS  = 5, /**< Réception d'un paquet d'état modem */
  XBEE_CB_SENSOR        = 6, /**< Réception d'un paquet capteur (S2 seulement) */
  XBEE_CB_NODE_IDENT    = 7, /**< Réception d'un paquet d'identification de paquet */
  XBEE_CB_UNKNOWN       = -1,
  XBEE_CB_FIRST         = XBEE_CB_AT_LOCAL,
  XBEE_CB_LAST          = XBEE_CB_NODE_IDENT
} eXBeeCbType;

#ifndef __DOXYGEN__
// A modifier si eXBeeCbType modifié
#define XBEE_SIZEOF_CB 8
#endif

/**
 * @brief Types de paquets géré par le module
 *
 * Cette valeur est stockée dans le champ type de \ref xXBeePkt
 */
typedef enum {
  /* S1 & S2 Series */
  XBEE_PKT_TYPE_ATCMD             = 0x08,
  XBEE_PKT_TYPE_QATCMD            = 0x09, /* wait til an immed param or apply cmd */
  XBEE_PKT_TYPE_REMOTE_ATCMD      = 0x17,
  XBEE_PKT_TYPE_ATCMD_RESP        = 0x88,
  XBEE_PKT_TYPE_MODEM_STATUS      = 0x8a,
  XBEE_PKT_TYPE_REMOTE_ATCMD_RESP = 0x97,

  /* S1 Series */
  XBEE_PKT_TYPE_TX64              = 0x00,
  XBEE_PKT_TYPE_TX16              = 0x01,
  XBEE_PKT_TYPE_RX64              = 0x80,
  XBEE_PKT_TYPE_RX16              = 0x81,
  XBEE_PKT_TYPE_RX64_IO           = 0x82,
  XBEE_PKT_TYPE_RX16_IO           = 0x83,
  XBEE_PKT_TYPE_TX_STATUS         = 0x89,

  /* S2 Series */
  XBEE_PKT_TYPE_ZB_TX_REQ         = 0x10,
  XBEE_PKT_TYPE_ZB_CMD_FRAME      = 0x11, /* Not yet impl */
  XBEE_PKT_TYPE_ZB_TX_STATUS      = 0x8b,
  XBEE_PKT_TYPE_ZB_RX             = 0x90,
  XBEE_PKT_TYPE_ZB_RX_IO          = 0x92,
  XBEE_PKT_TYPE_ZB_RX_SENSOR      = 0x94,
  XBEE_PKT_TYPE_ZB_NODE_IDENT     = 0x95,
} eXBeePktType;

/**
 * @brief Réponses à une commande AT
 */
typedef enum {

  XBEE_PKT_STATUS_OK = 0,
  XBEE_PKT_STATUS_ERROR = 1,
  XBEE_PKT_STATUS_INVALID_COMMAND = 2,
  XBEE_PKT_STATUS_INVALID_PARAMETER = 3,
  XBEE_PKT_STATUS_TX_FAILURE = 4,
  XBEE_PKT_STATUS_UNKNOWN = -1
} eXBeeCmdStatus;

/**
 * @brief Status Modem
 */
typedef enum {

  XBEE_PKT_MODEM_HARDWARE_RESET = 0,
  XBEE_PKT_MODEM_WATCHDOG_TIMER_RESET = 1,
  XBEE_PKT_MODEM_JOINED_NETWORK = 2,
  XBEE_PKT_MODEM_DISASSOCIATED = 3,
  XBEE_PKT_MODEM_COORDINATOR_STARTED = 4,
  XBEE_PKT_MODEM_NETWORK_SECURITY_KEY_WAS_UPDATED = 7,
  XBEE_PKT_MODEM_VOLTAGE_SUPPLY_LIMIT_EXCEEDED = 0x0D,
  XBEE_PKT_MODEM_CONFIGURATION_CHANGED = 0x11,
  XBEE_PKT_MODEM_STACK_ERROR = 0x80
} eXBeeModemStatus;

/**
 * @brief Direction d'un échange XBee
 */
typedef enum {

  XBEE_RECV = 0x00, /**< En réception */
  XBEE_XMIT = 0x01  /**< En transmission */
} eXBeeDirection;

/**
 * @brief Serie du module XBee
 */
typedef enum {

  XBEE_SERIES_S1  = 1,
  XBEE_SERIES_S2  = 2,
  XBEE_SERIES_S2B = 3,
  XBEE_SERIES_UNKNOWN = -1
} eXBeeSeries;

/**
 * @brief Type de noeud
 */
typedef enum {

  XBEE_DEVICE_COORDINATOR = 0,
  XBEE_DEVICE_ROUTER  = 1,
  XBEE_DEVICE_END_DEVICE = 2,
  XBEE_DEVICE_UNKNOWN = -1
} eXBeeDeviceType;

/**
 * @brief Type de noeud
 */
typedef enum {

  XBEE_EVENT_BUTTON   = 1,
  XBEE_EVENT_JOIN     = 2,
  XBEE_EVENT_POWER_ON = 3,
  XBEE_EVENT_UNKNOWN  = -1
} eXBeeSourceEvent;

/* structures =============================================================== */
/**
 * @brief Entête de paquet
 *
 * Un paquet XBee commence toujours par cet entête
 */
typedef struct xXBeePktHdr {
  uint8_t         start; /**< Flag 0x7E */
  uint16_t        len;   /**< Taille du paquet, entête et CRC exclu */
} __attribute__ ( (__packed__)) xXBeePktHdr;

/**
 * @brief Paquet XBee générique
 *
 * Un paquet est constitué d'un entête, de données (payload) et d'un CRC
 */
typedef struct xXBeePkt {
  xXBeePktHdr  hdr;         /**< Entête */
  uint8_t         type;     /**< Type de paquet \ref eXBeePktType */
  uint8_t         data[0];  /**< Données du paquet (tableau de taille variable) */
} __attribute__ ( (__packed__)) xXBeePkt;

/**
 * @brief Contexte d'un module XBee
 *
 * Cette structure est opaque pour l'utilisateur
 */
#if defined(__DOXYGEN__)
typedef struct xXBee xXBee;
#else
typedef struct xXBee {
  struct {
    uint8_t bytes_left;
    uint8_t bytes_rcvd;
    xXBeePkt *packet;
    uint8_t hdr_data[sizeof (xXBeePktHdr)];
    iXBeeRxCB user_cb[XBEE_SIZEOF_CB];
  } __attribute__ ( (__packed__)) in;
  struct {
    uint8_t frame_id;
  } __attribute__ ( (__packed__)) out;
  int fd;
  eXBeeSeries series;
  void *user_context; // yours to pass data around with
  pthread_mutex_t mutex;
#ifdef XBEE_DEBUG
  int rx_crc_error, rx_error, rx_dropped;
  int tx_error, tx_dropped;
#endif
} __attribute__ ( (__packed__)) xXBee;
#endif


/* internal public functions ================================================ */
/**
 * @brief Initialise le contexte du module XBee
 *
 * Cette fonction doit être appellée avant toute utilisation du contexte xbee.
 * 
 * @param series Modèle du module utilisé
 * @param fd descripteur de fichier du port série ouvert en lecture-écriture
 *  connecté au module XBee, doit être ouvert en mode non-bloquant.
 * @return 0, -1 si erreur
 */
int iXBeeInit (xXBee *xbee, eXBeeSeries series, int fd);

/**
 * @brief Modifie un gestionnaire de réception
 *
 * Cette bibliothèque gère la réception de tous les types de paquets énumérés
 * par \ref eXBeePktType. Cette fonction permet à l'utilisateur d'installer
 * ces propres fonctions gérant chaque événement. \n
 * Si un paquet est reçu et qu'il n'y a pas de gestionnaire correspondant à son
 * type, il est tout simplement détruit.
 */
void vXBeeSetCB (xXBee *xbee, eXBeeCbType cb_type, iXBeeRxCB cb);

/**
 * @brief Scrute le flux relié au module en attente de réception d'octet
 *
 * Cette fonction doit être appellée dans la boucle principale aussi souvent que
 * possible. \n
 * @warning Il faut appeller cette fonction sinon aucune réception n'est possible.
 * @return 0, -1 si erreur
 */
int iXBeePoll (xXBee *xbee, int timeout);

/**
 * @brief Envoi une commande AT locale
 *
 * @param xbee pointeur sur le contexte
 * @param cmd commande en ASCII (2 lettres)
 * @param params paramètres de la commande (dépend de la commande)
 * @return le numéro de trame (valeur positive), une valeur négative si erreur
 */
int iXBeeSendAt (xXBee *xbee,
                 const char cmd[],
                 const uint8_t *params,
                 uint8_t param_len);

/**
 * @brief Envoi une commande AT à un module distant
 *
 * @warning Disponible uniquement sur module série 2 et série 1 récente
 *
 * @param xbee pointeur sur le contexte
 * @param cmd commande en ASCII (2 lettres)
 * @param params paramètres de la commande (dépend de la commande)
 * @param addr64 adresse 64-bit du destinataire
 * @param addr16 adresse 16-bit du destinataire
 * @param apply booléen qui indique si la commande doit ête appliquée immédiatement
 *        par le module distant (0 pas appliquée)
 * @return le numéro de trame (valeur positive), une valeur négative si erreur
 */
int iXBeeSendRemoteAt (xXBee *xbee,
                       const char cmd[],
                       const uint8_t params[],
                       uint8_t param_len,
                       const uint8_t addr64[8],
                       const uint8_t addr16[2],
                       uint8_t apply);


/**
 * @brief Envoi d'un paquet de données à un module distant
 *
 * @warning Uniquement pour les modules de série 2.

 * @param xbee pointeur sur le contexte
 * @param data pointeur sur les données
 * @param len longueur en octets des données
 * @param addr64 adresse 64-bit du destinataire
 * @param addr16 adresse 16-bit du destinataire
 * @param opt voir description du paquet de type 0x10 dans le datasheet
 * @param radius voir description du paquet de type 0x10 dans le datasheet
 * @return le numéro de trame (valeur positive), une valeur négative si erreur
 */
int iXBeeZbSend (xXBee *xbee,
                 const void *data,
                 uint8_t len,
                 const uint8_t addr64[8],
                 const uint8_t addr16[2],
                 uint8_t opt,
                 uint8_t radius);

/**
 * @brief Envoi d'un paquet de données au coordinateur du réseau
 *
 * @warning Uniquement pour les modules de série 2.

 * @param xbee pointeur sur le contexte
 * @param data pointeur sur les données
 * @param len longueur en octets des données
 * @return le numéro de trame (valeur positive), une valeur négative si erreur
 */
int iXBeeZbSendToCoordinator (xXBee *xbee, const void *data, uint8_t len);

/**
 * @brief Envoi d'un paquet de données de diffusion sur le réseau
 *
 * @warning Uniquement pour les modules de série 2.

 * @param xbee pointeur sur le contexte
 * @param data pointeur sur les données
 * @param len longueur en octets des données
 * @return le numéro de trame (valeur positive), une valeur négative si erreur
 */
int iXBeeZbSendBroadcast (xXBee *xbee, const void *data, uint8_t len);

/**
 * @brief Envoi d'un paquet de données à un module distant par son adresse 64-bit
 *
 * @warning Uniquement pour les modules de série 1.

 * @param xbee pointeur sur le contexte
 * @param data pointeur sur les données
 * @param len longueur en octets des données
 * @param addr adresse 64-bit du destinataire
 * @param opt voir description du paquet de type 0x10 dans le datasheet
 * @return le numéro de trame (valeur positive), une valeur négative si erreur
 */
int iXBeeSend64 (xXBee *xbee,
                 const void *data,
                 uint8_t len,
                 const uint8_t addr[8],
                 uint8_t opt);

/**
 * @brief Envoi d'un paquet de données à un module distant par son adresse 16-bit
 *
 * @warning Uniquement pour les modules de série 1.

 * @param xbee pointeur sur le contexte
 * @param data pointeur sur les données
 * @param len longueur en octets des données
 * @param addr adresse 64-bit du destinataire
 * @param opt voir description du paquet de type 0x10 dans le datasheet
 * @return le numéro de trame (valeur positive), une valeur négative si erreur
 */
int iXBeeSend16 (xXBee *xbee,
                 const void *data,
                 uint8_t len,
                 const uint8_t addr[2],
                 uint8_t opt);


#ifdef __DOXYGEN__
/**
 * @brief Alloue la mémoire pour un paquet
 *
 * Cette fonction est utilisée par la bibliothèque pour allouer de la mémoire. \n
 * Le système d'allocation mémoire peut être adapté en fonction des besoins. \n
 * Par défaut, AvrIO utilise malloc() pour l'allocation et ne tiens pas compte de
 * direction. Si l'utilisateur ne souhaite pas utiliser ce mécanisme, il devra
 * réimplémenter cette fonction ainsi que \ref vXBeeFreePkt(). \n
 * Il pourra alors utiliser le paramètre direction pour différencier le
 * mécansime d'allocation.
 *
 * @param xbee pointeur sur le contexte
 * @param direction permet de diférencier le mécanisme d'allocation conforme à \ref eXBeeDirection (inutilisé par défaut)
 * @param len taille en octet du buffer demandé
 * @return pointeur sur le buffer alloué
 */
void *pvXBeeAllocPkt (xXBee *xbee, uint8_t direction, uint8_t len);

/**
 * @brief Libère un paquet alloué avec pvXBeeAllocPkt()
 *
 * Par défaut, AvrIO utilise free(). Si l'utilisateur ne souhaite pas utiliser
 * ce mécanisme, il devra réimplémenter cette fonction ainsi
 * que \ref pvXBeeAllocPkt().
 */
void vXBeeFreePkt (xXBee *xbee, xXBeePkt *pkt);

#else
// weak permet à l'utilisateur de redéfinir ces fonctions...
void *pvXBeeAllocPkt (xXBee *xbee, uint8_t direction, uint8_t len) __attribute__ ( (weak));
void vXBeeFreePkt (xXBee *xbee, xXBeePkt *pkt) __attribute__ ( (weak));
#endif

/**
 * @brief Lecture du type de paquet
 *
 * Conforme à \ref eXBeePktType
 */
uint8_t ucXBeePktType (xXBeePkt *pkt);

/**
 * @brief Taille du paquet en octets
 *
 * Cette valeur correspond au champs length de la trame, cette taille est donc
 * comprise entre les champs length et crc exclus.
 */
uint16_t usXBeePktLength (xXBeePkt *pkt);

/**
 * @brief Adresse 64-bits source du paquet
 *
 * @param pkt pointeur sur le paquet
 * @return pointeur sur l'adresse (Big Endian) ou 0 si erreur
 */
uint8_t * pucXBeePktAddrSrc64 (xXBeePkt *pkt);

/**
 * @brief Adresse réseau 16-bits source du paquet
 *
 * @param pkt pointeur sur le paquet
 * @return pointeur sur l'adresse (Big Endian) ou 0 si erreur
 */
uint8_t * pucXBeePktAddrSrc16 (xXBeePkt *pkt);

/**
 * @brief Adresse réseau 16-bits destination du paquet (Série 2)
 *
 * @param pkt pointeur sur le paquet
 * @return pointeur sur l'adresse (Big Endian) ou 0 si erreur
 */
uint8_t * iXBeePktDst16 (xXBeePkt *pkt);

/**
 * @brief Pointeur sur les données du paquet
 *
 * La suite d'octets n'est pas terminée par un octet nul (mais par le CRC du
 * paquet qui pourra être écrasé par un octet nul)
 *
 * @param pkt pointeur sur le paquet
 * @return pointeur sur le premier octet ou 0 si erreur
 */
uint8_t * pucXBeePktData (xXBeePkt *pkt);

/**
 * @brief Nombre d'octet de données (CRC exclu)
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktDataLen (xXBeePkt *pkt);

/**
 * @brief Identifiant de paquet
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktFrameId (xXBeePkt *pkt);

/**
 * @brief Status de paquet
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktStatus (xXBeePkt *pkt);

/**
 * @brief Status de découverte de paquet (Série 2)
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktDiscovery (xXBeePkt *pkt);

/**
 * @brief Nombre de tentatives de paquet (Série 2)
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktRetry (xXBeePkt *pkt);

/**
 * @brief Nom de la commande AT
 *
 * Le nom a une longueur de 2 caractères et n'est pas terminé par un octet nul.
 * Il est donc nécessire d'utiliser strncmp() pour faire une comparaison !
 *
 * @param pkt pointeur sur le paquet
 * @return pointeur sur le premier octet ou 0 si erreur
 */
char * pcXBeePktCommand (xXBeePkt *pkt);

/**
 * @brief Paramètres de la commande AT
 *
 * @param pkt pointeur sur le paquet
 * @return pointeur sur le premier octet ou 0 si erreur
 */
uint8_t * pucXBeePktParam (xXBeePkt *pkt);

/**
 * @brief Copie les paramètres de commande AT
 * 
 * Le paquet transmis doit un paquet de réponse à une commande AT. Si la taille
 * de pcDest est suffisante, un caractère null est ajouté à la fin.
 * 
 * @param pcDest Destination de la copie
 * @param pkt pointeur sur le paquet
 * @param iDestSize Taille en caractères de la destination
 * @return Le nombre de caractères copiés, -1 si erreur
 */
int iXBeePktParamGetStr (char * pcDest, xXBeePkt *pkt, int iDestSize);

/**
 * @brief Copie d'un mot long des paramètres de commande AT
 * @param ulDest mot long résultat
 * @param pkt pointeur sur le paquet
 * @return 0, -1 sur erreur
 */
int iXBeePktParamGetULong (uint32_t * ulDest, xXBeePkt *pkt, int iOffset);

/**
 * @brief Copie d'un mot long des paramètres de commande AT
 * @param ulDest mot long résultat
 * @param pkt pointeur sur le paquet
 * @return 0, -1 sur erreur
 */
int iXBeePktParamGetULong (uint32_t * ulDest, xXBeePkt *pkt, int iOffset);

/**
 * @brief Copie d'un très long mot des paramètres de commande AT
 * @param ullDest très long mot résultat
 * @param pkt pointeur sur le paquet
 * @return 0, -1 sur erreur
 */
int iXBeePktParamGetULongLong (uint64_t * ullDest, xXBeePkt *pkt, int iOffset);

/**
 * @brief Copie d'un mot des paramètres de commande AT
 * @param usDest mot résultat
 * @param pkt pointeur sur le paquet
 * @return 0, -1 sur erreur
 */
int iXBeePktParamGetUShort (uint16_t * usDest, xXBeePkt *pkt, int iOffset);

/**
 * @brief Longueur des paramètres de la commande AT
 *
 * @param pkt pointeur sur le paquet
 * @return nombre d'octets de paramètre
 */
int iXBeePktParamLen (xXBeePkt *pkt);

/**
 * @brief Champs option du paquet
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktOptions (xXBeePkt *pkt);

/**
 * @brief Champs radius du paquet (Série 2)
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktRadius (xXBeePkt *pkt);

/**
 * @brief Champs apply du paquet
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktApply (xXBeePkt *pkt);

/**
 * @brief Champs rssi du paquet (Série 1)
 *
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktRssi (xXBeePkt *pkt);

/**
 * @brief Adresse 64-bits distante du paquet (Série 2)
 * 
 * @note champs présent dans un paquet d'identification de noeud
 *
 * @param pkt pointeur sur le paquet
 * @return pointeur sur l'adresse (Big Endian) ou 0 si erreur
 */
uint8_t * pucXBeePktAddrRemote64 (xXBeePkt *pkt);

/**
 * @brief Adresse réseau 16-bits distante du paquet (Série 2)
 *
 * @note champs présent dans un paquet d'identification de noeud
 * 
 * @param pkt pointeur sur le paquet
 * @return pointeur sur l'adresse (Big Endian) ou 0 si erreur
 */
uint8_t * pucXBeePktAddrRemote16 (xXBeePkt *pkt);

/**
 * @brief Identifiant texte du noeud (Série 2)
 * 
 * @note champs présent dans un paquet d'identification de noeud
 * 
 * @param pkt pointeur sur le paquet
 * @return 
 */
char * pcXBeePktNiString (xXBeePkt * pkt);

/**
 * @brief Adresse réseau 16-bits parent du paquet (Série 2)
 *
 * @note champs présent dans un paquet d'identification de noeud
 * 
 * @param pkt pointeur sur le paquet
 * @return pointeur sur l'adresse (Big Endian) ou 0 si erreur
 */
uint8_t * pucXBeePktAddrParent16 (xXBeePkt *pkt);

/**
 * @brief Type de noeud (Série 2)
 * 
 * @note champs présent dans un paquet d'identification de noeud
 * 
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
eXBeeDeviceType eXBeePktDeviceType (xXBeePkt * pkt);

/**
 * @brief Source de l'événement (Série 2)
 * 
 * @note champs présent dans un paquet d'identification de noeud
 * 
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
eXBeeSourceEvent eXBeePktSourceEvent (xXBeePkt * pkt);

/**
 * @brief Identifiant du profile Digi (Série 2)
 * 
 * @note champs présent dans un paquet d'identification de noeud
 * 
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktProfileId (xXBeePkt * pkt);

/**
 * @brief Identification du fabricant du module (Série 2)
 * 
 * @note champs présent dans un paquet d'identification de noeud
 * 
 * @param pkt pointeur sur le paquet
 * @return la valeur ou -1 si erreur
 */
int iXBeePktManufacturerId (xXBeePkt * pkt);


/**
 * @brief Indique si le paquet est un broadcast
 * 
 * @param pkt pointeur sur le paquet
 * @return true ou false, -1 si erreur
 */
int iXBeePktIsBroadcast (xXBeePkt *pkt);


/**
 * @brief Vérifie l'égalité de 2 adresses réseau de len octets
 */
bool bXBeePktAddressIsEqual (const uint8_t *a1, const uint8_t *a2, uint8_t len);

/**
 * @brief Adresse 16-bits inconnue (0xFFFE)
 *
 * Cette fonction simplifie l'utilisation de cette valeur pour l'appel des
 * fonctions de transmission ou de comparaison de la bibliothèque.
 */
const uint8_t * pucXBeeAddr16Unknown (void);

/**
 * @brief Adresse 64-bits inconnue (0xFFFFFFFFFFFFFFFF)
 *
 * Cette fonction simplifie l'utilisation de cette valeur pour l'appel des
 * fonctions de transmission ou de comparaison de la bibliothèque.
 */
const uint8_t * pucXBeeAddr64Unknown (void);

/**
 * @brief Adresse 64-bits du cordinateur Zigbee (0x0000000000000000)
 *
 * Cette fonction simplifie l'utilisation de cette valeur pour l'appel des
 * fonctions de transmission ou de comparaison de la bibliothèque.
 */
const uint8_t * pucXBeeAddr64Coordinator (void);

/**
 * @brief Adresse 64-bits de broadcast (0x000000000000FFFF)
 *
 * Cette fonction simplifie l'utilisation de cette valeur pour l'appel des
 * fonctions de transmission ou de comparaison de la bibliothèque.
 */
const uint8_t * pucXBeeAddr64Broadcast (void);

/*==============================================================================
 *
 * TODO: Partie non finalisée par manque de temps
 *
 *============================================================================*/
#ifndef __DOXYGEN__
int iXBeePktDigital (xXBeePkt * pkt, int array[9], unsigned int index);
int iXBeePktAnalog (xXBeePkt * pkt, int array[6], unsigned int index);
int iXBeePktSamples (xXBeePkt * pkt);

#endif /* __DOXYGEN__ not defined */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /*  _SYSIO_XBEE_H_ defined */
