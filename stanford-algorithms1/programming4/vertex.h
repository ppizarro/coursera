#ifndef __VERTEX_H__
#define __VERTEX_H__

typedef struct _Vertex Vertex;

Vertex * vertex_new (long id);

Vertex * vertex_ref (Vertex * vertex);

void vertex_unref (Vertex * vertex);

void vertex_add_vertex (Vertex * vertex, Vertex * head);

long vertex_id (Vertex * vertex);

long vertex_leader (Vertex * vertex);

int vertex_is_unexplored (Vertex * vertex);

void vertex_dfs_finishing_time (Vertex * vertex, int * finishing_time, int * t);

void vertex_dfs_discover_SCCs (Vertex * vertex, long leader);

void vertex_dump (Vertex * vertex);

#endif /* __VERTEX_H__ */
