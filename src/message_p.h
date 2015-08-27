/**
 * @file message_p.h
 * xPLLib internal include
 * 
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _XPL4LINUX_MESSAGE_PRIVATE_HEADER_
#define _XPL4LINUX_MESSAGE_PRIVATE_HEADER_

#include <xPL/message.h>
#include "utils_p.h"

/* constants ================================================================ */
#define MSG_MAX_SIZE 1500

/* public variables ========================================================= */
extern char messageBuff[];
extern itemCache * MessageCache;
extern int totalMessageAlloc;

/* internal public functions ================================================ */

/* message.c */
// void xPL_receiveMessage(int, int, int);

/* listeners.c */
bool xPL_dispatchRawEvent(char *, int);
bool xPL_dispatchMessageEvent(xPL_Message *);
bool xPL_dispatchServiceEvent(xPL_Service *, xPL_Message *);
bool xPL_dispatchServiceConfigChangedEvent(xPL_Service *);

void xPL_FreeMessage(xPL_Message *);
xPL_Message * xPL_AllocMessage(void);

void xPL_FreeService(xPL_Service *);
xPL_Service * xPL_AllocService(void);

/* ========================================================================== */
#endif /* _XPL4LINUX_MESSAGE_PRIVATE_HEADER_ defined */
