#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "vertex.h"
#include "dlist.h"

typedef struct _Edge {
    Vertex * head;
    long distance;
} Edge;

struct _Vertex {
    long ref;
    long id;
    long key;
    int explored;
    DList * edges;
};

typedef struct {
    Vertex * tail;
    VertexEdgeFunc func;
    void * user_data;
} EdgeClosure;

static Edge * _edge_alloc (Vertex * head, long distance)
{
    Edge * edge = malloc (sizeof(Edge));
    if (edge == NULL) return NULL; 

    edge->head = vertex_ref (head);
    edge->distance = distance;
    return edge;
}

static void _edge_free (void * data, void * user_data)
{
    Edge * edge = (Edge*)data;
    if (edge == NULL) return;

    vertex_unref (edge->head);
    free (edge);
}

static void _edge_func (void * data, void * user_data)
{
    Edge * edge = (Edge*)data;
    if (edge == NULL) return;

    EdgeClosure * closure = (EdgeClosure *)user_data;
    if (closure == NULL) return;
        
    closure->func (closure->tail, edge->head, edge->distance, closure->user_data);
}

static void _edge_dump (void * data, void * user_data)
{
    Edge * edge = (Edge*)data;
    if (edge == NULL) return;
    Vertex * head = edge->head;
    if (head == NULL) return;

    printf (" %li,%li", head->id, edge->distance);
}

static void _vertex_free (Vertex * vertex)
{
    dlist_foreach (vertex->edges, _edge_free, NULL);
    dlist_free (vertex->edges);
    free (vertex);
}

Vertex * vertex_new (long id)
{
    Vertex * vertex = malloc (sizeof(Vertex));
    if (vertex == NULL) return NULL; 

    vertex->ref = 1;
    vertex->id = id;
    vertex->explored = 0;
    vertex->key = LONG_MAX;
    vertex->edges = dlist_new ();

    return vertex;
}

Vertex * vertex_ref (Vertex * vertex)
{
    if (vertex == NULL) return NULL;

    vertex->ref++;
    return vertex;
}

void vertex_unref (Vertex * vertex)
{
    if (vertex == NULL) return;
    if (--vertex->ref == 0) _vertex_free (vertex);
}

void vertex_add_edge (Vertex * vertex, Vertex * head, long distance)
{
    if (vertex == NULL || head == NULL) return;

    Edge * edge = _edge_alloc (head, distance);
    dlist_append(vertex->edges, edge);
}

long vertex_id (Vertex * vertex)
{
    if (vertex == NULL) return 0;
    return vertex->id;
}

int vertex_explored (Vertex * vertex)
{
    if (vertex == NULL) return 0;
    return vertex->explored;
}

void vertex_do_explored (Vertex * vertex)
{
    if (vertex == NULL) return;
    vertex->explored = 1;    
}

long vertex_key (Vertex * vertex)
{
    if (vertex == NULL) return LONG_MAX;
    return vertex->key;
}

void vertex_set_key (Vertex * vertex, long key)
{
    if (vertex == NULL) return;
    vertex->key = key;
}

void vertex_foreach_edge (Vertex * vertex, VertexEdgeFunc func, void * user_data)
{
    if (vertex == NULL) return;

    EdgeClosure closure = { vertex, func, user_data };
    dlist_foreach (vertex->edges, _edge_func, &closure);
}

void vertex_dump (Vertex * vertex)
{
    if (vertex == NULL) return;
    
    printf ("vertex: %li [%s] edges:",
            vertex->id,
            vertex->explored?"explored":"unexplored");
    dlist_foreach (vertex->edges, _edge_dump, NULL);
    printf ("\n");
}
