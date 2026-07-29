#include "test_common.h"

/* Write a bitmap to disk using the streaming FILE* variant,
 * read it back through the same streaming API, and verify the
 * contents match the original.  Then repeat the round trip using
 * the convenience path-based wrappers (abmp_write_file /
 * abmp_read_file) to ensure both APIs produce identical results. */
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
    assert(abmp_write_file_p_using_direct(file, &original) == ABMP_OK);
    rewind(file);
    assert(abmp_read_file_p_using_direct(file, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&original, &decoded) == 0);
    fclose(file);
    abmp_free(&decoded);

    /* Verify that the convenience path API preserves the same bitmap contents. */
    assert(abmp_write_filepath_using_memory(ABMP_TEST_OUTPUT_PATH, &original) == ABMP_OK);
    memset(&decoded, 0, sizeof(decoded));
    assert(abmp_read_filepath_using_memory(ABMP_TEST_OUTPUT_PATH, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&original, &decoded) == 0);
    remove(ABMP_TEST_OUTPUT_PATH);

    abmp_free(&decoded);
    abmp_free(&original);
    return 0;
}

/* Read a BMP file by path using the convenience wrapper abmp_read_file.
 * Verify that the header and pixel data match those obtained with the
 * manual memory-buffer approach, and check pixel access helpers. */
static int test_read_bitmap_from_file(void)
{
    ABMP_BITMAP bitmap = {0};

    /* abmp_read_file opens the file, reads header + data, and fills the
     * ABMP_BITMAP structure in one call. */
    assert(abmp_read_filepath_using_memory(ABMP_TEST_SAMPLE_PATH, &bitmap) == ABMP_OK);
    assert(bitmap.header.signature[0] == 'B');
    assert(bitmap.header.signature[1] == 'M');
    assert(bitmap.header.width == 6);
    assert(bitmap.header.height == 4);
    assert(bitmap.header.bits_per_pixel == 24);
    assert(bitmap.header.imagesize == 80);

    /* Bottom-left pixel (raw y=0, x=0) must be white. */
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0)] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 2] == 255);

    /* Top-left pixel (0,0) must be black (BGR: 0,0,0). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 1] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 2] == 0);

    /* Top-left pixel (2,0) must be red (BGR: 36,28,237). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0)] == 36);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 1] == 28);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 2] == 237);

    abmp_free(&bitmap);
    return 0;
}

/* Read a BMP file through a FILE* stream using abmp_read_file_p (the
 * streaming variant that reads data from an already-opened FILE*).
 * Verify header fields and key pixel values. */
static int test_read_bitmap_from_file_stream(void)
{
    FILE *file;
    ABMP_BITMAP bitmap = {0};

    file = fopen(ABMP_TEST_SAMPLE_PATH, "rb");
    assert(file != NULL);

    /* abmp_read_file_p reads header and pixel data from the open FILE*. */
    assert(abmp_read_file_p_using_memory(file, &bitmap) == ABMP_OK);
    fclose(file);

    assert(bitmap.header.signature[0] == 'B');
    assert(bitmap.header.signature[1] == 'M');
    assert(bitmap.header.width == 6);
    assert(bitmap.header.height == 4);
    assert(bitmap.header.bits_per_pixel == 24);
    assert(bitmap.header.imagesize == 80);

    /* Bottom-left pixel is white. */
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0)] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 2] == 255);

    /* Top-left pixel is black. */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 1] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 2] == 0);

    abmp_free(&bitmap);
    return 0;
}

/* Read twoofpadding.bmp using the higher-level abmp_file_read_file
 * (which opens the file, reads header + data together), modify pixel
 * (2,2) to brown (BGR 42,42,165), write it back with
 * abmp_file_write_file (path-based convenience), read it back and
 * verify the brown pixel persisted through the round trip. */
static int test_file_api_read_write_brown_pixel(void)
{
    ABMP_BITMAP bitmap = {0};
    ABMP_ERRORS status;

    /* abmp_file_read_file opens the file by path and performs both
     * header and data reading in a single convenience call. */
    status = abmp_read_filepath_using_direct(ABMP_TEST_SAMPLE_PATH, &bitmap);
    assert(status == ABMP_OK);

    /* Pixel (2,2) is currently white: BGR(255,255,255). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    /* Paint pixel (2,2) brown: BGR = (42, 42, 165). */
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] = 42;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] = 42;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] = 165;

    /* Verify the in-memory change took effect. */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 42);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 42);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 165);

    /* Write using abmp_file_write_file (path-based convenience). */
    assert(abmp_write_filepath_using_direct(ABMP_TEST_OUTPUT_PATH, &bitmap) == ABMP_OK);
    abmp_free(&bitmap);

    /* Re-read and verify the brown pixel survived the round trip. */
    status = abmp_read_filepath_using_memory(ABMP_TEST_OUTPUT_PATH, &bitmap);
    assert(status == ABMP_OK);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 42);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 42);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 165);

    remove(ABMP_TEST_OUTPUT_PATH);
    abmp_free(&bitmap);
    return 0;
}

int main(void)
{
    assert(test_file_round_trips() == 0);
    assert(test_read_bitmap_from_file() == 0);
    assert(test_read_bitmap_from_file_stream() == 0);
    assert(test_file_api_read_write_brown_pixel() == 0);

    puts("test_file passed.");
    return 0;
}
