/**
 * @file sysio/dlist.h
 * @brief Liste doublement chaînée
 *
 * Inspiré par la lecture du livre "Maitrise des algorithmes en C" de K. Loudon
 *
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#ifndef _SYSIO_DLIST_H_
#define _SYSIO_DLIST_H_
#include <stdlib.h>
#include <sysio/defs.h>

__BEGIN_C_DECLS
/* ========================================================================== */
typedef struct xDListElmt xDListElmt;

/**
 *  @defgroup sysio_dlist Liste doublement chaînée
 *
 *  Ce module fournit les fonctions permettant de gérer des listes doublement chaînées.
 *  @{
 */

/* types ==================================================================== */
/**
 * @brief Fonction qui libère la mémoire allouée à une donnée de la liste
 */
typedef void (*vDListElmtDestroy) (void *data);

/**
 * @brief Fonction de comparaison de 2 clés
 * @return 0 si key1 == key2, positif si key1 > key2, négatif si key1 < key2, 
 */
typedef int  (*iDListElmtMatch) (const void *key1, const void *key2);

/**
 * @brief Retourne la clé d'un élément
 */
typedef const void * (*pvDListElmtKey) (const xDListElmt * element);

/* structures =============================================================== */

/**
 * @brief Elément d'une liste doublement chaînée
 */
typedef struct xDListElmt {

  void               *data;
  struct xDListElmt  *prev;
  struct xDListElmt  *next;

} xDListElmt;

/**
 * @brief Liste doublement chaînée
 */
typedef struct xDList {

  int                 size;
  xDListElmt          *head;
  xDListElmt          *tail;
  vDListElmtDestroy   destroy;
  pvDListElmtKey      key;
  iDListElmtMatch     match;
} xDList;

/* macros =================================================================== */
/**
 * @brief Retourne le pointeur sur la donnée convertit en pointeur sur type t
 */
#define pxDListElmtDataPtr(element, t) ((t *)pvDListElmtData(element))

/* internal public functions ================================================ */
/**
 * @brief Initialise une liste doublement chaînée
 * @param list pointeur sur la liste
 * @param fdestroy fonction chargée de libébrer la mémoire allouée aux données,
 *        NULL si les données sont statiques
 */
int iDListInit (xDList *list, vDListElmtDestroy fdestroy);

/**
 * @brief Initialise les fonctions de recherche
 * @param list pointeur sur la liste
 * @param fkey fonction de clé
 * @param fmatch fonction de comparaison
 */
int iDListInitSearch (xDList *list, pvDListElmtKey fkey, iDListElmtMatch fmatch);

/**
 * @brief Destruction d'une liste doublement chaînée
 * 
 * La mémoire allouée aux données de chaque élément est libérée si une fonction
 * destroy() a été fournie à l'initialisation.
 * @param list pointeur sur la liste
 */
int iDListDestroy (xDList *list);

/**
 * @brief Insertion avant l'élément
 * @param list pointeur sur la liste
 * @param element précédent le nouveau
 * @param data à affecter au nouvel élément
 * @return 0, -1 si erreur
 */
int iDListInsertAfter (xDList *list, xDListElmt *element, const void *data);

/**
 * @brief Insertion après l'élément
 * @param list pointeur sur la liste
 * @param element suivant le nouveau
 * @param data à affecter au nouvel élément
 * @return 0, -1 si erreur
 */
int iDListInsertBefore (xDList *list, xDListElmt *element, const void *data);

/**
 * @brief Retire un élément
 * @param list pointeur sur la liste
 * @param element à retier
 * @param data pointeur sur la donnée de l'élément retiré (mémoire non libérée)
 * @return 0, -1 si erreur
 */
int iDListRemove (xDList *list, xDListElmt *element, void **data);

/**
 * @brief Chercher le premier élément correspondant à une clé
 * @param list pointeur sur la liste
 * @param key clé de l'élément à trouver
 * @return premier élément trouvé, NULL si pas trouvé
 */
xDListElmt * pxDListFindFirst (xDList *list, const void * key);

/**
 * @brief Nombre d'élément 
 * @param list pointeur sur la liste
 * @return la valeur
 */
int iDListSize (const xDList *list);

/**
 * @brief Insertion en début de liste
 * @param list pointeur sur la liste
 * @param data à affecter au nouvel élément
 * @return 0, -1 si erreur
 */
int iDListPrepend (xDList *list,const void *data);

/**
 * @brief Insertion en fin de liste
 * @param list pointeur sur la liste
 * @param data à affecter au nouvel élément
 * @return 0, -1 si erreur
 */
int iDListAppend (xDList *list,const void *data);

/**
 * @brief Retourne l'élément en tête de liste
 * @param list pointeur sur la liste
 * @return la valeur
 */
xDListElmt * pxDListHead (const xDList *list);

/**
 * @brief Retourne l'élément en queue de liste
 * @param list pointeur sur la liste
 * @return la valeur
 */
xDListElmt * pxDListTail (const xDList *list);

/**
 * @brief Indique si l'élément est en tête de liste
 * @param element pointeur sur l'élément concerné
 * @return la valeur
 */
bool bDListElmtIsHead (const xDListElmt *element);

/**
 * @brief Indique si l'élément est en queue de liste
 * @param element pointeur sur l'élément concerné
 * @return la valeur
 */
bool bDListElmtIsTail (const xDListElmt *element);

/**
 * @brief Pointeur sur la donnée de l'élément
 * @param element pointeur sur l'élément concerné
 * @return la valeur
 */
void * pvDListElmtData (const xDListElmt *element);

/**
 * @brief Pointeur sur l'élément suivant dans la liste
 * @param element pointeur sur l'élément concerné
 * @return la valeur
 */
xDListElmt * pxDListElmtNext (const xDListElmt *element);

/**
 * @brief Pointeur sur l'élément précédent dans la liste
 * @param element pointeur sur l'élément concerné
 * @return la valeur
 */
xDListElmt * pxDListElmtPrev (const xDListElmt *element);

/**
 * @}
 */

/* ========================================================================== */
__END_C_DECLS
#endif /* _SYSIO_DLIST_H_ */
