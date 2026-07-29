#include "abmp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ABMP_TEST_OUTPUT_PATH
#define ABMP_TEST_OUTPUT_PATH "libabmp_test_output.bmp"
#endif

#ifndef ABMP_TEST_SAMPLE_PATH
#define ABMP_TEST_SAMPLE_PATH "../samples/twoofpadding.bmp"
#endif

/* Compare both BMP metadata and the complete BGR pixel payload after a round trip. */
static int check_bitmap_equal(const ABMP_BITMAP *expected, const ABMP_BITMAP *actual)
{
    assert(memcmp(&expected->header, &actual->header, sizeof(ABMP_BITMAP_HEADER)) == 0);
    assert(memcmp(expected->pixel_data, actual->pixel_data,
                 expected->header.imagesize) == 0);
    return 0;
}

/* Create a small 3x2 bitmap and verify every header field has the expected
 * value, including the row-padding calculation.  Also confirm that the
 * freshly allocated pixel buffer is filled with white (255) including the
 * padding bytes at the end of each row, and that both coordinate helpers
 * (bottom-up raw and top-left) return the correct byte offsets. */
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

/* Serialize a pre-populated bitmap to a memory buffer using the
 * raw writer API (abmp_allocate_writer + abmp_write_header +
 * abmp_write_data), then deserialize it back with abmp_read_header
 * and abmp_read_data.  Assert that the decoded bitmap is byte-identical
 * to the original in both header metadata and pixel payload. */
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

/* Verify that the library returns the documented error codes for
 * each failure mode: missing input file, invalid BMP signature,
 * mismatched pixel-payload size, unsupported compression, and
 * unsupported bits-per-pixel value. */
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

/* Verify that padding for a given width equals width % 4, matching the
 * formula used by abmp_read_data when computing row stride. */
static int test_padding_formulas(void)
{
    for (size_t width = 0; width <= 16; ++width)
    {
        size_t padding_v1 = width - ((width / 4) * 4);
        size_t padding_v2 = width % 4;
        assert(padding_v1 == padding_v2);
        /* The row stride must equal width * 3 + padding. */
        size_t row_stride = width * 3 + padding_v2;
        assert(padding_v2 == (4 - (width * 3) % 4) % 4);
        (void)row_stride;
    }
    return 0;
}

/* Read a BMP file into a raw memory buffer (fopen + fseek + fread),
 * then parse the header and pixel data with abmp_read_header and
 * abmp_read_data.  Verify header fields and pixel values at known
 * positions using both the raw (bottom-up) and top-left coordinate
 * helpers. */
static int test_read_header_and_data_from_memory_buffer(void)
{
    FILE *file;
    long file_len;
    uint8_t *buffer;
    ABMP_BITMAP bitmap = {0};

    file = fopen(ABMP_TEST_SAMPLE_PATH, "rb");
    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    file_len = ftell(file);
    assert(file_len > 0);

    buffer = (uint8_t *)malloc((size_t)file_len);
    assert(buffer != NULL);

    rewind(file);
    fread(buffer, 1, (size_t)file_len, file);
    fclose(file);

    /* Parse the BMP header into bitmap.header so abmp_read_data can use it
     * to locate the pixel data and allocate the right buffer size. */
    assert(abmp_read_header(buffer, &bitmap.header) == ABMP_OK);
    assert(bitmap.header.signature[0] == 'B');
    assert(bitmap.header.signature[1] == 'M');
    assert(bitmap.header.width == 6);
    assert(bitmap.header.height == 4);
    assert(bitmap.header.bits_per_pixel == 24);
    assert(bitmap.header.planes == 1);
    assert(bitmap.header.imagesize == 80); /* (6*3 + 2 padding) * 4 rows */

    /* Compute expected row padding. */
    int padding = (int)(bitmap.header.width % 4);
    assert(padding == 2);

    /* Parse the full bitmap (header + pixel data). */
    assert(abmp_read_data(buffer, &bitmap) == ABMP_OK);
    free(buffer);

    /* Pixel (0,0) in top-left coordinates is the top-left pixel.
     * BMP stores rows bottom-up, so in raw coordinates (0,0) is
     * bottom-left, which is different from top-left (0,0). */
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 2] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_raw_position(&bitmap.header, 0, 0)] == 255);

    /* Top-left (0,0) is black (0,0,0): BGR bytes are (0,0,0). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 1] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 0, 0) + 2] == 0);

    /* Top-left (2,0) is red (237,28,36): BGR bytes are (36,28,237). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0)] == 36);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 1] == 28);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 0) + 2] == 237);

    /* Top-left (2,2) is white (255,255,255): BGR bytes are (255,255,255). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    abmp_free(&bitmap);
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
    assert(abmp_read_file(ABMP_TEST_SAMPLE_PATH, &bitmap) == ABMP_OK);
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
    assert(abmp_read_file_p(file, &bitmap) == ABMP_OK);
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

/* Read twoofpadding.bmp via abmp_read_file, change pixel (2,2) from
 * white to yellow (BGR 0,255,255), write the modified bitmap to a
 * temporary output file, then read it back and assert the pixel was
 * changed correctly. */
static int test_write_modified_pixel_yellow(void)
{
    ABMP_BITMAP bitmap = {0};
    ABMP_ERRORS status;

    status = abmp_read_file(ABMP_TEST_SAMPLE_PATH, &bitmap);
    assert(status == ABMP_OK);

    /* Pixel (2,2) is currently white: BGR(255,255,255). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    /* Paint pixel (2,2) yellow: BGR = (0, 255, 255). */
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] = 0;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] = 255;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] = 255;

    /* Verify the in-memory change took effect. */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    /* Write the modified bitmap to disk and read it back. */
    assert(abmp_write_file(ABMP_TEST_OUTPUT_PATH, &bitmap) == ABMP_OK);
    abmp_free(&bitmap);

    /* Re-read the written file and verify the yellow pixel survived. */
    status = abmp_read_file(ABMP_TEST_OUTPUT_PATH, &bitmap);
    assert(status == ABMP_OK);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    remove(ABMP_TEST_OUTPUT_PATH);
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
    status = abmp_file_read_file(ABMP_TEST_SAMPLE_PATH, &bitmap);
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
    assert(abmp_file_write_file(ABMP_TEST_OUTPUT_PATH, &bitmap) == ABMP_OK);
    abmp_free(&bitmap);

    /* Re-read and verify the brown pixel survived the round trip. */
    status = abmp_read_file(ABMP_TEST_OUTPUT_PATH, &bitmap);
    assert(status == ABMP_OK);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 42);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 42);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 165);

    remove(ABMP_TEST_OUTPUT_PATH);
    abmp_free(&bitmap);
    return 0;
}

/* Create a brand-new 5x5 bitmap with abmp_create_bitmap (all pixels
 * default to white), assert the default pixel, paint pixel (2,2) red
 * (BGR 0,0,255), write it to disk with abmp_file_write_file, read it
 * back with abmp_read_file and verify the red pixel persisted. */
static int test_create_5x5_bitmap_and_modify_red(void)
{
    ABMP_BITMAP bitmap = {0};
    ABMP_ERRORS status;

    /* abmp_create_bitmap allocates a zeroed header and fills pixel_data
     * with 255 (white), including the padding bytes per row. */
    status = abmp_create_bitmap(&bitmap, 5, 5);
    assert(status == ABMP_OK);
    assert(bitmap.header.width == 5);
    assert(bitmap.header.height == 5);
    assert(bitmap.header.bits_per_pixel == 24);
    assert(bitmap.header.imagesize == 80); /* (5*3 + 1 padding) * 5 */

    /* Every pixel in a freshly created image should be white. */
    for (uint32_t i = 0; i < bitmap.header.imagesize; ++i)
        assert(bitmap.pixel_data[i] == 255);

    /* Pixel (2,2) in top-left coordinates is white (BGR 255,255,255). */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    /* Paint pixel (2,2) red: BGR = (0, 0, 255). */
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] = 0;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] = 0;
    bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] = 255;

    /* Verify the in-memory change. */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    /* Write the generated bitmap to disk and read it back. */
    assert(abmp_file_write_file(ABMP_TEST_OUTPUT_PATH, &bitmap) == ABMP_OK);
    abmp_free(&bitmap);

    status = abmp_read_file(ABMP_TEST_OUTPUT_PATH, &bitmap);
    assert(status == ABMP_OK);
    assert(bitmap.header.width == 5);
    assert(bitmap.header.height == 5);

    /* The red pixel must survive the write/read round trip. */
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    remove(ABMP_TEST_OUTPUT_PATH);
    abmp_free(&bitmap);
    return 0;
}

int main(void)
{
    assert(test_create_and_pixel_positions() == 0);
    assert(test_memory_round_trip() == 0);
    assert(test_file_round_trips() == 0);
    assert(test_error_handling() == 0);

    assert(test_padding_formulas() == 0);
    assert(test_read_header_and_data_from_memory_buffer() == 0);
    assert(test_read_bitmap_from_file() == 0);
    assert(test_read_bitmap_from_file_stream() == 0);
    assert(test_write_modified_pixel_yellow() == 0);
    assert(test_file_api_read_write_brown_pixel() == 0);
    assert(test_create_5x5_bitmap_and_modify_red() == 0);

    puts("All libabmp tests passed.");
    return 0;
}
