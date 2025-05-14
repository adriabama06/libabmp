#include "abmp.h"

#include <stdio.h>
#include <stdlib.h>

void abmp_hello(void) {
    printf("Hello world from libabmp\n");

    ABMP_BITMAP test;
    printf("ABMP_BITMAP_HEADER = %d\n", sizeof(test.header));

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

    ABMP_BITMAP_HEADER header = abmp_read_header(data);

    printf("%dx%d\n", header.width, header.height);

    ABMP_BITMAP bmp = abmp_read_data(data, header);


    int padding = bmp.header.width % 4;

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

    printf("--- V2 ---\n");

    for (int i = 0; i < bmp.header.height; i++)
    {
        for (int j = 0; j < bmp.header.width; j++)
        {
            uint8_t b = bmp.pixel_data[bmp.header.width*i*3 + padding*i + 3*j];
            uint8_t g = bmp.pixel_data[bmp.header.width*i*3 + padding*i + 3*j + 1];
            uint8_t r = bmp.pixel_data[bmp.header.width*i*3 + padding*i + 3*j + 2];

            printf("(%d,%d,%d) ", r, g, b);
        }
        printf("\n");
    }
}
