/*
 * sysio_demo_xbee_coordinator.c
 * Démo coordinateur XBee
 * - Affiche l'identification des nouveaux noeuds
 * - Affiche le contenu des paquets de données reçus
 * - Transmet périodiquement un paquet de test en broadcast
 *
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sysio/xbee.h>
#include <sysio/serial.h>
#include <sysio/delay.h>

#include <assert.h>

/* constants ================================================================ */
#define DEFAULT_BAUDRATE  38400
#define TX_INTERVAL_DELAY  20

/* private variables ======================================================== */
static xXBee * xbee;
static const char * cDev;
static char cMyNi[21];
volatile int frame_id = 0;

/* private functions ======================================================== */
int iDataCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len);
int iTxStatusCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len);
int iNodeIdCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len);
int iLocalAtCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len);
void vPrintAddr (uint8_t * pucAddr, int iSize);
void vSigExitHandler (int sig);
void vSigAlarmHandler (int sig);

/* internal public functions ================================================ */
int
main (int argc, char **argv) {
  int ret;
  xSerialIos xIosSet = {
    .baud = DEFAULT_BAUDRATE, .dbits = SERIAL_DATABIT_8, .parity = SERIAL_PARITY_NONE,
    .sbits = SERIAL_STOPBIT_ONE, .flow = SERIAL_FLOW_RTSCTS, .flag = 0
  };

  if (argc < 2) {

    printf ("Usage: %s device device [baud]\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  cDev = argv[1];
  if (argc > 2) {

    xIosSet.baud = atoi (argv[2]);
  }

  /*
   * Init liaison série vers module XBee
   * Mode lecture/écriture, non bloquant, avec contrôle de flux matériel
   */
  if ( (xbee = xXBeeOpen (cDev, &xIosSet, XBEE_SERIES_S2)) == NULL) {

    perror ("xXBeeOpen failed !");
    exit (EXIT_FAILURE);
  }
  
  printf ("*** SysIO XBee Coordinator Test ***\n");

  /*
   * Init XBee, mise en place des gestionnaires de réception
   */
  vXBeeSetCB (xbee, XBEE_CB_DATA, iDataCB);
  vXBeeSetCB (xbee, XBEE_CB_TX_STATUS, iTxStatusCB);
  vXBeeSetCB (xbee, XBEE_CB_NODE_IDENT, iNodeIdCB);
  vXBeeSetCB (xbee, XBEE_CB_AT_LOCAL, iLocalAtCB);

  ret = iXBeeSendAt (xbee, XBEE_CMD_NODE_ID, NULL, 0);
  assert (ret >= 0);

  signal (SIGTERM, vSigExitHandler);
  signal (SIGINT,  vSigExitHandler);
  signal (SIGALRM, vSigAlarmHandler);
  alarm (1);

  for (;;) {

    // Scrute la réception des paquets
    ret = iXBeePoll (xbee, 10);
    assert (ret == 0);
  }

  return 0;
}

// -----------------------------------------------------------------------------
// Gestionnaire de réponse à une commande AT locale
int
iLocalAtCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {

  char * cCmd = pcXBeePktCommand (pkt);

  if (iXBeePktStatus (pkt) == XBEE_PKT_STATUS_OK) {

    if (strncmp (cCmd, XBEE_CMD_NODE_ID, 2) == 0) {
      // Ni String
      int ret = iXBeePktParamGetStr (cMyNi, pkt, 21);
      assert (ret >= 0);
      printf ("My Ni String is '%s'\n", cMyNi);
    }
  }
  else {
    // Erreur
    fprintf (stderr, "Local AT command %2s failed with 0x%02X status\n",
             cCmd, iXBeePktStatus (pkt));
  }
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
// Gestionnaire de réception des paquets de données
int
iDataCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  int size;

  size = iXBeePktDataLen (pkt);
  if (size > 0) {
    volatile char * p;
    uint8_t * src64 = pucXBeePktAddrSrc64 (pkt);

    // Affiche l'adresse 64-bits de la source et le contenu du paquet
    vPrintAddr (src64, 8);

    // Indique si le paquet est en broadcast
    if (iXBeePktIsBroadcast (pkt)) {

      putchar ('*');
    }

    // puis le contenu du paquet
    p = (char *) pucXBeePktData (pkt);
    p[size] = 0; // Ajout d'un caractère de fin de chaine
    printf (">\n%s\n", p);
  }

  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
// Gestionnaire de status de fin de transmission
int
iTxStatusCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {

  if (iXBeePktFrameId (pkt) == frame_id) {
    int status = iXBeePktStatus (pkt);

    if (status) {

      printf ("Tx%d Err. %d\n", frame_id, status);
    }
    else {

      printf ("Tx%d Ok\n", frame_id);
    }
    frame_id = 0;
  }
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
int
iNodeIdCB (xXBee *xbee, xXBeePkt *pkt, uint8_t len) {
  const char * str;

  printf ("A new node has joined the network:\n\taddr64: ");
  vPrintAddr (pucXBeePktAddrRemote64 (pkt), 8);

  printf ("\n\taddr16: ");
  vPrintAddr (pucXBeePktAddrRemote16 (pkt), 2);

  printf ("\n\tNi:'%s'\n", pcXBeePktNiString (pkt));

  switch (eXBeePktDeviceType (pkt)) {

    case XBEE_DEVICE_COORDINATOR:
      str = "Coordinator";
      break;
    case XBEE_DEVICE_ROUTER:
      str = "Router";
      break;
    case XBEE_DEVICE_END_DEVICE:
      str = "End device";
      break;
    default:
      str = "Unknown";
      break;
  }
  printf ("\tDevice Type: %s\n", str);
  vXBeeFreePkt (xbee, pkt);
  return 0;
}

// -----------------------------------------------------------------------------
void
vSigExitHandler (int sig) {

  iXBeeClose (xbee);
  printf ("\n%s closed.\nHave a nice day !\n", cDev);
  exit (EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
void
vSigAlarmHandler (int sig) {
  char message[33];
  static int iCount = 1;

  snprintf (message, 32, "%s #%d", cMyNi, iCount++);

  frame_id = iXBeeZbSendBroadcast (xbee, message, strlen (message));
  assert (frame_id >= 0);
  printf ("Tx%d>'%s'\n", frame_id, message);
  alarm (TX_INTERVAL_DELAY);
}

// -----------------------------------------------------------------------------
void
vPrintAddr (uint8_t * pucAddr, int iSize) {

  for (int i = 0; i < iSize; i++) {

    printf ("%02X", pucAddr[i]);
  }
}

/* ========================================================================== */
