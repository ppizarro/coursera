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

static void parse_file (char * addr, int length, GHashTable * hash)
{
    char * line = addr;
    char * end = addr + length;
    char * eof = NULL;

    do {
        eof = find_end_of_line (line, length);
        if (eof == NULL)
             break;
        *eof = '\0';

        g_hash_table_add (hash, GINT_TO_POINTER(atoi(line)));

        line = eof + 1;
    } while (line < end);
}

int load_hash(const char * filename, GHashTable * hash)
{
    int fd;
    struct stat sb;
    size_t length;
    char * addr = NULL;

    if (hash == NULL) return -1;

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

    parse_file (addr, length, hash);

    munmap (addr, length);
    close (fd);
    return 0;
}
