#include "test_common.h"

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
    assert(abmp_read_filepath_using_memory("this-file-does-not-exist.bmp", &bitmap) == ABMP_FILE_NOT_EXIST);

    assert(abmp_create_bitmap(&bitmap, 1, 1) == ABMP_OK);
    /* Reject a signature if either byte differs from the required "BM" marker. */
    header = bitmap.header;
    header.signature[1] = 'X';
    assert(abmp_write_header_to_memory(buffer, &header) == ABMP_IS_NOT_BMP_FILE);
    memcpy(buffer, &header, sizeof(header));
    assert(abmp_read_header_from_memory(buffer, &header) == ABMP_IS_NOT_BMP_FILE);

    /* The pixel payload size must agree with dimensions and row padding. */
    header = bitmap.header;
    header.imagesize++;
    assert(abmp_write_header_to_memory(buffer, &header) == ABMP_BMP_DATA_IS_CORRUPTED);

    /* Unsupported compression and indexed-color formats fail explicitly. */
    bitmap.header.compression = 1;
    assert(abmp_read_pixeldata_from_memory(buffer, &bitmap) == ABMP_COMPRESSION_IS_NOT_SUPPORTED);
    bitmap.header.compression = 0;
    bitmap.header.bits_per_pixel = 8;
    assert(abmp_read_pixeldata_from_memory(buffer, &bitmap) == ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED);

    abmp_free(&bitmap);
    return 0;
}

int main(void)
{
    assert(test_error_handling() == 0);

    puts("test_errors passed.");
    return 0;
}
