#include <stdlib.h>
#include <strings.h>

#include "dlist.h"

typedef struct _DListElement DListElement;

struct _DListElement
{
    void * data;
    DListElement * previous;
    DListElement * next;
};

struct _DList {
    int length;
    DListElement * head;
    DListElement * tail;
};

static DListElement * _dlist_element_alloc ()
{
    DListElement * element = malloc (sizeof(DListElement));
    if (element == NULL) return NULL;
    bzero (element, sizeof(DListElement));
   return element;
}

static void _dlist_element_free (DListElement * element)
{
    if (element == NULL) return;
    free (element);
}

static DListElement * _dlist_element_find (DList * list, const void * data)
{
    DListElement * tmp = list->head;
    while (tmp) {
        if (tmp->data == data)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

static void _dlist_element_remove_all (DList * list)
{
    DListElement * tmp = list->head;
    while (tmp) {
        DListElement * aux = tmp;
        tmp = tmp->next;
        _dlist_element_free (aux);
    }
    list->head = list->tail = NULL;
}

DList * dlist_new ()
{
    DList * list = malloc (sizeof(DList));
    if (list == NULL) return NULL;
    bzero (list, sizeof(DList));
   return list;
}

void dlist_free (DList * list)
{
    if (list == NULL) return;
    _dlist_element_remove_all (list);
    free (list);
}

void dlist_prepend (DList * list, void * data)
{
    DListElement * element = _dlist_element_alloc ();
    element->data = data;
    element->previous = NULL;

    list->length++;

    if (list->head == NULL) {
        list->head = list->tail = element;
        element->next = NULL;
    } else {
        element->next = list->head;
        list->head->previous = element;
        list->head = element;
    }
}

void dlist_append (DList * list, void * data)
{
    DListElement * element = _dlist_element_alloc ();
    element->data = data;
    element->next = NULL;

    list->length++;

    if (list->head == NULL) {
        list->head = list->tail = element;
        element->previous = NULL;
    } else {
        list->tail->next = element;
        element->previous = list->tail;
        list->tail = element;
    }
}

void dlist_remove (DList * list, const void * data)
{
    list->length--;

    DListElement * element = _dlist_element_find (list, data);
    if (element == NULL) return;

    if (element == list->head) {
        // element is the first
        list->head = element->next;
        _dlist_element_free (element);
        if (list->head == NULL)
        {
            // list is empty
            list->tail = NULL;
            return;
        }
        list->head->previous = NULL;
        return;
    }  

    if (element == list->tail) {
        // element is the last
        list->tail = element->previous;
        list->tail->next = NULL;
        _dlist_element_free (element);
        return;
    }

    // element is in the middle
    element->previous->next = element->next;
    element->next->previous = element->previous;
    _dlist_element_free (element);
}

void dlist_foreach (DList *list,
                    DFunc func,
                    const void * user_data)
{
    if (func == NULL) return;

    DListElement * element = list->head;
    while (element) {
        func(element->data, user_data);
        element = element->next;
    }
}

void * dlist_find (DList * list,
                   DFindFunc func,
                   const void * user_data)
{
    if (func == NULL) return NULL;

    DListElement * element = list->head;
    while (element) {
        if (func(element->data, user_data) == 0)
            return element->data;
        element = element->next;
    }
    return NULL;
}

int dlist_length (DList * list)
{
    return list->length;
}
