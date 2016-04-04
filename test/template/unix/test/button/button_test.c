/*
 * sysio_demo_button.c
 * Démo d'utilisation de dinput pour gérer des boutons poussoirs
 * Affiche les appuis et relachement de la façon suivante:
 * 000 - P: 0
 * 001 - R: 0
 * 002 - P: 1
 * 003 - R: 1
 * 004 - P: 2
 * 005 - R: 2
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
 * Description des broches d'entrée reliées aux boutons poussoir
 */
static const xDin xMyButtons[] = {
  { .num = GPIO22, .act = 0, .pull = ePullUp }, /* BP sur GPIO22 (Broche 15 GPIO), actif à l'état bas, résistance de pull-up activée... */
  { .num = GPIO23, .act = 0, .pull = ePullUp },
  { .num = GPIO24, .act = 0, .pull = ePullUp }
} ;

/* private variables ======================================================== */
/* Port pour gérer les boutons poussoir */
static xDinPort * xButtonPort;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
int
iCallback (unsigned uInputMask, unsigned uInputNumber, eDinEdge edge, void *udata) {
  static int counter;

  printf ("%03d - %c: %u\n", counter++, (edge == eEdgeRising) ? 'P' : 'R', uInputNumber);
  return 0;
}

// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  iDinClose (xButtonPort);
  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {

  // Ouverture du port
  xButtonPort = xDinOpen (xMyButtons, 3);
  if (xButtonPort == NULL) {

    printf ("Failed to open port !\n");
    return -1;
  }
  
  /*
   * Installation de la fonction iCallback() qui gère les appuis sur les boutons
   * Elle est appelée à chaque front montant et descendant (eEdgeBoth)
   */
  iDinSetGrpCallback (eEdgeBoth, iCallback, NULL, xButtonPort);

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
