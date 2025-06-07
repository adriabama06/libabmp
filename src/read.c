#include "abmp.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

const char __BMP_MEMORY_SIZES[] = {
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

size_t __BMP_MEMORY_OFFSETS(size_t count)
{
    size_t offset = 0;

    for (size_t i = 1; i < count; i++)
    {
        offset += __BMP_MEMORY_SIZES[i - 1];
    }

    return offset;
}

/**
 * It returns 0 if ok, any other code means other errors
 */
size_t abmp_read_header(uint8_t* data, ABMP_BITMAP_HEADER* header)
{
    if(sizeof(ABMP_BITMAP_HEADER) == 54) 
    {
        memcpy(header, data, sizeof(ABMP_BITMAP_HEADER));
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;
        
        memcpy(&header->signature,        data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->filesize,         data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->reserved,         data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->dataoffset,       data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->size,             data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->width,            data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->height,           data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->planes,           data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->bits_per_pixel,   data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->compression,      data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->imagesize,        data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->y_pixels_per_m,   data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->x_pixels_per_m,   data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->colors_used,      data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->important_colors, data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
    }

    if(header->signature[0] != 'B' && header->signature[1] != 'M')
    {
        // This is not a BMP file or it's corrupted. Let the user wipe by it self the header data.
        return 1;
    }

    if(header->width * header->height * 3 + (header->width % 4) * header->height != header->imagesize)
    {
        // A: This is not a BMP file, B: The file is wrong.
        return 2;
    }

    return 0;
}

ABMP_BITMAP abmp_read_data(uint8_t* data, ABMP_BITMAP_HEADER header)
{
    ABMP_BITMAP bitmap;

    bitmap.header = header;

    if(bitmap.header.bits_per_pixel <= 8) {
        // Do something with ColorTable
    }

    if(bitmap.header.compression != 0) {
        // Has compression
    }

    bitmap.pixel_data = (char*) malloc(bitmap.header.imagesize);

    memcpy(bitmap.pixel_data, data + bitmap.header.dataoffset, bitmap.header.imagesize);

    return bitmap;
}
