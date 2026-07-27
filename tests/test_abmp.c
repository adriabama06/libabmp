#include "abmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ABMP_TEST_OUTPUT_PATH
#define ABMP_TEST_OUTPUT_PATH "libabmp_test_output.bmp"
#endif

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int check_bitmap_equal(const ABMP_BITMAP *expected, const ABMP_BITMAP *actual)
{
    CHECK(memcmp(&expected->header, &actual->header, sizeof(ABMP_BITMAP_HEADER)) == 0);
    CHECK(memcmp(expected->pixel_data, actual->pixel_data,
                 expected->header.imagesize) == 0);
    return 0;
}

static int test_create_and_pixel_positions(void)
{
    ABMP_BITMAP bitmap = {0};
    CHECK(abmp_create_bitmap(&bitmap, 3, 2) == ABMP_OK);

    CHECK(bitmap.header.signature[0] == 'B');
    CHECK(bitmap.header.signature[1] == 'M');
    CHECK(bitmap.header.dataoffset == ABMP_HEADER_SIZE);
    CHECK(bitmap.header.width == 3);
    CHECK(bitmap.header.height == 2);
    CHECK(bitmap.header.bits_per_pixel == 24);
    CHECK(bitmap.header.planes == 1);
    CHECK(bitmap.header.imagesize == 24); /* (3 * 3 + 3 padding) * 2 */
    CHECK(bitmap.header.filesize == 78);

    for (uint32_t i = 0; i < bitmap.header.imagesize; ++i)
        CHECK(bitmap.pixel_data[i] == 255);

    CHECK(abmp_get_pixel_raw_position(&bitmap.header, 0, 0) == 0);
    CHECK(abmp_get_pixel_raw_position(&bitmap.header, 2, 1) == 18);
    CHECK(abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) == 12);
    CHECK(abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 1) == 6);

    abmp_free(&bitmap);
    return 0;
}

static int make_bitmap(ABMP_BITMAP *bitmap)
{
    CHECK(abmp_create_bitmap(bitmap, 3, 2) == ABMP_OK);
    for (uint32_t i = 0; i < bitmap->header.imagesize; ++i)
        bitmap->pixel_data[i] = (uint8_t)(i * 11U + 3U);
    return 0;
}

static int test_memory_round_trip(void)
{
    ABMP_BITMAP original = {0};
    ABMP_BITMAP decoded = {0};
    uint8_t *encoded;

    if (make_bitmap(&original) != 0)
        return 1;

    encoded = abmp_allocate_writer(&original.header);
    CHECK(encoded != NULL);
    CHECK(abmp_write_header(encoded, &original.header) == ABMP_OK);
    CHECK(abmp_write_data(encoded, &original) == ABMP_OK);
    CHECK(abmp_read_header(encoded, &decoded.header) == ABMP_OK);
    CHECK(abmp_read_data(encoded, &decoded) == ABMP_OK);
    CHECK(check_bitmap_equal(&original, &decoded) == 0);

    free(encoded);
    abmp_free(&decoded);
    abmp_free(&original);
    return 0;
}

static int test_file_round_trips(void)
{
    ABMP_BITMAP original = {0};
    ABMP_BITMAP decoded = {0};
    FILE *file;

    if (make_bitmap(&original) != 0)
        return 1;

    file = tmpfile();
    CHECK(file != NULL);
    CHECK(abmp_file_write_file_p(file, &original) == ABMP_OK);
    rewind(file);
    CHECK(abmp_file_read_file_p(file, &decoded) == ABMP_OK);
    CHECK(check_bitmap_equal(&original, &decoded) == 0);
    fclose(file);
    abmp_free(&decoded);

    CHECK(abmp_write_file(ABMP_TEST_OUTPUT_PATH, &original) == ABMP_OK);
    memset(&decoded, 0, sizeof(decoded));
    CHECK(abmp_read_file(ABMP_TEST_OUTPUT_PATH, &decoded) == ABMP_OK);
    CHECK(check_bitmap_equal(&original, &decoded) == 0);
    remove(ABMP_TEST_OUTPUT_PATH);

    abmp_free(&decoded);
    abmp_free(&original);
    return 0;
}

static int test_error_handling(void)
{
    ABMP_BITMAP bitmap = {0};
    ABMP_BITMAP_HEADER header = {0};
    uint8_t buffer[ABMP_HEADER_SIZE] = {0};

    CHECK(abmp_read_file("this-file-does-not-exist.bmp", &bitmap) == ABMP_FILE_NOT_EXIST);

    CHECK(abmp_create_bitmap(&bitmap, 1, 1) == ABMP_OK);
    header = bitmap.header;
    header.signature[1] = 'X';
    CHECK(abmp_write_header(buffer, &header) == ABMP_IS_NOT_BMP_FILE);
    memcpy(buffer, &header, sizeof(header));
    CHECK(abmp_read_header(buffer, &header) == ABMP_IS_NOT_BMP_FILE);

    header = bitmap.header;
    header.imagesize++;
    CHECK(abmp_write_header(buffer, &header) == ABMP_BMP_DATA_IS_CORRUPTED);

    bitmap.header.compression = 1;
    CHECK(abmp_read_data(buffer, &bitmap) == ABMP_COMPRESSION_IS_NOT_SUPPORTED);
    bitmap.header.compression = 0;
    bitmap.header.bits_per_pixel = 8;
    CHECK(abmp_read_data(buffer, &bitmap) == ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED);

    abmp_free(&bitmap);
    return 0;
}

int main(void)
{
    CHECK(test_create_and_pixel_positions() == 0);
    CHECK(test_memory_round_trip() == 0);
    CHECK(test_file_round_trips() == 0);
    CHECK(test_error_handling() == 0);

    puts("All libabmp tests passed.");
    return 0;
}
