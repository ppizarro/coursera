#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include "parse-file.h"

//#define MAX_TARGETS 20001

//static int targets[MAX_TARGETS];

/*
 * 2-SUM algorithm via hash table.
 * O(n) time.
 */
int twosum_hash (GHashTable * hash, int target)
{
    GHashTableIter iter;
    int x = 0;

    g_hash_table_iter_init (&iter, hash);
    while (g_hash_table_iter_next (&iter, (gpointer*)&x, NULL)) {
        int y = target - x;
        if (x != y && g_hash_table_contains (hash, GINT_TO_POINTER(y))) {
            //targets[target + 10000] = 1;
            //printf ("target[%i] x[%i] y[%i] t[%i]\n", target, x, y, target + 10000);
            return TRUE;
        }
    }
    return FALSE;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    GHashTable * hash = g_hash_table_new (g_direct_hash, g_direct_equal);

    if (load_hash(argv[1], hash) < 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    //bzero(targets, MAX_TARGETS * sizeof(int));

    int count = 0;
    int t = -10000;
    for(; t <= 10000 ; t++) {
        if (twosum_hash (hash, t))
            count++;
    }
    printf("2sum: %i\n",  count);
/*
    t = 0;
    count = 0;
    for(; t < MAX_TARGETS ; t++) {
        if(targets[t]) {
            //printf("target: %i\n",  t - 10000);
            count++;
        }
    }
    printf("2sum: %i\n",  count);
*/
    return 0;
}
