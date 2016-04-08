/*
 * serial_test.c
 * Serial port test
 * - Transmits all the keys pressed on the serial port (no echo)
 * - Displays all characters received from the serial port
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sysio/delay.h>
#include <sysio/serial.h>

/* constants ================================================================ */
#define BAUDRATE  9600
#define DATABITS  SERIAL_DATABIT_8
#define PARITY    SERIAL_PARITY_NONE
#define STOPBITS  SERIAL_STOPBIT_ONE
#define FLOW      SERIAL_FLOW_NONE

/* private variables ======================================================== */
static FILE * serial_port;

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int c;
  int fd;
  struct termios ts, ts_save;
  const char * cDev;
  xSerialIos xIosSet = { .baud = BAUDRATE, .dbits = DATABITS, .parity = PARITY,
                         .sbits = STOPBITS, .flow = FLOW
                       };

  if (argc < 2) {

    printf ("Usage: %s serial_port [baud]\n", argv[0]);
    exit (EXIT_FAILURE);
  }
  cDev = argv[1];
  if (argc > 2) {

    xIosSet.baud = atoi (argv[2]);
  }

  if ( (fd = iSerialOpen (cDev, &xIosSet)) < 0) {

    perror ("open");
    exit (EXIT_FAILURE);
  }
  serial_port = fdopen (fd, "r+");

  // non-blocking, noecho for stdin
  tcgetattr (STDIN_FILENO, &ts);
  ts_save = ts; // save to restore
  ts.c_lflag &= ~ (ICANON | ECHO | ISIG);
  ts.c_cc[VMIN] = 0;
  ts.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSANOW, &ts);

  printf ("Serial port %s %s\nPress Ctrl+C to abort ...\n", cDev, sSerialAttrStr (fd));
  for (;;) {

    // serial port -> stdout
    c = fgetc (serial_port);
    if (c != EOF) {
      if (isprint (c) || (c == '\n') || (c == '\r')) {
        putchar (c);
      }
      else {
        // if not printable, print the hex code
        printf ("[%02X]", c);
      }
    }

    // stdin -> serial port
    c = getchar();
    if (c != EOF) {
      if (c == 3) {
        
        break; // CTRL + C
      }
      fputc (c, serial_port);
    }

    delay_ms (10);
  }
  
  // restore stdin settings
  tcsetattr (STDIN_FILENO, TCSANOW, &ts_save);
  
  /* Close the device when finished: */
  if (fclose (serial_port) < 0) {
    /* Error handling. See errno. */
    perror ("fclose");
    exit (EXIT_FAILURE);
  }
  
  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);

  return 0;
}
/* ========================================================================== */
