#include <stdio.h>
#include <stdlib.h>

#include "vertex.h"
#include "dlist.h"

struct _Vertex {
    long ref;
    long id;
    int unexplored;
    long leader;
    DList * arcs;
};

typedef struct {
    int * finishing_time;
    int * t;
} FinishingTime;

static void _arc_dump (const void * data, const void * user_data)
{
    Vertex * vertex = (Vertex*)data;
    if (vertex == NULL) return;

    printf (" %li", vertex->id);
}

static void _arc_dfs_finishing_time (const void * data, const void * user_data)
{
    Vertex * vertex = (Vertex*)data;
    if (vertex == NULL) return;

    if (vertex_is_unexplored (vertex)) {
        FinishingTime * finishing = (FinishingTime*)user_data;
        vertex_dfs_finishing_time (vertex, finishing->finishing_time, finishing->t);
    }
}

static void _arc_dfs_discover_SCCs (const void * data, const void * user_data)
{
    Vertex * vertex = (Vertex*)data;
    if (vertex == NULL) return;

    long leader = (long)user_data;
    if (vertex_is_unexplored (vertex))
        vertex_dfs_discover_SCCs (vertex, leader);
}

static inline void _vertex_set_explored (Vertex * vertex)
{
    vertex->unexplored = 0;
}

static inline void _vertex_set_leader (Vertex * vertex, long leader)
{
    vertex->leader = leader;
}

static void _vertex_free (Vertex * vertex)
{
    dlist_free (vertex->arcs);
    free (vertex);
}

Vertex * vertex_new (long id)
{
    Vertex * vertex = malloc (sizeof(Vertex));
    if (vertex == NULL) return NULL; 

    vertex->ref = 1;
    vertex->id = id;
    vertex->unexplored = 1;
    vertex->leader = -1;
    vertex->arcs = dlist_new ();

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

void vertex_add_vertex (Vertex * vertex, Vertex * head)
{
    if (vertex == NULL || head == NULL) return;

    dlist_append(vertex->arcs, vertex_ref(head));
}

long vertex_id (Vertex * vertex)
{
    if (vertex == NULL) return 0;
    return vertex->id;
}

long vertex_leader (Vertex * vertex)
{
    if (vertex == NULL) return 0;
    return vertex->leader;
}

int vertex_is_unexplored (Vertex * vertex)
{
    if (vertex == NULL) return 0;
    return vertex->unexplored;
}

void vertex_dfs_finishing_time (Vertex * vertex, int * finishing_time, int * t)
{
    FinishingTime user_data = { finishing_time, t};

    if (vertex == NULL) return;

    _vertex_set_explored (vertex);
    dlist_foreach (vertex->arcs, _arc_dfs_finishing_time, &user_data);
    //finishing_time[vertex->id - 1] = *t;
    finishing_time[*t] = vertex->id - 1;
    *t = *t + 1;
}

void vertex_dfs_discover_SCCs (Vertex * vertex, long leader)
{
    if (vertex == NULL) return;

    _vertex_set_explored (vertex);
    _vertex_set_leader (vertex, leader);
    dlist_foreach (vertex->arcs, _arc_dfs_discover_SCCs, (void *)leader);
}

void vertex_dump (Vertex * vertex)
{
    if (vertex == NULL) return;
    
    printf ("vertex: %li [%s] leader: %li arcs:",
            vertex->id,
            vertex->unexplored?"unexplored":"explored",
            vertex->leader);
    dlist_foreach (vertex->arcs, _arc_dump, NULL);
    printf ("\n");
}
