#ifndef ABMP_TEST_COMMON_H
#define ABMP_TEST_COMMON_H

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
static inline int check_bitmap_equal(const ABMP_BITMAP *expected, const ABMP_BITMAP *actual)
{
    assert(memcmp(&expected->header, &actual->header, sizeof(ABMP_BITMAP_HEADER)) == 0);
    assert(memcmp(expected->pixel_data, actual->pixel_data,
                 expected->header.imagesize) == 0);
    return 0;
}

static inline int make_bitmap(ABMP_BITMAP *bitmap)
{
    assert(abmp_create_bitmap(bitmap, 3, 2) == ABMP_OK);
    for (uint32_t i = 0; i < bitmap->header.imagesize; ++i)
        bitmap->pixel_data[i] = (uint8_t)(i * 11U + 3U);
    return 0;
}

#endif /* ABMP_TEST_COMMON_H */
