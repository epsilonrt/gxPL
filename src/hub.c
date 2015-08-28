/**
 * @file hub.c
 * Built-in xPL hub support
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>

#include "message_p.h"
#include "io_p.h"

/* constants ================================================================ */
#define GROW_CLIENT_LIST_BY 8
#define MAX_LOCAL_ADDR 128

/* structures =============================================================== */
typedef struct _hubClient {
  int clientPort;
  int heartbeatInterval;
  time_t lastHeardFromAt;
  int clientSocket;
  char * clientIdent;
  struct sockaddr_in clientAddr;
} hubClient;

/* private variables ======================================================== */
/* Hub status */
static bool hubRunning = FALSE;

/* Hub client list */
static int clientCount = 0;
static int clientAllocCount = 0;
static hubClient * clientList = NULL;

static int localIPAddrCount = 0;
static char * *localIPAddrs = NULL;

/* static functions ========================================================= */

/* -----------------------------------------------------------------------------
 */
static void buildLocalIPList (void) {
  int ifIndex, localIndex;
  struct ifconf interfaceList;
  struct ifreq * ifInfo;
  char ifBuff[sizeof (struct ifreq) * MAX_LOCAL_ADDR];
  int localNetCount = 0;

  /* Init the interface info request */
  bzero (ifBuff, sizeof (struct ifreq) * MAX_LOCAL_ADDR);
  interfaceList.ifc_len = MAX_LOCAL_ADDR;
  interfaceList.ifc_buf = ifBuff;

  /* Get our interfaces */
  if (ioctl (xPL_getFD(), SIOCGIFCONF, &interfaceList) != 0) {
    xPL_Debug ("Unable to get IP addr list");
    return;
  }

  /* Determine how many interfaces are real ones */
  for (ifIndex = 0; ifIndex < interfaceList.ifc_len; ifIndex++) {
    ifInfo = & (interfaceList.ifc_req[ifIndex]);
    if (ifInfo->ifr_addr.sa_family != AF_INET) {
      continue;
    }
    localNetCount++;
  }

  /* Free last list, if any */
  if (localIPAddrCount) {
    for (localIndex = 0; localIndex < localIPAddrCount; localIndex++) {
      STR_FREE (localIPAddrs[localIndex]);
    }
    free (localIPAddrs);
    localIPAddrCount = 0;
  }

  /* Allocate sufficiant space */
  xPL_Debug ("There are %d local network addresses on this system");
  localIPAddrs = (char * *) malloc (sizeof (char *) * localNetCount);
  for (ifIndex = 0; ifIndex < interfaceList.ifc_len; ifIndex++) {
    ifInfo = & (interfaceList.ifc_req[ifIndex]);
    if (ifInfo->ifr_addr.sa_family != AF_INET) {
      continue;
    }

    localIPAddrs[localIPAddrCount++] = xPL_StrDup (inet_ntoa ( ( (struct sockaddr_in *) & (ifInfo->ifr_addr))->sin_addr));
    xPL_Debug ("  Found %s", localIPAddrs[localIPAddrCount - 1]);
  }
}

/* -----------------------------------------------------------------------------
 * Return TRUE if the received heartbeat message is on our local host
 */
static bool 
isLocalClient (xPL_Message * theMessage) {
  int ipIndex ;

  /* Get clients IP address */
  char * clientIPAddr = xPL_getMessageNamedValue (theMessage, "remote-ip");
  if (clientIPAddr == NULL) {
    xPL_Debug ("No client address (remote-ip) found in message -- client message ignored");
    return FALSE;
  }

  /* See if this address matches any known local address */
  for (ipIndex = 0; ipIndex < localIPAddrCount; ipIndex++) {
    if (!strcmp (clientIPAddr, localIPAddrs[ipIndex])) {
      return TRUE;
    }
  }

  return FALSE;
}

/* -----------------------------------------------------------------------------
 * Return true if this is a heartbeat message 
 */
static bool isHeartbeatMessage (xPL_Message * theMessage) {
  char * schemaClass = xPL_getSchemaClass (theMessage);
  if (schemaClass == NULL) {
    xPL_Debug ("Message is missing schema class -- not a heartbeat message");
    return FALSE;
  }

  /* Either of these types is considered a heartbeat message */
  return ( (xPL_strcmpIgnoreCase (schemaClass, "hbeat") == 0)
           || (xPL_strcmpIgnoreCase (schemaClass, "config") == 0));
}

/* -----------------------------------------------------------------------------
 * Return true if this is a signoff  message
 */
static bool isSignoffMessage (xPL_Message * theMessage) {
  char * schemaType = xPL_getSchemaType (theMessage);
  if (schemaType == NULL) {
    xPL_Debug ("Message is missing schema type -- not a heartbeat message");
    return FALSE;
  }

  return (xPL_strcmpIgnoreCase (schemaType, "end") == 0);
}

/* -----------------------------------------------------------------------------
 * Extract the clients port # and return it (or -1)
 */
static int 
getClientPort (xPL_Message * theMessage) {
  int clientPort = -1;

  /* Ge the port # */
  char * clientPortNum = xPL_getMessageNamedValue (theMessage, "port");
  if (clientPortNum == NULL) {
    xPL_Debug ("Heartbeat message is missing port specification");
    return -1;
  }

  /* Attempt to convert to an int */
  if (!xPL_strToInt (clientPortNum, &clientPort)) {
    xPL_Debug ("Heartbeat message has invalid numeric port -- %s", clientPortNum);
    return -1;
  }

  /* Return Port # */
  return clientPort;
}

/* -----------------------------------------------------------------------------
 * Extract heartbeat interval
 */
static int 
getHeartbeatInterval (xPL_Message * theMessage) {
  int theInterval = -1;

  /* Ge the interval */
  char * heartbeatInterval = xPL_getMessageNamedValue (theMessage, "interval");
  if (heartbeatInterval == NULL) {
    xPL_Debug ("Heartbeat message is missing interval -- defaulting to 5 minutes");
    return 5;
  }

  /* Attempt to convert to an int */
  if (!xPL_strToInt (heartbeatInterval, &theInterval)) {
    xPL_Debug ("Heartbeat message has invalid numeric interval -- %s -- changing to 5 minutes", heartbeatInterval);
    return 5;
  }

  /* Constrain interval */
  if (theInterval > 30) {
    xPL_Debug ("Heartbeat interval of %d out of range -- contrained to 30 minutes");
    theInterval = 30;
  }

  /* Return interval */
  return theInterval;
}

/* -----------------------------------------------------------------------------
 */
static void 
updateClientIdent (xPL_Message * theMessage, hubClient * theClient) {
  char identBuff[64];

  /* Build an ident for this message */
  strcpy (identBuff, xPL_getSourceVendor (theMessage));
  strcat (identBuff, "-");
  strcat (identBuff, xPL_getSourceDeviceID (theMessage));
  strcat (identBuff, ".");
  strcat (identBuff, xPL_getSourceInstanceID (theMessage));

  /* See if there is a change */
  if (theClient->clientIdent == NULL) {
    theClient->clientIdent = xPL_StrDup (identBuff);
    return;
  }

  /* See if there is a change */
  if (!strcmp (theClient->clientIdent, identBuff)) {
    return;
  }
  xPL_Debug ("Client changing Ident from %s to %s", theClient->clientIdent, identBuff);
  STR_FREE (theClient->clientIdent);
  theClient->clientIdent = xPL_StrDup (identBuff);
}

/* -----------------------------------------------------------------------------
 * Search the list of known clients for a matching port If not found, return NULL
 */
static hubClient * findClientByPort (int clientPortNum) {
  int clientIndex;

  for (clientIndex = 0; clientIndex < clientCount; clientIndex++) {
    if (clientList[clientIndex].clientPort == clientPortNum) {
      return & (clientList[clientIndex]);
    }
  }

  return NULL;
}

/* -----------------------------------------------------------------------------
 * Add the passed client to the client list
 */
static hubClient * addNewClient (xPL_Message * theMessage, int clientPort, int heartbeatInterval) {
  hubClient * theClient = NULL;
  int flag = 1;
  struct protoent *ppe;

  /* Grow the client list, if needed */
  if (clientCount == clientAllocCount) {
    clientAllocCount += GROW_CLIENT_LIST_BY;
    clientList = realloc (clientList, (clientAllocCount * sizeof (hubClient)));
  }

  /* Allocate a client */
  theClient = & (clientList[clientCount++]);
  bzero (theClient, sizeof (hubClient));

  /* Install info */
  theClient->clientPort = clientPort;
  theClient->heartbeatInterval = heartbeatInterval;
  theClient->lastHeardFromAt = time (NULL);
  updateClientIdent (theMessage, theClient);

  /* Map protocol name */
  if ( (ppe = getprotobyname ("udp")) == 0) {
    xPL_Debug ("Unable to lookup UDP protocol info -- cannot setup new client");
    clientCount--;
    return NULL;
  }

  /* Attempt to create a socket */
  if ( (theClient->clientSocket = socket (AF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {
    xPL_Debug ("Unable to create broadcast socket %s (%d) -- cannot setup new client ", strerror (errno), errno);
    clientCount--;
    return NULL;
  }

  /* Mark as a broadcasting socket */
  if (setsockopt (theClient->clientSocket, SOL_SOCKET, SO_BROADCAST, &flag, sizeof (flag)) < 0) {
    xPL_Debug ("Unable to set SO_BROADCAST on socket %s (%d) -- cannot setup new client", strerror (errno), errno);
    close (theClient->clientSocket);
    clientCount--;
    return NULL;
  }

  /* Setup the broadcast address for it */
  theClient->clientAddr.sin_family = AF_INET;
  theClient->clientAddr.sin_port = htons (clientPort);
  theClient->clientAddr.sin_addr.s_addr = inet_addr (xPL_getListenerIPAddr());

  /* And we are ready to go */
  xPL_Debug ("Added new client %s on port %d, interval is %d, client count now %d",
             theClient->clientIdent, clientPort, heartbeatInterval, clientCount);

  /* And return it */
  return theClient;
}

/* -----------------------------------------------------------------------------
 * Given the passed client, remove it from our list and return TRUE.  If
 * client is not found, return FALSE
 */
static bool releaseClient (hubClient * theClient) {
  int clientIndex;

  /* Find the client */
  for (clientIndex = 0; clientIndex < clientCount; clientIndex++) {
    /* Skip when not us */
    if (& (clientList[clientIndex]) != theClient) {
      continue;
    }

    /* Close the clients socket */
    close (theClient->clientSocket);

    /* Reduce count */
    clientCount--;

    /* Slide remaining clients down */
    if (clientIndex < clientCount) {
      memcpy (& (clientList[clientIndex]), & (clientList[clientIndex + 1]), (clientCount - clientIndex) * sizeof (hubClient));
    }

    /* And we are done */
    return TRUE;
  }

  /* Never found the client */
  return FALSE;
}

/* -----------------------------------------------------------------------------
 * Release all clients
 */
static void 
releaseAllClients (void) {
  int clientIndex;

  for (clientIndex = clientCount - 1; clientIndex >= 0; clientIndex--) {
    releaseClient (&clientList[clientIndex]);
  }
}

/* -----------------------------------------------------------------------------
 * Check the message to see if it's an update to our list of clients
 */
static void 
checkForClientUpdates (xPL_Message * theMessage) {
  int clientPort = -1;
  int clientHeartbeatInterval = -1;
  bool clientSigningOff = FALSE;
  hubClient * theClient = NULL;

  /* See if this is a heaertbeat message */
  if (!isHeartbeatMessage (theMessage)) {
    return;
  }

  /* See if this is a local message */
  if (!isLocalClient (theMessage)) {
    return;
  }

  /* See if this is a signoff message or not */
  clientSigningOff = isSignoffMessage (theMessage);

  /* Get the port #, if any */
  if ( (clientPort = getClientPort (theMessage)) == -1) {
    return;
  }

  /* Get the interval for updates */
  clientHeartbeatInterval = getHeartbeatInterval (theMessage);

  /* Lookup client */
  if ( (theClient = findClientByPort (clientPort)) == NULL) {
    /* If this is a signoff, we are done with it (signoff of */
    /* an unknown client is of no interest to us             */
    if (clientSigningOff) {
      return;
    }

    /* New client -- add to the list */
    addNewClient (theMessage, clientPort, clientHeartbeatInterval);
    return;
  }

  /* Check for client ident change */
  updateClientIdent (theMessage, theClient);

  /* See if heartbeat interval has changed */
  if (clientHeartbeatInterval != theClient->heartbeatInterval) {
    xPL_Debug ("Changing heartbeat interval for client on port %s from %s minutes to %d",
               theClient->clientPort, theClient->heartbeatInterval, clientHeartbeatInterval);
    theClient->heartbeatInterval = clientHeartbeatInterval;
  }

  /* Update last heart from time */
  theClient->lastHeardFromAt = time (NULL);
}

/* -----------------------------------------------------------------------------
 * Broadcast the passed message data to the passed client.  If all goes
 * well, return TRUE.  If there is an error, return FALSE
 */
static bool 
rebroadcastMessage (hubClient * theClient, char * theData, int dataLen) {
  int bytesSent;

  /* Try to send the message */
  if ( (bytesSent = sendto (theClient->clientSocket, theData, dataLen, 0,
                            (struct sockaddr *) & (theClient->clientAddr), sizeof (struct sockaddr_in))) != dataLen) {
    xPL_Debug ("Unable to rebroadcast message to client on port %d, %s (%d)", theClient->clientPort, strerror (errno), errno);
    return FALSE;
  }
  xPL_Debug ("Broadcasted %d bytes (of %d attempted)", bytesSent, dataLen);

  /* Okey dokey then */
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Rebroadcast the passed message to all current clients
 */

static void rebroadcastMessageToClients (xPL_Message * theMessage) {
  char * formattedText;
  int formattedSize, clientIndex;

  /* If there are no clients, skip this */
  if (clientCount == 0) {
    return;
  }

  /* Format this message back into text */
  if ( (formattedText = xPL_formatMessage (theMessage)) == NULL) {
    xPL_Debug ("Unable to format message for rebroadcast -- message lost");
    return;
  }

  /* Get the message size */
  formattedSize = strlen (formattedText);

  /* Send it to each client */
  for (clientIndex = 0; clientIndex < clientCount; clientIndex++) {
    rebroadcastMessage (&clientList[clientIndex], formattedText, formattedSize);
  }
}

/* -----------------------------------------------------------------------------
 * Handle a newly received message.  Update our recorded clients
 * and rebroadcast to them */
static void 
handleXPLMessage (xPL_Message * theMessage, xPL_Object * userValue) {
  /* Do client checks */
  checkForClientUpdates (theMessage);

  /* Rebroadcast message to all clients */
  rebroadcastMessageToClients (theMessage);
}

/* -----------------------------------------------------------------------------
 * Check each client to see how long it's been since we
 * last heard from it.  If it's twice as long as the heartbeat
 * interval we expect, declare them dead and remove them */
static void 
doClientTimeoutChecks (int actualElapsedTime, xPL_Object * userValue) {
  int clientIndex, elapsedTime;
  time_t rightNow = time (NULL);
  hubClient * theClient;

  xPL_Debug ("Checking for client timeouts...");

  for (clientIndex = clientCount - 1; clientIndex >= 0; clientIndex--) {
    theClient = &clientList[clientIndex];

    /* Figure elapsed time since we've last heard from this client */
    elapsedTime = (rightNow - theClient->lastHeardFromAt) / 60;
    xPL_Debug ("  Checking client #%d on port %d -- %d minutes since last heard from", clientIndex, theClient->clientPort, elapsedTime);

    /* See if this exceeds twice the expected heartbeat interval */
    if (elapsedTime < ( (theClient->heartbeatInterval * 2) + 1)) {
      continue;
    }

    /* We have not heard from this client -- remove it */
    xPL_Debug ("Havn't heard from client @ port %d in %d minutes (heartbeats expected every %d minutes) -- Removing it",
               theClient->clientPort, elapsedTime, theClient->heartbeatInterval);
    releaseClient (theClient);
  }
}

/* public functions ========================================================= */

/* -----------------------------------------------------------------------------
 * Public
 * Once called, this instance of gxPLib will be running a
 * hub process.   Note: The gxPLib must be started in
 * standalone mode for this to work (which is NOT the
 * default). */
bool
xPL_startHub (void) {
  /* If the hub is already running, we're done */
  if (hubRunning) {
    return TRUE;
  }

  /* Create a list of local IP addresses */
  buildLocalIPList();

  /* And a listener for all xPL messages */
  xPL_addMessageListener (handleXPLMessage, NULL);

  /* Add timeout handler to check for dead clients */
  xPL_addTimeoutHandler (doClientTimeoutChecks, 60, NULL);

  /* And we are running */
  hubRunning = TRUE;
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Public
 * Disable the hub */
void
xPL_stopHub (void) {
  /* If we are not running, skip this */
  if (!hubRunning) {
    return;
  }

  /* Remove handlers */
  xPL_removeMessageListener (handleXPLMessage);
  xPL_removeTimeoutHandler (doClientTimeoutChecks);

  /* Clear all clients */
  releaseAllClients();

  /* Clear running flag */
  hubRunning = FALSE;
}

/* -----------------------------------------------------------------------------
 * Public
 * Return if hub is running or not */
bool
xPL_isHubRunning (void) {
  return hubRunning;
}
/* ========================================================================== */
