#include "abmp.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

ABMP_BITMAP_HEADER abmp_read_header(uint8_t* data)
{
    ABMP_BITMAP_HEADER header;

    memcpy(&header, data, sizeof(ABMP_BITMAP_HEADER));

    return header;
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
