#include "abitmap.h"

#include <stdio.h>

#ifndef ABMP_H
#define ABMP_H

#ifdef __cplusplus
extern "C" {
#endif

#define ABMP_HEADER_SIZE 54

typedef enum ABMP_ERRORS_E {
    ABMP_OK = 0,
    ABMP_IS_NOT_BMP_FILE,
    ABMP_BMP_DATA_IS_CORRUPTED,
    ABMP_COMPRESSION_IS_NOT_SUPPORTED,
    ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED,
    ABMP_OUT_OF_MEMORY,
    ABMP_FILE_SIZE_IS_LOWER_THAN_HEADER_SIZE,
    ABMP_FILE_NOT_EXIST
} ABMP_ERRORS;

void abmp_hello(void);
void abmp_hello2(void);
void abmp_hello3(void);
ABMP_ERRORS abmp_read_header(uint8_t* data, ABMP_BITMAP_HEADER* header);
ABMP_ERRORS abmp_read_data(uint8_t* data, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_file_p(FILE* file, ABMP_BITMAP* bitmap);
ABMP_ERRORS abmp_read_file(char* path, ABMP_BITMAP* bitmap);
void abmp_free(ABMP_BITMAP* bitmap);

uint32_t abmp_get_pixel_raw_position(ABMP_BITMAP_HEADER* header, uint32_t x, uint32_t y);
uint32_t abmp_get_pixel_position_from_top_left(ABMP_BITMAP_HEADER* header, uint32_t x, uint32_t y);

#ifdef __cplusplus
}
#endif

#endif // ABMP_H
