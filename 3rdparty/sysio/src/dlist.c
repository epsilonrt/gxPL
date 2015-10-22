/**
 * @file sysio/dlist.c
 * @brief Liste doublement chaînée (Implémentation)
 *
 * Inspiré par la lecture du livre "Maitrise des algorithmes en C" de K. Loudon
 *
 * Copyright © 2015 Pascal JEAN aka epsilonRT <pascal.jean--AT--btssn.net>
 * All rights reserved.
 * This software is governed by the CeCILL license <http://www.cecill.info>
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sysio/dlist.h>

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
int
iDListInit (xDList *list, vDListElmtDestroy destroy) {
  if (list == NULL) {
    errno = EFAULT;
    return -1;
  }

  //  Initialize the list.
  memset (list, 0, sizeof (xDList));
  list->destroy = destroy;
  return 0;
}

// -----------------------------------------------------------------------------
int
iDListInitSearch (xDList *list, pvDListElmtKey fkey, iDListElmtMatch fmatch) {
  if (list == NULL) {
    errno = EFAULT;
    return -1;
  }

  list->key = fkey;
  list->match = fmatch;
  return 0;
}

// -----------------------------------------------------------------------------
int
iDListRemove (xDList *list, xDListElmt *element, void **data) {
  if (list == NULL) {
    errno = EFAULT;
    return -1;
  }

  //  Do not allow a NULL element or removal from an empty list.
  if (element == NULL || iDListSize (list) == 0) {
    return -1;
  }

  //  Remove the element from the list.
  *data = element->data;

  if (element == list->head) {

    //  Handle removal from the head of the list.
    list->head = element->next;

    if (list->head == NULL) {
      list->tail = NULL;
    }
    else {
      element->next->prev = NULL;
    }
  }
  else {

    //  Handle removal from other than the head of the list.
    element->prev->next = element->next;

    if (element->next == NULL) {
      list->tail = element->prev;
    }
    else {
      element->next->prev = element->prev;
    }
  }

  //  Free the storage allocated by the abstract data type.
  free (element);

  //  Adjust the size of the list to account for the removed element.
  list->size--;
  return 0;
}

// -----------------------------------------------------------------------------
int
iDListDestroy (xDList *list) {

  if (list) {
    void * data;

    //  Remove each element.
    while (iDListSize (list) > 0) {

      if (iDListRemove (list, pxDListTail (list), (void **) &data) == 0 && list->
          destroy != NULL) {

        //  Call a user-defined function to free dynamically allocated data.
        list->destroy (data);
      }
    }

    //  No operations are allowed now, but clear the structure as a precaution.
    memset (list, 0, sizeof (xDList));
    return 0;
  }

  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
iDListInsertAfter (xDList *list, xDListElmt *element, const void *data) {

  if (list) {
    xDListElmt * new_element;

    //  Do not allow a NULL element unless the list is empty.
    if (element == NULL && iDListSize (list) != 0) {
      return -1;
    }

    //  Allocate storage for the element.
    if ( (new_element = (xDListElmt *) malloc (sizeof (xDListElmt))) == NULL) {
      return -1;
    }

    //  Insert the new element into the list.
    new_element->data = (void *) data;

    if (iDListSize (list) == 0) {

      //  Handle insertion when the list is empty.
      list->head = new_element;
      list->head->prev = NULL;
      list->head->next = NULL;
      list->tail = new_element;
    }
    else {

      //  Handle insertion when the list is not empty.
      new_element->next = element->next;
      new_element->prev = element;

      if (element->next == NULL) {
        list->tail = new_element;
      }
      else {
        element->next->prev = new_element;
      }

      element->next = new_element;
    }

    //  Adjust the size of the list to account for the inserted element.
    list->size++;

    return 0;
  }
  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
int
iDListInsertBefore (xDList *list, xDListElmt *element, const void *data) {

  if (list) {
    xDListElmt * new_element;

    //  Do not allow a NULL element unless the list is empty.
    if (element == NULL && iDListSize (list) != 0) {
      return -1;
    }

    //  Allocate storage to be managed by the abstract data type.
    if ( (new_element = (xDListElmt *) malloc (sizeof (xDListElmt))) == NULL) {
      return -1;
    }

    //  Insert the new element into the list.
    new_element->data = (void *) data;

    if (iDListSize (list) == 0) {

      //  Handle insertion when the list is empty.
      list->head = new_element;
      list->head->prev = NULL;
      list->head->next = NULL;
      list->tail = new_element;
    }
    else {

      //  Handle insertion when the list is not empty.
      new_element->next = element;
      new_element->prev = element->prev;

      if (element->prev == NULL) {
        list->head = new_element;
      }
      else {
        element->prev->next = new_element;
      }

      element->prev = new_element;
    }

    //  Adjust the size of the list to account for the new element.
    list->size++;

    return 0;
  }
  errno = EFAULT;
  return -1;
}

// -----------------------------------------------------------------------------
xDListElmt *
pxDListFindFirst (xDList *list, const void * key) {
  
  if (list == NULL) {
    errno = EFAULT;
    return NULL;
  }

  if ( (list->key) && (list->match) && (iDListSize (list))) {
    xDListElmt * element = pxDListHead (list);

    do {
      if (list->match (key, list->key (element)) == 0) {
        return element;
      }
      element = pxDListElmtNext (element);
    }
    while (element);
  }
  return NULL;
}


// -----------------------------------------------------------------------------
int
iDListSize (const xDList *list) {
  
  if (list == NULL) {
    errno = EFAULT;
    return -1;
  }
  return list->size;
}

// -----------------------------------------------------------------------------
xDListElmt *
pxDListHead (const xDList *list) {
  
  if (list == NULL) {
    errno = EFAULT;
    return NULL;
  }
  return list->head;
}

// -----------------------------------------------------------------------------
xDListElmt *
pxDListTail (const xDList *list) {
  
  if (list == NULL) {
    errno = EFAULT;
    return NULL;
  }
  return list->tail;
}


// -----------------------------------------------------------------------------
int 
iDListPrepend (xDList *list,const void *data) {
  
  if (list == NULL) {
    errno = EFAULT;
    return -1;
  }
  return iDListInsertBefore (list, pxDListHead(list), data);
}

// -----------------------------------------------------------------------------
int 
iDListAppend (xDList *list,const void *data) {
  
  if (list == NULL) {
    errno = EFAULT;
    return -1;
  }
  return iDListInsertAfter (list, pxDListTail(list), data);
}

// -----------------------------------------------------------------------------
bool
bDListElmtIsHead (const xDListElmt *element) {
  
  if (element == NULL) {
    errno = EFAULT;
    return -1;
  }
  return element->prev == NULL ? 1 : 0;
}

// -----------------------------------------------------------------------------
bool
bDListElmtIsTail (const xDListElmt *element) {
  
  if (element == NULL) {
    errno = EFAULT;
    return -1;
  }
  return element->next == NULL ? 1 : 0;
}

// -----------------------------------------------------------------------------
void *
pvDListElmtData (const xDListElmt *element) {
  
  if (element == NULL) {
    errno = EFAULT;
    return NULL;
  }
  return element->data;
}

// -----------------------------------------------------------------------------
xDListElmt *
pxDListElmtNext (const xDListElmt *element) {
  
  if (element == NULL) {
    errno = EFAULT;
    return NULL;
  }
  return element->next;
}

// -----------------------------------------------------------------------------
xDListElmt *
pxDListElmtPrev (const xDListElmt *element) {
  
  if (element == NULL) {
    errno = EFAULT;
    return NULL;
  }
  return element->prev;
}

/* ========================================================================== */
