/**
 * @file
 * xPL Hardware Layer, POSIX Socket UDP/IP (unix source code)
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifdef  __unix__
#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define GXPL_IO_INTERNALS
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
  xVector addr_list;
} udp_data;

/* macros =================================================================== */
#define dp ((udp_data *)io->pdata)

/* types ==================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
// -----------------------------------------------------------------------------
int
prvIpMatch (const void *key1, const void *key2) {

  return strcmp ( (const char *) key1, (const char *) key2);
}

// -----------------------------------------------------------------------------
const void *
prvIpKey (const void * ip) {

  return ip;
}

/* -----------------------------------------------------------------------------
 * Make a fd non-blocking */
static int
prvSetSocketNonblock (int fd) {
  int status;

  if ( (status = fcntl (fd, F_GETFL, 0)) != -1) {
    return fcntl (fd, F_SETFL, status | O_NONBLOCK);
  }
  PERROR ("%s", strerror (errno));
  return -1;
}

/* -----------------------------------------------------------------------------
 * Try to increase the receive buffer as big as possible.
 * if we make it bigger, return 0.
 * Otherwise, if no change, -1
 */
static int
prvMaximizeRxbufferSize (int fd) {
  int initial_size, ideal_size, final_size;
  socklen_t size_len = sizeof (int);

  /* Get current receive buffer size */
  if (getsockopt (fd, SOL_SOCKET, SO_RCVBUF, &initial_size, &size_len) != 0) {

    PERROR ("Unable to read receive socket buffer size - %s (%d)",
            strerror (errno), errno);
  }
  else {

    PDEBUG ("Initial receive socket buffer size is %d bytes",
            initial_size);
  }

  /* Try to increase the buffer (maybe multiple times) */
  for (ideal_size = 1024000; ideal_size > initial_size;) {

    /* Attempt to set the buffer size */
    if (setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &ideal_size, sizeof (int)) != 0) {

      PDEBUG ("Not able to set receive buffer to %d bytes - retrying",
              ideal_size);
      ideal_size -= 64000;
      continue;
    }

    /* We did it!  Get the current size and bail out */
    size_len = sizeof (int);
    if (getsockopt (fd, SOL_SOCKET, SO_RCVBUF, &final_size, &size_len) != 0) {

      PERROR ("Unable to read receive socket buffer size - %s (%d)",
              strerror (errno), errno);
    }
    else {

      PDEBUG ("Actual receive socket buffer size is %d bytes",
              final_size);
    }

    return (final_size > initial_size) ? 0 : -1;
  }

  /* We weren't able to increase it */
  PWARNING ("Unable to increase receive buffer size - dang!");
  return -1;
}

/* -----------------------------------------------------------------------------
 * When no interface is selected, scan the list of interface
 * and choose the first one that looks OK.
 * If nothing is found, then return -1.
 * Otherwise, install the name as the interface and return 0
 */
static int
prvFindDefaultIface (int fd, gxPLIo * io) {
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
  PDEBUG ("%d interfaces found on this system", ifcount);

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

    PDEBUG ("Checking if interface %s is valid w/flags 0x%x",
            ifinfo.ifr_name, ifinfo.ifr_flags);

    // Insure this interface is active and not loopback
    if ( (ifinfo.ifr_flags & IFF_UP) == 0) {

      PDEBUG ("  %s is down, continue...", ifinfo.ifr_name);
      continue;
    }

    if ( (ifinfo.ifr_flags & IFF_LOOPBACK) != 0) {

      PDEBUG ("  %s is loopback, continue...", ifinfo.ifr_name);
      continue;
    }

    // If successful, use this interface
    strcpy (io->setting->iface, ifr[i].ifr_name);
    PDEBUG ("Choose interface %s as default interface",
            io->setting->iface);
    return 0;
  }

  // No good interface found
  errno = ENONET;
  return -1;
}

/* -----------------------------------------------------------------------------
 * Create a socket for broadcasting messages
 */
static int
prvMakeBroadcastConnection (gxPLIo * io) {
  int fd;
  int flag = 1;
  struct protoent *ppe;
  struct ifreq ifinfo;
  struct in_addr ifnetmask;

  // Map protocol name
  if ( (ppe = getprotobyname ("udp")) == 0) {

    PERROR ("Unable to lookup UDP protocol info");
    return -1;
  }

  // Attempt to create a socket
  if ( (fd = socket (AF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {

    PERROR ("Unable to create broadcast socket %s (%d)",
            strerror (errno), errno);
    return -1;
  }

  // Mark as a broadcasting socket
  if (setsockopt (fd, SOL_SOCKET, SO_BROADCAST, &flag,
                  sizeof (flag)) < 0) {

    PERROR ("Unable to set SO_BROADCAST on socket %s (%d)",
            strerror (errno), errno);
    close (fd);
    return -1;
  }

  // See if we need to find a default interface
  if (strlen (io->setting->iface) == 0) {
    int ret;
    unsigned timeout = 0;

    do {

      errno = 0;
      ret = prvFindDefaultIface (fd, io);
      if (ret == -1) {

        if (errno == ENONET) {
          
          /* The machine is not on the network, we fall asleep before resuming 
           * the search until the timeout is not reached. 
           */
          sleep (1);
          timeout++;
          if (timeout < io->setting->iotimeout) {
            
            continue;
          }
        }

        PERROR ("Could not find a working, non-loopback network interface");
        close (fd);
        return -1;
      }
    }
    while (ret != 0);
  }

  // Init the interface info request
  memset (&ifinfo, 0, sizeof (struct ifreq));
  ifinfo.ifr_addr.sa_family = AF_INET;
  strcpy (ifinfo.ifr_name, io->setting->iface);

  // Get our interface address
  if (ioctl (fd, SIOCGIFADDR, &ifinfo) != 0) {

    PERROR ("Unable to get IP addr for interface %s", io->setting->iface);
    close (fd);
    return -1;
  }

  dp->local_addr.s_addr = ( (struct sockaddr_in *) &ifinfo.ifr_addr)->sin_addr.s_addr;
  PDEBUG ("Assigned IP address to %s", inet_ntoa (dp->local_addr));

  // Get interface netmask
  memset (&ifinfo, 0, sizeof (struct ifreq));
  ifinfo.ifr_addr.sa_family = AF_INET;
  ifinfo.ifr_broadaddr.sa_family = AF_INET;
  strcpy (ifinfo.ifr_name, io->setting->iface);
  if (ioctl (fd, SIOCGIFNETMASK, &ifinfo) != 0) {

    PERROR ("Unable to extract the interface net mask");
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
  prvSetSocketNonblock (fd);

  // And we are done
  PDEBUG ("Assigned broadcast address to %s:%d",
          inet_ntoa (dp->bcast_addr.sin_addr),
          XPL_PORT);
  return 0;
}

/* -----------------------------------------------------------------------------
 * make a bind connection */
static int
prvMakeConnection (gxPLIo * io) {
  int fd;
  int flag = 1;
  struct protoent *ppe;
  struct sockaddr_in socket_info;

  int socket_size = sizeof (struct sockaddr_in);

  /* Init the socket definition */
  memset (&socket_info, 0, sizeof (socket_info));
  socket_info.sin_family = AF_INET;
  socket_info.sin_addr.s_addr = INADDR_ANY;
  if (io->setting->connecttype == gxPLConnectViaHub) {

    socket_info.sin_port = htons (0);
  }
  else {

    socket_info.sin_port = htons (XPL_PORT);
  }
  /* Map protocol name */
  if ( (ppe = getprotobyname ("udp")) == 0) {

    PERROR ("Unable to lookup UDP protocol info");
    return -1;
  }

  /* Attempt to creat the socket */
  if ( (fd = socket (PF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {
    PERROR ("Unable to create listener socket %s (%d)",
            strerror (errno), errno);
    return -1;
  }

  if (io->setting->connecttype != gxPLConnectViaHub) {

    if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &flag,
                    sizeof (flag)) < 0) {

      PERROR ("Unable to set SO_REUSEADDR on socket %s (%d)",
              strerror (errno), errno);
      close (fd);
      return -1;
    }
  }

  /* Mark as a broadcast socket */
  if (setsockopt (fd, SOL_SOCKET, SO_BROADCAST, &flag,
                  sizeof (flag)) < 0) {

    PERROR ("Unable to set SO_BROADCAST on socket %s (%d)",
            strerror (errno), errno);
    close (fd);
    return -1;
  }

  /* Attempt to bind */
  if ( (bind (fd, (struct sockaddr *) &socket_info,
              socket_size)) < 0) {

    PERROR ("Unable to bind listener socket to port %d, %s (%d)",
            ntohs (socket_info.sin_port), strerror (errno), errno);
    close (fd);
    return -1;
  }

  if (io->setting->connecttype == gxPLConnectViaHub) {

    /* Fetch the actual socket port # */
    if (getsockname (fd, (struct sockaddr *) &socket_info,
                     (socklen_t *) &socket_size)) {

      PERROR ("Unable to fetch socket info for bound listener, %s (%d)",
              strerror (errno), errno);
      close (fd);
      return -1;
    }
  }

  /* We are ready to go */
  dp->ifd = fd;
  dp->iport = ntohs (socket_info.sin_port);
  if (dp->iport == XPL_PORT) {

    io->setting->connecttype = gxPLConnectStandAlone;
  }
  prvSetSocketNonblock (dp->ifd);
  prvMaximizeRxbufferSize (dp->ifd);
  return 0;
}

/* -----------------------------------------------------------------------------
 * Figure out what sort of connection to make and do it */
static int
prvMakeBindConnection (gxPLIo * io) {

  /* Try an stand along connection */
  if ( (io->setting->connecttype == gxPLConnectStandAlone) ||
       (io->setting->connecttype == gxPLConnectAuto)) {


    /* Attempt the connection */
    PDEBUG ("Attemping standalone xPL connection");
    if (prvMakeConnection (io) < 0) {

      /* If we failed and this what we want, bomb out */
      PERROR ("Standalone connect failed - %d %d",
              io->setting->connecttype, gxPLConnectStandAlone);
      return -1;
    }

    PDEBUG ("xPL Starting in standalone mode on port %d",
            dp->iport);
    return 0;
  }

  /* Try a hub based connection */
  PDEBUG ("Attempting via hub xPL connection");
  if (prvMakeConnection (io) < 0) {

    return -1;
  }

  PDEBUG ("xPL Starting in Hub mode on port %d",
          dp->iport);
  return 0;
}


/* -----------------------------------------------------------------------------
 */
static int
prvBuildLocalIpList (gxPLIo * io) {
  struct ifconf iface_list;
  struct ifreq * iface;
  int len = 5;

  char * buf = calloc (len, sizeof (struct ifreq));
  assert (buf);
  len *= sizeof (struct ifreq);

  /* Get our interfaces */
  iface_list.ifc_len = len;

  while (iface_list.ifc_len >= len) {

    iface_list.ifc_buf = buf;

    if (ioctl (dp->ifd, SIOCGIFCONF, &iface_list) != 0) {

      free (buf);
      PERROR ("Unable to get IP addr list");
      return -1;
    }

    if (iface_list.ifc_len == len) {

      len *= 2;
      buf = realloc (buf, len);
      iface_list.ifc_len = len;
    }
  }

  for (int i = 0; i < (iface_list.ifc_len / sizeof (struct ifreq)); i++) {

    iface = & (iface_list.ifc_req[i]);
    if (iface->ifr_addr.sa_family == AF_INET) {

      char * src = inet_ntoa ( ( (struct sockaddr_in *) & (iface->ifr_addr))->sin_addr);
      char * dst = malloc (strlen (src) + 1);
      assert (dst);
      strcpy (dst, src);
      if (iVectorAppend (&dp->addr_list, dst) != 0) {

        free (buf);
        PERROR ("Unable to append ip addr to list");
        return -1;
      }
    }
  }
  free (buf);
  return 0;
}

// -----------------------------------------------------------------------------
static int
prvIoPoll (gxPLIo * io, int * available_data, int timeout_ms) {
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
    if (errno != EINTR) {
      PERROR ("failed to poll listen socket: %s", strerror (errno));
    }
    else {
      ret = 0;
    }
  }
  else if ( (ret > 0) && (FD_ISSET (dp->ifd, &set))) {

    ret = ioctl (dp->ifd, FIONREAD, available_data);
  }

  return ret;
}

/* private API functions ==================================================== */

// -----------------------------------------------------------------------------
static int
gxPLUdpOpen (gxPLIo * io) {

  if (io->pdata == NULL) {

    io->pdata = calloc (1, sizeof (udp_data));
    assert (io->pdata);
    dp->ifd = -1;
    dp->ofd = -1;

    // Setup the broadcasting interface
    if (prvMakeBroadcastConnection (io) < 0) {

      free (io->pdata);
      return -1;
    }

    // Attempt to make bind connection
    if (prvMakeBindConnection (io) < 0) {
      int ret;

      ret = close (dp->ofd);
      if (ret != 0) {
        PERROR ("failed to close broadcast socket: %s", strerror (errno));
      }
      free (io->pdata);
      return -1;
    }

    iVectorInit (&dp->addr_list, 1, NULL, free);
    iVectorInitSearch (&dp->addr_list, prvIpKey, prvIpMatch);

    return prvBuildLocalIpList (io);
  }

  return -1;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpRecv (gxPLIo * io, void * buffer, int count, gxPLIoAddr * source) {
  int ret;
  struct sockaddr_in client;
  socklen_t addrlen = sizeof (client);

  ret = recvfrom (dp->ifd, buffer, count, 0, (struct sockaddr *) &client, &addrlen);

  if (ret >= 0) {

    if (source)  {
      // Send, get and copy the source address
      source->family = gxPLNetFamilyInet4;
      source->addrlen = MIN (sizeof (source->addr), sizeof (client.sin_addr.s_addr));
      source->port = ntohs (client.sin_port);
      source->flag = 0;
      memcpy (source->addr, &client.sin_addr.s_addr, source->addrlen);
    }
  }
  else {

    // Expected response when queue is empty
    if (errno == EAGAIN) {

      return 0;
    }

    // Note the error and bail
    PERROR ("Error reading xPL message from network - %s (%d)",
            strerror (errno), errno);
    return -1;
  }

  return ret;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpSend (gxPLIo * io, const void * buffer, int count, const gxPLIoAddr * target) {
  int bytes_sent, addrlen = sizeof (struct sockaddr_in);
  struct sockaddr_in a;
  struct sockaddr * addrdst = (struct sockaddr *) &dp->bcast_addr;

  if (target) {

    if ( (target->isbroadcast == 0) && (target->family == gxPLNetFamilyInet4)) {

      a.sin_family = AF_INET;
      memcpy (&a.sin_addr.s_addr, target->addr,  sizeof (a.sin_addr.s_addr));
      a.sin_port = htons (target->port);
      addrdst = (struct sockaddr *) &a;
    }
  }

  // Try to send the message
  if ( (bytes_sent = sendto (dp->ofd, buffer, count, 0, addrdst, addrlen)) != count) {
    PERROR ("Unable to deliver the message, %s (%d)",
            strerror (errno), errno);
    return -1;
  }
  PDEBUG ("Send %d bytes (of %d attempted)", bytes_sent, count);

  return bytes_sent;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpClose (gxPLIo * io) {
  int ret;

  // If already stopped, bail
  if (io->pdata == NULL) {

    return -1;
  }

  ret = close (dp->ofd);
  if (ret != 0) {
    PERROR ("failed to close broadcast socket: %s", strerror (errno));
  }

  ret = close (dp->ifd);
  if (ret != 0) {
    PERROR ("failed to close bind socket: %s", strerror (errno));
  }
  vVectorDestroy (&dp->addr_list);
  free (io->pdata);
  io->pdata = NULL;
  return ret;
}

// -----------------------------------------------------------------------------
static int
gxPLUdpCtl (gxPLIo * io, int c, va_list ap) {
  int ret = 0;

  switch (c) {

      // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncPoll, int * available_bytes, int timeout_ms)
    case gxPLIoFuncPoll: {
      int * available_bytes = va_arg (ap, int*);
      int timeout_ms = va_arg (ap, int);
      ret = prvIoPoll (io, available_bytes, timeout_ms);
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetBcastAddr, gxPLIoAddr * bcast_addr)
    case gxPLIoFuncGetBcastAddr: {
      gxPLIoAddr * bcast_addr = va_arg (ap, gxPLIoAddr*);
      bcast_addr->family = gxPLNetFamilyInet4;
      bcast_addr->addrlen = sizeof (dp->bcast_addr.sin_addr.s_addr);
      bcast_addr->port = XPL_PORT;
      bcast_addr->flag = 0;
      bcast_addr->isbroadcast = 1;
      memcpy (bcast_addr->addr, &dp->bcast_addr.sin_addr.s_addr,
              bcast_addr->addrlen);
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetNetInfo, gxPLIoAddr * local_addr)
    case gxPLIoFuncGetNetInfo: {
      gxPLIoAddr * local_addr = va_arg (ap, gxPLIoAddr*);
      local_addr->family = gxPLNetFamilyInet4;
      local_addr->addrlen = sizeof (dp->local_addr.s_addr);
      local_addr->port = dp->iport;
      local_addr->flag = 0;
      memcpy (local_addr->addr, &dp->local_addr.s_addr, local_addr->addrlen);
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncNetAddrToString, gxPLIoAddr * net_addr, char ** str_addr)
    case gxPLIoFuncNetAddrToString: {
      gxPLIoAddr * addr = va_arg (ap, gxPLIoAddr*);

      if (addr->family == gxPLNetFamilyInet4) {
        char ** str_addr = va_arg (ap, char**);
        struct in_addr net_addr;

        memcpy (&net_addr.s_addr, addr->addr, sizeof (net_addr.s_addr));
        *str_addr = inet_ntoa (net_addr);
      }
      else {

        errno = EINVAL;
        ret = -1;
      }
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncNetAddrFromString, gxPLIoAddr * net_addr, const char * str_addr)
    case gxPLIoFuncNetAddrFromString: {
      gxPLIoAddr * addr = va_arg (ap, gxPLIoAddr*);
      const char * str_addr = va_arg (ap, char*);
      struct in_addr net_addr;

      addr->family = gxPLNetFamilyInet4;
      addr->addrlen = sizeof (net_addr.s_addr);
      ret = inet_aton (str_addr, &net_addr);

      if (ret != 0) {

        memcpy (addr->addr, &net_addr.s_addr, addr->addrlen);
        ret = 0;
      }
      else {

        ret = -1;
      }
    }
    break;

    // int gxPLIoCtl (gxPLIo * io, gxPLIoFuncGetLocalAddrList, const xVector ** addr_list)
    case gxPLIoFuncGetLocalAddrList: {
      const xVector ** addr_list = (const xVector **) va_arg (ap, xVector**);
      *addr_list = &dp->addr_list;
      return 0;
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
  .recv  = gxPLUdpRecv,
  .send  = gxPLUdpSend,
  .close = gxPLUdpClose,
  .ctl   = gxPLUdpCtl
};

/* public functions ========================================================= */

// -----------------------------------------------------------------------------
void __gxplio_init
gxPLUdpInit (void) {

  (void) gxPLIoRegister (IO_NAME, &ops);
}

// -----------------------------------------------------------------------------
void __gxplio_exit
gxPLUdpExit (void) {

  (void) gxPLIoUnregister (IO_NAME);
}

#endif /* __unix__ defined */
/* ========================================================================== */
