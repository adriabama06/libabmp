#ifndef _WIN32

#include "abmp.h"

#include <sys/mman.h>
#include <sys/stat.h>

ABMP_ERRORS abmp_mmap_openfile_fd(int fd, ABMP_BITMAP* bitmap)
{
    struct stat st;

    if(fstat(fd, &st) < 0) return ABMP_ERROR_OPENING_FILE;

    size_t len = (size_t)st.st_size;

    if(len < ABMP_HEADER_SIZE) return ABMP_FILE_SIZE_IS_LOWER_THAN_HEADER_SIZE;

    // MAP_PRIVATE: writable copy-on-write view, writes never reach the file
    uint8_t* data = (uint8_t*) mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if(data == MAP_FAILED) return ABMP_OUT_OF_MEMORY;

    ABMP_ERRORS status = abmp_cast_from_memory(data, bitmap);

    if(status != ABMP_OK)
    {
        munmap(data, len);
        return status;
    }

    // The mapping length must match dataoffset + imagesize, otherwise abmp_mmap_close would unmap the wrong range
    if((size_t) bitmap->header.dataoffset + bitmap->header.imagesize != len)
    {
        munmap(data, len);
        return ABMP_FILE_IMAGESIZE_MISSMATCH;
    }

    return status;
}

#endif
