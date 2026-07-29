#include "abmp.h"


/**
 * @example
 * ABMP_BITMAP bitmap;
 * ABMP_ERRORS status = abmp_create_bitmap(&bitmap, 6, 4);
 * if(status != ABMP_OK) return "Oh no...";
 * abmp_free(&bitmap);
 */
ABMP_ERRORS abmp_create_bitmap(ABMP_BITMAP* bitmap, uint32_t width, uint32_t height)
{
    memset(&bitmap->header, 0, sizeof(ABMP_BITMAP_HEADER));

    bitmap->header.signature[0] = 'B';
    bitmap->header.signature[1] = 'M';

    bitmap->header.dataoffset = ABMP_HEADER_SIZE;

    bitmap->header.width = width;
    bitmap->header.height = height;

    bitmap->header.bits_per_pixel = 24;
    bitmap->header.planes = 1;
    bitmap->header.size = 40;

    uint32_t padding = width % 4;
    
    /*   imagesize = width*height*3(colors BGR) + padding * height   */
    bitmap->header.imagesize = width * height * 3 + padding * height;

    bitmap->header.filesize = ABMP_HEADER_SIZE + bitmap->header.imagesize;

    bitmap->pixel_data = (uint8_t*) malloc(bitmap->header.imagesize * sizeof(uint8_t));

    if(bitmap->pixel_data == NULL) return ABMP_OUT_OF_MEMORY;

    memset(bitmap->pixel_data, 255, bitmap->header.imagesize * sizeof(uint8_t));

    return ABMP_OK;
}

void abmp_free(ABMP_BITMAP* bitmap)
{
    free(bitmap->pixel_data);
    memset(&bitmap->header, 0, sizeof(ABMP_BITMAP_HEADER));
    bitmap->pixel_data = NULL;
}
