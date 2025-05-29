#include "abmp.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

ABMP_BITMAP_HEADER abmp_read_header(uint8_t* data)
{
    ABMP_BITMAP_HEADER header;

    if(sizeof(ABMP_BITMAP_HEADER) != 54) // This means __attribute__((__packed__)) is not working, leaving to a "manual" but tricky read
    {
        if(sizeof(ABMP_BITMAP_HEADER) != 56) // This can be fixed doing a real manual map to memory, this tricky implementation is only for 56 bytes case
        {
            printf("Unexpected sizeof(ABMP_BITMAP_HEADER) = %d; Exiting...\n", sizeof(ABMP_BITMAP_HEADER));
            exit(1);
        }

        // The __attribute__((__packed__)) only affects on the signature, adding 2 more of uint8_t
        memcpy(&header.signature, data, sizeof(uint8_t)*2);

        memcpy(&header.filesize, data + sizeof(uint8_t)*2, 52);
    }
    else
    {
        memcpy(&header, data, sizeof(ABMP_BITMAP_HEADER));
    }

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
