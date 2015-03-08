#ifndef __DLIST_H__
#define __DLIST_H__

typedef struct _DList DList;

typedef int (*DFindFunc) (const void * data, const void * user_data);

typedef void (*DFunc) (const void * data, const void * user_data);

DList * dlist_new ();

void dlist_free (DList * list);

void dlist_prepend (DList * list, void * data);

void dlist_append (DList * list, void * data);

void dlist_remove (DList * list, const void * data);

void * dlist_find (DList * list,
	               DFindFunc func,
	               const void * user_data);

void dlist_foreach (DList *list,
                    DFunc func,
                    const void * user_data);

int dlist_length (DList * list);

#endif /* __DLIST_H__ */
