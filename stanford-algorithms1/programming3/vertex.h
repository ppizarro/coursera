#ifndef __VERTEX_H__
#define __VERTEX_H__

typedef struct _vertex_element vertex_element;

#include "edge.h"

vertex_element * vertex_make (int id);

void vertex_destroy (vertex_element * vertex);

void vertex_add_edge (vertex_element * vertex, edge_element * edge);

int vertex_num_edges (vertex_element * vertex);

void edge_contract_vertices (edge_element * edge);

int vertex_id (vertex_element * vertex);

int vertex_size ();

void vertex_vertices_dump ();

#endif /* __VERTEX_H__ */
