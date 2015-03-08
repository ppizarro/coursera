#include <stdio.h>
#include <string.h>

#define MAX_INTS 100000

static int integers[MAX_INTS];

static int array_tmp[MAX_INTS];

int load_integers(const char * filename)
{
    FILE *fp = NULL;
    int value;
    int i = -1;

    if ((fp = fopen (filename, "r")) == NULL)
        return -1;

    while (fscanf (fp, "%i", &value) && !feof(fp) && ++i < MAX_INTS ) 
        integers[i] = value;

    fclose (fp);
    return i + 1;
}

long long merge_and_split_count_inversions(int* left_sort,  int left_length,
                                           int* right_sort, int right_length,
                                           int* array_sort, int length)
{
    int k = 0;
    int i = 0;
    int j = 0;
    long long inversions = 0;

    for (; k < length; k++) {
        if (left_sort[i] < right_sort[j]) {
           array_tmp[k] = left_sort[i];
           if (++i == left_length) {
               while (++k < length) {
                   array_tmp[k] = right_sort[j++];
               }
               break;
           }
        } else {
           array_tmp[k] = right_sort[j];
           inversions += left_length - i;
           if (++j == right_length) {
               while (k++ < length) {
                   array_tmp[k] = left_sort[i++];
               }
               break;
           }
        }
    }
    
    memcpy(array_sort, array_tmp, length * sizeof(int));
    return inversions;
}

long long sort_and_count_inversions(int* array, int length)
{
    if (length == 1) return 0;

    int* left = array;
    int left_length = length / 2;
    int* right = array + left_length;
    int right_length = length - left_length;

    return sort_and_count_inversions(left,  left_length) +
           sort_and_count_inversions(right, right_length) +
           merge_and_split_count_inversions (left,  left_length,
                                             right, right_length,
                                             array, length);
}

int main(int argc, char *argv[])
{
    int num_ints = 0;

    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    if ((num_ints = load_integers(argv[1])) <= 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    long long inversions = sort_and_count_inversions(integers, num_ints);
    printf("inversions: %lli\n", inversions);
    return 0;
}

