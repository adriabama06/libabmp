#include "abmp.h"

// Reserve the total memory required by the output
uint8_t* abmp_allocate_filedata(const ABMP_BITMAP_HEADER* header)
{
    if(header == NULL) return NULL;
    return (uint8_t*) calloc(header->dataoffset + header->imagesize, sizeof(uint8_t));
}

/**
 * @param data Data len must be >= ABMP_HEADER_SIZE (54)
 * @return It returns ABMP_OK (0) if ok, any other number means other errors
 */
ABMP_ERRORS abmp_write_header_to_memory(uint8_t* data, const ABMP_BITMAP_HEADER* header)
{
    if(data == NULL || header == NULL) return ABMP_INVALID_PARAMETERS;

    ABMP_ERRORS status;

    if((status = abmp_check_header(header)) != ABMP_OK) return status;

    if(sizeof(ABMP_BITMAP_HEADER) == ABMP_HEADER_SIZE) 
    {
        memcpy(data, header, sizeof(ABMP_BITMAP_HEADER));
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;

        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->signature,        __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->filesize,         __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->reserved,         __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->dataoffset,       __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->size,             __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->width,            __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->height,           __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->planes,           __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->bits_per_pixel,   __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->compression,      __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->imagesize,        __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->y_pixels_per_m,   __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->x_pixels_per_m,   __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->colors_used,      __BMP_MEMORY_SIZES[count++]);
        memcpy(data + _BMP_MEMORY_OFFSETS(count), &header->important_colors, __BMP_MEMORY_SIZES[count++]);
    }

    return ABMP_OK;
}

/**
 * @param data Data must point at the start of the file (not any offset or end of header)
 */
ABMP_ERRORS abmp_write_pixeldata_to_memory(uint8_t* data, const ABMP_BITMAP* bitmap)
{
    if(data == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    memcpy(data + bitmap->header.dataoffset, bitmap->pixel_data, bitmap->header.imagesize);

    return ABMP_OK;
}

ABMP_ERRORS abmp_write_file_p_using_memory(FILE* file, const ABMP_BITMAP* bitmap)
{
    if(file == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    ABMP_ERRORS status;

    uint8_t* file_data = abmp_allocate_filedata(&bitmap->header);

    if(file_data == NULL) return ABMP_OUT_OF_MEMORY;

    status = abmp_write_header_to_memory(file_data, &bitmap->header);

    if(status != ABMP_OK)
    {
        free(file_data);
        return status;
    }

    // No need to manually add the space betwen header and dataoffset because we call calloc and it already reserves the dataoffset that is the headersize + extra (if need), and also reserves the size for imagesize

    status = abmp_write_pixeldata_to_memory(file_data, bitmap);

    if(status != ABMP_OK)
    {
        free(file_data);
        return status;
    }

    size_t f_status = fwrite(file_data, sizeof(uint8_t), bitmap->header.dataoffset + bitmap->header.imagesize, file);

    free(file_data);

    if(f_status != bitmap->header.dataoffset + bitmap->header.imagesize)
    {
        return ABMP_ERROR_WRITING_FILE;
    }

    return status;
}

ABMP_ERRORS abmp_write_filepath_using_memory(const char* path, const ABMP_BITMAP* bitmap)
{
    if(path == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    // Open file
    FILE* file = fopen(path, "wb");

    if(file == NULL) return ABMP_ERROR_OPENING_FILE;

    ABMP_ERRORS status = abmp_write_file_p_using_memory(file, bitmap);

    fclose(file);

    return status;
}
