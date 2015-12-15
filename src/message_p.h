/**
 * @file
 * gxPLMessage internal include
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
struct _gxPLMessage {

  gxPLMessageType type;
  int hop;

  gxPLId source;
  gxPLId target;
  gxPLSchema schema;

  gxPLMessageState state;

  xVector body;
  union {
    unsigned int flag;
    struct {

      unsigned int isbroadcast: 1;
      unsigned int isreceived: 1;
      unsigned int isvalid: 1;
      unsigned int iserror: 1;
      unsigned int isgrouped: 1;
    };
  };
};

/* ========================================================================== */
#endif /* _GXPL_MESSAGE_PRIVATE_HEADER_ defined */
