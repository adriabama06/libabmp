#include "abmp.h"

#include <stdio.h>

void abmp_print_header(ABMP_BITMAP_HEADER* header)
{
    printf("signature: %c%c\n", header->signature[0], header->signature[1]);
    printf("filesize: %u\n", header->filesize);
    printf("reserved: %u\n", header->reserved);
    printf("dataoffset: %u\n", header->dataoffset);
    printf("size: %u\n", header->size);
    printf("width: %u\n", header->width);
    printf("height: %u\n", header->height);
    printf("planes: %u\n", header->planes);
    printf("bits_per_pixel: %u\n", header->bits_per_pixel);
    printf("compression: %u\n", header->compression);
    printf("imagesize: %u\n", header->imagesize);
    printf("y_pixels_per_m: %u\n", header->y_pixels_per_m);
    printf("x_pixels_per_m: %u\n", header->x_pixels_per_m);
    printf("colors_used: %u\n", header->colors_used);
    printf("important_colors: %u\n", header->important_colors);
}
