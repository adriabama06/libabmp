#include "test_common.h"

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
    assert(abmp_write_filepath_using_direct(ABMP_TEST_OUTPUT_PATH, &bitmap) == ABMP_OK);
    abmp_free(&bitmap);

    status = abmp_read_filepath_using_memory(ABMP_TEST_OUTPUT_PATH, &bitmap);
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
    assert(test_padding_formulas() == 0);
    assert(test_create_5x5_bitmap_and_modify_red() == 0);

    puts("test_create passed.");
    return 0;
}
