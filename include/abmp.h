#include "abitmap.h"

#ifndef ABMP_H
#define ABMP_H

#ifdef __cplusplus
extern "C" {
#endif

void abmp_hello(void);
size_t abmp_read_header(uint8_t* data, ABMP_BITMAP_HEADER* header);
size_t abmp_read_data(uint8_t* data, ABMP_BITMAP* bitmap);
uint32_t abmp_get_pixel_raw_position(ABMP_BITMAP_HEADER header, uint32_t x, uint32_t y);
uint32_t abmp_get_pixel_position_from_top_left(ABMP_BITMAP_HEADER header, uint32_t x, uint32_t y);

#ifdef __cplusplus
}
#endif

#endif // ABMP_H
