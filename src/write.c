#include "abmp.h"

uint8_t* abmp_allocate_writer(ABMP_BITMAP_HEADER* header)
{
    return (uint8_t*) malloc(header->dataoffset + header->imagesize);
}

/**
 * It returns 0 if ok, any other code means other errors
 */
ABMP_ERRORS abmp_write_header(uint8_t* data, ABMP_BITMAP_HEADER* header)
{
    if(header->signature[0] != 'B' && header->signature[1] != 'M')
    {
        // This is not a BMP file or it's corrupted. Let the user wipe by it self the header data.
        return ABMP_IS_NOT_BMP_FILE;
    }

    if(header->width * header->height * 3 + (header->width % 4) * header->height != header->imagesize && header->imagesize != 0) // It is valid to set imagesize = 0 if compression = 0
    {
        // A: This is not a BMP file, B: The file is wrong.
        return ABMP_BMP_DATA_IS_CORRUPTED;
    }

    if(sizeof(ABMP_BITMAP_HEADER) == ABMP_HEADER_SIZE) 
    {
        memcpy(data, header, sizeof(ABMP_BITMAP_HEADER));
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;

        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->signature,        __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->filesize,         __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->reserved,         __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->dataoffset,       __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->size,             __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->width,            __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->height,           __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->planes,           __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->bits_per_pixel,   __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->compression,      __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->imagesize,        __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->y_pixels_per_m,   __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->x_pixels_per_m,   __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->colors_used,      __BMP_MEMORY_SIZES[count++]);
        memcpy(data + __BMP_MEMORY_OFFSETS(count), &header->important_colors, __BMP_MEMORY_SIZES[count++]);
    }

    return ABMP_OK;
}

ABMP_ERRORS abmp_write_data(uint8_t* data, ABMP_BITMAP* bitmap)
{
    memcpy(data + bitmap->header.dataoffset, bitmap->pixel_data, bitmap->header.imagesize);

    return ABMP_OK;
}

ABMP_ERRORS abmp_write_file_p(FILE* file, ABMP_BITMAP* bitmap)
{
    ABMP_ERRORS status;

    uint8_t* file_data = abmp_allocate_writer(&bitmap->header);

    status = abmp_write_header(file_data, &bitmap->header);

    if(status != ABMP_OK)
    {
        free(file_data);
        return status;
    }

    status = abmp_write_data(file_data, bitmap);

    if(status != ABMP_OK)
    {
        free(file_data);
        return status;
    }

    size_t f_status = fwrite(file_data, sizeof(char), bitmap->header.dataoffset + bitmap->header.imagesize, file);

    free(file_data);

    if(f_status != bitmap->header.dataoffset + bitmap->header.imagesize)
    {
        return ABMP_ERROR_WRITING_FILE;
    }

    return status;
}

ABMP_ERRORS abmp_write_file(char* path, ABMP_BITMAP* bitmap)
{
    // Open file
    FILE* file = fopen(path, "wb");

    if(file == NULL)
    {
        return ABMP_ERROR_OPENING_FILE;
    }

    ABMP_ERRORS status = abmp_write_file_p(file, bitmap);

    fclose(file);

    return status;
}
