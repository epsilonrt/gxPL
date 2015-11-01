/**
 * @file sysio/defs.h
 * @brief Défintions pour l'interface système
 *
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#ifndef _SYSIO_DEFS_H_
#define _SYSIO_DEFS_H_

#if !defined(__DOXYGEN__)
// -----------------------------------------------------------------------------
// Définitions à usage interne
#ifndef __BEGIN_C_DECLS
# if defined(__cplusplus)
#   define __BEGIN_C_DECLS  extern "C" {
#   define __END_C_DECLS    }
# else
#   define __BEGIN_C_DECLS
#   define __END_C_DECLS
# endif
#endif

/* constants ================================================================ */
#ifndef TRUE
#  define TRUE  (1==1)
#  define FALSE (1==2)
#endif

// -----------------------------------------------------------------------------
#endif /* !defined(__DOXYGEN__) */

#ifndef __ASSEMBLER__
__BEGIN_C_DECLS
/* ==========================Partie Langage C============================== */

#if !defined(__DOXYGEN__)
// -----------------------------------------------------------------------------
// Partie non documentée
#ifndef __need_NULL
# define __need_NULL
#endif
#ifndef __need_size_t
# define __need_size_t
#endif
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define BASENAME(f) (f)

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
/* UNIX-style OS */
# define SYSIO_OS_UNIX
# include <libgen.h>
# undef BASENAME
# define BASENAME(f) basename(f)

# if defined(_POSIX_VERSION)
/* POSIX compliant */
#   define SYSIO_OS_POSIX
# endif

# if defined(__linux__)
#   define SYSIO_OS_LINUX
# endif

# if defined(__APPLE__)
#   define SYSIO_OS_APPLE
# endif

#elif defined(_WIN32) || defined(WIN32)
# define SYSIO_OS_WIN32

#else
# error Unable to detect operating system !
#endif

// ntohs, htons, ntohl, htonl, ntohll, htonll 
#include <arpa/inet.h>

#ifndef htonll
/* htonll not defined ------------------------------------------------------- */
#if defined(SYSIO_OS_LINUX)
#include <byteswap.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define be64_to_cpu(val) bswap_64(val)
#elif BYTE_ORDER == BIG_ENDIAN
#define be64_to_cpu(val) (val)
#else
#error unknwon BYTE_ORDER
#endif

#elif defined(SYSIO_OS_APPLE)
#warning TODO:  be64_to_cpu(val) for darwin

#elif defined(SYSIO_OS_WIN32)
#warning TODO:  be64_to_cpu(val) for win32

#endif /* SYSIO_OS_WIN32 defined */

#define ntohll(x) be64_to_cpu(x)
#define htonll(x) ntohll(x)
/* htonll not defined ------------------------------------------------------- */
#endif

/* internal public functions ================================================ */
/*
 * Modifie le message de la dernière erreur
 */
int iSysIoSetStrError (const char *format, ...);

/* macros =================================================================== */
#ifndef _BV
#define _BV(i) (1<<(i))
#endif

#ifndef ABS
# define ABS(n)  (((n) < 0) ? -(n) : (n))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? a : b)
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? a : b)
#endif

#ifndef MSB16
#define  MSB16(x) ((uint8_t) (x >> 8) & 0xFF)
#endif

#ifndef LSB
#define  LSB(x) ((uint8_t) (x & 0xFF))
#endif

#ifndef COUNTOF
# define COUNTOF(a)  (sizeof(a) / sizeof(*(a)))
#endif

#ifndef STRUCT_FIELD_OFFSET
# define STRUCT_FIELD_OFFSET(s,f)  ((size_t)&(((s *)0)->f))
#endif

#define PERROR(fmt,...) do { \
  vLog (LOG_ERR, "%s:%d: %s(): " fmt, BASENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); \
  iSysIoSetStrError ("%s:%d: %s(): Error: " fmt "\n", \
               BASENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); \
} while (0)
  
#define PWARNING(fmt,...) do { \
  vLog (LOG_WARNING, "%s:%d: %s(): " fmt, BASENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); \
  iSysIoSetStrError ("%s:%d: %s(): Warning: " fmt "\n", \
               BASENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); \
} while (0)


#ifdef DEBUG
#define PINFO(fmt,...) vLog (LOG_INFO, "%s:%d: %s(): " fmt, BASENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define PDEBUG(fmt,...) vLog (LOG_DEBUG, "%s:%d: %s(): " fmt, BASENAME(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define PDEBUG(fmt,...)
#define PINFO(fmt,...) vLog (LOG_INFO, fmt, ##__VA_ARGS__)
#endif

  /* GCC attributes */
#define FORMAT(type,fmt,first)  __attribute__((__format__(type, fmt, first)))
#define NORETURN                __attribute__((__noreturn__))
#define UNUSED_ARG(type,arg)    __attribute__((__unused__)) type arg
#define UNUSED_VAR(type,name)   __attribute__((__unused__)) type name
#define USED_VAR(type,name)     __attribute__((__used__)) type name
#define INLINE                  static inline __attribute__((__always_inline__))
#define NOINLINE                __attribute__((noinline))
#define LIKELY(x)               __builtin_expect(!!(x), 1)
#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define PURE_FUNC               __attribute__((pure))
#define CONST_FUNC              __attribute__((const))
#define UNUSED_FUNC             __attribute__((unused))
#define USED_FUNC               __attribute__((__used__))
#define RESTRICT                __restrict__
#define MUST_CHECK              __attribute__((warn_unused_result))
#define PACKED                  __attribute__((packed))
#define ALIGNED(x)              __attribute__ ((__aligned__(x)))

// -----------------------------------------------------------------------------
#endif /* !defined(__DOXYGEN__) */

/**
 *  @defgroup sysio_static Erreurs et versions
 * @{
 */

/* internal public functions ================================================ */

/**
 * @brief Renvoie la version de la bibliothèque sous forme de string
 *
 * @return la version sous la forme M.n[-b-gccccc] M: majeur, m: mineur, b:
 * build, ccccc:commit
 */
const char * sSysIoVersion (void);

/**
 * @brief Renvoie le majeur de la version de la bibliothèque
 *
 * @return le numéro de majeur ou -1 en cas d'erreur interne
 */
int iSysIoMajor (void);

/**
 * @brief Renvoie le mineur de la version de la bibliothèque
 *
 * @return le numéro de mineur ou -1 en cas d'erreur interne
 */
int iSysIoMinor (void);

/**
 * @brief Renvoie le numéro de build de la version de la bibliothèque
 *
 * @return le numéro de build ou -1 en cas d'erreur interne
 */
int iSysIoBuild (void);

/**
 * @brief Renvoie l'identifiant de commit git de la bibliothèque
 *
 * @return l'identifiant git du commit de la version sous la forme gccccc
 */
const char * sSysIoGitCommit (void);

/**
 * @brief Renvoie le message de la dernière erreur
 */
const char * sSysIoStrError (void);

/**
 * @brief Renvoie true si LOG_ASSERT a été défini lors de la compilation
 */
bool bSysIoLogAssert (void);

/**
 * @}
 */

/* =========================Fin Partie Langage C============================= */
__END_C_DECLS
#endif /* __ASSEMBLER__ not defined */


#endif /* _SYSIO_DEFS_H_ defined */
