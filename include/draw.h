#ifndef ABMP_DRAW_H
#define ABMP_DRAW_H

#include "abitmap.h"

#include "get.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: Add tests for abmp_draw

static inline void abmp_draw(ABMP_BITMAP* bitmap, uint32_t x, uint32_t y, uint8_t R, uint8_t G, uint8_t B)
{
    // Use module to make sure user don't draw out of pixeldata
    uint32_t pos = abmp_get_pixel_position_from_top_left(&bitmap->header, x % bitmap->header.width, y % bitmap->header.height) % bitmap->header.imagesize;

    // Bitmaps are BGR, so start from end
    bitmap->pixel_data[pos + 2] = R;
    bitmap->pixel_data[pos + 1] = G;
    bitmap->pixel_data[pos] = B;
}

#ifdef __cplusplus
}
#endif

#endif // ABMP_DRAW_H
