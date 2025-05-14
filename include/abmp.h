#include "abitmap.h"

#ifndef ABMP_H
#define ABMP_H

#ifdef __cplusplus
extern "C" {
#endif

void abmp_hello(void);
ABMP_BITMAP_HEADER abmp_read_header(uint8_t* data);
ABMP_BITMAP abmp_read_data(uint8_t* data, ABMP_BITMAP_HEADER header);

#ifdef __cplusplus
}
#endif

#endif // ABMP_H
