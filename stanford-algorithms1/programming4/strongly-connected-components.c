#include <stdio.h>
#include <string.h>

#include "vertex.h"

#define MAX_VERTICES 1024 * 1024

static Vertex * vertices_original[MAX_VERTICES];

static Vertex * vertices_reversed[MAX_VERTICES];

static int finishing_time[MAX_VERTICES];

static int leaders[MAX_VERTICES];

static int num_vertices = 0;

static int last_index = -1;

static int top5_leaders_size[5] = {0, 0, 0, 0, 0};

static Vertex * vertice_original_get_or_build (long id)
{
    if (id <= 0 || id >= MAX_VERTICES)
        return NULL;

    int index = id - 1;
    Vertex * vertex = vertices_original[index];
    if (vertex == NULL) {
        vertex = vertex_new (id);
        vertices_original[index] = vertex;
        if (last_index < index)
            last_index = index;
        num_vertices++;
    }
    return vertex;
}

static Vertex * vertice_reversed_get_or_build (long id)
{
    if (id <= 0 || id >= MAX_VERTICES)
        return NULL;

    int index = id - 1;
    Vertex * vertex = vertices_reversed[index];
    if (vertex == NULL) {
        vertex = vertex_new (id);
        vertices_reversed[index] = vertex;
    }
    return vertex;
}

static int load_arcs(const char * filename)
{
    FILE *fp = NULL;
    long tail, head;

    if ((fp = fopen (filename, "r")) == NULL)
        return -1;

    while (fscanf (fp, "%li %li", &tail, &head) && !feof(fp))
    {
        //printf("%li %li \n", tail, head);
        vertex_add_vertex (vertice_original_get_or_build (tail),
                           vertice_original_get_or_build (head));

        vertex_add_vertex (vertice_reversed_get_or_build (head),
                           vertice_reversed_get_or_build (tail));
    }
    fclose (fp);

    return 0;
}

static void compute_magical_ordering ()
{
    int i = 0;
    int t = 0;
    while (i <= last_index) {
        Vertex * vertex = vertices_reversed[i++];
        //if (vertex == NULL) continue;
        if (vertex_is_unexplored (vertex))
            vertex_dfs_finishing_time (vertex, finishing_time, &t);
    }
}

static void discover_SCCs ()
{
    int t = num_vertices - 1;
    while (t >= 0) {
        int i = finishing_time[t--];
        Vertex * vertex = vertices_original[i];
        if (vertex_is_unexplored (vertex))
            vertex_dfs_discover_SCCs (vertex, vertex_id(vertex));
    }
}

static void compute_leaders ()
{
    int i = 0;
    for (; i <= last_index; i++) {
        Vertex * vertex = vertices_original[i];
        if (vertex == NULL) continue;
        long leader = vertex_leader (vertex);
        leaders[leader - 1]++;
    }
}

static void insert_top5 (int pos, int leader_size)
{
    int i = 4;
    for (; i > pos; i--) {
        top5_leaders_size[i] = top5_leaders_size[i - 1];
    }
    top5_leaders_size[pos] = leader_size;
}

static void show_top5_leaders_size ()
{
    int i = 0;
    for (; i <= last_index; i++) {
        int leader_size = leaders[i];
        if (leader_size > 0) {
            if (leader_size > top5_leaders_size[0]) {
                insert_top5 (0, leader_size);
            } else if (leader_size > top5_leaders_size[1]) {
                insert_top5 (1, leader_size);
            } else if (leader_size > top5_leaders_size[2]) {
                insert_top5 (2, leader_size);
            } else if (leader_size > top5_leaders_size[3]) {
                insert_top5 (3, leader_size);
            } else if (leader_size > top5_leaders_size[4]) {
                insert_top5 (4, leader_size);
            }
        }
    }

    for (i=0; i < 4; i++) {
        printf("%i,", top5_leaders_size[i]);
    }
    printf("%i\n", top5_leaders_size[4]);
}

/*
static void finishing_time_dump ()
{
    printf ("------- finishing time ----------\n");
    int i = 0;
    for (; i < num_vertices; i++) {
        printf("f[%i] = %i\n", i + 1, finishing_time[i] + 1);
    }
}

static void vertices_dump (Vertex ** vertices)
{
    printf ("#vertices: %i\n", num_vertices);
    printf ("#last_index: %i\n", last_index);
    int i = 0;
    while (i <= last_index) {
        vertex_dump (vertices[i++]);
    }
}

static void leaders_dump ()
{
    printf ("------- leaders ----------\n");
    int i = 0;
    for (; i <= last_index; i++) {
        if (leaders[i] > 200)
            printf("leader[%i] = %i\n", i + 1, leaders[i]);
    }
}
*/

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    bzero (vertices_original, MAX_VERTICES * sizeof(Vertex*));
    bzero (vertices_reversed, MAX_VERTICES * sizeof(Vertex*));
    bzero (finishing_time, MAX_VERTICES * sizeof(int));
    bzero (leaders, MAX_VERTICES * sizeof(int));

    if (load_arcs(argv[1]) < 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    //printf ("---------------- Reversed ---------------\n");
    //vertices_dump (vertices_reversed);

    compute_magical_ordering ();

    //finishing_time_dump ();

    discover_SCCs ();

    //printf ("---------------- Original ---------------\n");
    //vertices_dump (vertices_original);

    compute_leaders ();

    //leaders_dump ();

    show_top5_leaders_size ();

    return 0;
}
