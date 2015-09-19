/**
 * @file io_udp.c
 * xPL Hardware Layer, POSIX Socket UDP/IP
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sysio/log.h>

#include <gxPL.h>
#include "io_p.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/* constants ================================================================ */
#define IO_NAME "udp"

/* structures =============================================================== */
typedef struct udp_data {
  int ofd;
  struct sockaddr_in bcast_addr;
  int ifd;
  int iport;
  struct in_addr local_addr;
} udp_data;

/* macros =================================================================== */
#define dp ((udp_data *)gxpl->io->pdata)

/* types ==================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */

/* -----------------------------------------------------------------------------
 * Make a fd non-blocking */
static int
set_nonblock (int fd) {
  int status;

  if ( (status = fcntl (fd, F_GETFL, 0)) != -1) {
    return fcntl (fd, F_SETFL, status | O_NONBLOCK);
  }
  vLog (LOG_ERR, "%s", strerror (errno));
  return -1;
}


/* -----------------------------------------------------------------------------
 * Try to increase the receive buffer as big as possible.
 * if we make it bigger, return 0.
 * Otherwise, if no change, -1
 */
static int
maximize_rxbuffer_size (int fd) {
  int initial_size, ideal_size, final_size;
  socklen_t size_len = sizeof (int);

  /* Get current receive buffer size */
  if (getsockopt (fd, SOL_SOCKET, SO_RCVBUF, &initial_size, &size_len) != 0) {

    vLog (LOG_ERR, "Unable to read receive socket buffer size - %s (%d)",
          strerror (errno), errno);
  }
  else {

    vLog (LOG_DEBUG, "Initial receive socket buffer size is %d bytes",
          initial_size);
  }

  /* Try to increase the buffer (maybe multiple times) */
  for (ideal_size = 1024000; ideal_size > initial_size;) {

    /* Attempt to set the buffer size */
    if (setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &ideal_size, sizeof (int)) != 0) {

      vLog (LOG_DEBUG, "Not able to set receive buffer to %d bytes - retrying",
            ideal_size);
      ideal_size -= 64000;
      continue;
    }

    /* We did it!  Get the current size and bail out */
    size_len = sizeof (int);
    if (getsockopt (fd, SOL_SOCKET, SO_RCVBUF, &final_size, &size_len) != 0) {

      vLog (LOG_ERR, "Unable to read receive socket buffer size - %s (%d)",
            strerror (errno), errno);
    }
    else {

      vLog (LOG_DEBUG, "Actual receive socket buffer size is %d bytes",
            final_size);
    }

    return (final_size > initial_size) ? 0 : -1;
  }

  /* We weren't able to increase it */
  vLog (LOG_WARNING, "Unable to increase receive buffer size - dang!");
  return -1;
}

/* -----------------------------------------------------------------------------
 * When no interface is selected, scan the list of interface
 * and choose the first one that looks OK.
 * If nothing is found, then return -1.
 * Otherwise, install the name as the interface and return 0
 */
static int
find_default_iface (int fd, gxPL * gxpl) {
  struct ifconf iflist;
  struct ifreq * ifr;
  struct ifreq ifinfo;
  int i, ifcount;

  // get the necessary buffer size in bytes for receiving all available addresses
  iflist.ifc_buf = NULL;
  if (ioctl (fd, SIOCGIFCONF, &iflist) < 0) {

    return -1;
  }

  // Request list of interfaces
  iflist.ifc_buf = malloc (iflist.ifc_len);
  if (ioctl (fd, SIOCGIFCONF, &iflist) < 0) {

    return -1;
  }
  ifcount = iflist.ifc_len / sizeof (struct ifreq);
  vLog (LOG_DEBUG, "%d interfaces found on this system", ifcount);

  // Try each interface until one works
  ifr = iflist.ifc_req;
  for (i = 0; i < ifcount; i++) {

    // Init the interface info request *
    memset (&ifinfo, 0, sizeof (struct ifreq));
    ifinfo.ifr_addr.sa_family = AF_INET;
    strcpy (ifinfo.ifr_name, ifr[i].ifr_name);

    // Get device flags
    if (ioctl (fd, SIOCGIFFLAGS, &ifinfo) != 0) {

      continue;
    }

    vLog (LOG_DEBUG, "Checking if interface %s is valid w/flags 0x%x",
          ifinfo.ifr_name, ifinfo.ifr_flags);

    // Insure this interface is active and not loopback
    if ( (ifinfo.ifr_flags & IFF_UP) == 0) {

      vLog (LOG_DEBUG, "  %s is down, continue...", ifinfo.ifr_name);
      continue;
    }

    if ( (ifinfo.ifr_flags & IFF_LOOPBACK) != 0) {

      vLog (LOG_DEBUG, "  %s is loopback, continue...", ifinfo.ifr_name);
      continue;
    }

    // If successful, use this interface
    strcpy (gxpl->config->iface, ifr[i].ifr_name);
    vLog (LOG_DEBUG, "Choose interface %s as default interface",
          gxpl->config->iface);
    return 0;
  }

  // No good interface found
  return -1;
}

/* -----------------------------------------------------------------------------
 * Create a socket for broadcasting messages
 */
static int
make_broadcast_connection (gxPL * gxpl) {
  int fd;
  int flag = 1;
  struct protoent *ppe;
  struct ifreq ifinfo;
  struct in_addr ifnetmask;

  // Map protocol name
  if ( (ppe = getprotobyname ("udp")) == 0) {

    vLog (LOG_ERR, "Unable to lookup UDP protocol info");
    return -1;
  }

  // Attempt to create a socket
  if ( (fd = socket (AF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {

    vLog (LOG_ERR, "Unable to create broadcast socket %s (%d)",
          strerror (errno), errno);
    return -1;
  }

  // Mark as a broadcasting socket
  if (setsockopt (fd, SOL_SOCKET, SO_BROADCAST, &flag,
                  sizeof (flag)) < 0) {

    vLog (LOG_ERR, "Unable to set SO_BROADCAST on socket %s (%d)",
          strerror (errno), errno);
    close (fd);
    return -1;
  }

  // See if we need to find a default interface
  if (strlen (gxpl->config->iface) == 0) {

    if (find_default_iface (fd, gxpl)) {

      vLog (LOG_ERR, "Could not find a working, non-loopback network interface");
      close (fd);
      return -1;
    }
  }

  // Init the interface info request
  memset (&ifinfo, 0, sizeof (struct ifreq));
  ifinfo.ifr_addr.sa_family = AF_INET;
  strcpy (ifinfo.ifr_name, gxpl->config->iface);

  // Get our interface address
  if (ioctl (fd, SIOCGIFADDR, &ifinfo) != 0) {

    vLog (LOG_ERR, "Unable to get IP addr for interface %s", gxpl->config->iface);
    close (fd);
    return -1;
  }

  dp->local_addr.s_addr = ( (struct sockaddr_in *) &ifinfo.ifr_addr)->sin_addr.s_addr;
  vLog (LOG_DEBUG, "Assigned IP address to %s", inet_ntoa (dp->local_addr));

  // Get interface netmask
  memset (&ifinfo, 0, sizeof (struct ifreq));
  ifinfo.ifr_addr.sa_family = AF_INET;
  ifinfo.ifr_broadaddr.sa_family = AF_INET;
  strcpy (ifinfo.ifr_name, gxpl->config->iface);
  if (ioctl (fd, SIOCGIFNETMASK, &ifinfo) != 0) {

    vLog (LOG_ERR, "Unable to extract the interface net mask");
    close (fd);
    return -1;
  }
  ifnetmask.s_addr = ( (struct sockaddr_in *) &ifinfo.ifr_netmask)->sin_addr.s_addr;

  // Build our broadcast addr
  memset (&dp->bcast_addr, 0, sizeof (dp->bcast_addr));
  dp->bcast_addr.sin_family = AF_INET;
  dp->bcast_addr.sin_addr.s_addr = dp->local_addr.s_addr | ~ifnetmask.s_addr;
  dp->bcast_addr.sin_port = htons (XPL_PORT);

  dp->ofd = fd;
  set_nonblock (fd);

  // And we are done
  vLog (LOG_DEBUG, "Assigned broadcast address to %s:%d",
        inet_ntoa (dp->bcast_addr.sin_addr),
        XPL_PORT);
  return 0;
}

/* -----------------------------------------------------------------------------
 * make a bind connection */
static int
make_connection (gxPL * gxpl) {
  int fd;
  int flag = 1;
  struct protoent *ppe;
  struct sockaddr_in socket_info;

  int socket_size = sizeof (struct sockaddr_in);

  /* Init the socket definition */
  memset (&socket_info, 0, sizeof (socket_info));
  socket_info.sin_family = AF_INET;
  socket_info.sin_addr.s_addr = INADDR_ANY;
  if (gxpl->config->connecttype == gxPLConnectViaHub) {

    socket_info.sin_port = htons (0);
  }
  else {

    socket_info.sin_port = htons (XPL_PORT);
  }
  /* Map protocol name */
  if ( (ppe = getprotobyname ("udp")) == 0) {

    vLog (LOG_ERR, "Unable to lookup UDP protocol info");
    return -1;
  }

  /* Attempt to creat the socket */
  if ( (fd = socket (PF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {
    vLog (LOG_ERR, "Unable to create listener socket %s (%d)",
          strerror (errno), errno);
    return -1;
  }

  if (gxpl->config->connecttype != gxPLConnectViaHub) {

    if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &flag,
                    sizeof (flag)) < 0) {

      vLog (LOG_ERR, "Unable to set SO_REUSEADDR on socket %s (%d)",
            strerror (errno), errno);
      close (fd);
      return -1;
    }
  }

  /* Mark as a broadcast socket */
  if (setsockopt (fd, SOL_SOCKET, SO_BROADCAST, &flag,
                  sizeof (flag)) < 0) {

    vLog (LOG_ERR, "Unable to set SO_BROADCAST on socket %s (%d)",
          strerror (errno), errno);
    close (fd);
    return -1;
  }

  /* Attempt to bind */
  if ( (bind (fd, (struct sockaddr *) &socket_info,
              socket_size)) < 0) {

    vLog (LOG_ERR, "Unable to bind listener socket to port %d, %s (%d)",
          ntohs (socket_info.sin_port), strerror (errno), errno);
    close (fd);
    return -1;
  }

  if (gxpl->config->connecttype == gxPLConnectViaHub) {

    /* Fetch the actual socket port # */
    if (getsockname (fd, (struct sockaddr *) &socket_info,
                     (socklen_t *) &socket_size)) {

      vLog (LOG_ERR, "Unable to fetch socket info for bound listener, %s (%d)",
            strerror (errno), errno);
      close (fd);
      return -1;
    }
  }

  /* We are ready to go */
  dp->ifd = fd;
  dp->iport = ntohs (socket_info.sin_port);
  if (dp->iport == XPL_PORT) {

    gxpl->config->connecttype = gxPLConnectStandAlone;
  }
  set_nonblock (dp->ifd);
  maximize_rxbuffer_size (dp->ifd);
  return 0;
}

/* -----------------------------------------------------------------------------
 * Figure out what sort of connection to make and do it */
static int
make_bind_connection (gxPL * gxpl) {

  /* Try an stand along connection */
  if ( (gxpl->config->connecttype == gxPLConnectStandAlone) ||
       (gxpl->config->connecttype == gxPLConnectAuto)) {


    /* Attempt the connection */
    vLog (LOG_DEBUG, "Attemping standalone xPL connection");
    if (make_connection (gxpl) < 0) {

      /* If we failed and this what we want, bomb out */
      vLog (LOG_ERR, "Standalone connect failed - %d %d",
            gxpl->config->connecttype, gxPLConnectStandAlone);
      return -1;
    }

    vLog (LOG_DEBUG, "xPL Starting in standalone mode on port %d",
          dp->iport);
    return 0;
  }

  /* Try a hub based connection */
  vLog (LOG_DEBUG, "Attempting via hub xPL connection");
  if (make_connection (gxpl) < 0) {

    return -1;
  }

  vLog (LOG_DEBUG, "xPL Starting in Hub mode on port %d",
        dp->iport);
  return 0;
}

// -----------------------------------------------------------------------------
static int
iopoll (gxPL * gxpl, int * available_data, int timeout_ms) {
  int ret;
  fd_set set;
  struct timeval timeout;
  long timeout_us = timeout_ms * 1000L;

  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (dp->ifd, &set);

  /* Initialize the timeout data structure. */
  timeout.tv_sec  = timeout_us / 1000000L;
  timeout.tv_usec = timeout_us % 1000000L;

  /* select returns 0 if timeout, 1 if input available, -1 if error. */
  ret = select (FD_SETSIZE, &set, NULL, NULL, &timeout);
  if (ret == -1) {

    vLog (LOG_ERR, "failed to poll listen socket: %s", strerror (errno));
  }
  else if (ret > 0) {

    ret = ioctl (dp->ifd, FIONREAD, available_data);
  }

  return ret;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpOpen (gxPL * gxpl) {

  if (gxpl->io->pdata == NULL) {

    gxpl->io->pdata = calloc (1, sizeof (udp_data));
    assert (gxpl->io->pdata);
    dp->ifd = -1;
    dp->ofd = -1;

    // Setup the broadcasting interface
    if (make_broadcast_connection (gxpl) < 0) {

      free (gxpl->io->pdata);
      return -1;
    }

    // Attempt to make bind connection
    if (make_bind_connection (gxpl) < 0) {
      int ret;

      ret = close (dp->ofd);
      if (ret != 0) {
        vLog (LOG_ERR, "failed to close broadcast socket: %s", strerror (errno));
      }
      free (gxpl->io->pdata);
      return -1;
    }

    return 0;
  }

  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpRead (gxPL * gxpl, void * buffer, int count) {
  int ret;

  /* Fetch the next message, if any */
  if ( (ret = recvfrom (dp->ifd, buffer, count, 0, NULL, NULL)) < 0) {

    /* Expected response when queue is empty */
    if (errno == EAGAIN) {

      return 0;
    }

    /* Note the error and bail */
    vLog (LOG_ERR, "Error reading xPL message from network - %s (%d)",
          strerror (errno), errno);
    return -1;
  }
  return ret;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpWrite (gxPL * gxpl, const void * buffer, int count) {
  int bytes_sent;

  /* Try to send the message */
  if ( (bytes_sent = sendto (dp->ofd, buffer, count, 0,
                             (struct sockaddr *) &dp->bcast_addr,
                             sizeof (struct sockaddr_in))) != count) {
    vLog (LOG_ERR, "Unable to broadcast message, %s (%d)",
          strerror (errno), errno);
    return -1;
  }
  vLog (LOG_DEBUG, "Broadcasted %d bytes (of %d attempted)", bytes_sent, count);

  /* Okey dokey then */
  return bytes_sent;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpClose (gxPL * gxpl) {
  int ret;

  // If already stopped, bail
  if (gxpl->io->pdata == NULL) {

    return -1;
  }

  ret = close (dp->ofd);
  if (ret != 0) {
    vLog (LOG_ERR, "failed to close broadcast socket: %s", strerror (errno));
  }

  ret = close (dp->ifd);
  if (ret != 0) {
    vLog (LOG_ERR, "failed to close bind socket: %s", strerror (errno));
  }

  free (gxpl->io->pdata);
  gxpl->io->pdata = NULL;
  return ret;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpCtl (gxPL * gxpl, int c, va_list ap) {
  int ret = 0;

  switch (c) {

      // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncPoll, int * available_bytes, int timeout_ms)
    case gxPLIoFuncPoll: {
      int * available_bytes = va_arg (ap, int*);
      int timeout_ms = va_arg (ap, int);
      ret = iopoll (gxpl, available_bytes, timeout_ms);
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetInetPort, int * iport)
    case gxPLIoFuncGetInetPort: {
      int * iport = va_arg (ap, int*);
      *iport = dp->iport;
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetBcastAddr, gxPLAddress * bcast_addr)
    case gxPLIoFuncGetBcastAddr: {
      gxPLAddress * bcast_addr = va_arg (ap, gxPLAddress*);
      bcast_addr->family = gxPLIoFamilyInet4;
      memcpy (bcast_addr->n_addr, &dp->bcast_addr.sin_addr.s_addr,
              sizeof (dp->bcast_addr.sin_addr.s_addr));
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncGetLocalAddr, gxPLAddress * local_addr)
    case gxPLIoFuncGetLocalAddr: {
      gxPLAddress * local_addr = va_arg (ap, gxPLAddress*);
      local_addr->family = gxPLIoFamilyInet4;
      memcpy (local_addr->n_addr, &dp->local_addr.s_addr,
              sizeof (dp->local_addr.s_addr));
    }
    break;

    // int gxPLIoCtl (gxPL * gxpl, gxPLIoFuncNetAddrToString, gxPLAddress * net_addr, char ** str_addr)
    case gxPLIoFuncNetAddrToString: {
      gxPLAddress * addr = va_arg (ap, gxPLAddress*);

      if (addr->family == gxPLIoFamilyInet4) {
        char ** str_addr = va_arg (ap, char**);
        struct in_addr net_addr;

        memcpy (&net_addr.s_addr, addr->n_addr, sizeof (net_addr.s_addr));
        *str_addr = inet_ntoa (net_addr);
      }
      else {

        errno = EINVAL;
        ret = -1;
      }
    }
    break;

    default:
      errno = EINVAL;
      ret = -1;
      break;
  }

  return ret;
}

/* private variables ======================================================== */
static gxPLIoOps
ops = {
  .open  = gxPLUdpOpen,
  .read  = gxPLUdpRead,
  .write = gxPLUdpWrite,
  .close = gxPLUdpClose,
  .ctl   = gxPLUdpCtl
};

/* public functions ========================================================= */

// -----------------------------------------------------------------------------
int __gxplio_init
gxPLUdpInit (void) {

  return gxPLIoRegister (IO_NAME, &ops);
}

// -----------------------------------------------------------------------------
int __gxplio_exit
gxPLUdpExit (void) {

  return gxPLIoUnregister (IO_NAME);
}

/* ========================================================================== */
