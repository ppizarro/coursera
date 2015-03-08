#ifndef __VERTEX_H__
#define __VERTEX_H__

typedef struct _Vertex Vertex;

typedef void (*VertexEdgeFunc) (Vertex * tail, Vertex * head, int distance, void * user_data);

Vertex * vertex_new (long id);

Vertex * vertex_ref (Vertex * vertex);

void vertex_unref (Vertex * vertex);

void vertex_add_edge (Vertex * vertex, Vertex * head, long distance);

long vertex_id (Vertex * vertex);

int vertex_explored (Vertex * vertex);

void vertex_do_explored (Vertex * vertex);

long vertex_key (Vertex * vertex);

void vertex_set_key (Vertex * vertex, long key);

void vertex_foreach_edge (Vertex * vertex, VertexEdgeFunc func, void * user_data);

void vertex_dump (Vertex * vertex);

#endif /* __VERTEX_H__ */
