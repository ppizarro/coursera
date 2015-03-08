#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include "parse-file-naive.h"

#define MAX_INTEGERS 1024 * 1024

#define MAX_TARGETS 20001

static int targets[MAX_TARGETS];

/*
 * 2-SUM algorithm via hash table.
 * O(n) time.
 */
int twosum_hash (int * data, int num_integers, GHashTable * hash, int target)
{
    int i = 0;
    int * key = NULL;

    while (i < num_integers) {
        int x = (int)(data[i++]);
        int y = target - x;
        //if (x != y && g_hash_table_lookup (hash, &y) != NULL) {
        //if (g_hash_table_lookup (hash, GINT_TO_POINTER(y)) != NULL && x != y) {
        if (x != y && g_hash_table_contains (hash, GINT_TO_POINTER(y))) {
            targets[target + 10000] = 1;
	    //printf ("target[%i] x[%i] y[%i] t[%i]\n", target, x, y, target + 10000);
            return TRUE;
        }
    }
    return FALSE;
}
/*
void dump_hash (GHashTable * hash)
{
    GHashTableIter iter;
    int * key = NULL;

    g_hash_table_iter_init (&iter, hash);
    while (g_hash_table_iter_next (&iter, (gpointer*)&key, NULL)) {
        int x = *key;
	printf ("x[%i]\n", x);
    }
}
*/
int main(int argc, char *argv[])
{
    static int data[MAX_INTEGERS];

    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    int num_integers = 0;

    if (load_integers(argv[1], data, MAX_INTEGERS, &num_integers) < 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    GHashTable * hash = g_hash_table_new (g_direct_hash, g_direct_equal);
    int i = 0;
    for (; i < num_integers; i++) {
        g_hash_table_add (hash, GINT_TO_POINTER(data[i]));
    }
    
    //dump_hash (hash);
    printf ("size: %i\n", g_hash_table_size (hash));

    bzero(targets, MAX_TARGETS * sizeof(int));

    int count = 0;
    int t = -10000;
    for(; t <= 10000 ; t++) {
        if (twosum_hash (data, num_integers, hash, t))
            count++;
    }
    printf("2sum: %i\n",  count);

    t = 0;
    count = 0;
    for(; t < MAX_TARGETS ; t++) {
        if(targets[t]) {
            //printf("target: %i\n",  t - 10000);
            count++;
        }
    }
    printf("2sum: %i\n",  count);

    return 0;
}
