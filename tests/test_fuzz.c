#include "test_common.h"

#include <time.h>

/* Fill the pixel buffer of *bitmap with pseudo-random BGR values,
 * treating the entire imagesize (including row padding) as the
 * byte range to randomize.  The padding bytes are included so the
 * write->read round-trip comparison is exact. */
static void fuzz_randomize_pixels(ABMP_BITMAP *bitmap)
{
    for (uint32_t i = 0; i < bitmap->header.imagesize; ++i)
        bitmap->pixel_data[i] = (uint8_t)(rand() & 0xFF);
}

/* Draw a random pixel at (x, y) with random R/G/B components. */
static void fuzz_draw_random_pixel(ABMP_BITMAP *bitmap)
{
    uint32_t x = (uint32_t)(rand() % bitmap->header.width);
    uint32_t y = (uint32_t)(rand() % bitmap->header.height);
    uint8_t r = (uint8_t)(rand() & 0xFF);
    uint8_t g = (uint8_t)(rand() & 0xFF);
    uint8_t b = (uint8_t)(rand() & 0xFF);
    abmp_draw(bitmap, x, y, r, g, b);
}

/* Write *bitmap to *path, read it back as *decoded, and assert
 * header equality and pixel-perfect match. */
static int fuzz_round_trip(const char *path, ABMP_BITMAP *bitmap, ABMP_BITMAP *decoded)
{
    assert(abmp_write_filepath_using_memory(path, bitmap) == ABMP_OK);
    memset(decoded, 0, sizeof(*decoded));
    assert(abmp_read_filepath_using_memory(path, decoded) == ABMP_OK);
    assert(check_bitmap_equal(bitmap, decoded) == 0);
    remove(path);
    return 0;
}

/* Create random-width/random-height bitmaps, fill with random pixels,
 * optionally draw a few random pixels (so that abmp_draw is exercised),
 * write each to a unique temp file under /tmp, read it back, verify
 * the round-trip is pixel-perfect, then free and remove the file. */
static int test_bruteforce_random_bitmaps(void)
{
    srand((unsigned)time(NULL));

    for (int iter = 0; iter < 64; ++iter)
    {
        uint32_t w = 1 + (uint32_t)(rand() % 64);
        uint32_t h = 1 + (uint32_t)(rand() % 64);
        ABMP_BITMAP bitmap = {0};
        ABMP_BITMAP decoded = {0};
        char path[64];

        snprintf(path, sizeof(path), "/tmp/abmp_fuzz_%d.bmp", iter);

        assert(abmp_create_bitmap(&bitmap, w, h) == ABMP_OK);
        fuzz_randomize_pixels(&bitmap);

        /* Draw a handful of random pixels to exercise abmp_draw
         * with varying coordinates and colors. */
        for (int d = 0; d < 8; ++d)
            fuzz_draw_random_pixel(&bitmap);

        assert(fuzz_round_trip(path, &bitmap, &decoded) == 0);
        abmp_free(&bitmap);
        abmp_free(&decoded);
    }
    return 0;
}

/* Same brute-force loop but exercise the FILE*-based streaming
 * APIs (abmp_write_file_p_using_memory + abmp_read_file_p_using_memory)
 * instead of the path-based convenience wrappers. */
static int test_bruteforce_streaming_bitmaps(void)
{
    srand((unsigned)time(NULL));

    for (int iter = 0; iter < 64; ++iter)
    {
        uint32_t w = 1 + (uint32_t)(rand() % 64);
        uint32_t h = 1 + (uint32_t)(rand() % 64);
        ABMP_BITMAP bitmap = {0};
        ABMP_BITMAP decoded = {0};
        char path[64];
        FILE *file;

        snprintf(path, sizeof(path), "/tmp/abmp_fuzz_stream_%d.bmp", iter);

        assert(abmp_create_bitmap(&bitmap, w, h) == ABMP_OK);
        fuzz_randomize_pixels(&bitmap);

        for (int d = 0; d < 8; ++d)
            fuzz_draw_random_pixel(&bitmap);

        file = fopen(path, "wb");
        assert(file != NULL);
        assert(abmp_write_file_p_using_memory(file, &bitmap) == ABMP_OK);
        fclose(file);

        file = fopen(path, "rb");
        assert(file != NULL);
        assert(abmp_read_file_p_using_memory(file, &decoded) == ABMP_OK);
        fclose(file);

        assert(check_bitmap_equal(&bitmap, &decoded) == 0);
        remove(path);
        abmp_free(&bitmap);
        abmp_free(&decoded);
    }
    return 0;
}

/* Stress-test the allocator and free paths by creating and
 * destroying bitmaps of many different sizes in rapid succession,
 * checking that valgrind reports zero allocations leaked. */
static int test_create_free_alloc_stress(void)
{
    for (int iter = 0; iter < 256; ++iter)
    {
        uint32_t w = 1 + (uint32_t)(rand() % 128);
        uint32_t h = 1 + (uint32_t)(rand() % 128);
        ABMP_BITMAP bitmap = {0};
        assert(abmp_create_bitmap(&bitmap, w, h) == ABMP_OK);
        abmp_free(&bitmap);
    }
    return 0;
}

int main(void)
{
    assert(test_bruteforce_random_bitmaps() == 0);
    assert(test_bruteforce_streaming_bitmaps() == 0);
    assert(test_create_free_alloc_stress() == 0);

    puts("test_fuzz passed.");
    return 0;
}