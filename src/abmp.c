#include "abmp.h"

#include <stdio.h>
#include <stdlib.h>

void abmp_hello(void) {
    printf("Hello world from libabmp\n");

    ABMP_BITMAP test;
    printf("ABMP_BITMAP_HEADER = %d\n", sizeof(test.header));
    printf("ABMP_BITMAP_HEADER = %d\n", sizeof(ABMP_BITMAP_HEADER));

    printf("--- VERSION 1 ---\n");
    for (size_t i = 0; i <= sizeof(int)*2; i++)
    {
        printf("Padding for %d width: %d\n", i, i - ((i / 4) * 4));
    }

    printf("--- VERSION 2 ---\n");
    for (size_t i = 0; i <= sizeof(int)*2; i++)
    {
        printf("Padding for %d width: %d\n", i, i % 4);
    }

    char path[] = "/workspaces/libabmp/samples/twoofpadding.bmp";

    FILE* f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    int len = ftell(f);

    char* data = (char*) malloc(len * sizeof(char));

    rewind(f);

    fread(data, sizeof(char), len, f);

    ABMP_BITMAP_HEADER header;
    
    size_t status = abmp_read_header(data, &header);
    int padding = header.width % 4;

    printf("%d: %dx%dx3+%dx%d =? %d\n", status, header.width, header.height, padding, header.height, header.imagesize);

    ABMP_BITMAP bmp;

    bmp.header = header; // or directly create ^^^ bmp and pass to abmp_read_header(data, &bmp.header);
    
    status = abmp_read_data(data, &bmp);

    free(data);
    fclose(f);


    printf("--- V1 ---\n");

    for (int i = 0; i < bmp.header.imagesize;)
    {
        for (int j = 0; j < bmp.header.width; j++)
        {
            uint8_t b = bmp.pixel_data[i++];
            uint8_t g = bmp.pixel_data[i++];
            uint8_t r = bmp.pixel_data[i++];

            printf("(%d,%d,%d) ", r, g, b);
        }
        printf("\n");

        i += padding;
    }

    printf("--- V2 (Fixed vertically) ---\n");

    for (int i = 0; i < bmp.header.height; i++) // this is easy to convert directly into a single math eval to get the position based on x & y --> abmp_get_pixel(bmp, x, y);
    {
        for (int j = 0; j < bmp.header.width; j++)
        {
            uint8_t b = bmp.pixel_data[bmp.header.width*(bmp.header.height - i - 1)*3 + padding*(bmp.header.height - i - 1) + 3*j];
            uint8_t g = bmp.pixel_data[bmp.header.width*(bmp.header.height - i - 1)*3 + padding*(bmp.header.height - i - 1) + 3*j + 1];
            uint8_t r = bmp.pixel_data[bmp.header.width*(bmp.header.height - i - 1)*3 + padding*(bmp.header.height - i - 1) + 3*j + 2];

            printf("(%d,%d,%d) ", r, g, b);
        }
        printf("\n");
    }

    printf("--- Direct pixels V1 ---\n");

    // First +2,+1,+0 --> Because of the conversion of BGR -> RGB
    printf("0, 0 : (%d,%d,%d)\n",
        bmp.pixel_data[abmp_get_pixel_raw_position(bmp.header, 0, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_raw_position(bmp.header, 0, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_raw_position(bmp.header, 0, 0)]);
    
    printf("0, 0 : (%d,%d,%d)\n",
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(bmp.header, 0, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(bmp.header, 0, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(bmp.header, 0, 0)]);
    
    printf("2, 0 : (%d,%d,%d)\n",
        bmp.pixel_data[abmp_get_pixel_raw_position(bmp.header, 2, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_raw_position(bmp.header, 2, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_raw_position(bmp.header, 2, 0)]);

    printf("2, 0 : (%d,%d,%d)\n",
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(bmp.header, 2, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(bmp.header, 2, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(bmp.header, 2, 0)]);


    free(bmp.pixel_data);
}
