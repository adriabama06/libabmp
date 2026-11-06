#include "abmp.h"

ABMP_ERRORS abmp_cast_from_memory(uint8_t* data, ABMP_BITMAP* bitmap)
{
    if(data == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    ABMP_ERRORS status = abmp_read_header_from_memory(data, &bitmap->header);

    if(status != ABMP_OK) return status;

    if(abmp_header_compatibility_check(&bitmap->header) != ABMP_OK) return abmp_header_compatibility_check(&bitmap->header);

    bitmap->pixel_data = data + bitmap->header.dataoffset;

    return ABMP_OK;
}
