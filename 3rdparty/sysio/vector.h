/**
 * @file vector.h
 * @brief Tableau dynamique
 *
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */
#ifndef _SYSIO_VECTOR_H_
#define _SYSIO_VECTOR_H_

#include <sysio/defs.h>
__BEGIN_C_DECLS
/* ========================================================================== */

/**
 *  @defgroup sysio_vector Vecteur
 *
 *  Ce module fournit les fonctions permettant de gérer des tableaux à taille dynamique.
 *  @{
 */

/* macros =================================================================== */

/* types ==================================================================== */

/**
 * @brief Fonction qui retourne un élément alloué avec une valeur par défaut
 */
typedef void * (*vVectorElmtNew) (void);

/**
 * @brief Fonction qui libère la mémoire allouée à une donnée de le vecteur
 */
typedef void (*vVectorElmtDestroy) (void *data);

/**
 * @brief Fonction de comparaison de 2 clés
 * @return 0 si key1 == key2, positif si key1 > key2, négatif si key1 < key2,
 */
typedef int (*iVectorElmtMatch) (const void *key1, const void *key2);

/**
 * @brief Retourne la clé d'un élément
 */
typedef const void * (*pvVectorElmtKey) (const void * element);

/* structures =============================================================== */
/**
 * @class vVector
 * @brief Vecteur dynamique
 */
typedef struct vVector {
  void** data; /**< pointeur sur le tableau d'éléments */
  int size; /**< Taille en nombre d'éléments */
  int alloc; /**< nombre d'éléments alloués */
  int growth; /**< nombre d'éléments à ajouter lors d'une réallocation à la hausse */
  union {
    uint16_t flag;
    struct {
      uint16_t malloc: 1; /** Vecteur alloué dynamiquement */
    };
  };
  vVectorElmtNew       fnew;
  vVectorElmtDestroy   fdestroy;
  pvVectorElmtKey      fkey;
  iVectorElmtMatch     fmatch;
} xVector;

/* internal public functions ================================================ */

/**
 * @brief Initialise un tableau dynamique
 * 
 * @param vector pointeur sur le vecteur
 * @param growth nombre d'éléments à ajouter lors d'une réallocation à la hausse
 * @param fnew fonction qui retourne un élément alloué avec une valeur par défaut,
 *        NULL si les données sont statiques
 * @param fdestroy fonction chargée de libébrer la mémoire allouée aux données,
 *        NULL si les données sont statiques
 * @return 0, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorInit (xVector * vector, int growth, vVectorElmtNew fnew, vVectorElmtDestroy fdestroy);

/**
 * @brief Initialise les fonctions de recherche
 * @param vector pointeur sur le vecteur
 * @param fkey fonction de clé
 * @param fmatch fonction de comparaison
 * @return 0, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorInitSearch (xVector *vector, pvVectorElmtKey fkey, iVectorElmtMatch fmatch);

/**
 * @brief Taille du tableau en nombre d'éléments
 * @param vector pointeur sur le vecteur
 * @return la valeur, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorSize (const xVector * vector);

/**
 * @brief Modifie la taille
 * 
 * Si la nouvelle taille est inférieure à l'ancienne les éléments en excés sont
 * retirés avec destroy() si définie. Les nouvelles cases sont initialisées
 * avec fnew si fournie ou à NULL.
 * 
 * @param vector pointeur sur le vecteur
 * @param new_size nouvelle taille
 * @return 0, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorResize (xVector * vector, int new_size);

/**
 * @brief Destruction d'un tableau dynamique
 * 
 * La mémoire allouée aux données de chaque élément est libérée si une fonction
 * destroy() a été fournie à l'initialisation, puis le vecteur est effacé ou
 * libéré si son champs malloc = 1.
 * 
 * @param vector pointeur sur le vecteur
 */
void vVectorDestroy (xVector * vector);

/**
 * @brief Vide un tableau dynamique
 * 
 * La mémoire allouée aux données de chaque élément est libérée si une fonction
 * destroy() a été fournie à l'initialisation.
 * @param vector pointeur sur le vecteur
 * @return 0, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorClear (xVector * vector);

/**
 * @brief Ajout en fin de tableau
 * 
 * Si la taille du tableau est importante, cette fonction peut-être lente
 * car chaque élément pourrait être recopié dans une nouvelle zone mémoire. 
 * On préférera dans ce cas utiliser une liste.
 * 
 * @param vector pointeur sur le vecteur
 * @param data élément à affecter à la case
 * @return 0, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorAppend (xVector * vector, void * data);

/**
 * @brief Pointeur sur l'élément
 * @param index index de l'élément concerné
 * @return le pointeur sur l'élément ou NULL si erreur dans ce cas errno contient le code d'erreur
 */
void * pvVectorGet (const xVector * vector, int index);

/**
 * @brief Retire un élément
 * 
 * La mémoire allouée à l'élément est libérée si une fonction destroy() a été 
 * fournie à l'initialisation. La taille est réduite de 1.
 * 
 * @param vector pointeur sur le vecteur
 * @param index index de l'élément concerné
 * @return 0, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorRemove (xVector * vector, int index);

/**
 * @brief Modifie l'élément pointé par une case
 * 
 * Il est de la responsabilité de l'utilisateur de libérer éventuellement la 
 * mémoire allouée à l'élément remplacé.
 * 
 * @warning pour modifier le contenu de l'élément pointé par une case, il ne faut
 * pas utiliser cette fonction mais pvVectorGet() afin de récupérer le pointeur
 * sur l'élément, puis modifier l'élément à partir de ce pointeur.
 * 
 * @param vector pointeur sur le vecteur
 * @param index index de l'élément concerné
 * @param data à affecter à l'élément
 * @return 0, -1 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorReplace (xVector * vector, int index, void * data);

/**
 * @brief Chercher le premier élément correspondant à une clé
 * @param vector pointeur sur le vecteur
 * @param key clé de l'élément à trouver
 * @return premier élément trouvé, NULL si pas trouvé ou erreur
 */
void * pvVectorFindFirst (const xVector * vector, const void * key);

/**
 * @brief Chercher l'index du premier élément correspondant à une clé
 * @param vector pointeur sur le vecteur
 * @param key clé de l'élément à trouver
 * @return index, -1 si pas trouvé, -2 si erreur dans ce cas errno contient le code d'erreur
 */
int iVectorFindFirstIndex (const xVector * vector, const void * key);

/**
 * @}
 */


/* ========================================================================== */
__END_C_DECLS
#endif /*_SYSIO_VECTOR_H_ defined */
