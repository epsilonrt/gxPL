/*
 * eeprom_test.c
 * I2C EEPROM Test
 * Writing of the entire memory, page by page, with random byte sequences.
 * After each write page, a reading / verification is performed.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sysio/delay.h>
#include <sysio/i2c.h>

/* constants ================================================================ */
#define DEFAULT_I2C_BUS       "/dev/i2c-1"
#define DEFAULT_EE_MEM_SIZE   4096  // 32 kbits
#define DEFAULT_EE_PAGE_SIZE  32
#define DEFAULT_EE_FLAGS      I2CMEM_FLAG_ADDR16
#define DEFAULT_EE_I2CADDR    0x50

/* private functions ======================================================== */
/** ----------------------------------------------------------------------------
 * @brief Convert string to long integer
 * @param str C-string beginning with the representation of an integral number.
 * @param l pointer on converted integral number, if no valid conversion could 
 *  be performed, the pointed value is not changed.
 * @return 0, -1 if error occurs
 */
static int
iStrToLong (const char * str, long * l) {
  char *endptr;

  long t = strtol (str, &endptr, 0);
  if (endptr == str) {

    return -1;
  }

  *l = t;
  return 0;
}

/* main ===================================================================== */
int
main (int argc, char **argv) {
  int ret = 0;
  long n;
  bool bWriteProtect = false;
  const char * i2cbus = DEFAULT_I2C_BUS;
  int i2caddr = DEFAULT_EE_I2CADDR;
  uint32_t mem_size = DEFAULT_EE_MEM_SIZE;
  uint16_t page_size = DEFAULT_EE_PAGE_SIZE;
  uint8_t flags = DEFAULT_EE_FLAGS;
  uint8_t * wbuf, * rbuf;
  xI2cMem * m;

  // eeprom_test [i2cbus] [mem_size] [page_size] [offset_bits]
  // take the given parameters on the command line
  if (argc > 1) {
    i2cbus = argv[1];
  }
  if (argc > 2) {
    if (iStrToLong (argv[2], &n) == 0) {
      mem_size = n;
    }
  }
  if (argc > 3) {
    if (iStrToLong (argv[3], &n) == 0) {
      page_size = n;
    }
  }
  if (argc > 4) {
    if (iStrToLong (argv[4], &n) == 0) {
      if (n == 8) {
        flags &= I2CMEM_FLAG_ADDR16;
      }
    }
  }

  printf ("I2c EEPROM Test\n"
          "Bus:\t\t%s\n"
          "I2c Addr:\t0x%X\n"
          "Mem. size:\t%d bytes\n"
          "Page size:\t%d bytes\n"
          "Options:\t%s offset, %s\n"
          "<NOTE> The number of write cycles in an EEPROM memory is limited,\n"
          "it would be better to limit the use of this program ...\n"
          , i2cbus, i2caddr, mem_size, page_size,
          flags & I2CMEM_FLAG_ADDR16 ? "16-bits" : "8-bits",
          flags & I2CMEM_FLAG_READONLY ? "read-only" : "read-write"
         );

  m = xI2cMemOpen (i2cbus, i2caddr, mem_size, page_size, flags);
  if (m == NULL) {

    PERROR ("Failed to open memory (%d): %s", errno, strerror (errno) );
    exit (EXIT_FAILURE);
  }

  srand (time (NULL) ); // init. pseudorandom generator
  wbuf = malloc (page_size);
  assert (wbuf);
  rbuf = malloc (page_size);
  assert (rbuf);

  /*
   * Writing of the entire memory, page by page, with random byte sequences.
   * After each write page, a reading / verification is performed.
   */
  for (uint32_t offset = 0; offset < mem_size; offset += page_size) {

    if (bWriteProtect == false) {

      // Generating a random byte page
      for (int i = 0; i < page_size; i++) {

        double d = (double) rand() / RAND_MAX * 255; // 0 < d < 255

        wbuf[i] = (unsigned) d;
      }

      // Writing the page
      ret = iI2cMemWrite (m, offset, wbuf, page_size);

      if (ret != 0) {

        int bytes_read = iI2cMemRead (m, offset, rbuf, page_size);

        if (bytes_read == page_size) {

          if (bWriteProtect == false) {

            printf ("\n\n                      <<WARNING>>\n"
                        "It is likely that WP is active, switches to read-only mode....\n");
            bWriteProtect = true;
          }
        }
        else {

          PERROR ("write data fail(%d): %s", errno, strerror (errno) );
        }
      }
      else {

        // Writing successfully completed, reads the page ...
        ret = iI2cMemRead (m, offset, rbuf, page_size);

        if (ret != page_size) {

          PERROR ("read data fail(%d): %s", errno, strerror (errno) );
          break;
        }
        else {

          // check the read page
          if (memcmp (rbuf, wbuf, page_size) == 0) {

            putchar ('.');
          }
          else {

            PERROR ("bytes read are different from those that have been written");
            printf ("written->");

            for (int i = 0; i < page_size; i++) {
              printf ("%02X ", wbuf[i]);
            }
            printf ("\nreads  ->");
            for (int i = 0; i < page_size; i++) {
              printf ("%02X ", rbuf[i]);
            }
            putchar ('\n');
            break;
          }
        }
      }
    }

    if (bWriteProtect) {

      int bytes_read = iI2cMemRead (m, offset, rbuf, page_size);

      if (bytes_read == page_size) {
        
        for (int j = 0; j < page_size; j += 16) {
          
          printf ("[%04X] ", offset);
          for (int i = j; i < (j + 16); i++) {

            printf ("%02X ", rbuf[i]);
          }
          putchar ('\n');
        }
      }
      else {

        PERROR ("read data fail(%d): %s", errno, strerror (errno) );
        break;
      }
    }

    fflush (stdout);
  }

  if (iI2cMemClose (m) != 0) {

    PERROR ("iI2cMemClose failed (%d): %s", errno, strerror (errno) );
    ret = -1;
  }
  free (wbuf);
  free (rbuf);

  if (ret < 0) {

    exit (EXIT_FAILURE);
  }

  printf ("\nthe memory has been fully tested, %d bytes.\n"
          "everything was closed.\nHave a nice day !\n", mem_size);

  return 0;
}
/* ========================================================================== */
