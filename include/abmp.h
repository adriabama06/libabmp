#include "abitmap.h"

#include <stdio.h>

#ifndef ABMP_H
#define ABMP_H

#ifdef __cplusplus
extern "C" {
#endif

#define ABMP_HEADER_SIZE 54

void abmp_hello(void);
void abmp_hello2(void);
void abmp_hello3(void);
size_t abmp_read_header(uint8_t* data, ABMP_BITMAP_HEADER* header);
size_t abmp_read_data(uint8_t* data, ABMP_BITMAP* bitmap);
size_t abmp_read_file_p(FILE* file, ABMP_BITMAP* bitmap);
size_t abmp_read_file(char* path, ABMP_BITMAP* bitmap);
void abmp_free(ABMP_BITMAP* bitmap);

uint32_t abmp_get_pixel_raw_position(ABMP_BITMAP_HEADER* header, uint32_t x, uint32_t y);
uint32_t abmp_get_pixel_position_from_top_left(ABMP_BITMAP_HEADER* header, uint32_t x, uint32_t y);

#ifdef __cplusplus
}
#endif

#endif // ABMP_H
