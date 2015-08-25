/**
 * @file xPL-private.h
 * xPLLib internal include
 * 
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#include <sys/types.h>
#include <xPL.h>
#include "version.h"
#include "config.h"

/* macros =================================================================== */

/*
 * man 3 free:
 * ... If ptr is NULL, no operation is performed. 
 */
#define SAFE_FREE(x) if (x != NULL) { free(x); x = NULL; }
#define STR_FREE(x) if (x != NULL) { xPL_FreeStr(x); x = NULL; }

/* public variables ========================================================= */
extern int xPLFD;   /* FD We are connecting on */
extern pid_t xPLPID;
extern bool xPL_DebugMode;

/* internal public functions ================================================ */

/* xPL-utils.c */
xPL_NameValueList * xPL_newNamedValueList(void);
void xPL_freeNamedValueList(xPL_NameValueList *);
xPL_NameValuePair * xPL_newNamedValuePair(xPL_NameValueList *, char *);
void xPL_freeNamedValuePair(xPL_NameValuePair *);

/* xPL-service.c */
bool xPL_sendHeartbeat(xPL_Service *);
bool xPL_sendGoodbyeHeartbeat(xPL_Service *);
void xPL_sendTimelyHeartbeats(void);
void xPL_handleServiceMessage(xPL_Message *, xPL_Object *);
void xPL_releaseServiceConfigurables(xPL_Service * theService);
void xPL_disableAllServices(void);


/* xPL-message.c */
void xPL_receiveMessage(int, int, int);

/* xPL-listeners.c */
bool xPL_dispatchRawEvent(char *, int);
bool xPL_dispatchMessageEvent(xPL_Message *);
bool xPL_dispatchServiceEvent(xPL_Service *, xPL_Message *);
bool xPL_dispatchServiceConfigChangedEvent(xPL_Service *);

/* xPL-io.c */
bool xPL_sendRawMessage(char *, int);
char * xPL_getFairlyUniqueIdent(void);

/* xPL-store.c */
void xPL_FreeNVPair(xPL_NameValuePair *);
xPL_NameValuePair * xPL_AllocNVPair(void);

void xPL_FreeNVList(xPL_NameValueList *);
xPL_NameValueList * xPL_AllocNVList(void);

void xPL_FreeMessage(xPL_Message *);
xPL_Message * xPL_AllocMessage(void);

void xPL_FreeService(xPL_Service *);
xPL_Service * xPL_AllocService(void);

void xPL_FreeStr(char *);
char * xPL_StrDup(char *);
char * xPL_StrNDup(char *, int);
char * xPL_StrAlloc(int);

/* xPL-config.c */
void xPL_releaseServiceConfig(xPL_Service *);
