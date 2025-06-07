#include <stdint.h>
#include <stdlib.h>

#ifndef ABMP_ABITMAP_H
#define ABMP_ABITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

// http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
typedef struct
#if ABMP_ABITMAP_NOT_USE_PACKED_HEADER == 1 // Let the user select if want to use or not padding in header
ABMP_BITMAP_HEADER_S
#else
__attribute__((__packed__))ABMP_BITMAP_HEADER_S
#endif
{
    uint8_t signature[2];
    uint32_t filesize;
    uint32_t reserved;
    uint32_t dataoffset;

    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t y_pixels_per_m;
    uint32_t x_pixels_per_m;
    uint32_t colors_used;
    uint32_t important_colors;
} ABMP_BITMAP_HEADER;

// Append the dynamic sized structs
typedef struct ABMP_BITMAP_S
{
    ABMP_BITMAP_HEADER header;
    uint8_t* color_table;
    uint8_t* pixel_data;
} ABMP_BITMAP;

#ifdef __cplusplus
}
#endif

#endif // ABMP_ABITMAP_H
