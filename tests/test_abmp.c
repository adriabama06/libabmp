#include "abmp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ABMP_TEST_OUTPUT_PATH
#define ABMP_TEST_OUTPUT_PATH "libabmp_test_output.bmp"
#endif

/* Compare both BMP metadata and the complete BGR pixel payload after a round trip. */
static int check_bitmap_equal(const ABMP_BITMAP *expected, const ABMP_BITMAP *actual)
{
    assert(memcmp(&expected->header, &actual->header, sizeof(ABMP_BITMAP_HEADER)) == 0);
    assert(memcmp(expected->pixel_data, actual->pixel_data,
                 expected->header.imagesize) == 0);
    return 0;
}

static int test_create_and_pixel_positions(void)
{
    ABMP_BITMAP bitmap = {0};
    /* Width 3 forces three padding bytes per row, exercising row-stride logic. */
    assert(abmp_create_bitmap(&bitmap, 3, 2) == ABMP_OK);

    assert(bitmap.header.signature[0] == 'B');
    assert(bitmap.header.signature[1] == 'M');
    assert(bitmap.header.dataoffset == ABMP_HEADER_SIZE);
    assert(bitmap.header.width == 3);
    assert(bitmap.header.height == 2);
    assert(bitmap.header.bits_per_pixel == 24);
    assert(bitmap.header.planes == 1);
    assert(bitmap.header.imagesize == 24); /* (3 * 3 + 3 padding) * 2 */
    assert(bitmap.header.filesize == 78);

    /* Newly-created images are initialized to white, including padding bytes. */
    for (uint32_t i = 0; i < bitmap.header.imagesize; ++i)
        assert(bitmap.pixel_data[i] == 255);

    /* BMP rows are stored bottom-up; the helper exposes both coordinate systems. */
    assert(abmp_get_pixel_raw_position(&bitmap.header, 0, 0) == 0);
    assert(abmp_get_pixel_raw_position(&bitmap.header, 2, 1) == 18);
    assert(abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) == 12);
    assert(abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 1) == 6);

    abmp_free(&bitmap);
    return 0;
}

static int make_bitmap(ABMP_BITMAP *bitmap)
{
    assert(abmp_create_bitmap(bitmap, 3, 2) == ABMP_OK);
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

    /* Exercise the memory-only writer and reader without using the filesystem. */
    encoded = abmp_allocate_writer(&original.header);
    assert(encoded != NULL);
    assert(abmp_write_header(encoded, &original.header) == ABMP_OK);
    assert(abmp_write_data(encoded, &original) == ABMP_OK);
    assert(abmp_read_header(encoded, &decoded.header) == ABMP_OK);
    assert(abmp_read_data(encoded, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&original, &decoded) == 0);

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

    /* Verify the streaming FILE* API independently of path-based wrappers. */
    file = tmpfile();
    assert(file != NULL);
    assert(abmp_file_write_file_p(file, &original) == ABMP_OK);
    rewind(file);
    assert(abmp_file_read_file_p(file, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&original, &decoded) == 0);
    fclose(file);
    abmp_free(&decoded);

    /* Verify that the convenience path API preserves the same bitmap contents. */
    assert(abmp_write_file(ABMP_TEST_OUTPUT_PATH, &original) == ABMP_OK);
    memset(&decoded, 0, sizeof(decoded));
    assert(abmp_read_file(ABMP_TEST_OUTPUT_PATH, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&original, &decoded) == 0);
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

    /* A missing input path must return the documented error instead of succeeding. */
    assert(abmp_read_file("this-file-does-not-exist.bmp", &bitmap) == ABMP_FILE_NOT_EXIST);

    assert(abmp_create_bitmap(&bitmap, 1, 1) == ABMP_OK);
    /* Reject a signature if either byte differs from the required "BM" marker. */
    header = bitmap.header;
    header.signature[1] = 'X';
    assert(abmp_write_header(buffer, &header) == ABMP_IS_NOT_BMP_FILE);
    memcpy(buffer, &header, sizeof(header));
    assert(abmp_read_header(buffer, &header) == ABMP_IS_NOT_BMP_FILE);

    /* The pixel payload size must agree with dimensions and row padding. */
    header = bitmap.header;
    header.imagesize++;
    assert(abmp_write_header(buffer, &header) == ABMP_BMP_DATA_IS_CORRUPTED);

    /* Unsupported compression and indexed-color formats fail explicitly. */
    bitmap.header.compression = 1;
    assert(abmp_read_data(buffer, &bitmap) == ABMP_COMPRESSION_IS_NOT_SUPPORTED);
    bitmap.header.compression = 0;
    bitmap.header.bits_per_pixel = 8;
    assert(abmp_read_data(buffer, &bitmap) == ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED);

    abmp_free(&bitmap);
    return 0;
}

int main(void)
{
    assert(test_create_and_pixel_positions() == 0);
    assert(test_memory_round_trip() == 0);
    assert(test_file_round_trips() == 0);
    assert(test_error_handling() == 0);

    puts("All libabmp tests passed.");
    return 0;
}
