/*
 * sysio_demo_switch.c
 * Démo d'utilisation de dinput pour gérer des micro-switchs
 * Affiche les basculements de switchs de la façon suivante:
 * 000 - SW: 0x1 (P0)
 * 001 - SW: 0x3 (P1)
 * 002 - SW: 0x7 (P2)
 * 003 - SW: 0xF (P3)
 *
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <sysio/delay.h>
#include <sysio/dinput.h>
#include <sysio/rpi.h>

/* constants ================================================================ */
/*
 * Description des broches reliées aux micro-switchs
 */
static const xDin xMySwitches[] = {
  { .num = GPIO7, .act = 0, .pull = ePullUp }, /* Switch0 (poids faible) sur GPIO7, actif à l'état bas, résistance de pull-up activée... */
  { .num = GPIO10, .act = 0, .pull = ePullUp },
  { .num = GPIO9, .act = 0, .pull = ePullUp },
  { .num = GPIO11, .act = 0, .pull = ePullUp }
} ;

/* private variables ======================================================== */
/* Port pour gérer les micro-switchs */
static xDinPort * xSwitchPort;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
// Fonction appelée à chaque changement de micro-switch
int
iCallback (unsigned uInputMask, unsigned uInputNumber, eDinEdge edge, void *udata) {
  static int counter;

  printf ("%03d - SW: 0x%X (%c%d)\n", counter++, uInputMask,
    (edge == eEdgeRising) ? 'P' : 'R', uInputNumber);
  return 0;
}

// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  iDinClose (xSwitchPort);
  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {

  // Ouverture du port
  xSwitchPort = xDinOpen (xMySwitches, 4);
  if (xSwitchPort == NULL) {

    printf ("Failed to open port !\n");
    return -1;
  }

  /*
   * Installation de la fonction iCallback() qui gère les basculement de switchs
   * Elle est appelée à chaque front montant et descendant (eEdgeBoth)
   */
  iDinSetGrpCallback (eEdgeBoth, iCallback, NULL, xSwitchPort);

  // vSigIntHandler() intercepte le CTRL+C
  signal (SIGINT, vSigIntHandler);

  printf ("Press Ctrl+C to abort ...\n");
  for (;;) {

    /*
     *  Rien à faire qu'à attendre indéfiniment les appuis (gérés par un thread
     *  installé par iDinSetGrpCallback())
     */
    sleep (-1);
  }

  return 0;
}
/* ========================================================================== */
