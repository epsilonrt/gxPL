/**
 * @file
 * xPL Utilities functions
 *
 * Copyright 2015 (c), epsilonRT                
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 */
#ifndef _GXPL_UTIL_HEADER_
#define _GXPL_UTIL_HEADER_

#include <gxPL/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */


/* internal public functions ================================================ */

/**
 * @defgroup gxPLUtilDoc Utilities
 * Functions for miscellaneous purpose
 * @{
 */

/**
 * @defgroup gxPLUtilTextDoc Text functions
 * @{
 */

/**
 * @brief Copy string in accordance with xPL
 *
 * Copies the C string pointed by src into the array pointed by dst,
 * including the terminating null character (and stopping at that point).
 * src must contain only the following characters: A-Z, a-z, 0-9 and "-"
 * (letters, numbers and the hyphen/dash character - ASCII 45). If this is not
 * the case, the copying is interrupted and -1 is returned.

 * @param dst This is the pointer to an array of char elements where the
 * resulting C string is stored.
 * @param src source string
 * @return If successful, the total number of characters written is returned
 * excluding the null-character appended at the end of the string, otherwise a
 * negative number is returned in case of failure.
 */
int gxPLStrCpy (char * dst, const char * src);

/**
 * @brief Quickly to convert an long integer to string
 * @param value
 * @return a static const string
 */
const char * gxPLLongToStr (long value);

/**
 * @brief Quickly to convert an double to string
 * @param value
 * @param precision (8 max)
 * @return a static const string
 */
const char * gxPLDoubleToStr (double value, int precision);

/**
 * @}
 */

/**
 * @defgroup gxPLUtilConfigDoc Configuration file
 * @{
 */

/**
 * @brief Returns the path of a configuration file
 *
 * @param filename the file name
 * @return If filename is a basename, add a directory that depends on the
 * context and the host system. On a Unix system, the added directory is
 * /etc/gxpl if the user is root, $HOME/.gxpl] otherwise. \n
 * If filename is not a basename, filename is returned
 */
const char * gxPLConfigPath (const char * filename);

/**
 * @brief
 * @param filename
 * @param vendor_id
 * @param device_id
 * @return
 */
xVector * gxPLConfigReadFile (const char * filename, const char * vendor_id,
                              const char * device_id);
/**
 * @}
 */

/**
 * @defgroup gxPLUtilPairDoc Name/Value pairs
 * @{
 */

/**
 * @brief Describe a name=value pair
 */
typedef struct _gxPLPair {
  char * name;
  char * value;
} gxPLPair;

/**
 * @brief Sets a name/value form a string
 * @param str string name=value\0
 * @return the pair, NULL if error occurs
 */
gxPLPair * gxPLPairFromString (char *str);

/**
 * @brief Sets a name/value form a string
 * @param line string name=value\n
 * @return the pair, NULL if error occurs
 */
gxPLPair * gxPLPairFromLine (char * line);

/**
 * @brief Release a pair allocte on the heap
 * Used by sysio vector and dlist
 * @param pair
 */
void gxPLPairDelete (void * pair);

/**
 * @brief Compares name of 2 pair s
 * @param key1
 * @param key2
 * @return 0 equal, -1 less than, 1 more than
 */
int gxPLPairMatch (const void *key1, const void *key2);

/**
 * @brief Returns the name of a pair as a const void * pointer
 * Used by sysio vector
 * @param pair
 */
const void * gxPLPairKey (const void * pair);

/**
 * @}
 */

/**
 * @defgroup gxPLUtilIdDoc Identifiers
 * @{
 */

/**
 * @brief Sets an identifier
 * @param id pointer to the identifier
 * @param vendor_id vendor identifier as string
 * @param device_id device identifier as string
 * @param instance_id instance identifier as string
 * @return 0, -1 if an error occurs
 */
int gxPLIdSet (gxPLId * id, const char * vendor_id, const char * device_id, const char * instance_id);

/**
 * @brief Sets vendor identifier
 * @param id pointer to the identifier
 * @param vendor_id vendor identifier as string
 * @return 0, -1 if an error occurs
 */
int gxPLIdVendorIdSet (gxPLId * id, const char * vendor_id);

/**
 * @brief Sets device identifier
 * @param id pointer to the identifier
 * @param device_id device identifier as string
 * @return 0, -1 if an error occurs
 */
int gxPLIdDeviceIdSet (gxPLId * id, const char * device_id);

/**
 * @brief Sets instance identifier
 * @param id pointer to the identifier
 * @param instance_id instance identifier as string
 * @return 0, -1 if an error occurs
 */
int gxPLIdInstanceIdSet (gxPLId * id, const char * instance_id);

/**
 * @brief Copy two identifiers
 * @param dst destination
 * @param src source
 * @return 0, -1 if an error occurs
 */
int gxPLIdCopy (gxPLId * dst, const gxPLId * src);

/**
 * @brief Compare two identifiers
 * @param id1 first id
 * @param id2 second id
 * @return It returns an integer less than, equal to, or greater than zero if id1
 * is found, respectively, to be less than, to match, or be greater than id2.
 */
int gxPLIdCmp (const gxPLId * id1, const gxPLId * id2);

/**
 * @brief Gets an identifier from a string
 * @param dest destination
 * @param src source string vendor-device.instance\0, this string is modified
 * by the function and is no longer valid after call.
 * @return 0, -1 if an error occurs
 */
int gxPLIdFromString (gxPLId * dest, char * src);

/**
 * @}
 */

/**
 * @defgroup gxPLUtilSchemaDoc Schemas
 * @{
 */

/**
 * @brief Compare two schemas
 * @param s1 schema 1
 * @param s2 schema 2
 * @return It returns an integer less than, equal to, or greater than zero if s1
 * is found, respectively, to be less than, to match, or be greater than s2.
 */
int gxPLSchemaCmp (const gxPLSchema * s1, const gxPLSchema * s2);

/**
 * @brief Compare two schemas
 * @param s1 schema 1
 * @param s2 schema 2
 * @return It returns an integer less than, equal to, or greater than zero if s1
 * is found, respectively, to be less than, to match, or be greater than s2.
 */
int gxPLSchemaMatch (const gxPLSchema * s1, const char * schema_class, const char * schema_type);

/**
 * @brief
 * @param s
 * @param schema_class
 * @return
 */
int gxPLSchemaClassSet (gxPLSchema * s, const char * schema_class);

/**
 * @brief
 * @param s
 * @param schema_type
 * @return
 */
int gxPLSchemaTypeSet (gxPLSchema * s, const char * schema_type);

/**
 * @brief
 * @param schema
 * @param schema_class
 * @param schema_type
 * @return
 */
int gxPLSchemaSet (gxPLSchema * schema, const char * schema_class, const char * schema_type);

/**
 * @brief
 * @param dst
 * @param src
 * @return
 */
int gxPLSchemaCopy (gxPLSchema * dst, const gxPLSchema * src);

/**
 * @brief
 * @param schema
 * @param str
 * @return
 */
int gxPLSchemaFromString (gxPLSchema * schema, const char * str);

/**
 * @brief
 * @param schema
 * @return
 */
int gxPLSchemaIsEmpty (const gxPLSchema * schema);
/**
 * @}
 */

/**
 * @defgroup gxPLUtilTimeDoc Time
 * @{
 */
/**
 * @brief System time
 * This time can be an absolute time (a date) or relative to the system startup
 * (on embedded platform, for example)
 * @return time in seconds
 */
unsigned long gxPLTime (void);

/**
 * @brief System time in milliseconds
 * @param ms pointer on the result
 * @return 0, < 0 if error occurs
 */
int gxPLTimeMs (unsigned long * ms);

/**
 * @brief converts the system time t into a null-terminated string
 * @param t time return by gxPLTime
 * @param format same as strftime() on unix, NULL if default (yyyymmddhhmmss)
 * @return system time t into a null-terminated string
 */
char * gxPLDateTimeStr (unsigned long t, const char * format);

/**
 * @brief converts the system time t into a null-terminated string
 * @param t time return by gxPLTime
 * @param format same as strftime() on unix, NULL if default (yyyymmdd)
 * @return system time t into a null-terminated string
 */
char * gxPLDateStr (unsigned long t, const char * format);

/**
 * @brief converts the system time t into a null-terminated string
 * @param t time return by gxPLTime
 * @param format same as strftime() on unix, NULL if default (hhmmss)
 * @return system time t into a null-terminated string
 */
char * gxPLTimeStr (unsigned long t, const char * format);

/**
 * @brief suspends execution for (at least) ms milliseconds
 * @param ms delay in milliseconds
 * @return 0, < 0 if error occurs
 */
int gxPLTimeDelayMs (unsigned long ms);


/**
 * @}
 */
/**
 * @}
 */

#if !defined(__DOXYGEN__)
// -----------------------------------------------------------------------------

#if defined(__AVR__)
// -----------------------------------------------------------------------------
#include <avrio/memdebug.h>

INLINE int
gxPLDynamicMemoryUsed (void) {
  
  return ulMemoryUsed();
}
#elif (__unix__)
// -----------------------------------------------------------------------------
#include <malloc.h>

INLINE int
gxPLDynamicMemoryUsed (void) {
  
  struct mallinfo mi = mallinfo();
  return mi.uordblks;
}
#else /* __unix__ && __AVR__ not defined */
// -----------------------------------------------------------------------------
INLINE int
gxPLDynamicMemoryUsed (void) {
  return -1;
}
#endif /* __unix__ && __AVR__ not defined */
// -----------------------------------------------------------------------------
#endif /* !defined(__DOXYGEN__) */

/* ========================================================================== */
__END_C_DECLS
#endif /* _GXPL_UTIL_HEADER_ defined */
