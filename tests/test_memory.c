#include "test_common.h"

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
    encoded = abmp_allocate_filedata(&original.header);
    assert(encoded != NULL);
    assert(abmp_write_header_to_memory(encoded, &original.header) == ABMP_OK);
    assert(abmp_write_pixeldata_to_memory(encoded, &original) == ABMP_OK);
    assert(abmp_read_header_from_memory(encoded, &decoded.header) == ABMP_OK);
    assert(abmp_read_pixeldata_from_memory(encoded, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&original, &decoded) == 0);

    free(encoded);
    abmp_free(&decoded);
    abmp_free(&original);
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
    assert(abmp_read_header_from_memory(buffer, &bitmap.header) == ABMP_OK);
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
    assert(abmp_read_pixeldata_from_memory(buffer, &bitmap) == ABMP_OK);
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

/* Read twoofpadding.bmp via abmp_read_file, change pixel (2,2) from
 * white to yellow (BGR 0,255,255), write the modified bitmap to a
 * temporary output file, then read it back and assert the pixel was
 * changed correctly. */
static int test_write_modified_pixel_yellow(void)
{
    ABMP_BITMAP bitmap = {0};
    ABMP_ERRORS status;

    status = abmp_read_filepath_using_memory(ABMP_TEST_SAMPLE_PATH, &bitmap);
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
    assert(abmp_write_filepath_using_memory(ABMP_TEST_OUTPUT_PATH, &bitmap) == ABMP_OK);
    abmp_free(&bitmap);

    /* Re-read the written file and verify the yellow pixel survived. */
    status = abmp_read_filepath_using_memory(ABMP_TEST_OUTPUT_PATH, &bitmap);
    assert(status == ABMP_OK);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2)] == 0);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 1] == 255);
    assert(bitmap.pixel_data[abmp_get_pixel_position_from_top_left(&bitmap.header, 2, 2) + 2] == 255);

    remove(ABMP_TEST_OUTPUT_PATH);
    abmp_free(&bitmap);
    return 0;
}

int main(void)
{
    assert(test_memory_round_trip() == 0);
    assert(test_read_header_and_data_from_memory_buffer() == 0);
    assert(test_write_modified_pixel_yellow() == 0);

    puts("test_memory passed.");
    return 0;
}
