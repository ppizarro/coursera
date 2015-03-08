#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "parse-file.h"

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define DELIMIT " \t\r"

static void parse_line (char * line)
{
    char * token = NULL;
    char * saveptr = NULL;

    //printf("line: %s\n", line);
    token = strtok_r(line, DELIMIT, &saveptr);
    if (token == NULL)
        return;
    vertex_element * left = vertex_make (atoi(token));
    //printf("vertex: %s ", token);

    for (;;) {
        token = strtok_r(NULL, DELIMIT, &saveptr);
        if (token == NULL)
            break;
        vertex_element * right = vertex_make (atoi(token));

        edge_make (left, right);
    }

    //printf("edges: %i\n\n", vertex_num_edges(left));
}

static inline char * find_end_of_line (char * addr, int length)
{
    return memchr(addr, '\n', length);
}

static void parse_file (char * addr, int length)
{
    char * line = addr;
    char * end = addr + length;
    char * eof = NULL;

    do {
        eof = find_end_of_line (line, length);
        if (eof == NULL)
             break;
        *eof = '\0';

        parse_line (line);

        line = eof + 1;
    } while (line < end);
}

int load_matrix(const char * filename)
{
    int fd;
    struct stat sb;
    size_t length;
    char * addr = NULL;

    if ((fd = open (filename, O_RDONLY)) == -1)
        handle_error("open");

    if (fstat (fd, &sb) == -1)
        handle_error("fstat");
    length = sb.st_size;

    addr = mmap (NULL, length, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
        handle_error("mmap");

    parse_file (addr, length);

    munmap (addr, length);
    close (fd);
    return 0;
}

