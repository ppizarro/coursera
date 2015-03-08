#ifndef __HEAP_H__
#define __HEAP_H__

typedef struct _Heap Heap;

typedef long (*HeapDataKeyFunc) (void * data);

typedef void (*HeapDataDestroyFunc) (void * data);

Heap * heap_new (int max_size,
	             HeapDataKeyFunc data_key_func,
	             HeapDataDestroyFunc data_destroy_func);

Heap * heap_new_with_data (void ** array, int size, int free_array,
	                       HeapDataKeyFunc data_key_func,
	                       HeapDataDestroyFunc data_destroy_func);

Heap * heap_ref (Heap * heap);

void heap_unref (Heap * heap);

void heap_push (Heap * heap, void * data);

void * heap_pop (Heap * heap);

void * heap_peek (Heap * heap, int pos);

void heap_remove (Heap * heap, void * data);

int heap_size (Heap * heap);

int heap_max_size (Heap * heap);

void heap_dump (Heap * heap);

#endif /* __HEAP_H__ */
