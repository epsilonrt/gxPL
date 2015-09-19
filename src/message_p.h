/**
 * @file message_p.h
 * gxPLib internal include
 * 
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License") 
 */
#ifndef _GXPL_MESSAGE_PRIVATE_HEADER_
#define _GXPL_MESSAGE_PRIVATE_HEADER_

#include <gxPL/message.h>

/* macros =================================================================== */
/* constants ================================================================ */
/* structures =============================================================== */

/*
 * @brief Describe a xPL message
 */
typedef struct _gxPLMessage {

  gxPLMessageType type;
  int hop;

  gxPLMessageId source;
  gxPLMessageId target;
  gxPLMessageSchema schema;

  union {
    int flag;
    struct {

      int isbroadcast: 1;
      int isreceived: 1; /* TRUE if received, FALSE if being sent */
    };
  };

  xVector body;
} gxPLMessage;


/* types ==================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
/* public variables ========================================================= */
/* internal public functions ================================================ */

// -----------------------------------------------------------------------------


/* ========================================================================== */
#endif /* _GXPL_MESSAGE_PRIVATE_HEADER_ defined */
