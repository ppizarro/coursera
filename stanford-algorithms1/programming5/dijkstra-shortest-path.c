#include <stdio.h>
#include <string.h>

#include "vertex.h"
#include "parse-file.h"
#include "heap.h"

#define MAX_VERTICES 1024 * 1024

static Vertex * vertices[MAX_VERTICES];

static int shortest_distances[MAX_VERTICES + 1];

static int num_vertices = 0;

static void recompute_key (Vertex * tail, Vertex * head, int distance, void * user_data)
{
    if (vertex_explored (head)) return;

    Heap * heap = (Heap*) user_data;   
    heap_remove (heap, head);

    long key = vertex_key (head);
    long dijkstra_score = shortest_distances[vertex_id(tail)] + distance;

    vertex_set_key (head, key < dijkstra_score ? key : dijkstra_score);

    heap_push (heap, head);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s <file>\n", argv[0]);
        return -1;
    }

    if (load_graph(argv[1], vertices, MAX_VERTICES, &num_vertices) < 0) {
        fprintf(stderr, "Error on reading intergers from file: %s\n", argv[1]);
        return -1;
    }

    bzero (shortest_distances, (MAX_VERTICES + 1) * sizeof(int));

    Vertex * source = vertices[0];
    vertex_set_key (source, 0);
    
    Heap * heap = heap_new_with_data ((void **)vertices, num_vertices, 0,
                                      (HeapDataKeyFunc)vertex_key,
                                      (HeapDataDestroyFunc)vertex_unref);
    while (heap_size (heap) > 0) {
        //heap_dump (heap);

        Vertex * vertex = heap_pop (heap);

        printf ("Vertex: %li score: %li\n",
                vertex_id (vertex),
                vertex_key (vertex));

        vertex_do_explored (vertex);
        shortest_distances[vertex_id(vertex)] = vertex_key (vertex);
        vertex_foreach_edge(vertex, recompute_key, heap);
    }

    printf("%i,%i,%i,%i,%i,%i,%i,%i,%i,%i\n",
           shortest_distances[7],
           shortest_distances[37],
           shortest_distances[59],
           shortest_distances[82],
           shortest_distances[99],
           shortest_distances[115],
           shortest_distances[133],
           shortest_distances[165],
           shortest_distances[188],
           shortest_distances[197]);

    return 0;
}
