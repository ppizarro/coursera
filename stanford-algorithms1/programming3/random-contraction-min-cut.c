#include <stdio.h>
#include <stdlib.h>

#include "parse-file.h"
#include "vertex.h"
#include "edge.h"

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void configure_seed ()
{
    FILE * dev_urandom = NULL;
    unsigned int seed;

    dev_urandom = fopen("/dev/urandom", "rb");
    if (dev_urandom == NULL)
        handle_error ("Error on opening /dev/urandom");

    setvbuf (dev_urandom, NULL, _IONBF, 0);
    if (fread (&seed, sizeof (seed), 1, dev_urandom) != 1)
        handle_error ("Error on reading /dev/urandom");

    fclose (dev_urandom);

    srandom(seed);
}

static inline long int random_edge ()
{
    return random() % edge_size ();
}

static int random_contraction_min_cut()
{
    configure_seed ();

    while (vertex_size () > 2) {
        edge_contract_vertices (edge_index (random_edge ()));
    }
    //edge_dump ();
   return edge_size ();
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    if (load_matrix(argv[1]) < 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    //vertex_vertices_dump ();
    //edge_dump ();

    int min_cuts = random_contraction_min_cut ();
    printf("min cuts: %i\n", min_cuts);

    return min_cuts;
}

