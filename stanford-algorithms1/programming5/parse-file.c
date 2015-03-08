#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "parse-file.h"

#define DELIMIT " \t\r"

static Vertex * vertice_get_or_build (long id, Vertex ** vertices, int max_size, int * num_vertices)
{
    if (id <= 0 || id >= max_size)
        return NULL;

    int index = id - 1;
    Vertex * vertex = vertices[index];
    if (vertex == NULL) {
        vertex = vertex_new (id);
        vertices[index] = vertex;
        *num_vertices = *num_vertices + 1;
    }
    return vertex;
}

static void parse_line (char * line, Vertex ** vertices, int max_size, int * num_vertices)
{
    char * token = NULL;
    char * saveptr = NULL;

    //printf("line: %s\n", line);
    token = strtok_r(line, DELIMIT, &saveptr);
    if (token == NULL) return;

    Vertex * tail = vertice_get_or_build (atoi(token), vertices, max_size, num_vertices);

    for (;;) {
        token = strtok_r(NULL, DELIMIT, &saveptr);
        if (token == NULL) break;

        long id;
        long distance;
        if (sscanf (token, "%li,%li", &id, &distance) == 2) {
            Vertex * head = vertice_get_or_build (id, vertices, max_size, num_vertices);
            vertex_add_edge (tail, head, distance);
        }
    }
}

static inline char * find_end_of_line (char * addr, int length)
{
    return memchr(addr, '\n', length);
}

static void parse_file (char * addr, int length, Vertex ** vertices, int max_size, int * num_vertices)
{
    char * line = addr;
    char * end = addr + length;
    char * eof = NULL;

    do {
        eof = find_end_of_line (line, length);
        if (eof == NULL)
             break;
        *eof = '\0';

        parse_line (line, vertices, max_size, num_vertices);

        line = eof + 1;
    } while (line < end);
}

int load_graph(const char * filename, Vertex ** vertices, int max_size, int * num_vertices)
{
    int fd;
    struct stat sb;
    size_t length;
    char * addr = NULL;

    if (vertices == NULL) return -1;
    if (num_vertices == NULL) return -1;

    bzero (vertices, max_size * sizeof(Vertex *));

    if ((fd = open (filename, O_RDONLY)) == -1) {
        perror("error on open file!");
        return -1;
    }

    if (fstat (fd, &sb) == -1) {
        perror("error on fstat file!");
        return -1;
    }
    length = sb.st_size;

    addr = mmap (NULL, length, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        perror("error on mmap file!");
        return -1;
    }

    parse_file (addr, length, vertices, max_size, num_vertices);

    munmap (addr, length);
    close (fd);
    return 0;
}
