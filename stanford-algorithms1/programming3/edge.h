#ifndef __EDGE_H__
#define __EDGE_H__

typedef struct _edge_element edge_element;

#include "vertex.h"

void edge_make (vertex_element * left, vertex_element * right);

edge_element * edge_ref (edge_element * edge);

void edge_unref (edge_element * edge);

void edge_remove (edge_element * edge);

edge_element * edge_index (long int index);

vertex_element * edge_left (edge_element * edge);

vertex_element * edge_right (edge_element * edge);

void edge_change_vertex (edge_element * edge, vertex_element * old_vertex, vertex_element * new_vertex);

int edge_size ();

void edge_dump ();

void edge_remove_self_loops ();

#endif /* __EDGE_H__ */
