#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "heap.h"

struct _Heap {
    long ref;
    void ** array;
    int size;
    int max_size;
    int free_array;
    HeapDataKeyFunc data_key_func;
    HeapDataDestroyFunc data_destroy_func;
};

typedef void (*HeapForEachFunc) (void * data, const void * user_data);

/* ================================================================== */
/* Static functions                                                   */
/* ================================================================== */

static long default_data_key_func (void * data)
{
    return (long)data;
}

static inline void _heap_swap (Heap * heap, int pos_a, int pos_b)
{
    void * data = heap->array[pos_a];
      heap->array[pos_a] = heap->array[pos_b];
      heap->array[pos_b] = data;
}

static inline int _heap_key_smaller (Heap * heap, int a_pos, int b_pos)
{
    return heap->data_key_func(heap->array[a_pos]) < heap->data_key_func(heap->array[b_pos]);
}

static void _heap_bubble_up (Heap * heap, int child_pos)
{
    if (child_pos <= 0) return;

    int parent_pos = (child_pos - 1) >> 1;

    if (_heap_key_smaller (heap, child_pos, parent_pos)) {
        _heap_swap (heap, child_pos, parent_pos);
        _heap_bubble_up (heap, parent_pos);
    }
}

static void _heap_bubble_down (Heap * heap, int parent_pos)
{
    int last_pos = heap->size - 1;
    //if (parent_pos >= last_pos) return;

    int left_child_pos = 2 * parent_pos + 1;
    if (left_child_pos > last_pos) return; // no children

    int right_child_pos = left_child_pos + 1;
    if (right_child_pos > last_pos) {
        // only left child
        if (_heap_key_smaller (heap, left_child_pos, parent_pos)) {
            _heap_swap (heap, left_child_pos, parent_pos);
            //_heap_bubble_down (heap, left_child_pos);
        }
        return;
    }

    int smaller_child_pos = _heap_key_smaller (heap, left_child_pos, right_child_pos) ?
                                               left_child_pos :right_child_pos;
    if (_heap_key_smaller (heap, smaller_child_pos, parent_pos)) {
        _heap_swap (heap, smaller_child_pos, parent_pos);
        _heap_bubble_down (heap, smaller_child_pos);
    }
}

static void _heap_foreach (Heap * heap,
                           HeapForEachFunc func,
                           const void * user_data)
{
    if (func == NULL) return;

    void ** array = heap->array;
    int i = 0;
    for (; i < heap->size; i++) {
        func (array[i], user_data);
    }
}

static void _heap_dump_data (void * data, const void * user_data)
{
    HeapDataKeyFunc data_key_func = user_data;
    printf ("%li ", data_key_func(data));
}

static Heap * _heap_alloc (void ** array, int size, int max_size, int free_array,
                           HeapDataKeyFunc data_key_func,
                           HeapDataDestroyFunc data_destroy_func)
{
    Heap * heap = malloc (sizeof(Heap));
    if (heap == NULL) return NULL; 

    heap->ref = 1;
    heap->array = array;
    heap->size = size;
    heap->max_size = max_size;
    heap->free_array = free_array;
    heap->data_key_func = data_key_func?data_key_func:default_data_key_func;
    heap->data_destroy_func = data_destroy_func;
    return heap;    
}

static void _heap_free (Heap * heap)
{
    _heap_foreach (heap, (HeapForEachFunc)heap->data_destroy_func, NULL);
    if (heap->free_array) free (heap->array);
    free (heap);
}

/* ================================================================== */
/* API - Exported functions                                           */
/* ================================================================== */

Heap * heap_new (int max_size,
                 HeapDataKeyFunc data_key_func,
                 HeapDataDestroyFunc data_destroy_func)
{
    int bytes = max_size * sizeof(void *);
    void ** array = malloc (bytes);
    if (array == NULL) return NULL;
    bzero (array, bytes);
    
    Heap * heap = _heap_alloc (array, 0, max_size, 1,
                               data_key_func, data_destroy_func);
    if (heap == NULL) {
        free (array);
        return NULL;
    }

    return heap;
}

Heap * heap_new_with_data (void ** array,
                           int size,
                           int free_array,
                           HeapDataKeyFunc data_key_func,
                           HeapDataDestroyFunc data_destroy_func)
{
    if (array == NULL) return NULL;

    Heap * heap = _heap_alloc (array, size, size, free_array,
                               data_key_func, data_destroy_func);
    if (heap == NULL) {
        if (free_array) free (array);
        return NULL;
    }

    int pos = (size - 1) >> 1;
    for (; pos >= 0; pos--) {
        _heap_bubble_down (heap, pos);
    }
    return heap;
}

Heap * heap_ref (Heap * heap)
{
    if (heap == NULL) return NULL;

    heap->ref++;
    return heap;
}

void heap_unref (Heap * heap)
{
    if (heap == NULL) return;
    if (--heap->ref == 0) _heap_free (heap);
}

void heap_push (Heap * heap, void * data)
{
    if (heap == NULL) return;
    if (heap->size >= heap->max_size) return;

    int pos = heap->size++;
    heap->array[pos] = data;

      _heap_bubble_up (heap, pos);
}

void * heap_pop (Heap * heap)
{
    if (heap == NULL) return NULL;
    if (heap->size <= 0) return NULL;

      void * data = heap->array[0];

    if (--heap->size == 0) return data;     

      heap->array[0] = heap->array[heap->size];

      _heap_bubble_down (heap, 0);

    return data;
}

void * heap_peek (Heap * heap, int pos)
{
    if (pos < 0 || pos >= heap->size) return NULL;

    return heap->array[pos];
}

void heap_remove (Heap * heap, void * data)
{
    if (heap == NULL) return;

    int pos = 0;
    for (; pos < heap->size; pos++) {
        if (heap->array[pos] == data) {
            if (--heap->size == 0) return;
              heap->array[pos] = heap->array[heap->size];
              _heap_bubble_down (heap, pos);
            return;
        }
    }    
}

int heap_size (Heap * heap)
{
    if (heap == NULL) return 0;
    return heap->size;
}

int heap_max_size (Heap * heap)
{
    if (heap == NULL) return 0;
    return heap->max_size;
}

void heap_dump (Heap * heap)
{
    printf("size: %i\n", heap->size);
    printf("max_size: %i\n", heap->max_size);
    printf("array: ");
    _heap_foreach (heap, _heap_dump_data, heap->data_key_func);
    printf("\n");
}
