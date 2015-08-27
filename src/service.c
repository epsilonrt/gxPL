/**
 * @file xPL-service.c
 * xPL Lib Service Management
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "service_p.h"
#include "message_p.h"
#include "io_p.h"


/* constants ================================================================ */
#define GROW_SERVICE_LIST_BY 8

/* private variables ======================================================== */
static int serviceCount = 0;
static int serviceAllocCount = 0;
static xPL_Service **serviceList = NULL;

/* public variables ========================================================= */
itemCache * ServiceCache = NULL;
int totalServiceAlloc = 0;

/* types ==================================================================== */
typedef enum {
  HBEAT_NORMAL,
  HBEAT_CONFIG,
  HBEAT_NORMAL_END,
  HBEAT_CONFIG_END
} HeartbeatType;

/* macros =================================================================== */
#define CONFIRM_CACHE_OK do { if (ServiceCache == NULL) ServiceCache = xPL_AllocItemCache(); } while(0)

/* static functions ========================================================= */

// -----------------------------------------------------------------------------
static xPL_Message *
createHeartbeatMessage (xPL_Service * theService, HeartbeatType heartbeatType) {
  xPL_Message * theHeartbeat;

  /* Create the Heartbeat message */
  theHeartbeat = xPL_createBroadcastMessage (theService, xPL_MESSAGE_STATUS);

  /* Configure the heartbeat */
  switch (heartbeatType) {
    case HBEAT_NORMAL:
      xPL_setSchemaClass (theHeartbeat, "hbeat");
      xPL_setSchemaType (theHeartbeat, "app");
      xPL_addMessageNamedValue (theHeartbeat, "interval", 
      xPL_intToStr (theService->heartbeatInterval / 60));
      break;

    case HBEAT_NORMAL_END:
      xPL_setSchemaClass (theHeartbeat, "hbeat");
      xPL_setSchemaType (theHeartbeat, "end");
      xPL_addMessageNamedValue (theHeartbeat, "interval", 
      xPL_intToStr (theService->heartbeatInterval / 60));
      break;

    case HBEAT_CONFIG:
      xPL_setSchemaClass (theHeartbeat, "config");
      xPL_setSchemaType (theHeartbeat, "app");
      xPL_addMessageNamedValue (theHeartbeat, "interval", 
      xPL_intToStr (CONFIG_HEARTBEAT_INTERVAL / 60));
      break;

    case HBEAT_CONFIG_END:
      xPL_setSchemaClass (theHeartbeat, "config");
      xPL_setSchemaType (theHeartbeat, "end");
      xPL_addMessageNamedValue (theHeartbeat, "interval", 
      xPL_intToStr (CONFIG_HEARTBEAT_INTERVAL / 60));
      break;

    default:
      return NULL;
  }

  /* Add standard heartbeat data */
  xPL_addMessageNamedValue (theHeartbeat, "port", xPL_intToStr (xPL_getPort()));
  xPL_addMessageNamedValue (theHeartbeat, "remote-ip", xPL_getListenerIPAddr());
  if (theService->serviceVersion) {
    xPL_addMessageNamedValue (theHeartbeat, "version", theService->serviceVersion);
  }
  return theHeartbeat;
}


// -----------------------------------------------------------------------------
static xPL_Service *
newService (char * theVendor, char * theDeviceID, char * theInstanceID) {
  bool tempInstanceID = FALSE;
  xPL_Service * theService;

  /* Create the new service */
  if (serviceCount == serviceAllocCount) {
    serviceAllocCount += GROW_SERVICE_LIST_BY;
    serviceList = realloc (serviceList, sizeof (xPL_Service *) * serviceAllocCount);

  }
  theService = xPL_AllocService();
  serviceList[serviceCount++] = theService;

  /* Create a default instance ID, if needed */
  if (theInstanceID == NULL) {
    theInstanceID = xPL_getFairlyUniqueIdent();
    tempInstanceID = TRUE;
  }

  /* Install info */
  xPL_setServiceVendor (theService, theVendor);
  xPL_setServiceDeviceID (theService, theDeviceID);
  xPL_setServiceInstanceID (theService, theInstanceID);
  xPL_setHeartbeatInterval (theService, DEFAULT_HEARTBEAT_INTERVAL);

  /* Release allocated temp space */
  if (tempInstanceID) {
    STR_FREE (theInstanceID);
  }

  /* And hand it up a level */
  return theService;
}


// -----------------------------------------------------------------------------
/* Return true if the passed message matches the passed filter */
static bool doesFilterMatch (xPL_ServiceFilter * theFilter, xPL_Message * theMessage) {
  /* If there is a message type filter, skip out if it does not match the message */
  if ( (theFilter->matchOnMessageType != xPL_MESSAGE_ANY) && 
  (theFilter->matchOnMessageType != theMessage->messageType)) {
    return FALSE;
  }

  /* If there is a vendor filter and it doesn't match the message, skip the message */
  if ( (theFilter->matchOnVendor != NULL) && 
  (xPL_strcmpIgnoreCase (theFilter->matchOnVendor, theMessage->sourceVendor) != 0)) {
    return FALSE;
  }

  /* If there is a device ID filter and it doesn't match the message, skip the message */
  if ( (theFilter->matchOnDeviceID != NULL) && 
  (xPL_strcmpIgnoreCase (theFilter->matchOnDeviceID, theMessage->sourceDeviceID) != 0)) {
    return FALSE;
  }

  /* If there is a instance ID filter and it doesn't match the message, skip the message */
  if ( (theFilter->matchOnInstanceID != NULL) && 
  (xPL_strcmpIgnoreCase (theFilter->matchOnInstanceID, theMessage->sourceInstanceID) != 0)) {
    return FALSE;
  }

  /* If there is a schema class filter and it doesn't match the message, skip the message */
  if ( (theFilter->matchOnSchemaClass != NULL) && 
  (xPL_strcmpIgnoreCase (theFilter->matchOnSchemaClass, theMessage->schemaClass) != 0)) {
    return FALSE;
  }

  /* If there is a schema type filter and it doesn't match the message, skip the message */
  if ( (theFilter->matchOnSchemaType != NULL) && 
  (xPL_strcmpIgnoreCase (theFilter->matchOnSchemaType, theMessage->schemaType) != 0)) {
    return FALSE;
  }

  /* Matches all selectors of this filter (if any) */
  return TRUE;
}

// -----------------------------------------------------------------------------
/* Process a message and see if it applies to this service */
static void handleMessage (xPL_Service * theService, xPL_Message * theMessage) {
  int filterIndex, groupIndex, responseDelay;
  bool foundGroup = FALSE, foundFilter = FALSE;

  /* If we are not reporting our own messages, see if they */
  /* originated with us and if so, dump it */
  if (!theService->reportOwnMessages) {
    if (!strcmp (theService->serviceVendor, theMessage->sourceVendor)
        && !strcmp (theService->serviceDeviceID, theMessage->sourceDeviceID)
        && !strcmp (theService->serviceInstanceID, theMessage->sourceInstanceID)) {
      xPL_Debug ("Skipping message from self");
      return;
    }
  }

  /* Is this a broadcast message? */
  if (theMessage->isBroadcastMessage) {
    /* See if this is a request for a heartbeat */
    if ( (theMessage->messageType == xPL_MESSAGE_COMMAND)
         && !xPL_strcmpIgnoreCase (theMessage->schemaClass, "hbeat")
         && !xPL_strcmpIgnoreCase (theMessage->schemaType, "request")) {

      /* Compute a response delay (.5 to 2.5 seconds) */
      responseDelay = (int) ( ( (double) random() / (double) RAND_MAX) * 2000.0) + 500;
      xPL_Debug ("Sending heartbeat in response to discovery request after a %d millisecond delay", responseDelay);
      usleep (responseDelay);
      xPL_sendHeartbeat (theService);
    }

    /* If we have filters, see if they match */
    if (theService->filterCount != 0) {
      for (filterIndex = 0; filterIndex < theService->filterCount; filterIndex++) {
        if (doesFilterMatch (& (theService->messageFilterList[filterIndex]), theMessage)) {
          foundFilter = TRUE;
          break;
        }
      }

      /* If we had filters and none match, skip this service */
      if (!foundFilter) {
        return;
      }
    }
  }
  else {
    if (theMessage->isGroupMessage) {
      /* See if we have any groups and if any groups match */
      for (groupIndex = 0; groupIndex < theService->groupCount; groupIndex++) {
        if (!xPL_strcmpIgnoreCase (theMessage->groupName, theService->groupList[groupIndex])) {
          foundGroup = TRUE;
          break;
        }
      }

      /* If there is no group or no matching group in this service, skip it */
      if (!foundGroup) {
        return;
      }
    }
    else {
      /* Make sure this target matches */
      if ( (xPL_strcmpIgnoreCase (theMessage->targetVendor, theService->serviceVendor) != 0)
           || (xPL_strcmpIgnoreCase (theMessage->targetDeviceID, theService->serviceDeviceID) != 0)
           || (xPL_strcmpIgnoreCase (theMessage->targetInstanceID, theService->serviceInstanceID) != 0)) {
        return;
      }
    }
  }

  /* Message is a broadcast (that matches filters, if any) OR a group message */
  /* that matches a group assigned to this service OR is targeted to this     */
  /* service so dispatch it!                                                  */
  xPL_dispatchServiceEvent (theService, theMessage);
}

/* public functions ========================================================= */

/* -----------------------------------------------------------------------------
 * Public
 * Return number of services */
int xPL_getServiceCount (void) {
  return serviceCount;
}

/* -----------------------------------------------------------------------------
 * Public
 * Return a service at a given index.  If the index is out of 
 * range, return NULL
 */
xPL_Service * xPL_getServiceAt (int serviceIndex) {
  if ( (serviceIndex < 0) || (serviceIndex >= serviceCount)) {
    return NULL;
  }
  return serviceList[serviceIndex];
}

/* -----------------------------------------------------------------------------
 * Public
 * Create a new xPL service */
xPL_Service * xPL_createService (char * theVendor, char * theDeviceID, char * theInstanceID) {
  xPL_Service * theService;

  /* Create the service */
  if ( (theService = newService (theVendor, theDeviceID, theInstanceID)) == NULL) {
    return NULL;
  }

  /* And we are done */
  return theService;
}

/* -----------------------------------------------------------------------------
 * Public
 * Release an xPL service */
void xPL_releaseService (xPL_Service * theService) {
  int ctxIndex;

  for (ctxIndex = 0; ctxIndex < serviceCount; ctxIndex++) {
    if (theService != serviceList[ctxIndex]) {
      continue;
    }

    /* Disable any heartbeats */
    xPL_setServiceEnabled (theService, FALSE);

    /* Release heartbeat message, if any */
    if (theService->heartbeatMessage != NULL) {
      xPL_releaseMessage (theService->heartbeatMessage);
    }

    /* Release service resources */
    STR_FREE (theService->serviceVendor);
    STR_FREE (theService->serviceDeviceID);
    STR_FREE (theService->serviceInstanceID);

    /* Release group info */
    if (theService->groupList != NULL) {
      xPL_clearServiceGroups (theService);
      free (theService->groupList);
    }

    /* Release filters */
    if (theService->messageFilterList != NULL) {
      /* Release filter resources */
      xPL_clearServiceFilters (theService);

      /* Release block of filters */
      free (theService->messageFilterList);
    }

    /* Release any listeners */
    if (theService->serviceListenerList != NULL) {
      free (theService->serviceListenerList);
    }

    /* Release configuration data */
    xPL_releaseServiceConfigurables (theService);

    /* Free the service */
    serviceCount--;
    if (ctxIndex < serviceCount) {
      memcpy (&serviceList[ctxIndex], &serviceList[ctxIndex + 1], sizeof (xPL_Service *) * (serviceCount - ctxIndex));
    }
    xPL_FreeService (theService);
    return;
  }
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setServiceEnabled (xPL_Service * theService, bool isEnabled) {
  /* Skip if already enabled */
  if (theService->serviceEnabled == isEnabled) {
    return;
  }

  /* Mark the service */
  theService->serviceEnabled = isEnabled;

  /* Handle enabling a disabled service */
  if (theService->serviceEnabled) {
    /* If there is an existing heartbeat, release it and rebuild it */
    if (theService->heartbeatMessage != NULL) {
      xPL_releaseMessage (theService->heartbeatMessage);
      theService->heartbeatMessage = NULL;
    }

    /* Start sending heartbeats */
    xPL_sendHeartbeat (theService);
  }
  else {
    /* Send goodby heartbeat */
    xPL_sendGoodbyeHeartbeat (theService);
  }
}

/* -----------------------------------------------------------------------------
 * Public
 */
bool xPL_isServiceEnabled (xPL_Service * theService) {
  return theService->serviceEnabled;
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setServiceVendor (xPL_Service * theService, char * newVendor) {
  /* Skip if name is already this name */
  if ( (theService->serviceVendor != NULL) && !xPL_strcmpIgnoreCase (theService->serviceVendor, newVendor)) {
    return;
  }

  /* If the service is enabled, send a goodbye heartbeat with our old data */
  if (theService->serviceEnabled) {
    xPL_sendGoodbyeHeartbeat (theService);
  }

  /* Copy the name */
  STR_FREE (theService->serviceVendor);
  theService->serviceVendor = xPL_StrDup (newVendor);

  /* If service was enabled, send new heartbeat */
  if (theService->serviceEnabled) {
    xPL_sendHeartbeat (theService);
  }
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getServiceVendor (xPL_Service * theService) {
  return theService->serviceVendor;
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setServiceDeviceID (xPL_Service * theService, char * newDeviceID) {
  /* Skip if name is already this name */
  if ( (theService->serviceDeviceID != NULL) && !xPL_strcmpIgnoreCase (theService->serviceDeviceID, newDeviceID)) {
    return;
  }

  /* If the service is enabled, send a goodbye heartbeat with our old data */
  if (theService->serviceEnabled) {
    xPL_sendGoodbyeHeartbeat (theService);
  }

  /* Copy the name */
  STR_FREE (theService->serviceDeviceID);
  theService->serviceDeviceID = xPL_StrDup (newDeviceID);

  /* If service was enabled, send new heartbeat */
  if (theService->serviceEnabled) {
    xPL_sendHeartbeat (theService);
  }
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getServiceDeviceID (xPL_Service * theService) {
  return theService->serviceDeviceID;
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setServiceInstanceID (xPL_Service * theService, char * newInstanceID) {
  /* Skip if name is already this ID */
  if ( (theService->serviceInstanceID != NULL) && !xPL_strcmpIgnoreCase (theService->serviceInstanceID, newInstanceID)) {
    return;
  }

  /* If the service is enabled, send a goodbye heartbeat with our old data */
  if (theService->serviceEnabled) {
    xPL_sendGoodbyeHeartbeat (theService);
  }

  /* Copy the ID */
  STR_FREE (theService->serviceInstanceID);
  theService->serviceInstanceID = xPL_StrDup (newInstanceID);

  /* If service was enabled, send new heartbeat */
  if (theService->serviceEnabled) {
    xPL_sendHeartbeat (theService);
  }
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getServiceInstanceID (xPL_Service * theService) {
  return theService->serviceInstanceID;
}

/* -----------------------------------------------------------------------------
 * Public
 */
void xPL_setServiceVersion (xPL_Service * theService, char * theVersion) {
  /* Free any previous version */
  if (theService->serviceVersion != NULL) {
    STR_FREE (theService->serviceVersion);
  }


  /* Clear the service version */
  if ( (theVersion == NULL) || (strlen (theVersion) == 0)) {
    theService->serviceVersion = NULL;
    return;
  }

  /* Copy in string */
  theService->serviceVersion = xPL_StrDup (theVersion);
}

/* -----------------------------------------------------------------------------
 * Public
 */
char * xPL_getServiceVersion (xPL_Service * theService) {
  return theService->serviceVersion;
}

/* -----------------------------------------------------------------------------
 * Public
 * Change the current heartbeat interval
 */
void xPL_setHeartbeatInterval (xPL_Service * theService, int newInterval) {
  /* Skip out of range values */
  if ( (newInterval < 0) || (newInterval > 172800)) {
    return;
  }
  theService->heartbeatInterval = newInterval;
}

/* -----------------------------------------------------------------------------
 * Public
 * Return services heartbeat interval */
int xPL_getHeartbeatInterval (xPL_Service * theService) {
  return theService->heartbeatInterval;
}

/* -----------------------------------------------------------------------------
 * Public
 * Clear out any/all installed filters */
void xPL_clearServiceFilters (xPL_Service * theService) {
  int filterIndex;
  xPL_ServiceFilter * theFilter;

  /* Release filter resources */
  for (filterIndex = 0; filterIndex < theService->filterCount; filterIndex++) {
    theFilter = & (theService->messageFilterList[filterIndex]);
    theFilter->matchOnMessageType = xPL_MESSAGE_ANY;
    STR_FREE (theFilter->matchOnVendor);
    STR_FREE (theFilter->matchOnDeviceID);
    STR_FREE (theFilter->matchOnInstanceID);
    STR_FREE (theFilter->matchOnSchemaClass);
    STR_FREE (theFilter->matchOnSchemaType);
  }

  /* Reset filter count */
  theService->filterCount = 0;
}

/* -----------------------------------------------------------------------------
 * Public
 * Clear out all groups */
void xPL_clearServiceGroups (xPL_Service * theService) {
  int groupIndex;

  /* Release group info */
  if (theService->groupList != NULL) {
    for (groupIndex = 0; groupIndex < theService->groupCount; groupIndex++) {
      STR_FREE (theService->groupList[groupIndex]);
    }
  }

  /* Reset the group */
  theService->groupCount = 0;
}

/* -----------------------------------------------------------------------------
 * Public
 * Send a message out from this service.  If the message has not had it's
 * source set or the source does not match the sending service, it is
 * updated and the message sent */
bool xPL_sendServiceMessage (xPL_Service * theService, xPL_Message * theMessage) {
  if ( (theService == NULL) || (theMessage == NULL)) {
    return FALSE;
  }

  /* Insure the message comes from this service */
  theMessage->sourceVendor = theService->serviceVendor;
  theMessage->sourceDeviceID = theService->serviceDeviceID;
  theMessage->sourceInstanceID = theService->serviceInstanceID;

  /* Send it on it's merry way */
  return xPL_sendMessage (theMessage);
}

/* -----------------------------------------------------------------------------
 * Public
 * Stop (disable) all services, usually in preparation for shutdown, but
 * that isn't the only possible reason */
void xPL_disableAllServices (void) {
  int serviceIndex = 0;

  for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
    xPL_setServiceEnabled (serviceList[serviceIndex], FALSE);
  }
}

/* -----------------------------------------------------------------------------
 * Public: Not use
 * Return true if we have any filters */
bool xPL_isServiceFiltered (xPL_Service * theService) {
  return theService->filterCount != 0;
}

/* -----------------------------------------------------------------------------
 * Public: Not use
 */
void xPL_setRespondingToBroadcasts (xPL_Service * theService, bool respondToBroadcasts) {
  if (!theService->ignoreBroadcasts != respondToBroadcasts) {
    return;
  }
  theService->ignoreBroadcasts = !respondToBroadcasts;
}

/* -----------------------------------------------------------------------------
 * Public: Not use
 */
bool xPL_isRespondingToBroadcasts (xPL_Service * theService) {
  return !theService->ignoreBroadcasts;
}

/* -----------------------------------------------------------------------------
 * Public: Not use
 */
void xPL_setReportOwnMessages (xPL_Service * theService, bool reportOwnMessages) {
  if (theService->reportOwnMessages == reportOwnMessages) {
    return;
  }
  theService->reportOwnMessages = reportOwnMessages;
}

/* -----------------------------------------------------------------------------
 * Public: Not use
 */
bool xPL_isReportOwnMessages (xPL_Service * theService) {
  return theService->reportOwnMessages;
}

/* -----------------------------------------------------------------------------
 * Public: Not use
 * Return true if we have one or more groups */
bool xPL_doesServiceHaveGroups (xPL_Service * theService) {
  return theService->groupCount != 0;
}

/* private functions ======================================================== */

/* -----------------------------------------------------------------------------
 * Private
 * Send an XPL Heartbeat immediatly */
bool xPL_sendHeartbeat (xPL_Service * theService) {
  xPL_Message * theHeartbeat;

  /* Create the Heartbeat message, if needed */
  if (theService->heartbeatMessage == NULL) {
    /* Configure the heartbeat */
    if (theService->configurableService && !theService->serviceConfigured) {
      theHeartbeat = createHeartbeatMessage (theService, HBEAT_CONFIG);
    }
    else {
      theHeartbeat = createHeartbeatMessage (theService, HBEAT_NORMAL);
    }

    /* Install a new heartbeat message */
    theService->heartbeatMessage = theHeartbeat;
    xPL_Debug ("SERVICE:: Just allocated a new Heartbeat message for the service");
  }
  else {
    theHeartbeat = theService->heartbeatMessage;
  }

  /* Send the message */
  if (!xPL_sendMessage (theHeartbeat)) {
    return FALSE;
  }

  /* Update last heartbeat time */
  theService->lastHeartbeatAt = time (NULL);
  xPL_Debug ("Sent Heatbeat message");
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Private
 * Send an Goodbye XPL Heartbeat immediatly */
bool xPL_sendGoodbyeHeartbeat (xPL_Service * theService) {
  xPL_Message * theHeartbeat;

  /* Create a shutdown message */
  if (theService->configurableService && !theService->serviceConfigured) {
    theHeartbeat = createHeartbeatMessage (theService, HBEAT_CONFIG_END);
  }
  else {
    theHeartbeat = createHeartbeatMessage (theService, HBEAT_NORMAL_END);
  }

  /* Send the message */
  if (!xPL_sendMessage (theHeartbeat)) {
    return FALSE;
  }

  /* Release message */
  xPL_releaseMessage (theHeartbeat);

  xPL_Debug ("Sent Goodbye Heatbeat");
  return TRUE;
}

/* -----------------------------------------------------------------------------
 * Private
 * Check each known service for when it last sent a heart
 * beat and if it's time to send another, do it. */
void xPL_sendTimelyHeartbeats (void) {
  xPL_Service * theService;
  int serviceIndex;
  time_t rightNow = time (NULL);
  int timeElapsed;

  for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
    theService = serviceList[serviceIndex];
    if (!xPL_isServiceEnabled (theService)) {
      continue;
    }

    /* See how much time has gone by */
    timeElapsed = rightNow - theService->lastHeartbeatAt;

    /* If we are still waiting to hear from the hub, then send a */
    /* message every 3 seconds until we do                       */
    if (!xPL_isHubConfirmed()) {
      if (timeElapsed >= CONFIG_HUB_DISCOVERY_INTERVAL) {
        xPL_sendHeartbeat (theService);
      }
      continue;
    }

    /* If we are in configuration mode, we send out once a minute */
    if (theService->configurableService && !theService->serviceConfigured) {
      if ( (theService->lastHeartbeatAt < 1) || (timeElapsed >= CONFIG_HEARTBEAT_INTERVAL)) {
        xPL_sendHeartbeat (theService);
      }
      continue;
    }

    /* For normal heartbeats, once each "heartbeatInterval" */
    if ( (theService->lastHeartbeatAt < 1) || (timeElapsed >= theService->heartbeatInterval)) {
      xPL_sendHeartbeat (theService);
    }
  }
}

/* -----------------------------------------------------------------------------
 * Private
 * Run the passed message by each service and see who is interested */
void xPL_handleServiceMessage (xPL_Message * theMessage, xPL_Object * userData) {
  xPL_Service * theService;
  int serviceIndex;

  for (serviceIndex = serviceCount - 1; serviceIndex >= 0; serviceIndex--) {
    theService = serviceList[serviceIndex];
    handleMessage (theService, theMessage);
  }
}


/* -----------------------------------------------------------------------------
 * Private
 */
void 
xPL_FreeService (xPL_Service * theService) {
  CONFIRM_CACHE_OK;
  xPL_Debug ("STORE:: Releasing xPL_Service @ %p back to cache pool", theService);
  xPL_ReleaseItem (theService, ServiceCache);
}

/* -----------------------------------------------------------------------------
 * Private
 */
xPL_Service * 
xPL_AllocService (void) {
  xPL_Service * theService;

  CONFIRM_CACHE_OK;
  if ( (theService = (xPL_Service *) xPL_AllocItem (ServiceCache)) == NULL) {
    theService = malloc (sizeof (xPL_Service));
    xPL_Debug ("STORE:: Allocated new xPL_Service, now %d services allocated", ++totalServiceAlloc);
  }
  else {
    xPL_Debug ("STORE:: Reused xPL_Service @ %p from cache", theService);
  }

  bzero (theService, sizeof (xPL_Service));
  return theService;
}
/* ========================================================================== */
