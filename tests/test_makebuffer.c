#include "test_common.h"

static int test_makebuffer_null_bitmap(void)
{
    uint8_t *buf = NULL;
    assert(abmp_makebuffer(&buf, NULL) == ABMP_INVALID_PARAMETERS);
    return 0;
}

static int test_makebuffer_null_buffer(void)
{
    ABMP_BITMAP bitmap = {0};
    assert(make_bitmap(&bitmap) == 0);
    assert(abmp_makebuffer(NULL, &bitmap) == ABMP_INVALID_PARAMETERS);
    abmp_free(&bitmap);
    return 0;
}

static int test_makebuffer_round_trip(void)
{
    ABMP_BITMAP original = {0};
    ABMP_BITMAP decoded = {0};
    uint8_t *buf = NULL;

    assert(make_bitmap(&original) == 0);

    assert(abmp_makebuffer(&buf, &original) == ABMP_OK);
    assert(buf != NULL);

    assert(abmp_read_header_from_memory(buf, &decoded.header) == ABMP_OK);
    assert(abmp_read_pixeldata_from_memory(buf, &decoded) == ABMP_OK);

    assert(check_bitmap_equal(&original, &decoded) == 0);

    free(buf);
    abmp_free(&decoded);
    abmp_free(&original);
    return 0;
}

static int test_makebuffer_matches_manual_write(void)
{
    ABMP_BITMAP bitmap = {0};
    uint8_t *manual = NULL;
    uint8_t *makebuf = NULL;

    assert(make_bitmap(&bitmap) == 0);

    size_t total = bitmap.header.dataoffset + bitmap.header.imagesize;

    manual = abmp_allocate_filedata(&bitmap.header);
    assert(manual != NULL);
    assert(abmp_write_header_to_memory(manual, &bitmap.header) == ABMP_OK);
    assert(abmp_write_pixeldata_to_memory(manual, &bitmap) == ABMP_OK);

    assert(abmp_makebuffer(&makebuf, &bitmap) == ABMP_OK);
    assert(makebuf != NULL);

    assert(memcmp(manual, makebuf, total) == 0);

    free(manual);
    free(makebuf);
    abmp_free(&bitmap);
    return 0;
}

int main(void)
{
    assert(test_makebuffer_null_bitmap() == 0);
    assert(test_makebuffer_null_buffer() == 0);
    assert(test_makebuffer_round_trip() == 0);
    assert(test_makebuffer_matches_manual_write() == 0);

    puts("test_makebuffer passed.");
    return 0;
}
