#include "abmp.h"

ABMP_ERRORS abmp_check_header(const ABMP_BITMAP_HEADER* header)
{
    if(header->signature[0] != 'B' || header->signature[1] != 'M')
    {
        // This is not a BMP file or it's corrupted. Let the user wipe by it self the header data.
        return ABMP_IS_NOT_BMP_FILE;
    }

    if(header->width * header->height * 3 + (header->width % 4) * header->height != header->imagesize && header->imagesize != 0) // It is valid to set imagesize = 0 if compression = 0
    {
        // A: This is not a BMP file, B: The file is wrong.
        return ABMP_BMP_DATA_IS_CORRUPTED;
    }

    return ABMP_OK;
}

ABMP_ERRORS abmp_header_compatibility_check(const ABMP_BITMAP_HEADER* header)
{
    if(header->compression != 0)
    {
        // Has compression
        return ABMP_COMPRESSION_IS_NOT_SUPPORTED;
    }

    if(header->bits_per_pixel <= 8)
    {
        // Do something with ColorTable
        return ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED;
    }

    return ABMP_OK;
}
