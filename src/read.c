#include "abmp.h"

// Define extern const unsigned long __BMP_MEMORY_SIZES[];
const unsigned long __BMP_MEMORY_SIZES[] = {
    sizeof(uint8_t) * 2,
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint16_t),
    sizeof(uint16_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t),
    sizeof(uint32_t)
};

size_t __BMP_MEMORY_OFFSETS(size_t count)
{
    size_t offset = 0;

    for (size_t i = 1; i < count; i++)
    {
        offset += __BMP_MEMORY_SIZES[i - 1];
    }

    return offset;
}

/**
 * It returns 0 if ok, any other code means other errors
 */
ABMP_ERRORS abmp_read_header(uint8_t* data, ABMP_BITMAP_HEADER* header)
{
    if(sizeof(ABMP_BITMAP_HEADER) == ABMP_HEADER_SIZE) 
    {
        memcpy(header, data, sizeof(ABMP_BITMAP_HEADER));
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;

        memcpy(&header->signature,        data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->filesize,         data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->reserved,         data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->dataoffset,       data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->size,             data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->width,            data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->height,           data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->planes,           data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->bits_per_pixel,   data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->compression,      data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->imagesize,        data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->y_pixels_per_m,   data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->x_pixels_per_m,   data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->colors_used,      data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
        memcpy(&header->important_colors, data + __BMP_MEMORY_OFFSETS(count), __BMP_MEMORY_SIZES[count++]);
    }

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

    return ABMP_OK;
}

ABMP_ERRORS abmp_read_data(uint8_t* data, ABMP_BITMAP* bitmap)
{
    if(bitmap->header.compression != 0)
    {
        // Has compression
        return ABMP_COMPRESSION_IS_NOT_SUPPORTED;
    }

    if(bitmap->header.bits_per_pixel <= 8)
    {
        // Do something with ColorTable
        return ABMP_LOW_BITS_PER_PIXEL_IS_NOT_SUPPORTED;
    }

    bitmap->pixel_data = (uint8_t*) malloc(bitmap->header.imagesize);

    // Not enough memory
    if(bitmap->pixel_data == NULL) return ABMP_OUT_OF_MEMORY;

    memcpy(bitmap->pixel_data, data + bitmap->header.dataoffset, bitmap->header.imagesize);

    return ABMP_OK;
}

ABMP_ERRORS abmp_read_file_p(FILE* file, ABMP_BITMAP* bitmap)
{
    ABMP_ERRORS status;

    long file_start = ftell(file);

    // Get the file size
    fseek(file, 0, SEEK_END);

    long file_size = ftell(file) - file_start;

    if(file_size < ABMP_HEADER_SIZE) return ABMP_FILE_SIZE_IS_LOWER_THAN_HEADER_SIZE;

    // Copy the file content
    uint8_t* file_data = (uint8_t*) malloc(file_size * sizeof(uint8_t));

    if(file_data == NULL) return ABMP_OUT_OF_MEMORY;

    fseek(file, file_start, SEEK_SET);

    size_t f_status = fread(file_data, sizeof(uint8_t), file_size, file);

    if(f_status != file_size)
    {
        free(file_data);
        return ABMP_ERROR_READING_FILE;
    }

    // Read header & pixel_data
    status = abmp_read_header(file_data, &bitmap->header);

    if(status != ABMP_OK)
    {
        free(file_data);
        return status;
    }

    status = abmp_read_data(file_data, bitmap);

    free(file_data);

    return status;
}

ABMP_ERRORS abmp_read_file(char* path, ABMP_BITMAP* bitmap)
{
    // Open file
    FILE* file = fopen(path, "rb");

    if(file == NULL) return ABMP_FILE_NOT_EXIST;

    ABMP_ERRORS status = abmp_read_file_p(file, bitmap);

    fclose(file);

    return status;
}

void abmp_free(ABMP_BITMAP* bitmap)
{
    free(bitmap->pixel_data);
}
