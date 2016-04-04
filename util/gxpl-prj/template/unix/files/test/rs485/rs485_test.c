/*
 * sysio_demo_rs485.c
 * Transmet l'alphabet sur la liaison RS485 toutes les secondes
 *
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sysio/delay.h>
#include <sysio/serial.h>

/* constants ================================================================ */
#define BAUDRATE  38400
#define DATABITS  SERIAL_DATABIT_8
#define PARITY    SERIAL_PARITY_NONE
#define STOPBITS  SERIAL_STOPBIT_ONE
#define FLOW      SERIAL_FLOW_RS485_RTS_AFTER_SEND

/* private variables ======================================================== */
static int fd;
static const char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";

/* public variables ========================================================= */

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  /* Close the device when finished: */
  if (close (fd) < 0) {
    /* Error handling. See errno. */
    perror ("close");
  }
  else {
    printf ("\neverything was closed.\nHave a nice day !\n");
  }
  exit (EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {
  const char * cDev;
  xSerialIos xIosSet = { .baud = BAUDRATE, .dbits = DATABITS, .parity = PARITY,
                         .sbits = STOPBITS, .flow = FLOW
                       };

  if (argc < 2) {

    printf("Usage: %s serial_port [baud]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  cDev = argv[1];
  if (argc > 2) {

    xIosSet.baud = atoi(argv[2]);
  }


  /* Open your specific device : */
  if ( (fd = iSerialOpen (cDev, &xIosSet) ) < 0) {
    /* Error handling. See errno. */
    perror ("open");
    fprintf (stderr, "RS485 mode is probably not supported by this device !\n");
    exit (EXIT_FAILURE);
  }

  // vSigIntHandler() intercepte le CTRL+C
  signal (SIGINT, vSigIntHandler);

  printf ("Serial port %s (%s)\nPress Ctrl+C to abort ...\n", cDev, sSerialAttrStr(fd));
  for (;;) {

    if (write (fd, text, strlen (text) ) < 0) {
      /* Error handling. See errno. */
      perror ("write");
    }
    // Remplacer les lignes ci-dessous par votre code
    putchar ('.');
    fflush (stdout);
    delay_ms (1000);
  }

  return 0;
}
/* ========================================================================== */
