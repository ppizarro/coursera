#include <stdio.h>
#include <string.h>

#include "parse-file.h"
#include "heap.h"

#define MAX_INTEGERS 10 * 1024

#define INT_TO_POINTER(i) ((void*) (long) (i))
#define POINTER_TO_INT(p) ((long) (p))

#define HEAP_PUSH(h, x) (heap_push(h, INT_TO_POINTER(x)))
#define HEAP_POP(h) (POINTER_TO_INT(heap_pop(h)))
#define HEAP_PEEK(h, pos) (POINTER_TO_INT(heap_peek(h, pos)))

/*
 * Maintains the median of a stream
 * of numbers in 'real-time'
 * using O(log(n)) time for each
 * insert operation.
 */
static long median_maintenance(Heap * heap_low, Heap * heap_high, long x)
{
    if (heap_size(heap_low) == 0) {
        HEAP_PUSH(heap_low, x);
        return x;
    } 

    if (x > HEAP_PEEK(heap_low, 0)) {
        HEAP_PUSH(heap_high, x);
        if (heap_size(heap_high) > heap_size(heap_low)) {
            HEAP_PUSH(heap_low, HEAP_POP(heap_high));
        }
    } else {
        HEAP_PUSH(heap_low, x);
        if (heap_size(heap_low) > heap_size(heap_high) + 1) {
            HEAP_PUSH(heap_high, HEAP_POP(heap_low));
        }
    }

    return HEAP_PEEK(heap_low, 0);
}

/* Heap is a MIN-heap, so we need 
 * to revert elements in heap_low
 * (which should be a MAX-heap).
 */
static long heap_max_key (void * data)
{
    return -POINTER_TO_INT(data);
}

int main(int argc, char *argv[])
{
    long data[MAX_INTEGERS];
    int num_integers = 0;

    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    if (load_integers(argv[1], data, MAX_INTEGERS, &num_integers) < 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    Heap * heap_low = heap_new (num_integers, heap_max_key, NULL);
    Heap * heap_high = heap_new (num_integers, NULL, NULL);
  
    long median = 0; 
    int i = 0;
    for(; i < num_integers; i++) {
        median += median_maintenance (heap_low, heap_high, data[i]);
        median %= 10000;
    }

    printf("median: %li\n", median);
    return 0;
}
