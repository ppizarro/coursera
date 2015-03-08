#include <stdio.h>
#include <string.h>

#include "parse-file-naive.h"

#define MAX_INTEGERS 1024 * 1024

/*
 * Naive 2-SUM algorithm.
 *   
 *   O(n^2) time.
 */
int twosum_naive (int * data, int size, int target)
{
    printf ("target: %i\n", target);
    int i = 0;
    while(i < size) {
        int x = data[i++];
        int j = 0;
        while(j < size) {
            int y = data[j++];
            if (x != y && x + y == target)
                return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    static int data[MAX_INTEGERS];
    int num_integers = 0;

    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    if (load_integers(argv[1], data, MAX_INTEGERS, &num_integers) < 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    int count = 0;
    int t = -10000;
    for(; t <= 10000 ; t++) {
        if(twosum_naive (data, num_integers, t))
            count++;
    }
    printf("2sum naive: %i\n",  count);

    return 0;
}
