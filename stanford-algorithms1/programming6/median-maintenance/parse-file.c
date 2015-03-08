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

static inline char * find_end_of_line (char * addr, int length)
{
    return memchr(addr, '\n', length);
}

static void parse_file (char * addr, int length, long * data, int max_size, int * num_integers)
{
    char * line = addr;
    char * end = addr + length;
    char * eof = NULL;
    int pos = 0;

    do {
        eof = find_end_of_line (line, length);
        if (eof == NULL)
             break;
        *eof = '\0';

        data[pos++] = atol(line);
        if (pos >= max_size) {
            *num_integers = max_size;
            return;
        }

        line = eof + 1;
    } while (line < end);

    *num_integers = pos;
}

int load_integers(const char * filename, long * data, int max_size, int * num_integers)
{
    int fd;
    struct stat sb;
    size_t length;
    char * addr = NULL;

    if (data == NULL) return -1;
    if (num_integers == NULL) return -1;

    bzero (data, max_size * sizeof(long));

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

    parse_file (addr, length, data, max_size, num_integers);

    munmap (addr, length);
    close (fd);
    return 0;
}
