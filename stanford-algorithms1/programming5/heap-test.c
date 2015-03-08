#include <stdio.h>

#include "heap.h"

int main (int argc, char * argv[])
{
    long int array[] = { 10, 3, 4, 8, 2, 9, 7, 1, 2, 6, 5 };

    Heap * heap = heap_new_with_data ((void**)array, 11, 0, NULL, NULL);

    heap_remove(heap, (void *)2);

    while (heap_size(heap) > 0) {
        printf ("%li\n", (long int)heap_pop(heap));
    }
    return 0;
}
