#include <stdio.h>

#define MAX_INTS 100000

static unsigned int integers[MAX_INTS];

int load_integers(const char * filename)
{
    FILE *fp = NULL;
    unsigned int value;
    int i = -1;

    if ((fp = fopen (filename, "r")) == NULL)
        return -1;

    while (fscanf (fp, "%u", &value) && !feof(fp) && ++i < MAX_INTS ) 
        integers[i] = value;

    fclose (fp);
    return i + 1;
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

    int i = 0;
    int j = 0;
    long long int inversions = 0;
    for (; i<num_ints; i++) {
        for (j=i+1; j<num_ints; j++) {
            if (integers[i] > integers[j]) 
                inversions++;
        }
    }

    printf("inversions: %lli\n", inversions);
    return 0;
}

