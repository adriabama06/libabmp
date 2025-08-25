#include "abmp.h"

#include <stdio.h>

void abmp_print_header(ABMP_BITMAP_HEADER* header)
{
    printf("signature: %c%c\n", header->signature[0], header->signature[1]);
    printf("filesize: %d\n", header->filesize);
    printf("reserved: %d\n", header->reserved);
    printf("dataoffset: %d\n", header->dataoffset);
    printf("size: %d\n", header->size);
    printf("width: %d\n", header->width);
    printf("height: %d\n", header->height);
    printf("planes: %d\n", header->planes);
    printf("bits_per_pixel: %d\n", header->bits_per_pixel);
    printf("compression: %d\n", header->compression);
    printf("imagesize: %d\n", header->imagesize);
    printf("y_pixels_per_m: %d\n", header->y_pixels_per_m);
    printf("x_pixels_per_m: %d\n", header->x_pixels_per_m);
    printf("colors_used: %d\n", header->colors_used);
    printf("important_colors: %d\n", header->important_colors);
}
