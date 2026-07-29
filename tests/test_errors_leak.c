#include "test_common.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static int test_abmp_file_not_exist(void)
{
    ABMP_BITMAP bitmap = {0};

    assert(abmp_read_filepath_using_memory("this-file-does-not-exist.bmp", &bitmap) == ABMP_FILE_NOT_EXIST);
    assert(bitmap.pixel_data == NULL);

    return 0;
}

static int test_abmp_is_not_bmp_file(void)
{
    ABMP_BITMAP bitmap = {0};
    int fd;
    char path[] = "/tmp/abmp_not_bmp_XXXXXX";
    char buf[ABMP_HEADER_SIZE];

    fd = mkstemp(path);
    assert(fd >= 0);
    memset(buf, 'X', ABMP_HEADER_SIZE);
    buf[0] = 'N';
    buf[1] = 'O';
    assert(write(fd, buf, ABMP_HEADER_SIZE) == ABMP_HEADER_SIZE);
    close(fd);

    assert(abmp_read_filepath_using_memory(path, &bitmap) == ABMP_IS_NOT_BMP_FILE);

    remove(path);
    return 0;
}

/* Write a valid BMP header but with compression set to 1, then
 * attempt to read pixel data; abmp_read_pixeldata_from_memory
 * must return ABMP_COMPRESSION_IS_NOT_SUPPORTED.  After the
 * failure the bitmap struct may have been partially filled but
 * caller is responsible for freeing pixel_data if it was
 * allocated by a successful header parse. */
static int test_abmp_compression_not_supported(void)
{
    ABMP_BITMAP bitmap = {0};
    int fd;
    char path[] = "/tmp/abmp_compressed_XXXXXX";
    uint8_t header[ABMP_HEADER_SIZE];

    fd = mkstemp(path);
    assert(fd >= 0);

    abmp_create_bitmap(&bitmap, 4, 4);
    memcpy(header, &bitmap.header, ABMP_HEADER_SIZE);
    header[30] = 1; /* Set compression field to 1 (BI_RLE8). */
    write(fd, header, ABMP_HEADER_SIZE);
    write(fd, bitmap.pixel_data, bitmap.header.imagesize);
    close(fd);

    assert(abmp_read_filepath_using_memory(path, &bitmap) == ABMP_COMPRESSION_IS_NOT_SUPPORTED);

    remove(path);
    abmp_free(&bitmap);
    return 0;
}

/* Write a valid BMP header but with bits_per_pixel set to 8,
 * then attempt to read pixel data; abmp_read_pixeldata_from_memory
 * must return ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED. */
static int test_abmp_low_bits_per_pixel_not_supported(void)
{
    ABMP_BITMAP bitmap = {0};
    int fd;
    char path[] = "/tmp/abmp_8bpp_XXXXXX";
    uint8_t header[ABMP_HEADER_SIZE];

    fd = mkstemp(path);
    assert(fd >= 0);

    abmp_create_bitmap(&bitmap, 4, 4);
    memcpy(header, &bitmap.header, ABMP_HEADER_SIZE);
    header[28] = 8; /* bits_per_pixel = 8 */
    header[29] = 0;
    write(fd, header, ABMP_HEADER_SIZE);
    write(fd, bitmap.pixel_data, bitmap.header.imagesize);
    close(fd);

    assert(abmp_read_filepath_using_memory(path, &bitmap) == ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED);

    remove(path);
    abmp_free(&bitmap);
    return 0;
}

/* Write a valid BMP header but with imagesize set to a value that
 * does not match the actual pixel payload, triggering
 * ABMP_FILE_IMAGESIZE_MISSMATCH or a similar corruption error. */
static int test_abmp_corrupted_data_size(void)
{
    ABMP_BITMAP bitmap = {0};
    int fd;
    char path[] = "/tmp/abmp_corrupted_XXXXXX";
    uint8_t header[ABMP_HEADER_SIZE];

    fd = mkstemp(path);
    assert(fd >= 0);

    abmp_create_bitmap(&bitmap, 4, 4);
    memcpy(header, &bitmap.header, ABMP_HEADER_SIZE);
    header[34] = 0xFF; /* imagesize high byte = wrong value. */
    header[35] = 0xFF;
    header[36] = 0xFF;
    header[37] = 0xFF;
    write(fd, header, ABMP_HEADER_SIZE);
    write(fd, bitmap.pixel_data, bitmap.header.imagesize);
    close(fd);

    ABMP_ERRORS status = abmp_read_filepath_using_memory(path, &bitmap);
    assert(status != ABMP_OK);

    remove(path);
    abmp_free(&bitmap);
    return 0;
}

/* abmp_write_filepath_using_direct should fail with ABMP_ERROR_OPENING_FILE
 * when the target directory does not exist; after that failure any bitmap
 * allocated internally by the caller is still owned by the caller and must
 * be freed with abmp_free (no leak). */
static int test_abmp_write_to_missing_dir_no_leak(void)
{
    ABMP_BITMAP bitmap = {0};

    assert(abmp_create_bitmap(&bitmap, 4, 4) == ABMP_OK);

    /* abmp_write_filepath_using_direct opens the file for writing; it
     * must not touch bitmap->pixel_data if the open fails. */
    ABMP_ERRORS status = abmp_write_filepath_using_direct("/nonexistent_dir/abmp_write_leak_test.bmp", &bitmap);
    assert(status == ABMP_ERROR_OPENING_FILE || status == ABMP_ERROR_WRITING_FILE);

    /* Calling abmp_free must reclaim pixel_data without leaking or crashing. */
    abmp_free(&bitmap);
    return 0;
}

/* Same as above for the memory-write path: a write to a read-only
 * filesystem or non-existent directory must not leave bitmap->pixel_data
 * allocated and unreachable from the caller's perspective (the caller
 * is still responsible for freeing it). */
static int test_abmp_write_memory_to_missing_dir_no_leak(void)
{
    ABMP_BITMAP bitmap = {0};

    assert(abmp_create_bitmap(&bitmap, 4, 4) == ABMP_OK);

    ABMP_ERRORS status = abmp_write_filepath_using_memory("/nonexistent_dir/abmp_write_mem_leak_test.bmp", &bitmap);
    assert(status == ABMP_ERROR_OPENING_FILE || status == ABMP_ERROR_WRITING_FILE);

    abmp_free(&bitmap);
    return 0;
}

/* Create multiple bitmaps with varying sizes in a loop, free each one,
 * and verify valgrind reports zero leaks across thousands of create/free
 * cycles.  This also exercises the internal allocator for edge-case sizes
 * (sizes that produce non-trivial padding, odd widths, etc.). */
static int test_create_free_stress_cycles(void)
{
    for (uint32_t w = 1; w <= 64; ++w)
    {
        for (uint32_t h = 1; h <= 32; ++h)
        {
            ABMP_BITMAP bitmap = {0};
            assert(abmp_create_bitmap(&bitmap, w, h) == ABMP_OK);
            abmp_free(&bitmap);
        }
    }
    return 0;
}

int main(void)
{
    assert(test_abmp_file_not_exist() == 0);
    assert(test_abmp_is_not_bmp_file() == 0);
    assert(test_abmp_compression_not_supported() == 0);
    assert(test_abmp_low_bits_per_pixel_not_supported() == 0);
    assert(test_abmp_corrupted_data_size() == 0);
    assert(test_abmp_write_to_missing_dir_no_leak() == 0);
    assert(test_abmp_write_memory_to_missing_dir_no_leak() == 0);
    assert(test_create_free_stress_cycles() == 0);

    puts("test_errors_leak passed.");
    return 0;
}
