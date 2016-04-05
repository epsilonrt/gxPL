/*
 * eeprom_test.c
 * I2C EEPROM Test
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sysio/delay.h>
#include <sysio/i2c.h>

/* constants ================================================================ */
#define EE_SLAVE      0x50
#define EE_SIZE       4096  // 32 kbits
#define EE_PAGE_SIZE  32

/* private variables ======================================================== */
static int fd;

/* public variables ========================================================= */
open("/sys/bus/i2c/devices/0-0050/eeprom",O_RDWR)
lseek(fd,0x10,SEEK_SET); //set offset to 0x10

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
static void
vSigIntHandler (int sig) {

  if (iI2cClose (fd) != 0) {
    
    perror ("iI2cClose");
    exit (EXIT_FAILURE);
  }
  
  printf ("\neverything was closed.\nHave a nice day !\n");
  exit (EXIT_SUCCESS);
}

/* main ===================================================================== */
int
main (int argc, char **argv) {
  uint8_t ee_slvaddr = EE_SLAVE;
  unsigned int ee_size = EE_SIZE;
  unsigned int ee_pagesize = EE_PAGE_SIZE;
  
  uint8_t wbuf[EE_PAGE_SIZE];
  uint8_t rbuf[EE_PAGE_SIZE];
  uint16_t addr;

  fd = iI2cOpen ("/dev/i2c-0", ee_slvaddr);
  if (fd < 0) {

    perror ("Failed to open i2c ! ");
  }

  // vSigIntHandler() intercepte le CTRL+C
  signal (SIGINT, vSigIntHandler);
  printf ("Press Ctrl+C to abort ...\n\n");

  for (;;) {
    
    addr = 0;
    while (addr < )

    // Lecture d'un bloc de 9 octets
    if (iI2cReadBlock (fd, buf, 9) < 0) {

      perror ("Failed to read i2c ! ");
      continue;
    }

    // Extraction des données du bloc
    pred = (buf[0] << 8) + buf[1];
    status = buf[2];
    resistance = (buf[4] << 16) + (buf[5] << 8) + buf[6];
    tvoc = (buf[7] << 8) + buf[8];

    // Affichage
    switch (status) {
      case IAQ_OK:
        printf ("ok");
        break;
      case IAQ_BUSY:
        printf ("busy");
        break;
      case IAQ_RUNIN:
        printf ("warmup");
        break;
      case IAQ_ERROR:
        printf ("ERROR");
        break;
      default:
        printf ("Unknown status 0x%x", status);
        break;
    }
    printf (",%d,%d,%d\n", pred, resistance, tvoc);
    fflush (stdout);
    delay_ms (11000);
  }

  return 0;
}
/* ========================================================================== */
