#include "abitmap.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#ifndef ABMP_H
#define ABMP_H

#ifdef __cplusplus
extern "C" {
#endif

#define ABMP_HEADER_SIZE 54

// defined in the first line of read.c
static const unsigned long __BMP_MEMORY_SIZES[] = {
    sizeof(uint8_t) * 2,
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint16_t),
    sizeof(uint16_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t)
};

size_t __BMP_MEMORY_OFFSETS(size_t count);

typedef enum ABMP_ERRORS_E {
    ABMP_OK = 0,
    ABMP_IS_NOT_BMP_FILE,
    ABMP_BMP_DATA_IS_CORRUPTED,
    ABMP_COMPRESSION_IS_NOT_SUPPORTED,
    ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED,
    ABMP_OUT_OF_MEMORY,
    ABMP_FILE_SIZE_IS_LOWER_THAN_HEADER_SIZE,
    ABMP_FILE_NOT_EXIST,
    ABMP_ERROR_READING_FILE,
    ABMP_ERROR_WRITING_FILE,
    ABMP_ERROR_OPENING_FILE
} ABMP_ERRORS;

/* WORKING ON MEMORY (memory -> memory | memory -> memory) */
// Read
ABMP_ERRORS abmp_read_header_from_memory(uint8_t* data, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_read_pixeldata_from_memory(uint8_t* data, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_file_p_using_memory(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_filepath_using_memory(char* path, ABMP_BITMAP* bitmap);

// Write
uint8_t* abmp_allocate_filedata(ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_write_header_to_memory(uint8_t* data, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_write_pixeldata_to_memory(uint8_t* data, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_write_file_p_using_memory(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_write_filepath_using_memory(char* path, ABMP_BITMAP* bitmap);

/* WORKING DIRECTLY FROM A FILE (file -> memory | memory -> file) */
// Read
ABMP_ERRORS abmp_read_header_from_file(FILE* file, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_read_pixeldata_from_file(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_file_p_using_direct(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_filepath_using_direct(char* path, ABMP_BITMAP* bitmap);

// Write
ABMP_ERRORS abmp_write_header_to_file(FILE* file, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_write_pixeldata_to_file(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_write_file_p_using_direct(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_write_filepath_using_direct(char* path, ABMP_BITMAP* bitmap);

// TODO: Add tests for abmp_openfile & abmp_savefile

/* User helpers */
inline ABMP_ERRORS abmp_openfile(char* filepath, ABMP_BITMAP* bitmap) {
    return abmp_read_filepath_using_direct(filepath, bitmap);
}

inline ABMP_ERRORS abmp_savefile(char* filepath, ABMP_BITMAP* bitmap) {
    return abmp_write_filepath_using_direct(filepath, bitmap);
}

/* Bitmap helpers */
#include "get.h"
#include "draw.h"

void abmp_print_header(ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_create_bitmap(ABMP_BITMAP* bitmap, uint32_t width, uint32_t height);
void abmp_free(ABMP_BITMAP* bitmap);

#ifdef __cplusplus
}
#endif

#endif // ABMP_H
