/**
 * @file xPL/utils.h
 * xPL4Linux Utilities functions
 *
 * Copyright 2004 (c), Gerald R Duprey Jr
 * Copyright 2015 (c), Pascal JEAN aka epsilonRT
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _XPL4LINUX_UTILS_HEADER_
#define _XPL4LINUX_UTILS_HEADER_

#include <xPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/* internal public functions ================================================ */

/**
 * @defgroup xPLUtils Utilities
 * @{
 */

/**
 * @defgroup xPLUtilsNameValue Name/Value Pairs
 * @{
 */

/* structures =============================================================== */
/**
 * @brief A discrete name/value structure
 */
typedef struct _xPL_NameValuePair {

  char * itemName;
  char * itemValue;
  bool isBinary;
  int binaryLength;
} xPL_NameValuePair;

/**
 * @brief A list of name/value pairs
 */
typedef struct _xPL_NameValueList {

  int namedValueCount;
  int namedValueAlloc;
  xPL_NameValuePair **namedValues;
} xPL_NameValueList;

/**
 * @brief Return number of name value pairs
 * @param theList
 * @return 
 */
int xPL_getNamedValueCount (xPL_NameValueList * theList);

/**
 * @brief Return the value indexed at
 * @param theList
 * @param listIndex
 * @return 
 */
xPL_NameValuePair * xPL_getNamedValuePairAt (xPL_NameValueList * theList, int listIndex);

/**
 * @brief Add a new entry to a passed name/value pair list
 * @param nameValueList
 * @param theName
 * @return 
 */
xPL_NameValuePair * xPL_newNamedValuePair (xPL_NameValueList * nameValueList, char * theName);

/**
 * @brief Just add a simple entry to the list
 * @param theList
 * @param theName
 * @param theValue
 */
void xPL_addNamedValue (xPL_NameValueList * theList, char * theName, char * theValue);

/**
 * @brief Remove All name/value pairs from the passed list
 * @param theList
 */
void xPL_clearAllNamedValues (xPL_NameValueList * theList);

/**
 * @brief Remove the name/value pair specified by the passed index
 * @param theList
 * @param nameIndex
 */
void xPL_clearNamedValueAt (xPL_NameValueList * theList, int nameIndex);

/**
 * @brief Remove the name/value pai
 * @param theList
 * @param theName
 */
void xPL_clearNamedValue (xPL_NameValueList * theList, char * theName);

/**
 * @brief Return true if there is a matching named value
 * @param theList
 * @param theName
 * @return 
 */
bool xPL_doesNamedValueExist (xPL_NameValueList * theList, char * theName);

/**
 * @brief Search for a name in a list of name values
 * @param theList
 * @param theName
 * @return index into the list of the value or -1 if not found
 */
int xPL_getNamedValueIndex (xPL_NameValueList * theList, char * theName);

/**
 * @brief Attempt to update an existing name/value
 * if it is not existing, create and add a new one
 * @param theList
 * @param theName
 * @param theValue
 */
void xPL_setNamedValue (xPL_NameValueList * theList, char * theName, char * theValue);

/**
 * @brief 
 * @param theList
 */
void xPL_setNamedValues (xPL_NameValueList * theList, ...);

/**
 * @brief Find the specified name in the name/value pair or return NULL
 * @param theList
 * @param theName
 * @return 
 */
xPL_NameValuePair * xPL_getNamedValuePair (xPL_NameValueList * theList, char * theName);

/**
 * @brief Search for a name in a list of name values
 * @param theList
 * @param theName
 * @return index into the list of the value or -1 if not found 
 */
int xPL_getNamedValueIndex (xPL_NameValueList * theList, char * theName);

/**
 * @brief Find the specified name in the list and return it's value or NULL
 * @param theList
 * @param theName
 * @return 
 */
char * xPL_getNamedValue (xPL_NameValueList * theList, char * theName);

/**
 * @brief Allocate a new named/value list
 * @return 
 */
xPL_NameValueList * xPL_newNamedValueList (void);

/**
 * @brief Free an entire name/value list
 * @param theList
 */
void xPL_freeNamedValueList (xPL_NameValueList * theList);

/**
 * @brief Free an Name/Value pair
 * @param thePair
 */
void xPL_freeNamedValuePair (xPL_NameValuePair * thePair);

/**
 * @}
 */

/**
 * @defgroup xPLUtilsText Text functions
 * @{
 */

/**
 * @brief Convert a string to upper case
 * @param target
 */
void xPL_Upcase (char * target);

/**
 * @brief Do a string comparison ignoring upper/lower case difference
 * @param textA
 * @param textB
 * @return
 */
int xPL_strcmpIgnoreCase (char * textA, char * textB);

/**
 * @brief Do a string comparison ignoring upper/lower case difference
 * @param textA
 * @param textB
 * @param maxChars
 * @return
 */
int xPL_strncmpIgnoreCase (char * textA, char * textB, int maxChars);

/**
 * @brief Convert a passed string into a number
 * store in the passed value.  If the number is OK, then TRUE is returned.
 * If there is an error, FALSE
 * @param theValue
 * @param theResult
 * @return
 */
bool xPL_strToInt (char * theValue, int *theResult);

/**
 * @brief Convert an integer into a string
 * @param theValue
 * @return
 */
char * xPL_intToStr (int theValue);

/**
 * @brief Convert a two digit hex string to an integer value
 * If the string is invalid, FALSE is returned 
 * @param theHexValue
 * @param theValue
 * @return 
 */
bool xPL_hexToInt (char * theHexValue, int *theValue);

/**
 * @brief Convert a number to hex.
 * Only the lower 8 bits of the passed value are examined and converted
 * @param theValue
 * @return 
 */
char * xPL_intToHex (int theValue);

/**
 * @}
 */

/**
 * @defgroup xPLUtilsDebug Debugging
 * @{
 */

/**
 * @brief Set Debugging Mode
 * @param isDebugging
 */
void xPL_setDebugging (bool isDebugging);

/**
 * @brief  Return if debug mode in use
 * @return 
 */
bool xPL_isDebugging (void);

/**
 * @brief Write a debug message out (if we are debugging)
 * @param theFormat
 */
void xPL_Debug (char * theFormat, ...);

/**
 * @brief Write an error message out
 * @param theFormat
 */
void xPL_Error (char * theFormat, ...);

/**
 * @}
 */

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _XPL4LINUX_UTILS_HEADER_ defined */
