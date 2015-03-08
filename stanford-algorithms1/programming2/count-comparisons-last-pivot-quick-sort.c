#include <stdio.h>
#include <string.h>

#define MAX_INTS 100000

int load_integers(const char * filename, int* array, int length)
{
    FILE *fp = NULL;
    int value;
    int i = -1;

    if ((fp = fopen (filename, "r")) == NULL)
        return -1;

    while (fscanf (fp, "%i", &value) && !feof(fp) && ++i < length) 
        array[i] = value;

    fclose (fp);
    return i + 1;
}

inline void swap(int* array, int i, int j)
{
    if (i == j) return;

    int tmp = array[j];
    array[j] = array[i];
    array[i] = tmp;
}

int partition_first_pivot(int* array, int length)
{
    int pivot = array[0];
    int i = 1;
    int j = 1;

    for (; j < length; j++) {
        if (array[j] < pivot) {
           swap(array, i, j);
           i++;
        }
    }
    swap(array, 0, i - 1);
    return i - 1;
}

long long quick_sort_and_count_comparisons(int* array, int length)
{
    if (length <= 1) return 0;

    int comparisons = length - 1;

    swap(array, 0, comparisons);

    int pos = partition_first_pivot (array, length);

    return comparisons + 
           quick_sort_and_count_comparisons(array, pos) +
           quick_sort_and_count_comparisons(array + pos + 1, length - pos - 1);
}

int main(int argc, char *argv[])
{
    static int integers[MAX_INTS];

    int num_ints = 0;

    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    if ((num_ints = load_integers(argv[1], integers, MAX_INTS)) <= 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    long long comparisons = quick_sort_and_count_comparisons(integers, num_ints);
    printf("comparisons: %lli\n", comparisons);

    return 0;
}

