#include "test_common.h"

/* The mmap API is POSIX-only. Decide up-front whether this suite can run
 * at all: compile-time capability check, and the suite turns into a
 * graceful skip when mmap is not available. */
#if defined(_WIN32)
#   define ABMP_TEST_MMAP_SKIP
#else
#   include <fcntl.h>
#   include <unistd.h>

#   if !defined(_POSIX_MAPPED_FILES) || _POSIX_MAPPED_FILES <= 0
#       define ABMP_TEST_MMAP_SKIP
#   endif
#endif

#ifdef ABMP_TEST_MMAP_SKIP

int main(void)
{
    puts("test_mmap skipped: POSIX mmap is not available on this platform.");
    return 0;
}

#else

/* Map the sample BMP from an fd, verify the view is byte-for-byte equal
 * to a normal read (same header + same pixel payload), then release it
 * with abmp_mmap_close. */
static int test_mmap_openfile(void)
{
    ABMP_BITMAP view = {0};
    ABMP_BITMAP decoded = {0};
    int fd;

    fd = open(ABMP_TEST_SAMPLE_PATH, O_RDONLY);
    assert(fd >= 0);

    assert(abmp_mmap_openfile_fd(fd, &view) == ABMP_OK);

    /* The mapping is independent of the fd: it can be closed right away. */
    assert(close(fd) == 0);

    assert(abmp_read_filepath_using_memory(ABMP_TEST_SAMPLE_PATH, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&decoded, &view) == 0);

    assert(view.pixel_data != NULL);
    assert(view.header.width == 6);
    assert(view.header.height == 4);

    assert(abmp_mmap_close(&view) == ABMP_OK);
    abmp_free(&decoded);
    return 0;
}

/* The view is writable (MAP_PRIVATE), but edits must never reach the file. */
static int test_mmap_view_is_editable_and_private(void)
{
    ABMP_BITMAP view = {0};
    ABMP_BITMAP recheck = {0};
    int fd;

    fd = open(ABMP_TEST_SAMPLE_PATH, O_RDONLY);
    assert(fd >= 0);

    assert(abmp_mmap_openfile_fd(fd, &view) == ABMP_OK);
    assert(close(fd) == 0);

    /* First raw pixel byte starts at 255 (white background). */
    assert(view.pixel_data[0] == 255);

    /* Edit through the zero-copy pointer. */
    view.pixel_data[0] = 0;
    assert(view.pixel_data[0] == 0);

    assert(abmp_mmap_close(&view) == ABMP_OK);

    /* Read the sample back normally: the file was never touched. */
    assert(abmp_read_filepath_using_memory(ABMP_TEST_SAMPLE_PATH, &recheck) == ABMP_OK);
    assert(recheck.pixel_data[0] == 255);
    abmp_free(&recheck);
    return 0;
}

/* abmp_mmap_close must wipe the bitmap so nobody can keep using (or
 * abmp_free) the dangling pointer afterwards. A second close has no
 * mapping left, so munmap fails with ABMP_ERROR_MUNMAP. */
static int test_mmap_close_clears_bitmap(void)
{
    ABMP_BITMAP view = {0};
    ABMP_BITMAP_HEADER zeroed = {0};
    int fd;

    fd = open(ABMP_TEST_SAMPLE_PATH, O_RDONLY);
    assert(fd >= 0);

    assert(abmp_mmap_openfile_fd(fd, &view) == ABMP_OK);
    assert(close(fd) == 0);

    assert(abmp_mmap_close(&view) == ABMP_OK);
    assert(view.pixel_data == NULL);
    assert(memcmp(&view.header, &zeroed, sizeof(ABMP_BITMAP_HEADER)) == 0);

    assert(abmp_mmap_close(&view) == ABMP_ERROR_MUNMAP);
    return 0;
}

/* abmp_cast_from_memory is the zero-copy core behind the mmap path:
 * pixel_data must point INTO the caller buffer, never into a copy of it.
 * Corrupted headers must be rejected with the same errors a normal
 * reader would return. */
static int test_mmap_cast_from_memory(void)
{
    ABMP_BITMAP view = {0};
    uint8_t *raw;
    FILE *file;
    long size;

    file = fopen(ABMP_TEST_SAMPLE_PATH, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    size = ftell(file);
    assert(size >= ABMP_HEADER_SIZE);
    rewind(file);

    raw = malloc((size_t) size);
    assert(raw != NULL);
    assert(fread(raw, 1, (size_t) size, file) == (size_t) size);
    fclose(file);

    /* Zero-copy: pixel_data lives inside raw, not in a copy of it. */
    assert(abmp_cast_from_memory(raw, &view) == ABMP_OK);
    assert(view.pixel_data == raw + view.header.dataoffset);
    assert(view.header.dataoffset + view.header.imagesize == (uint32_t) size);

    /* Do NOT abmp_free the view: pixel_data is not owned by the bitmap. */
    memset(&view, 0, sizeof(view));

    /* Compressed BMP: rejected by the compatibility check.
     * (Byte offsets 28/30 are the standard little-endian BMP fields
     * bits_per_pixel / compression.) */
    raw[30] = 1;
    assert(abmp_cast_from_memory(raw, &view) == ABMP_COMPRESSION_IS_NOT_SUPPORTED);
    memset(&view, 0, sizeof(view));

    /* bits_per_pixel 8: rejected too. */
    raw[30] = 0;
    raw[28] = 8;
    assert(abmp_cast_from_memory(raw, &view) == ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED);
    memset(&view, 0, sizeof(view));

    /* dataoffset below the header size: rejected in abmp_check_header. */
    raw[28] = 24;
    raw[10] = 20;
    assert(abmp_cast_from_memory(raw, &view) == ABMP_INVALID_DATAOFFSET);
    memset(&view, 0, sizeof(view));

    free(raw);
    return 0;
}

/* Error paths of abmp_mmap_openfile_fd: invalid fd, NULL output,
 * truncated file, and a valid BMP with trailing junk bytes (the mmap
 * wrapper enforces mapping length == dataoffset + imagesize, otherwise
 * abmp_mmap_close would unmap the wrong range). */
static int test_mmap_errors(void)
{
    ABMP_BITMAP view = {0};
    FILE *src;
    FILE *dst;
    uint8_t *raw;
    long size;
    int fd;

    /* Invalid fd: fstat fails. */
    assert(abmp_mmap_openfile_fd(-1, &view) == ABMP_ERROR_OPENING_FILE);

    /* NULL output parameter. */
    fd = open(ABMP_TEST_SAMPLE_PATH, O_RDONLY);
    assert(fd >= 0);
    assert(abmp_mmap_openfile_fd(fd, NULL) == ABMP_INVALID_PARAMETERS);
    assert(close(fd) == 0);

    /* Load the sample bytes once for the destructive file tests. */
    src = fopen(ABMP_TEST_SAMPLE_PATH, "rb");
    assert(src != NULL);
    assert(fseek(src, 0, SEEK_END) == 0);
    size = ftell(src);
    rewind(src);
    raw = malloc((size_t) size);
    assert(raw != NULL);
    assert(fread(raw, 1, (size_t) size, src) == (size_t) size);
    fclose(src);

    /* Truncated copy: only the first 10 bytes. */
    dst = fopen(ABMP_TEST_OUTPUT_PATH, "wb");
    assert(dst != NULL);
    assert(fwrite(raw, 1, 10, dst) == 10);
    fclose(dst);

    fd = open(ABMP_TEST_OUTPUT_PATH, O_RDONLY);
    assert(fd >= 0);
    assert(abmp_mmap_openfile_fd(fd, &view) == ABMP_FILE_SIZE_IS_LOWER_THAN_HEADER_SIZE);
    assert(close(fd) == 0);

    /* Valid BMP plus one trailing byte: the mapping length no longer
     * matches dataoffset + imagesize, so the wrapper must refuse it
     * (the plain memory reader would simply ignore the tail). */
    dst = fopen(ABMP_TEST_OUTPUT_PATH, "wb");
    assert(dst != NULL);
    assert(fwrite(raw, 1, (size_t) size, dst) == (size_t) size);
    fputc('x', dst);
    fclose(dst);

    fd = open(ABMP_TEST_OUTPUT_PATH, O_RDONLY);
    assert(fd >= 0);
    assert(abmp_mmap_openfile_fd(fd, &view) == ABMP_FILE_IMAGESIZE_MISSMATCH);
    assert(close(fd) == 0);

    remove(ABMP_TEST_OUTPUT_PATH);
    free(raw);
    return 0;
}

/* The flags variant is the plumbing under both wrappers: MAP_PRIVATE
 * through it must behave exactly like abmp_mmap_openfile_fd. */
static int test_mmap_flags_variant(void)
{
    ABMP_BITMAP view = {0};
    ABMP_BITMAP decoded = {0};
    int fd;

    fd = open(ABMP_TEST_SAMPLE_PATH, O_RDONLY);
    assert(fd >= 0);

    assert(abmp_mmap_openfile_fd_flags(fd, MAP_PRIVATE, &view) == ABMP_OK);
    assert(close(fd) == 0);

    assert(abmp_read_filepath_using_memory(ABMP_TEST_SAMPLE_PATH, &decoded) == ABMP_OK);
    assert(check_bitmap_equal(&decoded, &view) == 0);

    assert(abmp_mmap_close(&view) == ABMP_OK);
    abmp_free(&decoded);
    return 0;
}

/* MAP_SHARED is the editable flavor: edits made through the view must
 * be flushed to the file when the mapping is closed. The test works on
 * a temporary copy so the fixture sample stays pristine. */
static int test_mmap_editablefile_persists(void)
{
    ABMP_BITMAP view = {0};
    ABMP_BITMAP recheck = {0};
    FILE *src;
    FILE *dst;
    uint8_t *raw;
    long size;
    int fd;

    /* Make a writable copy of the sample to edit. */
    src = fopen(ABMP_TEST_SAMPLE_PATH, "rb");
    assert(src != NULL);
    assert(fseek(src, 0, SEEK_END) == 0);
    size = ftell(src);
    rewind(src);
    raw = malloc((size_t) size);
    assert(raw != NULL);
    assert(fread(raw, 1, (size_t) size, src) == (size_t) size);
    fclose(src);

    dst = fopen(ABMP_TEST_OUTPUT_PATH, "wb");
    assert(dst != NULL);
    assert(fwrite(raw, 1, (size_t) size, dst) == (size_t) size);
    fclose(dst);
    free(raw);

    /* MAP_SHARED needs a fd opened for writing. */
    fd = open(ABMP_TEST_OUTPUT_PATH, O_RDWR);
    assert(fd >= 0);

    assert(abmp_mmap_editablefile_fd(fd, &view) == ABMP_OK);

    /* The mapping survives closing the fd. */
    assert(close(fd) == 0);

    /* Pixel (2, 2) is white before the edit. */
    assert(view.pixel_data[abmp_get_pixel_position_from_top_left(&view.header, 2, 2)] == 255);

    /* Paint pixel (2, 2) with draw: RGB(12, 34, 56) -> BGR(56, 34, 12). */
    abmp_draw(&view, 2, 2, 12, 34, 56);

    assert(view.pixel_data[abmp_get_pixel_position_from_top_left(&view.header, 2, 2)] == 56);
    assert(view.pixel_data[abmp_get_pixel_position_from_top_left(&view.header, 2, 2) + 1] == 34);
    assert(view.pixel_data[abmp_get_pixel_position_from_top_left(&view.header, 2, 2) + 2] == 12);

    /* Closing the view flushes the edit into the file. */
    assert(abmp_mmap_close(&view) == ABMP_OK);

    /* Read the file normally: the edit must be there. */
    assert(abmp_read_filepath_using_memory(ABMP_TEST_OUTPUT_PATH, &recheck) == ABMP_OK);
    assert(recheck.pixel_data[abmp_get_pixel_position_from_top_left(&recheck.header, 2, 2)] == 56);
    assert(recheck.pixel_data[abmp_get_pixel_position_from_top_left(&recheck.header, 2, 2) + 1] == 34);
    assert(recheck.pixel_data[abmp_get_pixel_position_from_top_left(&recheck.header, 2, 2) + 2] == 12);
    abmp_free(&recheck);

    remove(ABMP_TEST_OUTPUT_PATH);
    return 0;
}

int main(void)
{
#if defined(_SC_MAPPED_FILES)
    /* Runtime double-check: the system itself reports mmap support. */
    if (sysconf(_SC_MAPPED_FILES) < 0)
    {
        puts("test_mmap skipped: mmap is not available on this system.");
        return 0;
    }
#endif

    assert(test_mmap_openfile() == 0);
    assert(test_mmap_view_is_editable_and_private() == 0);
    assert(test_mmap_flags_variant() == 0);
    assert(test_mmap_editablefile_persists() == 0);
    assert(test_mmap_close_clears_bitmap() == 0);
    assert(test_mmap_cast_from_memory() == 0);
    assert(test_mmap_errors() == 0);

    puts("test_mmap passed.");
    return 0;
}

#endif /* ABMP_TEST_MMAP_SKIP */
