/*
 * sysio/posix/serial.c
 * @brief Liaison série (Implémentation)
 * Copyright © 2014 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sysio/serial.h>
#include <sysio/delay.h>
#include <sysio/log.h>


#ifdef ARCH_ARM_RASPBERRYPI
// Le code ci-dessous est pour le Raspberry Pi
/* macros =================================================================== */
#define F_CPU 700000000UL // Raspberry Pi System Clock: 700 MHz
#define BAUDRATE(baudrate_reg) (unsigned long)(F_CPU/(8UL*(baudrate_reg+1UL)))

/* constants ================================================================ */
#define MAX_BAUDRATE BAUDRATE(0)
#define MIN_BAUDRATE BAUDRATE(0xFFFF)

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
static int
iCheckBaudrate (unsigned long ulBaud) {

  if ( (ulBaud < MIN_BAUDRATE) || (ulBaud > MAX_BAUDRATE)) {

    vLog (LOG_ERR, "Baudrate: %lu Bd out of range {%lu,%lu}",
          ulBaud, MIN_BAUDRATE, MAX_BAUDRATE);
    return -1;
  }
  return 0;
}
#else
#define iCheckBaudrate(b) (0)
#endif

/* public variables ========================================================= */
extern const char sUnknown[];

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
int
iSerialSetAttr (int fd, const xSerialIos * xIos) {
  struct termios ts;
  int iRet;

  if ( (iRet = tcgetattr (fd, &ts)) == 0) {

    iRet = iSerialTermiosSetAttr (&ts, xIos);
    if (iRet == 0) {
      tcflush (fd, TCIOFLUSH);
      return tcsetattr (fd, TCSANOW, &ts);
    }
  }
  return iRet;
}

// -----------------------------------------------------------------------------
int
iSerialGetAttr (int fd, xSerialIos * xIos) {
  struct termios ts;
  int iRet;

  if ( (iRet = tcgetattr (fd, &ts)) == 0) {

    return iSerialTermiosGetAttr (&ts, xIos);
  }
  return iRet;

}

// -----------------------------------------------------------------------------
int
iSerialOpen (const char *device, xSerialIos * xIos) {
  struct termios ts;
  int fd, iRet;

  if ( (fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY)) == -1) {
    return -1;
  }

  fcntl (fd, F_SETFL, O_RDWR);

  // Get and modify current options:
  tcgetattr (fd, &ts);

  cfmakeraw (&ts);
  ts.c_cflag |= (CLOCAL | CREAD);
  ts.c_lflag &= ~ (ICANON | ECHO | ECHOE | ISIG);
  ts.c_oflag &= ~OPOST;

  if ( (iRet = iSerialTermiosSetAttr (&ts, xIos)) < 0) {

    close (fd);
    return iRet;
  }

  // read() should never block
  ts.c_cc [VMIN]  = 0;
  ts.c_cc [VTIME] = 0;

  tcflush (fd, TCIOFLUSH);
  tcsetattr (fd, TCSANOW, &ts);

  ioctl (fd, TIOCMGET, &iRet);
  iRet |= TIOCM_DTR;
  iRet |= TIOCM_RTS;
  ioctl (fd, TIOCMSET, &iRet);

  delay_ms (10);

  return fd;
}

// -----------------------------------------------------------------------------
int
iSerialPoll (int fd, int timeout_ms) {
  int ret;
  fd_set set;
  struct timeval timeout;
  long timeout_us = timeout_ms * 1000L;

  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (fd, &set);

  /* Initialize the timeout data structure. */
  timeout.tv_sec  = timeout_us / 1000000L;
  timeout.tv_usec = timeout_us % 1000000L;

  /* select returns 0 if timeout, 1 if input available, -1 if error. */
  ret = select (FD_SETSIZE, &set, NULL, NULL, &timeout);
  if (ret == -1) {
    if (errno != EINTR) {
      PERROR ("failed to poll serial port: %s", strerror (errno));
    }
    else {
      ret = 0;
    }
  }
  else if ( (ret > 0) && (FD_ISSET (fd, &set))) {
    int available_data;
    
    ret = ioctl (fd, FIONREAD, &available_data);
    if (ret == 0) {
      ret = available_data;
    }
  }

  return ret;
}

// -----------------------------------------------------------------------------
bool
bSerialFdIsValid (int fd) {
  struct termios ts;
  
  return (tcgetattr (fd, &ts) == 0) ? true : false;
}

// -----------------------------------------------------------------------------
void
vSerialFlush (int fd) {

  tcflush (fd, TCIOFLUSH);
}

// -----------------------------------------------------------------------------
void
vSerialClose (int fd) {

  close (fd);
}

// -----------------------------------------------------------------------------
int
iSerialDataAvailable (int fd) {
  int iDataAvailable;

  if (ioctl (fd, FIONREAD, &iDataAvailable) == -1) {
    return -1;
  }
  return iDataAvailable;
}

// -----------------------------------------------------------------------------
int
iSerialGetBaudrate (int fd) {
  struct termios ts;

  if (tcgetattr (fd, &ts) == 0) {

    return iSerialTermiosGetBaudrate (&ts);
  }
  return -1;
}

// -----------------------------------------------------------------------------
eSerialDataBits
eSerialGetDataBits (int fd) {
  struct termios ts;

  if (tcgetattr (fd, &ts) == 0) {

    return iSerialTermiosGetDataBits (&ts);
  }
  return -1;
}

// -----------------------------------------------------------------------------
eSerialStopBits
eSerialGetStopBits (int fd) {
  struct termios ts;

  if (tcgetattr (fd, &ts) == 0) {

    return iSerialTermiosGetStopBits (&ts);
  }
  return -1;
}

// -----------------------------------------------------------------------------
eSerialParity
eSerialGetParity (int fd) {
  struct termios ts;

  if (tcgetattr (fd, &ts) == 0) {

    return iSerialTermiosGetParity (&ts);
  }
  return -1;
}


// -----------------------------------------------------------------------------
eSerialFlow
eSerialGetFlow (int fd) {
  struct termios ts;

  if (tcgetattr (fd, &ts) == 0) {

    return iSerialTermiosGetFlow (&ts);
  }
  return -1;
}



// -----------------------------------------------------------------------------
int
iSerialSetBaudrate (int fd, int iBaudrate) {
  struct termios ts;
  int iRet;

  if ( (iRet = tcgetattr (fd, &ts)) == 0) {

    iRet = iSerialTermiosSetBaudrate (&ts, iBaudrate);
    if (iRet == 0) {
      tcflush (fd, TCIOFLUSH);
      return tcsetattr (fd, TCSANOW, &ts);
    }
  }
  return iRet;
}

// -----------------------------------------------------------------------------
int
iSerialSetDataBits (int fd, eSerialDataBits eDataBits) {
  struct termios ts;
  int iRet;

  if ( (iRet = tcgetattr (fd, &ts)) == 0) {

    iRet = iSerialTermiosSetDataBits (&ts, eDataBits);
    if (iRet == 0) {
      tcflush (fd, TCIOFLUSH);
      return tcsetattr (fd, TCSANOW, &ts);
    }
  }
  return iRet;
}

// -----------------------------------------------------------------------------
int
iSerialSetStopBits (int fd, eSerialStopBits eStopBits) {
  struct termios ts;
  int iRet;

  if ( (iRet = tcgetattr (fd, &ts)) == 0) {

    iRet = iSerialTermiosSetStopBits (&ts, eStopBits);
    if (iRet == 0) {
      tcflush (fd, TCIOFLUSH);
      return tcsetattr (fd, TCSANOW, &ts);
    }
  }
  return iRet;
}

// -----------------------------------------------------------------------------
int
iSerialSetParity (int fd, eSerialParity eParity) {
  struct termios ts;
  int iRet;

  if ( (iRet = tcgetattr (fd, &ts)) == 0) {

    iRet = iSerialTermiosSetParity (&ts, eParity);
    if (iRet == 0) {
      tcflush (fd, TCIOFLUSH);
      return tcsetattr (fd, TCSANOW, &ts);
    }
  }
  return iRet;
}

// -----------------------------------------------------------------------------
int
iSerialSetFlow (int fd, eSerialFlow eFlow) {
  struct termios ts;
  int iRet;

  if ( (iRet = tcgetattr (fd, &ts)) == 0) {

    iRet = iSerialTermiosSetFlow (&ts, eFlow);
    if (iRet == 0) {
      tcflush (fd, TCIOFLUSH);
      return tcsetattr (fd, TCSANOW, &ts);
    }
  }
  return iRet;
}


// -----------------------------------------------------------------------------
const char *
sSerialGetFlowStr (int fd) {

  return sSerialFlowToStr (eSerialGetFlow (fd));
}

// -----------------------------------------------------------------------------
const char *
sSerialAttrStr (int fd) {
  struct termios ts;

  if (tcgetattr (fd, &ts) == 0) {

    return sSerialTermiosToStr (&ts);
  }
  return NULL;
}

// -----------------------------------------------------------------------------
double
dSerialFrameDuration (int fd, size_t ulSize) {
  struct termios ts;

  if (tcgetattr (fd, &ts) == 0) {

    return dSerialTermiosFrameDuration (&ts, ulSize);
  }
  return -1;
}

// -----------------------------------------------------------------------------
//                                TermIos                                     //
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
int
iSerialTermiosSetAttr (struct termios * ts, const xSerialIos * xIos) {
  int iRet = -1;

  if (ts) {
    iRet = iSerialTermiosSetBaudrate (ts, xIos->baud);
    if (iRet == 0) {
      iRet = iSerialTermiosSetDataBits (ts, xIos->dbits);
      if (iRet == 0) {
        iRet = iSerialTermiosSetParity (ts, xIos->parity);
        if (iRet == 0) {
          iRet = iSerialTermiosSetStopBits (ts, xIos->sbits);
          if (iRet == 0) {
            return iSerialTermiosSetFlow (ts, xIos->flow);
          }
        }
      }
    }
  }
  return iRet;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosGetAttr (const struct termios * ts, xSerialIos * xIos) {
  int iRet = -1;

  if (ts) {
    iRet = iSerialTermiosGetBaudrate (ts);
    if (iRet >= 0) {
      xIos->baud = iRet;
      iRet = iSerialTermiosGetDataBits (ts);
      if (iRet >= 0) {
        xIos->dbits = iRet;
        iRet = iSerialTermiosGetParity (ts);
        if (iRet >= 0) {
          xIos->parity = iRet;
          iRet = iSerialTermiosGetStopBits (ts);
          if (iRet >= 0) {
            xIos->sbits = iRet;
            iRet = iSerialTermiosGetFlow (ts);
            if (iRet >= 0) {
              xIos->flow = iRet;
              return 0;
            }
          }
        }
      }
    }
  }
  return iRet;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosSetBaudrate (struct termios * ts, int iBaudrate) {

  if (ts) {
    speed_t s = eSerialIntToSpeed (iBaudrate);

    if (s != -1) {

      cfsetispeed (ts, s);
      cfsetospeed (ts, s);
      return 0;
    }
  }
  return EBADBAUD;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosSetDataBits (struct termios * ts, eSerialDataBits eDataBits) {

  if (ts) {

    switch (eDataBits) {

      case SERIAL_DATABIT_5:
        ts->c_cflag &= ~CSIZE;
        ts->c_cflag |= CS5;
        break;
      case SERIAL_DATABIT_6:
        ts->c_cflag &= ~CSIZE;
        ts->c_cflag |= CS6;
        break;
      case SERIAL_DATABIT_7:
        ts->c_cflag &= ~CSIZE;
        ts->c_cflag |= CS7;
        break;
      case SERIAL_DATABIT_8:
        ts->c_cflag &= ~CSIZE;
        ts->c_cflag |= CS8;
        break;
      default:
        return -1;
    }
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosSetStopBits (struct termios * ts, eSerialStopBits eStopBits) {

  if (ts) {

    switch (eStopBits) {

      case SERIAL_STOPBIT_ONE:
        ts->c_cflag &= ~CSTOPB;
        break;
      case SERIAL_STOPBIT_TWO:
        ts->c_cflag |= CSTOPB;
        break;
      default:
        return -1;
    }
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosSetParity (struct termios * ts, eSerialParity eParity) {

  if (ts) {

    switch (eParity) {

      case SERIAL_PARITY_NONE:
        ts->c_cflag &= ~ (PARENB | PARODD);
        break;
      case SERIAL_PARITY_EVEN:
        ts->c_cflag &= ~ PARODD;
        ts->c_cflag |= PARENB;
        break;
      case SERIAL_PARITY_ODD:
        ts->c_cflag |= PARENB | PARODD;
        break;
      default:
        return -1;
    }
    if (eParity == SERIAL_PARITY_NONE) {

      ts->c_iflag &= ~INPCK;
    }
    else {

      ts->c_iflag |= INPCK;
    }
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosSetFlow (struct termios * ts, eSerialFlow eFlow) {

  if (ts) {

    switch (eFlow) {

      case SERIAL_FLOW_NONE:
        ts->c_cflag &= ~CRTSCTS;
        ts->c_iflag &= ~ (IXON | IXOFF | IXANY);
        break;
      case SERIAL_FLOW_RTSCTS:
        ts->c_cflag |= CRTSCTS;
        ts->c_iflag &= ~ (IXON | IXOFF | IXANY);
        break;
      case SERIAL_FLOW_XONXOFF:
        ts->c_iflag |= IXON | IXOFF | IXANY;
        ts->c_cflag &= ~CRTSCTS;
        break;
      default:
        return -1;
    }
    return 0;
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosGetBaudrate (const struct termios * ts) {

  if (ts) {
    speed_t baud = cfgetospeed (ts);

    return iSerialSpeedToInt (baud);
  }
  return EBADBAUD;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosGetDataBits (const struct termios * ts) {

  if (ts) {
    switch (ts->c_cflag & CSIZE) {
      case CS5:
        return SERIAL_DATABIT_5;
      case CS6:
        return SERIAL_DATABIT_6;
      case CS7:
        return SERIAL_DATABIT_7;
      case CS8:
        return SERIAL_DATABIT_8;
      default:
        break;
    }
  }
  return SERIAL_DATABIT_UNKNOWN;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosGetStopBits (const struct termios * ts) {

  if (ts) {
    if (ts->c_cflag & CSTOPB) {
      return SERIAL_STOPBIT_TWO;
    }
    return SERIAL_STOPBIT_ONE;
  }
  return SERIAL_STOPBIT_UNKNOWN;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosGetParity (const struct termios * ts) {

  if (ts) {
    switch (ts->c_cflag & (PARENB | PARODD)) {
      case PARENB:
        return SERIAL_PARITY_EVEN;
      case PARENB | PARODD:
        return SERIAL_PARITY_ODD;
      default:
        break;
    }
    return SERIAL_PARITY_NONE;
  }
  return SERIAL_PARITY_UNKNOWN;
}

// -----------------------------------------------------------------------------
int
iSerialTermiosGetFlow (const struct termios * ts) {

  if (ts) {
    if (ts->c_cflag & CRTSCTS) {
      return SERIAL_FLOW_RTSCTS;
    }
    if (ts->c_iflag & (IXON | IXOFF | IXANY)) {
      return SERIAL_FLOW_XONXOFF;
    }
    return SERIAL_FLOW_NONE;
  }
  return SERIAL_FLOW_UNKNOWN;
}

// -----------------------------------------------------------------------------
const char *
sSerialTermiosToStr (const struct termios * ts) {
  // BBBBBBBB-DPSF

  if (ts) {
    xSerialIos ios;
    ios.baud = iSerialTermiosGetBaudrate (ts);
    ios.dbits = iSerialTermiosGetDataBits (ts);
    ios.parity = iSerialTermiosGetParity (ts);
    ios.sbits = iSerialTermiosGetStopBits (ts);
    ios.flow = iSerialTermiosGetFlow (ts);
    return sSerialAttrToStr (&ios);
  }
  return sUnknown;
}


// -----------------------------------------------------------------------------
double
dSerialTermiosFrameDuration (const struct termios * ts, size_t ulSize) {
  if (ts) {
    unsigned uByteWidth;

    uByteWidth = 1 + iSerialTermiosGetDataBits (ts) +
                 (iSerialTermiosGetParity (ts) == SERIAL_PARITY_NONE ? 0 : 1) +
                 iSerialTermiosGetStopBits (ts);

    return (double) ulSize * (double) uByteWidth /
           (double) iSerialTermiosGetBaudrate (ts);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iSerialSpeedToInt (speed_t speed) {
  int baud;

  switch (speed) {
    case B0:
      baud = 0;
      break;
    case B50:
      baud = 50;
      break;
    case B75:
      baud = 75;
      break;
    case B110:
      baud = 110;
      break;
    case B134:
      baud = 134;
      break;
    case B150:
      baud = 150;
      break;
    case B200:
      baud = 200;
      break;
    case B300:
      baud = 300;
      break;
    case B600:
      baud = 600;
      break;
    case B1200:
      baud = 1200;
      break;
    case B1800:
      baud = 1800;
      break;
    case B2400:
      baud = 2400;
      break;
    case B4800:
      baud = 4800;
      break;
    case B9600:
      baud = 9600;
      break;
    case B19200:
      baud = 19200;
      break;
    case B38400:
      baud = 38400;
      break;
    case   B57600:
      baud = 57600;
      break;
    case  B115200:
      baud = 115200;
      break;
    case  B230400:
      baud = 230400;
      break;
    case  B460800:
      baud = 460800;
      break;
    case  B500000:
      baud = 500000;
      break;
    case  B576000:
      baud = 576000;
      break;
    case  B921600:
      baud = 921600;
      break;
    case B1000000:
      baud = 1000000;
      break;
    case B1152000:
      baud = 1152000;
      break;
    case B1500000:
      baud = 1500000;
      break;
    case B2000000:
      baud = 2000000;
      break;
    case B2500000:
      baud = 2500000;
      break;
    case B3000000:
      baud = 3000000;
      break;
    case B3500000:
      baud = 3500000;
      break;
    case B4000000:
      baud = 4000000;
      break;
    default:
      baud = -1;
      break;
  }
  return baud;
}

// -----------------------------------------------------------------------------
speed_t
eSerialIntToSpeed (int baud) {
  speed_t speed = iCheckBaudrate (baud);

  if (speed == 0) {

    switch (baud) {
      case 0:
        speed = B0;
        break;
      case 50:
        speed = B50;
        break;
      case 75:
        speed = B75;
        break;
      case 110:
        speed = B110;
        break;
      case 134:
        speed = B134;
        break;
      case 150:
        speed = B150;
        break;
      case 200:
        speed = B200;
        break;
      case 300:
        speed = B300;
        break;
      case 600:
        speed = B600;
        break;
      case 1200:
        speed = B1200;
        break;
      case 1800:
        speed = B1800;
        break;
      case 2400:
        speed = B2400;
        break;
      case 4800:
        speed = B4800;
        break;
      case 9600:
        speed = B9600;
        break;
      case 19200:
        speed = B19200;
        break;
      case 38400:
        speed = B38400;
        break;
      case 57600:
        speed = B57600;
        break;
      case 115200:
        speed = B115200;
        break;
      case 230400:
        speed = B230400;
        break;
      case 460800:
        speed = B460800;
        break;
      case 500000:
        speed = B500000;
        break;
      case 576000:
        speed = B576000;
        break;
      case 921600:
        speed = B921600;
        break;
      case 1000000:
        speed = B1000000;
        break;
      case 1152000:
        speed = B1152000;
        break;
      case 1500000:
        speed = B1500000;
        break;
      case 2000000:
        speed = B2000000;
        break;
      case 2500000:
        speed = B2500000;
        break;
      case 3000000:
        speed = B3000000;
        break;
      case 3500000:
        speed = B3500000;
        break;
      case 4000000:
        speed = B4000000;
        break;
      default:
        speed = -1;
        break;
    }
  }
  return speed;
}

/* ========================================================================== */
