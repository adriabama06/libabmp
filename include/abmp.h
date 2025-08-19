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
extern const unsigned long __BMP_MEMORY_SIZES[];

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
ABMP_ERRORS abmp_read_header(uint8_t* data, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_read_data(uint8_t* data, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_file_p(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_file(char* path, ABMP_BITMAP* bitmap);
void abmp_free(ABMP_BITMAP* bitmap);

// Write
uint8_t* abmp_allocate_writer(ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_write_header(uint8_t* data, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_write_data(uint8_t* data, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_write_file_p(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_write_file(char* path, ABMP_BITMAP* bitmap);

/* WORKING DIRECTLY FROM A FILE (file -> memory | memory -> file) */
// Read
ABMP_ERRORS abmp_file_read_header(FILE* file, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_file_read_data(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_file_read_file_p(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_file_read_file(char* path, ABMP_BITMAP* bitmap);

// Write
ABMP_ERRORS abmp_file_write_header(FILE* file, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_file_write_data(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_file_write_file_p(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_file_write_file(char* path, ABMP_BITMAP* bitmap);

/* Bitmap helpers */
uint32_t abmp_get_pixel_raw_position(ABMP_BITMAP_HEADER* header, uint32_t x, uint32_t y);
uint32_t abmp_get_pixel_position_from_top_left(ABMP_BITMAP_HEADER* header, uint32_t x, uint32_t y);
ABMP_ERRORS abmp_create_bitmap(ABMP_BITMAP* bitmap, uint32_t width, uint32_t height);

// Only aviable during development for testing
void abmp_hello(void);
void abmp_hello2(void);
void abmp_hello3(void);
void abmp_hello4(void);
void abmp_hello5(void);
void abmp_hello6(void);

#ifdef __cplusplus
}
#endif

#endif // ABMP_H
