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
    unsigned int flag;
    struct {

      unsigned int isbroadcast: 1;
      unsigned int isreceived: 1;
      unsigned int isvalid: 1;
      unsigned int iserror: 1;
    };
  };
  gxPLMessageState state;

  xVector body;
} gxPLMessage;

/* ========================================================================== */
#endif /* _GXPL_MESSAGE_PRIVATE_HEADER_ defined */
