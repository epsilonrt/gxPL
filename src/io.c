/**
 * @file io.c
 * xPL I/O handling
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <netdb.h>
#include <termios.h>

#include "io_p.h"
#include "message_p.h"
#include "service_p.h"

/* types ==================================================================== */
/* private functions ======================================================== */
/* public variables ========================================================= */
/* internal public functions ================================================ */

/* constants ================================================================ */
#define POLL_GROW_BY 32
#define GROW_TIMEOUT_LIST_BY 4

/* macros =================================================================== */
#define VALID_CHAR(theChar) (((theChar >= 32) && (theChar < 123)) || (theChar = 124) || (theChar = 126))

/* structures =============================================================== */
/* User defined data for each poll entry */
struct _pollUserInfo {
  xPL_IOHandler ioHandler;
  int userValue;
};

/* Timeout support structures */
typedef struct {
  int timeoutInterval;
  time_t lastTimedoutAt;
  xPL_TimeoutHandler timeoutHandler;
  xPL_Object * userValue;
} TimeoutHandler;

/* private variables ======================================================== */
/* Info on poll space usage */
static int pollInfoSize  = 0;     /* # of allocated slots in pollInfo */
static int pollInfoCount = 0;     /* # of slots in use in pollInfo */
static struct pollfd * pollInfo = NULL;
static struct _pollUserInfo * pollUserInfo = NULL;


static int timeoutCount = 0;
static int timeoutAllocCount = 0;
static TimeoutHandler *timeoutList = NULL;
static time_t timeoutChecksLastDoneAt = -1;

/* The xPL FD */
int xPLFD = -1;
pid_t xPLPID;

/* Connection Info */
static xPL_ConnectType xPLconnectType = xcViaHub;  /* Default is via hub */
static int xPLPort = XPL_PORT;                /* Current actual port */

static int xPLBroadcastFD = -1;                    /* FD for broadcast socket */
static char xPLInterfaceName[64] = "";             /* Interface to connect to (empty=auto assign) */
static struct in_addr xPLInterfaceAddr;            /* Address associated with interface */
static struct sockaddr_in xPLBroadcastAddr;        /* Broadcasing address */

static time_t heartbeatCheckLastDoneAt = -1;

/* Status/state */
static bool xPL_IODeviceInstalled = FALSE;

/* Conversion buffers */
static char uniqueIDPrefix[9] = "";
static char base36Table[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                              'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                              'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                              'u', 'v', 'w', 'x', 'y', 'z'
                            };

static char blockHeaderBuff[128];
static char blockNameBuff[256];
static char blockValueBuff[MSG_MAX_SIZE];
static bool hubConfirmed = FALSE;

/* static functions ========================================================= */

/* -----------------------------------------------------------------------------
 * Convert a long value into an 8 digit base36 number 
 * and concatenate it onto the passed string */
static void 
longToBase32 (unsigned long theValue, char * theBuffer) {
  int charPtr, buffLen;

  /* Fill with zeros */
  strcat (theBuffer, "00000000");
  buffLen = strlen (theBuffer);

  /* Handle the simple case */
  if (theValue == 0) {
    return;
  }

  for (charPtr = buffLen - 1; charPtr >= (buffLen - 8); charPtr--) {
    theBuffer[charPtr] = base36Table[theValue % 36];
    if (theValue < 36) {
      break;
    }
    theValue = theValue / 36;
  }
}

/* -----------------------------------------------------------------------------
 * Convert a text HEX character rep into actual binary data
 * If there is an error in the data, NULL is returned */
static char * 
textToBinary (char * theText, int *binaryLength) {
  int theLength = strlen (theText);
  char * theData = (char *) malloc (theLength / 2);
  char * dataStr = theData;
  int theValue;
  int charPtr;

  for (charPtr = 0; charPtr < theLength;) {
    /* Convert hex into a value */
    if (!xPL_hexToInt (&theText[charPtr], &theValue)) {
      free (theData);
      return NULL;
    }

    /* Store that value & advance */
    *dataStr++ = (theValue & 255);
    charPtr += 2;
  }

  /* Return the data */
  return theData;
}

/* -----------------------------------------------------------------------------
 * Parse the name/value pairs for this message.  
 * If they are all found and valid, then we return TRUE.  Otherwise, FALSE.
 */
static bool 
parseMessageHeader (xPL_Message * theMessage, xPL_NameValueList * nameValueList) {
  int hopCount;
  char * dashstr = NULL;
  char * periodstr = NULL;
  xPL_NameValuePair * theNameValue;
  char * theVendor;
  char * theDeviceID;
  char * theInstanceID;
  char groupNameBuffer[40];

  /* Parse the hop count */
  if ( (theNameValue = xPL_getNamedValuePair (nameValueList, "HOP")) == NULL) {
    xPL_Debug ("Message missing HOP count");
    return FALSE;
  }
  if (!xPL_strToInt (theNameValue->itemValue, &hopCount) || (hopCount < 1)) {
    xPL_Debug ("Message HOP Count invalid");
    return FALSE;
  }
  theMessage->hopCount = hopCount;

  /* Parse the source */
  if ( (theNameValue = xPL_getNamedValuePair (nameValueList, "SOURCE")) == NULL) {
    xPL_Debug ("Message missing SOURCE");
    return FALSE;
  }
  theVendor = theNameValue->itemValue;
  if ( (theDeviceID = strchr (theVendor, '-')) == NULL) {
    xPL_Debug ("SOURCE Missing Device ID - %s", theVendor);
    return FALSE;
  }

  dashstr = theDeviceID;
  *theDeviceID++ = '\0';
  if ( (theInstanceID = strchr (theDeviceID, '.')) == NULL) {
    xPL_Debug ("SOURCE Missing Instance ID - %s.%s", theVendor, theDeviceID);
    return FALSE;
  }

  periodstr = theInstanceID;
  *theInstanceID++ = '\0';

  /* Install source into message */
  xPL_setSource (theMessage, theVendor, theDeviceID, theInstanceID);

  /* Fix mangled string */
  if (dashstr != NULL) {
    *dashstr = '-';
  }
  if (periodstr != NULL) {
    *periodstr = '.';
  }


  /* Parse the target (if anything) */
  if ( (theNameValue = xPL_getNamedValuePair (nameValueList, "TARGET")) == NULL) {
    xPL_Debug ("Message missing TARGET");
    return FALSE;
  }

  /* Parse the target */
  dashstr = NULL;
  periodstr = NULL;

  /* Check for a wildcard */
  if (!strcmp (theNameValue->itemValue, "*")) {
    xPL_setBroadcastMessage (theMessage, TRUE);
  }
  else {
    /* Parse vendor and such */
    theVendor = theNameValue->itemValue;
    if ( (theDeviceID = strchr (theVendor, '-')) == NULL) {
      xPL_Debug ("TARGET Missing Device ID - %s", theVendor);
      return FALSE;
    }

    dashstr = theDeviceID;
    *theDeviceID++ = '\0';
    if ( (theInstanceID = strchr (theDeviceID, '.')) == NULL) {
      xPL_Debug ("TARGET Missing Instance ID - %s.%s", theVendor, theDeviceID);
      return FALSE;
    }

    periodstr = theInstanceID;
    *theInstanceID++ = '\0';

    /* See if this was a group message */
    if ( (xPL_strcmpIgnoreCase (theVendor, "XPL") == 0) && (xPL_strcmpIgnoreCase (theDeviceID, "GROUP") == 0)) {
      strcpy (groupNameBuffer, "XPL-GROUP.");
      strncat (groupNameBuffer, theInstanceID, 16);
      groupNameBuffer[16] = '\0';
      xPL_setTargetGroup (theMessage, groupNameBuffer);
    }
    else {
      xPL_setTarget (theMessage, theVendor, theDeviceID, theInstanceID);
    }

    /* Fix mangled string */
    if (dashstr != NULL) {
      *dashstr = '-';
    }
    if (periodstr != NULL) {
      *periodstr = '.';
    }
  }

  /* Header parsed OK */
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Parse data until end of block as a block.  If the block is valid, then the number of bytes
 * parsed is returned.  If there is an error, a negated number of bytes read thus far is
 * returned (ABS of this number points to the failing character)
 * If we run out of bytes before we start a new block, it's likely end of stream garbage and
 * we return 0 (which means parsing this message is done) */
static int 
parseBlock (char * theText, char * *blockHeader, xPL_NameValueList * nameList, bool forceUpperCase) {
  int curState = 0, curIndex, theLength = strlen (theText);
  char theChar;
  char * headerBuff = blockHeaderBuff;
  char * nameBuff = NULL;
  char * valueBuff = NULL;
  bool isBinaryValue = FALSE, blockStarted = FALSE;
  xPL_NameValuePair * theNameValue;

  /* Parse character by character */
  for (curIndex = 0; curIndex < theLength; curIndex++) {
    theChar = theText[curIndex];

    /* Convert identifiers to upper case */
    /* if (((curState != 4) || forceUpperCase) && (theChar >= 97) && (theChar <= 122)) theChar -= 32; */
    if (forceUpperCase && (theChar >= 97) && (theChar <= 122)) {
      theChar -= 32;
    }

    switch (curState) {
      case 0:
        /* Handle an LF transition */
        if ( (theChar == '\n') && blockStarted) {
          *headerBuff = '\0';
          curState = 1;
          continue;
        }

        /* Handle leading junk chars */
        if (!blockStarted && (theChar <= 32)) {
          continue;
        }

        /* Handle known good characters */
        if (VALID_CHAR (theChar)) {
          /* Handle normal letters */
          blockStarted = TRUE;
          *headerBuff++ = theChar;
          continue;
        }

        /* Handle error */
        xPL_Debug ("Got invalid character parsing block header - %c at position %d", theChar, curIndex);
        return -curIndex;

      case 1:
        /* Advance */
        if (theChar == '{') {
          curState = 2;
          continue;
        }

        /* Crapola */
        xPL_Debug ("Got invalid character parsing start of block - %c at position %d (wanted a {)", theChar, curIndex);
        return -curIndex;


      case 2:
        /* Advance */
        if (theChar == '\n') {
          curState = 3;
          nameBuff = blockNameBuff;
          continue;
        }

        /* Crapola */
        xPL_Debug ("Got invalid character parsing start of block -  %c at position %d (wanted a LF)", theChar, curIndex);
        return -curIndex;

      case 3:
        /* Handle end of name */
        if (theChar == '=') {
          *nameBuff = '\0';
          isBinaryValue = FALSE;
          valueBuff = blockValueBuff;
          curState = 4;
          continue;
        }

        /* Handle end of binary name */
        if (theChar == '!') {
          *nameBuff = '\0';
          isBinaryValue = TRUE;
          valueBuff = blockValueBuff;
          curState = 4;
          continue;
        }

        /* Handle end of block */
        if (theChar == '}') {
          curState = 5;
          continue;
        }

        /* Handle normal chars */
        if (VALID_CHAR (theChar)) {
          /* Buffer Name */
          *nameBuff++ = theChar;
          continue;
        }

        /* Bad chararters! */
        xPL_Debug ("Got invalid character parsing block name/value name -  %c at position %d", theChar, curIndex);
        return -curIndex;

      case 4:
        /* Handle end of line */
        if (theChar == '\n') {
          *valueBuff = '\0';

          /* Save off name/value pair */
          theNameValue = xPL_newNamedValuePair (nameList, blockNameBuff);
          theNameValue->isBinary = isBinaryValue;
          if (!isBinaryValue) {
            theNameValue->itemValue = xPL_StrDup (blockValueBuff);
          }
          else {
            if ( (theNameValue->itemValue = textToBinary (blockValueBuff, &theNameValue->binaryLength)) == NULL) {
              xPL_Debug ("Unable to xlate binary value for name %s", blockValueBuff);
              return -curIndex;
            }
          }

          /* Reset things */
          curState = 3;
          nameBuff = blockNameBuff;
          continue;
        }

        /* Handle normal characters */
        if (VALID_CHAR (theChar) || (theChar == 32)) {
          /* Buffer char */
          *valueBuff++ = theChar;
          continue;
        }

        /* Bad character! */
        xPL_Debug ("Got invalid character parsing name/value value -  %c at position %d", theChar, curIndex);
        return -curIndex;

      case 5:
        /* Should be an EOL - we are done if so */
        if (theChar == '\n') {
          /* Copy off block header */
          *blockHeader = xPL_StrDup (blockHeaderBuff);

          /* And we are done */
          return curIndex + 1;
        }

        /* Bad data */
        xPL_Debug ("Got invalid character parsing end of name/value -  %c at position %d (wanted a LF)", theChar, curIndex);
        return -curIndex;
    }
    break;
  }

  /* If we didn't start a block, then it's just end of the stream */
  if (!blockStarted) {
    return 0;
  }

  /* If we got here, we ran out of characters - this is an error too */
  xPL_Debug ("Ran out of characters parsing block");
  return -theLength;
}

/* -----------------------------------------------------------------------------
 * Create a new message based on a service */
static xPL_Message * 
createReceivedMessage (xPL_MessageType messageType) {
  xPL_Message * theMessage;

  /* Allocate the message */
  theMessage = xPL_AllocMessage();

  /* Set the version (NOT DYNAMIC) */
  theMessage->messageType = messageType;
  theMessage->receivedMessage = TRUE;

  /* Allocate a name/value list, if needed */
  if (theMessage->messageBody == NULL) {
    theMessage->messageBody = xPL_AllocNVList();
  }

  /* And we are done */
  return theMessage;
}

/* -----------------------------------------------------------------------------
 * Convert a text message into a xPL message.  
 * Return the message or NULL if there is a parse error
 */
static xPL_Message * 
parseMessage (char * theText) {
  int textLen = strlen (theText);
  int parsedChars, parsedThisTime;
  char * blockHeaderKeyword;
  char * blockDelimStr;
  char * periodstr = NULL;
  xPL_Message * theMessage;

  /* Allocate a message */
  theMessage = createReceivedMessage (xPL_MESSAGE_ANY);

  /* Parse the header */
  if ( (parsedThisTime = parseBlock (theText, &blockHeaderKeyword, theMessage->messageBody, FALSE)) <= 0) {
    xPL_Debug ("Error parsing message header");
    xPL_releaseMessage (theMessage);
    return NULL;
  }
  parsedChars = parsedThisTime;

  /* Parse the header */
  if (!xPL_strcmpIgnoreCase (blockHeaderKeyword, "XPL-CMND")) {
    xPL_setMessageType (theMessage, xPL_MESSAGE_COMMAND);
  }
  else if (!xPL_strcmpIgnoreCase (blockHeaderKeyword, "XPL-STAT")) {
    xPL_setMessageType (theMessage, xPL_MESSAGE_STATUS);
  }
  else if (!xPL_strcmpIgnoreCase (blockHeaderKeyword, "XPL-TRIG")) {
    xPL_setMessageType (theMessage, xPL_MESSAGE_TRIGGER);
  }
  else {
    xPL_Debug ("Unknown message header of %s - bad message", blockHeaderKeyword);
    STR_FREE (blockHeaderKeyword);
    xPL_releaseMessage (theMessage);
    return NULL;
  }

  /* We are done with this now - drop it while we are still thinking about it */
  STR_FREE (blockHeaderKeyword);

  /* Parse the name/values into the message */
  if (!parseMessageHeader (theMessage, theMessage->messageBody)) {
    xPL_Debug ("Unable to parse message header");
    xPL_releaseMessage (theMessage);
    return NULL;
  }

  /* Parse multiple blocks until we are done */
  for (; parsedChars < textLen;) {
    /* Clear the name/value list for the message */
    xPL_clearAllNamedValues (theMessage->messageBody);
    periodstr = NULL;

    /* Parse the next block */
    if ( (parsedThisTime = parseBlock (& (theText[parsedChars]), &blockHeaderKeyword, theMessage->messageBody, FALSE)) < 0) {
      xPL_Debug ("Error parsing message block");
      xPL_releaseMessage (theMessage);
      STR_FREE (blockHeaderKeyword);
      return NULL;
    }

    /* If we ran out of characters, no more blocks */
    if (parsedThisTime == 0) {
      break;
    }

    /* Up Parsed count */
    parsedChars += parsedThisTime;

    /* Parse the block header */
    if ( (blockDelimStr = strchr (blockHeaderKeyword, '.')) == NULL) {
      xPL_Debug ("Malformed message block header - %s", blockHeaderKeyword);
      xPL_releaseMessage (theMessage);
      STR_FREE (blockHeaderKeyword);
      return NULL;
    }
    periodstr = blockDelimStr;
    *blockDelimStr++ = '\0';

    /* Record the message schema class/type */
    xPL_setSchemaClass (theMessage, blockHeaderKeyword);
    xPL_setSchemaType (theMessage, blockDelimStr);

    /* Fix mangled string & release string */
    if (periodstr != NULL) {
      *periodstr = '.';
    }
    STR_FREE (blockHeaderKeyword);
    break;
  }

  /* Return the message */
  return theMessage;
}

/* -----------------------------------------------------------------------------
 * Check to see if the passed message is a hub echo.  */
static bool 
isHubEcho (xPL_Message * theMessage) {
  char * remoteIP;
  char * thePort;

  if (theMessage == NULL) {
    return FALSE;
  }

  /* If this is not a heartbeat, ignore it */
  if (! (!xPL_strcmpIgnoreCase (theMessage->schemaClass, "hbeat")
         || !xPL_strcmpIgnoreCase (theMessage->schemaClass, "config"))) {
    return FALSE;
  }

  /* Insure it has an IP address and port */
  if ( (remoteIP = xPL_getMessageNamedValue (theMessage, "remote-ip")) == NULL) {
    return FALSE;
  }
  if ( (thePort = xPL_getMessageNamedValue (theMessage, "port")) == NULL) {
    return FALSE;
  }

  /* Now See if the IP address & port matches ours */
  if (strcmp (remoteIP, xPL_getListenerIPAddr())) {
    return FALSE;
  }
  if (strcmp (thePort, xPL_intToStr (xPL_getPort()))) {
    return FALSE;
  }

  /* Clearly this is a message from us */
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Read, parse and dispatch an xPL message
 */
static void xPL_receiveMessage (int theFD, int thePollInfo, int userValue) {
  int bytesRead;
  xPL_Message * theMessage = NULL;

  for (;;) {
    /* Fetch the next message, if any */
    if ( (bytesRead = recvfrom (xPLFD, &messageBuff, CONFIG_MSG_BUFF_SIZE - 1, 0, NULL, NULL)) < 0) {
      /* Expected response when queue is empty */
      if (errno == EAGAIN) {
        return;
      }

      /* Note the error and bail */
      xPL_Debug ("Error reading xPL message from network - %s (%d)", strerror (errno), errno);
      return;
    }

    /* We receive a message - clean it up */
    messageBuff[bytesRead] = '\0';
    xPL_Debug ("Just read %d bytes as packet [%s]", bytesRead, messageBuff);

    /* Send the raw message to any raw message listeners */
    xPL_dispatchRawEvent (messageBuff, bytesRead);

    /* Parse the message */
    if ( (theMessage = parseMessage (messageBuff)) == NULL) {
      xPL_Debug ("Error parsing network message - ignored");
      continue;
    }

    /* See if we need to check the message for hub detection */
    if (!hubConfirmed && isHubEcho (theMessage)) {
      xPL_Debug ("Hub detected and confirmed existing");
      hubConfirmed = TRUE;
    }

    /* Dispatch the message */
    xPL_Debug ("Now dispatching valid message");
    xPL_dispatchMessageEvent (theMessage);

    /* Release the message */
    xPL_releaseMessage (theMessage);
  }
}

/* -----------------------------------------------------------------------------
 * Create a reasonably uniqe 16 character identifier */
char * xPL_getFairlyUniqueIdent (void) {
  char newIdent[32];
  uint32_t localAddr;
  struct timeval rightNow;
  unsigned long timeInMillis;

  /* First time around, format the prefix ident */
  if (*uniqueIDPrefix == '\0') {
    localAddr = htonl (xPLInterfaceAddr.s_addr);
    strcpy (uniqueIDPrefix, xPL_intToHex ( (localAddr >> 24) & 255));
    strcat (uniqueIDPrefix, xPL_intToHex ( (localAddr >> 16) & 255));
    strcat (uniqueIDPrefix, xPL_intToHex ( (localAddr >> 8) & 255));
    strcat (uniqueIDPrefix, xPL_intToHex (localAddr & 255));
  }

  /* Now tack on the time of day, radix-32 encoded (which allows   */
  /* packing in a lot more uniqueness for the 8 characters we have */
  gettimeofday (&rightNow, NULL);
  timeInMillis = (rightNow.tv_sec * 1000) + (rightNow.tv_usec / 1000);
  strcpy (newIdent, uniqueIDPrefix);
  longToBase32 (timeInMillis, newIdent);
  if (strlen (newIdent) > 16) {
    newIdent[16] = '\0';
  }

  /* Pass a copy off */
  return xPL_StrDup (newIdent);
}


/* -----------------------------------------------------------------------------
 * Make a fd non-blocking */
static bool markNonblocking (int thefd) {
  int theValue;

  if ( (theValue = fcntl (thefd, F_GETFL, 0)) != -1) {
    return (fcntl (thefd, F_SETFL, theValue | O_NONBLOCK) != -1);
  }
  else {
    return FALSE;
  }
}

/* -----------------------------------------------------------------------------
 * Try to increase the receive buffer as big as possible.  if 
 * we make it bigger, return TRUE.  Otherwise, if no change, FALSE */
static bool maximizeReceiveBufferSize (int thefd) {
  int startRcvBuffSize, idealRcvBuffSize, finalRcvBuffSize;
  socklen_t buffLen = sizeof (int);

  /* Get current receive buffer size */
  if (getsockopt (thefd, SOL_SOCKET, SO_RCVBUF, &startRcvBuffSize, &buffLen) != 0) {
    xPL_Debug ("Unable to read receive socket buffer size - %s (%d)", strerror (errno), errno);
  }
  else {
    xPL_Debug ("Initial receive socket buffer size is %d bytes", startRcvBuffSize);
  }

  /* Try to increase the buffer (maybe multiple times) */
  for (idealRcvBuffSize = 1024000; idealRcvBuffSize > startRcvBuffSize;) {
    /* Attempt to set the buffer size */
    if (setsockopt (thefd, SOL_SOCKET, SO_RCVBUF, &idealRcvBuffSize, sizeof (int)) != 0) {
      xPL_Debug ("Not able to set receive buffer to %d bytes - retrying", idealRcvBuffSize);
      idealRcvBuffSize -= 64000;
      continue;
    }

    /* We did it!  Get the current size and bail out */
    buffLen = sizeof (int);
    if (getsockopt (thefd, SOL_SOCKET, SO_RCVBUF, &finalRcvBuffSize, &buffLen) != 0) {
      xPL_Debug ("Unable to read receive socket buffer size - %s (%d)", strerror (errno), errno);
    }
    else {
      xPL_Debug ("Actual receive socket buffer size is %d bytes", finalRcvBuffSize);
    }

    return (finalRcvBuffSize > startRcvBuffSize);
  }

  /* We weren't able to increase it */
  xPL_Debug ("Unable to increase receive buffer size - dang!");
  return FALSE;
}

/* -----------------------------------------------------------------------------
 * Attempt to make a standalone connection */
static bool attemptStandaloneConnection (void) {
  int sockfd;
  int flag = 1;
  struct protoent *ppe;
  struct sockaddr_in theSockInfo;

  /* Init the socket definition */
  bzero (&theSockInfo, sizeof (theSockInfo));
  theSockInfo.sin_family = AF_INET;
  theSockInfo.sin_addr.s_addr = INADDR_ANY;
  theSockInfo.sin_port = htons (xPLPort);

  /* Map protocol name */
  if ( (ppe = getprotobyname ("udp")) == 0) {
    xPL_Debug ("Unable to lookup UDP protocol info");
    return FALSE;
  }

  /* Attempt to creat the socket */
  if ( (sockfd = socket (PF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {
    xPL_Debug ("Unable to create listener socket %s (%d)", strerror (errno), errno);
    return FALSE;
  }

  /* Allow re-use and restart */
  if (setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof (flag)) < 0) {
    xPL_Debug ("Unable to set SO_REUSEADDR on socket %s (%d)", strerror (errno), errno);
    close (sockfd);
    return FALSE;
  }

  /* Mark as a broadcast socket */
  if (setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof (flag)) < 0) {
    xPL_Debug ("Unable to set SO_BROADCAST on socket %s (%d)", strerror (errno), errno);
    close (sockfd);
    return FALSE;
  }

  /* Attempt to bind */
  if (bind (sockfd, (struct sockaddr *) &theSockInfo, sizeof (theSockInfo)) < 0) {
    xPL_Debug ("Unable to bind listener socket to port %d, %s (%d)", xPLPort, strerror (errno), errno);
    close (sockfd);
    return FALSE;
  }

  /* We are ready to go */
  xPLFD = sockfd;
  markNonblocking (xPLFD);
  maximizeReceiveBufferSize (xPLFD);
  xPL_Debug ("xPL Started in standalone mode");
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Attempt to make a hub based connection */
static bool attempHubConnection (void) {
  int sockfd;
  int flag = 1;
  int sockSize = sizeof (struct sockaddr_in);
  struct protoent *ppe;
  struct sockaddr_in theSockInfo;

  /* Init the socket definition */
  bzero (&theSockInfo, sizeof (theSockInfo));
  theSockInfo.sin_family = AF_INET;
  theSockInfo.sin_addr.s_addr = INADDR_ANY;
  theSockInfo.sin_port = htons (0);

  /* Map protocol name */
  if ( (ppe = getprotobyname ("udp")) == 0) {
    xPL_Debug ("Unable to lookup UDP protocol info");
    return FALSE;
  }

  /* Attempt to creat the socket */
  if ( (sockfd = socket (PF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {
    xPL_Debug ("Unable to create listener socket %s (%d)", strerror (errno), errno);
    return FALSE;
  }

  /* Mark as a broadcast socket */
  if (setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof (flag)) < 0) {
    xPL_Debug ("Unable to set SO_BROADCAST on socket %s (%d)", strerror (errno), errno);
    close (sockfd);
    return FALSE;
  }

  /* Attempt to bind */
  if ( (bind (sockfd, (struct sockaddr *) &theSockInfo, sockSize)) < 0) {
    xPL_Debug ("Unable to bind listener socket to port %d, %s (%d)", xPLPort, strerror (errno), errno);
    close (sockfd);
    return FALSE;
  }

  /* Fetch the actual socket port # */
  if (getsockname (sockfd, (struct sockaddr *) &theSockInfo, (socklen_t *) &sockSize)) {
    xPL_Debug ("Unable to fetch socket info for bound listener, %s (%d)", strerror (errno), errno);
    close (sockfd);
    return FALSE;
  }

  /* We are ready to go */
  xPLFD = sockfd;
  xPLPort = ntohs (theSockInfo.sin_port);
  markNonblocking (xPLFD);
  maximizeReceiveBufferSize (xPLFD);
  xPL_Debug ("xPL Starting in Hub mode on port %d", xPLPort);
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Figure out what sort of connection to make and do it */
static bool makeConnection (xPL_ConnectType theConnectType) {
  /* Try an stand along connection */
  if ( (theConnectType == xcStandAlone) || (theConnectType == xcAuto)) {
    /* Attempt the connection */
    xPL_Debug ("Attemping standalone xPL");
    if (attemptStandaloneConnection()) {
      xPLconnectType = xcStandAlone;
      return TRUE;
    }

    /* If we failed and this what we want, bomb out */
    xPL_Debug ("Standalong connect failed - %d %d", theConnectType, xcStandAlone);
    if (theConnectType == xcStandAlone) {
      return FALSE;
    }
  }

  /* Try a hub based connection */
  xPL_Debug ("Attempting hub based xPL");
  if (!attempHubConnection()) {
    return FALSE;
  }
  xPLconnectType = xcViaHub;
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * When no interface is selected, scan the list of interface
 * and choose the first one that looks OK.  If nothing is   
 * found, then return FALSE.  Otherwise, install the name as
 * the xPL interface and return TRUE */                        
static bool findDefaultInterface (int sockfd) {
  char intBuff[1024];
  struct ifconf ifc;
  struct ifreq *ifr;
  struct ifreq interfaceInfo;
  int intIndex;

  /* Request list of intefaces */
  ifc.ifc_len = sizeof (intBuff);
  ifc.ifc_buf = intBuff;
  if (ioctl (sockfd, SIOCGIFCONF, &ifc) < 0) {
    return FALSE;
  }

  /* Try each interface until one works */
  ifr = ifc.ifc_req;
  for (intIndex = 0; intIndex < ifc.ifc_len / sizeof (struct ifreq); intIndex++) {
    /* Init the interface info request */
    bzero (&interfaceInfo, sizeof (struct ifreq));
    interfaceInfo.ifr_addr.sa_family = AF_INET;
    strcpy (interfaceInfo.ifr_name, ifr[intIndex].ifr_name);

    /* Get device flags */
    if (ioctl (sockfd, SIOCGIFFLAGS, &interfaceInfo) != 0) {
      continue;
    }

    xPL_Debug ("Checking if interface %s is valid w/flags %d", interfaceInfo.ifr_name, interfaceInfo.ifr_flags);

    /* Insure this interface is active and not loopback */
    if ( (interfaceInfo.ifr_flags & IFF_UP) == 0) {
      continue;
    }
    if ( (interfaceInfo.ifr_flags & IFF_LOOPBACK) != 0) {
      continue;
    }

    /* If successful, use this interface */
    strcpy (xPLInterfaceName, ifr[intIndex].ifr_name);
    xPL_Debug ("Choose interface %s as default interface", xPLInterfaceName);
    return TRUE;
  }

  /* No good interface found */
  return FALSE;
}


/* -----------------------------------------------------------------------------
 * Create a socket for broadcasting messages */
static bool setupBroadcastAddr (void) {
  int sockfd;
  int flag = 1;
  struct protoent *ppe;
  struct ifreq interfaceInfo;
  struct in_addr interfaceNetmask;

  /* Map protocol name */
  if ( (ppe = getprotobyname ("udp")) == 0) {
    xPL_Error ("Unable to lookup UDP protocol info");
    return FALSE;
  }

  /* Attempt to create a socket */
  if ( (sockfd = socket (AF_INET, SOCK_DGRAM, ppe->p_proto)) < 0) {
    xPL_Error ("Unable to create broadcast socket %s (%d)", strerror (errno), errno);
    return FALSE;
  }

  /* Mark as a broadcasting socket */
  if (setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof (flag)) < 0) {
    xPL_Error ("Unable to set SO_BROADCAST on socket %s (%d)", strerror (errno), errno);
    close (sockfd);
    return FALSE;
  }

  /* See if we need to find a default interface */
  if ( (xPLInterfaceName == NULL) || (strlen (xPLInterfaceName) == 0)) {
    if (!findDefaultInterface (sockfd)) {
      xPL_Error ("Could not find a working, non-loopback network interface");
      close (sockfd);
      return FALSE;
    }
  }

  /* Init the interface info request */
  bzero (&interfaceInfo, sizeof (struct ifreq));
  interfaceInfo.ifr_addr.sa_family = AF_INET;
  strcpy (interfaceInfo.ifr_name, xPLInterfaceName);

  /* Get our interface address */
  if (ioctl (sockfd, SIOCGIFADDR, &interfaceInfo) != 0) {
    xPL_Error ("Unable to get IP addr for interface %s", xPLInterfaceName);
    close (sockfd);
    return FALSE;
  }
  xPLInterfaceAddr.s_addr = ( (struct sockaddr_in *) &interfaceInfo.ifr_addr)->sin_addr.s_addr;
  xPL_Debug ("Auto-assigning IP address of %s", inet_ntoa (xPLInterfaceAddr));

  /* Get interface netmask */
  bzero (&interfaceInfo, sizeof (struct ifreq));
  interfaceInfo.ifr_addr.sa_family = AF_INET;
  interfaceInfo.ifr_broadaddr.sa_family = AF_INET;
  strcpy (interfaceInfo.ifr_name, xPLInterfaceName);
  if (ioctl (sockfd, SIOCGIFNETMASK, &interfaceInfo) != 0) {
    xPL_Error ("Unable to extract the interface net mask");
    close (sockfd);
    return FALSE;
  }
  interfaceNetmask.s_addr = ( (struct sockaddr_in *) &interfaceInfo.ifr_netmask)->sin_addr.s_addr;

  /* Build our broadcast addr */
  bzero (&xPLBroadcastAddr, sizeof (xPLBroadcastAddr));
  xPLBroadcastAddr.sin_family = AF_INET;
  xPLBroadcastAddr.sin_addr.s_addr = xPLInterfaceAddr.s_addr | ~interfaceNetmask.s_addr;
  xPLBroadcastAddr.sin_port = htons (XPL_PORT);
  xPLBroadcastFD = sockfd;
  markNonblocking (sockfd);

  /* And we are done */
  xPL_Debug ("Assigned xPL Broadcast address of %s, port %d", inet_ntoa (xPLBroadcastAddr.sin_addr), XPL_PORT);
  return TRUE;
}


/* -----------------------------------------------------------------------------
 * Check for any registered timeouts */
static void 
checkAllTimeoutHandlers (void) {
  int timeoutIndex;
  int elapsedTime;
  time_t rightNow = time (NULL);

  for (timeoutIndex = timeoutCount - 1; timeoutIndex >= 0; timeoutIndex--) {
    elapsedTime = rightNow - timeoutList[timeoutIndex].lastTimedoutAt;
    if (elapsedTime >= timeoutList[timeoutIndex].timeoutInterval) {
      timeoutList[timeoutIndex].timeoutHandler (elapsedTime, timeoutList[timeoutIndex].userValue);
      timeoutList[timeoutIndex].lastTimedoutAt = rightNow;
    }
  }
}

/* -----------------------------------------------------------------------------
 * Timeout processing - a-periodic processing work */
static void 
doTimeouts (void) {
  time_t rightNow = time (NULL);

  /* See if it's time to do heartbeats */
  if ( (heartbeatCheckLastDoneAt == -1) || ( (rightNow - heartbeatCheckLastDoneAt) >= 5) || !xPL_isHubConfirmed()) {
    /* Send Heartbeats out (if needed) */
    xPL_sendTimelyHeartbeats();

    /* Flag when we did this last */
    heartbeatCheckLastDoneAt = rightNow;
  }

  /* Do timeout checks */
  if ( (timeoutChecksLastDoneAt == -1) || ( (rightNow - timeoutChecksLastDoneAt) >= 1)) {
    checkAllTimeoutHandlers();
    timeoutChecksLastDoneAt = rightNow;
  }
}

/* public functions ========================================================= */

/* -----------------------------------------------------------------------------
 * Public: Not used
 * Add an IO channel to monitor/dispatch to.  theFD is the FD that is
 * open and should be monitored.  ioHandler is the routine that is   
 * is called when there is activity on the channel.  userValue is an 
 * integer passed directly to the ioHandler, frequently used to track
 * context information. watchRead, watchWrite, watchError tell what  
 * sort of things need to be monitored. */                              
bool 
xPL_addIODevice (xPL_IOHandler theIOHandler, int userValue, int theFD, 
bool watchRead, bool watchWrite, bool watchError) {
  /* Make sure they are going to really do something */
  if (!watchRead && !watchWrite && !watchError) {
    return FALSE;
  }

  /* See if we need to allocate more space */
  if (pollInfoCount == pollInfoSize) {
    pollInfoSize += POLL_GROW_BY;
    pollInfo = (struct pollfd *) realloc (pollInfo, sizeof (struct pollfd) * pollInfoSize);
    pollUserInfo = (struct _pollUserInfo *) realloc (pollUserInfo, sizeof (struct _pollUserInfo) * pollInfoSize);
  }

  /* Install this */
  pollInfo[pollInfoCount].fd = theFD;
  pollInfo[pollInfoCount].events = 0;
  if (watchRead) {
    pollInfo[pollInfoCount].events |= POLLIN;
  }
  if (watchWrite) {
    pollInfo[pollInfoCount].events |= POLLOUT;
  }
  if (watchError) {
    pollInfo[pollInfoCount].events |= POLLERR;
  }
  pollUserInfo[pollInfoCount].ioHandler = theIOHandler;
  pollUserInfo[pollInfoCount].userValue = userValue;
  pollInfoCount++;

  xPL_Debug ("Added managed IO device, now %d devices", pollInfoCount);
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 * Remove an IO channel based on the passed fd.  If the
 * fd exists, it's removed and TRUE is returned.  If   
 * the fd doesn't exist, FALSE is returned. */           
bool 
xPL_removeIODevice (int theFD) {
  int infoIndex;

  /* Find the fd in the list */
  for (infoIndex = 0; infoIndex < pollInfoCount; infoIndex++) {
    /* Skip if this isn't the fd we care about */
    if (pollInfo[infoIndex].fd != theFD) {
      continue;
    }

    /* Remove the item and shuffle everything down */
    pollInfoCount--;
    if (infoIndex < pollInfoCount) {
      memcpy (&pollInfo[infoIndex], &pollInfo[infoIndex + 1], sizeof (struct pollfd) * (pollInfoCount - infoIndex));
      memcpy (&pollUserInfo[infoIndex], &pollUserInfo[infoIndex + 1], sizeof (struct _pollUserInfo) * (pollInfoCount - infoIndex));
    }

    xPL_Debug ("Removed managed IO device, now %d devices", pollInfoCount);
    return TRUE;
  }

  /* Never found it */
  xPL_Debug ("Unable to remove managed IO device, fd=%d not found", theFD);
  return FALSE;
}

/* -----------------------------------------------------------------------------
 * Public
 * Allocate a new timeout handler and install it into the list */
void 
xPL_addTimeoutHandler (xPL_TimeoutHandler timeoutHandler, int timeoutInSeconds, xPL_Object * userValue) {
  TimeoutHandler * theHandler;

  /* Allocate a handler */
  if (timeoutCount == timeoutAllocCount) {
    timeoutAllocCount += GROW_TIMEOUT_LIST_BY;
    timeoutList = (TimeoutHandler *) realloc (timeoutList, sizeof (TimeoutHandler) * timeoutAllocCount);
  }
  theHandler = &timeoutList[timeoutCount++];
  bzero (theHandler, sizeof (TimeoutHandler));

  /* Install values */
  theHandler->timeoutInterval = timeoutInSeconds;
  theHandler->timeoutHandler = timeoutHandler;
  theHandler->userValue = userValue;
  theHandler->lastTimedoutAt = time (NULL);
}

/* -----------------------------------------------------------------------------
 * Public
 * Remove a previously allocated timeout handler */
bool 
xPL_removeTimeoutHandler (xPL_TimeoutHandler timeoutHandler) {
  int timeoutIndex;

  for (timeoutIndex = 0; timeoutIndex < timeoutCount; timeoutIndex++) {
    if (timeoutList[timeoutIndex].timeoutHandler != timeoutHandler) {
      continue;
    }

    /* Reduce count */
    timeoutCount--;
    if (timeoutIndex < timeoutCount) {
      memcpy (& (timeoutList[timeoutIndex]), & (timeoutList[timeoutIndex + 1]), (timeoutCount - timeoutIndex) * sizeof (TimeoutHandler));
    }

    return TRUE;
  }

  /* No Match */
  return FALSE;
}

/* -----------------------------------------------------------------------------
 * Public
 * Return the xPL FD */
int 
xPL_getFD (void) {
  return xPLFD;
}

/* -----------------------------------------------------------------------------
 * Public
 * Return the current connection type */
xPL_ConnectType 
xPL_getConnectType (void) {
  return xPLconnectType;
}

/* -----------------------------------------------------------------------------
 * Public
 * Get the connection port */
int 
xPL_getPort (void) {
  return xPLPort;
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 * Return IP address */
char * xPL_getBroadcastIPAddr (void) {
  return inet_ntoa (xPLBroadcastAddr.sin_addr);
}

/* -----------------------------------------------------------------------------
 * Public
 * Return listing IP address */
char * 
xPL_getListenerIPAddr (void) {
  return inet_ntoa (xPLInterfaceAddr);
}

/* -----------------------------------------------------------------------------
 * Public: Not used
 * Get the xPL Interface */
char * xPL_getBroadcastInterface (void) {
  return xPLInterfaceName;
}

/* -----------------------------------------------------------------------------
 * Public
 * Set the interface */
void 
xPL_setBroadcastInterface (char * newInterfaceName) {
  /* Can't change the interface after it's open */
  if (xPLFD != -1) {
    return;
  }

  strcpy (xPLInterfaceName, newInterfaceName);
}

/* -----------------------------------------------------------------------------
 * Public
 * Send the passed string and return TRUE if it appears it
 * or FALSE if there was an error */
bool 
xPL_sendRawMessage (char * theData, int dataLen) {
  int bytesSent;

  /* Try to send the message */
  if ( (bytesSent = sendto (xPLBroadcastFD, theData, dataLen, 0,
                            (struct sockaddr *) &xPLBroadcastAddr, sizeof (struct sockaddr_in))) != dataLen) {
    xPL_Debug ("Unable to broadcast message, %s (%d)", strerror (errno), errno);
    return FALSE;
  }
  xPL_Debug ("Broadcasted %d bytes (of %d attempted)", bytesSent, dataLen);

  /* Okey dokey then */
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Public
 * Attempt to start the xPL library.  If we are already "started"
 * then bail out */
bool 
xPL_initialize (xPL_ConnectType theConnectType) {
  /* If running, we have nothing to do */
  if (xPLFD != -1) {
    return FALSE;
  }

  /* Setup the broadcasting address */
  if (!setupBroadcastAddr()) {
    return FALSE;
  }

  /* Attempt to make our connection */
  if (!makeConnection (theConnectType)) {
    return FALSE;
  }

  /* Install our pid */
  xPLPID = getpid();

  /* Install a listener for xPL oriented messages */
  if (!xPL_IODeviceInstalled) {
    if (xPL_addIODevice (xPL_receiveMessage, -1, xPLFD, TRUE, FALSE, FALSE)) {
      xPL_IODeviceInstalled = TRUE;
    }
  }

  /* Add a message listener for services */
  xPL_addMessageListener (xPL_handleServiceMessage, NULL);

  /* We are ready to go */
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Public
 * Stop the xPL library.  If already stopped, bail.  Otherwise,
 * we close our connection, release any/all resources and reset */
bool 
xPL_shutdown (void) {
  /* If already stopped, bail */
  if (xPLFD == -1) {
    return FALSE;
  }

  /* Shutdown all services */
  xPL_disableAllServices();

  /* Remove xPL Listener */
  if (xPL_removeIODevice (xPLFD)) {
    xPL_IODeviceInstalled = FALSE;
  }

  /* Close the connection */
  close (xPLBroadcastFD);
  xPLBroadcastFD = -1;
  close (xPLFD);
  xPLFD = -1;

  /** Other Stuff here **/

  /* And we are done */
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Public
 * Process xPL messages and I/O.  If theTimeout is 0, then we
 * process any pending messages and then immediatly return.  
 * If theTimeout > 0 then we process any pending messages,   
 * waiting up to theTimeout milliseconds and then return.  If
 * theTimeout is -1, then we process messages and wait and do
 * not return until gxPLib is stopped.  In all cases, if at  
 * lease one xPL message was processed during the duration of
 * this call, TRUE is returned.  otherwise, FALSE. */          
bool 
xPL_processMessages (int theTimeout) {
  bool xPLMessageProcessed = FALSE;
  bool timeoutDone = FALSE;
  int activeDevices, deviceIndex;
  int thisTimeout;
  int timeoutsSoFar = 0;

  for (; xPLFD != -1;) {
    /* Compute the timeout */
    if (theTimeout > 0) {
      /* If the timeout is more than once per second, we cap this to */
      /* once per second and track how long we've waited to make up  */
      if (theTimeout > 1000) {
        if ( (timeoutsSoFar + 1000) > theTimeout) {
          thisTimeout = theTimeout - timeoutsSoFar;
        }
        else {
          thisTimeout = 1000;
        }
      }
      else {
        thisTimeout = theTimeout;
      }
    }
    else if (theTimeout == -1) {
      thisTimeout = 1000;
    }
    else {
      thisTimeout = 0;
    }

    /* Poll for an active device */
    SYSCALL (activeDevices = poll (pollInfo, pollInfoCount, thisTimeout));
    if (xPLFD == -1) {
      break;
    }

    /* Handle errors */
    if (activeDevices == -1) {
      /* Andything else is an error */
      xPL_Debug ("Error while polling devices - %s (%d) - terminating", strerror (errno), errno);
      return xPLMessageProcessed;
    }

    /* Handle a timeout */
    if (activeDevices == 0) {
      /* If the timer was <1000, then bail out */
      if (thisTimeout < 1000) {
        break;
      }

      /* Service timeouts */
      doTimeouts();
      timeoutDone = TRUE;
      timeoutsSoFar += thisTimeout;
      continue;
    }

    /* Clear timeouts so far */
    timeoutsSoFar = 0;

    /* Find devices that have something to say and dispatch to them */
    for (deviceIndex = pollInfoCount - 1; deviceIndex >= 0; deviceIndex--) {
      /* Skip unless something happened */
      if (pollInfo[deviceIndex].revents == 0) {
        continue;
      }

      /* Dispatch to IO handler */
      pollUserInfo[deviceIndex].ioHandler (pollInfo[deviceIndex].fd, pollInfo[deviceIndex].revents, pollUserInfo[deviceIndex].userValue);

      /* See if the gxPLib is stopped */
      if (xPLFD == -1) {
        break;
      }

      /* Knock down our active device count.  If we hit zero, we've */
      /* processed all known active devices this time around.       */
      if (! (--activeDevices)) {
        break;
      }
    }
  }

  /* Return final status */
  if (!timeoutDone) {
    doTimeouts();
  }
  return xPLMessageProcessed;
}

/* -----------------------------------------------------------------------------
 * Public
 */
bool xPL_isHubConfirmed (void) {
  return hubConfirmed;
}
/* ========================================================================== */
