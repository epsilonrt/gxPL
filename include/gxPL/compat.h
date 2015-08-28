/**
 * @file gxPL/compat.h
 * Definitions to ensure compatibility with software using gxPL 1.3 or below
 *
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_COMPAT_HEADER_
#define _GXPL_COMPAT_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */
typedef xPL_Object * xPL_ObjectPtr;
typedef char * String;
typedef xPL_Message * xPL_MessagePtr;
typedef xPL_Service * xPL_ServicePtr;
typedef xPL_ServiceListenerDef * xPL_ServiceListenerDefPtr;
typedef xPL_ServiceChangedListenerDef * xPL_ServiceChangedListenerDefPtr;
typedef xPL_ServiceConfigurable * xPL_ServiceConfigurablePtr;
typedef xPL_ServiceFilter * xPL_ServiceFilterPtr;
typedef xPL_NameValuePair * xPL_NameValuePairPtr;
typedef xPL_NameValueList * xPL_NameValueListPtr;

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_COMPAT_HEADER_ defined */
