/**
 * @file sysio/log.h
 * @brief Gestion des logs
 *
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#ifndef _SYSIO_LOG_H_
#define _SYSIO_LOG_H_
#include <sysio/defs.h>
#include <syslog.h>
#include <stdarg.h>

__BEGIN_C_DECLS
/* ========================================================================== */

/**
 * @defgroup sysio_log Gestion des logs
 *
 * Ce module fournit les fonctions permettant d'afficher des messages sur
 * la sortie d'erreur ou dans syslog. \n
 * Les logs sont organisés en fonction de priorités, dans l'ordre décroissant
 * d'importance nous avons :
 * - LOG_EMERG      Le système est inutilisable.
 * - LOG_ALERT      Des actions doivent être entreprises immédiatement.
 * - LOG_CRIT       Les conditions sont critiques.
 * - LOG_ERR        Des erreurs se produisent.
 * - LOG_WARNING    Des avertissement se présentent.
 * - LOG_NOTICE     Condition normale, mais message significatif.
 * - LOG_INFO       Message d'information simple.
 * - LOG_DEBUG      Message de débogage.
 * .
 * 
 * Initialement le module est en mode "front-end", c'est à dire que les messages
 * sont affichés sur le terminal de sortie d'erreur. \n
 * En passant en mode daemon, les messages sont transmis au daemon syslog, plus
 * aucun message n'est affiché sur le terminal. \n
 * Si la macro LOG_ASSERT est définie avant l'inclusion de ce fichier, ce module
 * fournit aussi les macros assert() et assert_perror() qui se substitue à celles
 * de la la glibc de façon à pouvoir avorter le programme en mode daemon en 
 * laissant des traces du plantage dans les logs.
 *  @{
 */

/* public variables ========================================================= */

/**
 * @brief Nom du programme en cours
 */
extern const char* __progname;

/* internal public functions ================================================ */
/**
 * @brief Intialise le module
 * @param masque des priorités inital
 */
void vLogInit (int mask);

/**
 * @brief Passe ou sort du mode daemon
 * 
 * @param daemon true pour passer en mode daemon, false pour en sortir.
 */
void vLogDaemonize (bool daemon);

/**
 * @brief Fixe le masque des priorités
 * Les  huit  priorités sont LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, 
 * LOG_WARNING, LOG_NOTICE, LOG_INFO et LOG_DEBUG. \n
 * Le bit correspondant à la priorité p est LOG_MASK(p). Certains systèmes 
 * fournissent aussi une macro LOG_UPTO(p) pour le masque de toutes les 
 * priorités jusqu'à p incluses.
 * @param mask
 */
void vLogSetMask (int mask);

/**
 * @brief Lecture du masque des priorités en cours
 * @return la valeur du masque
 */
int  iLogMask (void);

/**
 * @brief Affichage ou écriture d'un message de log
 * 
 * @param priority priorité du message
 * @param format chaîne de format conforme à la syntaxe de la fonction printf()
 * suivie des paramètres optionnels.
 */
void vLog (int priority, const char *format, ...);

#if defined(__DOXYGEN__)

/**
 * @brief Terminer le programme en cas d'échec d'un test
 * 
 * Si la macro NDEBUG est définie au moment de la dernière inclusion de 
 * <sysio/log.h>, la macro assert() ne génère aucun code et ne fait rien. 
 * Sinon, la macro assert() affiche un message d'erreur sur la sortie d'erreur 
 * ou dans le journal des logs et termine l'exécution du programme en cours 
 * en appelant abort(3) si expression est fausse (égale à zéro).
 * 
 * @param expression expression qui doit être vrai (sinon abort)
 */
void assert(scalar expression);

/**
 * @brief Vérifier un code d'erreur et arrêter le programme
 * 
 * Si  la  macro  NDEBUG est définie lors de la dernière inclusion de 
 * <sysio/log.h>, la macro assert_perror() ne génère aucun code et n'a donc 
 * aucun effet. Sinon, la macro assert_perror() affiche un message d'erreur 
 * sur la sortie d'erreur ou dans le journal des logs et termine le programme 
 * en appelant abort(3) si errnum est non nul. \n
 * Le message contient le nom du fichier, le nom de  la fonction et le numéro 
 * de ligne de l'appel de la macro, ainsi que le résultat de strerror(errnum).
 * 
 * @param errnum numéro de l'erreur (on passe errno, la plupart du temps...)
 */
void assert_perror(int errnum);

/**
 * @brieh Bit correspondant à la priorité p
 */
#define LOG_MASK(p)

/**
 * @brieh Masque de toutes les priorités jusqu'à p incluses
 */
#define LOG_UPTO(p)

/**
 * @}
 */

#else
/* ========================================================================== */
#ifdef LOG_ASSERT
#ifdef  _ASSERT_H

# undef _ASSERT_H
# undef assert
# undef __ASSERT_VOID_CAST
# undef __ASSERT_FUNCTION

# ifdef __USE_GNU
#  undef assert_perror
# endif

#endif /* assert.h  */

#include <features.h>

/* This prints an "Assertion failed" message and aborts.  */
extern void __vLogAssertFail (__const char *__assertion, __const char *__file,
         unsigned int __line, __const char *__function)
     __THROW __attribute__ ((__noreturn__));

/* Likewise, but prints the error text for ERRNUM.  */
extern void __vLogAssertPerrorFail (int __errnum, __const char *__file,
          unsigned int __line,
          __const char *__function)
     __THROW __attribute__ ((__noreturn__));


#if defined __cplusplus && __GNUC_PREREQ (2,95)
# define __ASSERT_VOID_CAST static_cast<void>
#else
# define __ASSERT_VOID_CAST (void)
#endif

/* Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */
# if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define __ASSERT_FUNCTION  __PRETTY_FUNCTION__
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __ASSERT_FUNCTION  __func__
#  else
#   define __ASSERT_FUNCTION  ((__const char *) 0)
#  endif
# endif

# define assert(expr)             \
  ((expr)               \
   ? __ASSERT_VOID_CAST (0)           \
   : __vLogAssertFail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION))

# ifdef __USE_GNU
#  define assert_perror(errnum)           \
  (!(errnum)                \
   ? __ASSERT_VOID_CAST (0)           \
   : __vLogAssertPerrorFail ((errnum), __FILE__, __LINE__, __ASSERT_FUNCTION))
# endif

#else /* LOG_ASSERT defined */
#include <assert.h>
#endif /* LOG_ASSERT not defined */
// -----------------------------------------------------------------------------
#endif /* !defined(__DOXYGEN__) */

/* ========================================================================== */
__END_C_DECLS
#endif /* _SYSIO_LOG_H_ defined */
