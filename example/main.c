#include "abmp.h"

void abmp_hello(void) {
    printf("abmp_hello()\n");

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
        bmp.pixel_data[abmp_get_pixel_raw_position(&bmp.header, 0, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_raw_position(&bmp.header, 0, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_raw_position(&bmp.header, 0, 0)]);
    
    printf("0, 0 : (%d,%d,%d)\n",
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(&bmp.header, 0, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(&bmp.header, 0, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(&bmp.header, 0, 0)]);
    
    printf("2, 0 : (%d,%d,%d)\n",
        bmp.pixel_data[abmp_get_pixel_raw_position(&bmp.header, 2, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_raw_position(&bmp.header, 2, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_raw_position(&bmp.header, 2, 0)]);

    printf("2, 0 : (%d,%d,%d)\n",
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(&bmp.header, 2, 0) + 2],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(&bmp.header, 2, 0) + 1],
        bmp.pixel_data[abmp_get_pixel_position_from_top_left(&bmp.header, 2, 0)]);


    free(bmp.pixel_data);
}

void abmp_hello2(void)
{
    printf("abmp_hello2()\n");

    char path[] = "/workspaces/libabmp/samples/twoofpadding.bmp";

    ABMP_BITMAP bitmap;

    size_t status = abmp_read_file(path, &bitmap);

    printf("Reading %s status: %d\n", path, status);

    printf("--- V1 ---\n");

    int padding = bitmap.header.width % 4;

    printf("%dx%dx3+%dx%d =? %d\n", bitmap.header.width, bitmap.header.height, padding, bitmap.header.height, bitmap.header.imagesize);


    for (int i = 0; i < bitmap.header.imagesize;)
    {
        for (int j = 0; j < bitmap.header.width; j++)
        {
            uint8_t b = bitmap.pixel_data[i++];
            uint8_t g = bitmap.pixel_data[i++];
            uint8_t r = bitmap.pixel_data[i++];

            printf("(%d,%d,%d) ", r, g, b);
        }
        printf("\n");

        i += padding;
    }

    printf("--- V2 (Fixed vertically) ---\n");

    for (int i = 0; i < bitmap.header.height; i++) // this is easy to convert directly into a single math eval to get the position based on x & y --> abmp_get_pixel(bmp, x, y);
    {
        for (int j = 0; j < bitmap.header.width; j++)
        {
            uint8_t b = bitmap.pixel_data[bitmap.header.width*(bitmap.header.height - i - 1)*3 + padding*(bitmap.header.height - i - 1) + 3*j];
            uint8_t g = bitmap.pixel_data[bitmap.header.width*(bitmap.header.height - i - 1)*3 + padding*(bitmap.header.height - i - 1) + 3*j + 1];
            uint8_t r = bitmap.pixel_data[bitmap.header.width*(bitmap.header.height - i - 1)*3 + padding*(bitmap.header.height - i - 1) + 3*j + 2];

            printf("(%d,%d,%d) ", r, g, b);
        }
        printf("\n");
    }

    printf("--- Direct pixels V1 ---\n");

    // First +2,+1,+0 --> Because of the conversion of BGR -> RGB
    printf("0, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0)]);
    
    printf("0, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0)]);
    
    printf("2, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 2, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 2, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 2, 0)]);

    printf("2, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0)]);

    abmp_free(&bitmap);
}

void abmp_hello3(void)
{
    printf("abmp_hello3()\n");

    char path[] = "/workspaces/libabmp/samples/twoofpadding.bmp";

    FILE* file = fopen(path, "rb");

    ABMP_BITMAP bitmap;

    size_t status = abmp_read_file_p(file, &bitmap);

    fclose(file);

    printf("Reading %s status: %d\n", path, status);

    printf("--- V1 ---\n");

    int padding = bitmap.header.width % 4;

    printf("%dx%dx3+%dx%d =? %d\n", bitmap.header.width, bitmap.header.height, padding, bitmap.header.height, bitmap.header.imagesize);


    for (int i = 0; i < bitmap.header.imagesize;)
    {
        for (int j = 0; j < bitmap.header.width; j++)
        {
            uint8_t b = bitmap.pixel_data[i++];
            uint8_t g = bitmap.pixel_data[i++];
            uint8_t r = bitmap.pixel_data[i++];

            printf("(%d,%d,%d) ", r, g, b);
        }
        printf("\n");

        i += padding;
    }

    printf("--- V2 (Fixed vertically) ---\n");

    for (int i = 0; i < bitmap.header.height; i++) // this is easy to convert directly into a single math eval to get the position based on x & y --> abmp_get_pixel(bmp, x, y);
    {
        for (int j = 0; j < bitmap.header.width; j++)
        {
            uint8_t b = bitmap.pixel_data[bitmap.header.width*(bitmap.header.height - i - 1)*3 + padding*(bitmap.header.height - i - 1) + 3*j];
            uint8_t g = bitmap.pixel_data[bitmap.header.width*(bitmap.header.height - i - 1)*3 + padding*(bitmap.header.height - i - 1) + 3*j + 1];
            uint8_t r = bitmap.pixel_data[bitmap.header.width*(bitmap.header.height - i - 1)*3 + padding*(bitmap.header.height - i - 1) + 3*j + 2];

            printf("(%d,%d,%d) ", r, g, b);
        }
        printf("\n");
    }

    printf("--- Direct pixels V1 ---\n");

    // First +2,+1,+0 --> Because of the conversion of BGR -> RGB
    printf("0, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0)]);
    
    printf("0, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0)]);
    
    printf("2, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 2, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 2, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 2, 0)]);

    printf("2, 0 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0)]);

    abmp_free(&bitmap);
}

void abmp_hello4(void)
{
    ABMP_ERRORS status;

    printf("abmp_hello4()\n");

    char input_path[] = "/workspaces/libabmp/samples/twoofpadding.bmp";
    char output_path[] = "/workspaces/libabmp/samples/twoofpadding_edit.bmp";

    ABMP_BITMAP bitmap;

    status = abmp_read_file(input_path, &bitmap);

    printf("status = %d\n", status);

    printf("2, 2 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)]);

    // BGR -- RGB(255,255,0) is Yellow -- BGR(0,255,255) is Yellow
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] = 0;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] = 255;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] = 255;

    printf("2, 2 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)]);

    status = abmp_write_file(output_path, &bitmap);

    printf("status = %d\n", status);

    printf("Done copy from %s to %s and edit in (2, 2)\n", input_path, output_path);

    abmp_free(&bitmap);
}

void abmp_hello5(void)
{
    ABMP_ERRORS status;

    printf("abmp_hello5()\n");

    char input_path[] = "/workspaces/libabmp/samples/twoofpadding.bmp";
    char output_path[] = "/workspaces/libabmp/samples/twoofpadding_edit2.bmp";

    ABMP_BITMAP bitmap;

    status = abmp_file_read_file(input_path, &bitmap);

    abmp_print_header(&bitmap.header);

    printf("status = %d\n", status);

    printf("2, 2 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)]);

    // BGR -- RGB(165,42,42) is Brown -- BGR(42,42,165) is Brown    
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] = 42;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] = 42;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] = 165;

    printf("2, 2 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)]);

    status = abmp_file_write_file(output_path, &bitmap);

    printf("status = %d\n", status);

    printf("Done copy from %s to %s and edit in (2, 2)\n", input_path, output_path);

    abmp_free(&bitmap);
}

void abmp_hello6(void)
{
    ABMP_ERRORS status;

    printf("abmp_hello6()\n");

    char output_path[] = "/workspaces/libabmp/samples/generated.bmp";

    ABMP_BITMAP bitmap;

    status = abmp_create_bitmap(&bitmap, 5, 5);

    abmp_print_header(&bitmap.header);

    printf("status = %d\n", status);

    // By default all must be white
    printf("2, 2 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)]);

    // BGR -- RGB(255,0,0) is Red -- BGR(0,0,255) is Red    
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] = 0;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] = 0;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] = 255;

    printf("2, 2 : (%d,%d,%d)\n",
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1],
        bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)]);

    status = abmp_file_write_file(output_path, &bitmap);

    printf("status = %d\n", status);

    printf("Generated file %s\n", output_path);

    abmp_free(&bitmap);
}

int main() {
    abmp_hello();
    abmp_hello2();
    abmp_hello3();

    abmp_hello4();
    abmp_hello5();

    abmp_hello6();
    return 0;
}
