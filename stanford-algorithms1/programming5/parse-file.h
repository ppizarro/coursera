#ifndef __PARSE_FILE_H__
#define __PARSE_FILE_H__

#include "vertex.h"

int load_graph(const char * filename, Vertex ** vertices, int max_size, int * num_vertices);

#endif /* __PARSE_FILE_H__ */
