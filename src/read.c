#include "abmp.h"

size_t _BMP_MEMORY_OFFSETS(size_t count)
{
    if(count > sizeof(__BMP_MEMORY_SIZES)/sizeof(__BMP_MEMORY_SIZES[0])) {
        count = sizeof(__BMP_MEMORY_SIZES)/sizeof(__BMP_MEMORY_SIZES[0]);
    }

    size_t offset = 0;

    for (size_t i = 1; i < count; i++)
    {
        offset += __BMP_MEMORY_SIZES[i - 1];
    }

    return offset;
}

/**
 * @param data Data len must be >= ABMP_HEADER_SIZE (54)
 * @return It returns ABMP_OK (0) if ok, any other number means other errors
 */
ABMP_ERRORS abmp_read_header_from_memory(const uint8_t* data, ABMP_BITMAP_HEADER* header)
{
    if(data == NULL || header == NULL) return ABMP_INVALID_PARAMETERS;

    if(sizeof(ABMP_BITMAP_HEADER) == ABMP_HEADER_SIZE) 
    {
        memcpy(header, data, sizeof(ABMP_BITMAP_HEADER));
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;
        size_t offset = 0;

#define ABMP_COPY_FIELD(field, size)                 \
        memcpy(&header->field, data + offset, size); \
        offset += size;

        ABMP_COPY_FIELD(signature,        __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(filesize,         __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(reserved,         __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(dataoffset,       __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(size,             __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(width,            __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(height,           __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(planes,           __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(bits_per_pixel,   __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(compression,      __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(imagesize,        __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(y_pixels_per_m,   __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(x_pixels_per_m,   __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(colors_used,      __BMP_MEMORY_SIZES[count++]);
        ABMP_COPY_FIELD(important_colors, __BMP_MEMORY_SIZES[count]);

#undef ABMP_COPY_FIELD
    }

    ABMP_ERRORS status;

    if((status = abmp_check_header(header)) != ABMP_OK) return status;

    return ABMP_OK;
}

/**
 * @param data Data len must be >= ABMP_HEADER_SIZE + header.dataoffset + header.imagesize
 */
ABMP_ERRORS abmp_read_pixeldata_from_memory(const uint8_t* data, ABMP_BITMAP* bitmap)
{
    if(data == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

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

// TODO: Check fseek + After reading the header check if the remain file is long enough for the imagesize
ABMP_ERRORS abmp_read_file_p_using_memory(FILE* file, ABMP_BITMAP* bitmap)
{
    if(file == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    ABMP_ERRORS status;

    long file_start = ftell(file);

    if(file_start == -1) return ABMP_ERROR_FTELLING_FILE;

    // Move to the end to get the file size
    if(fseek(file, 0, SEEK_END) != 0) return ABMP_ERROR_SEEKING_FILE;

    long file_end = ftell(file);

    if(file_end < file_start) return ABMP_ERROR_FTELLING_FILE;

    long file_size = file_end - file_start;

    if(file_size < ABMP_HEADER_SIZE) return ABMP_FILE_SIZE_IS_LOWER_THAN_HEADER_SIZE;

    // Copy the file content
    uint8_t* file_data = (uint8_t*) malloc(file_size * sizeof(uint8_t));

    if(file_data == NULL) return ABMP_OUT_OF_MEMORY;

    if(fseek(file, file_start, SEEK_SET) != 0) {
        free(file_data);
        return ABMP_ERROR_SEEKING_FILE;
    }

    size_t f_status = fread(file_data, sizeof(uint8_t), file_size, file);

    if(f_status != (size_t) file_size)
    {
        free(file_data);
        return ABMP_ERROR_READING_FILE;
    }

    // Read header & pixel_data
    status = abmp_read_header_from_memory(file_data, &bitmap->header);

    if(status != ABMP_OK)
    {
        free(file_data);
        return status;
    }

    if(file_size - bitmap->header.dataoffset < bitmap->header.imagesize)
    {
        free(file_data);
        return ABMP_FILE_IMAGESIZE_MISSMATCH;
    }

    status = abmp_read_pixeldata_from_memory(file_data, bitmap);

    free(file_data);

    return status;
}

ABMP_ERRORS abmp_read_filepath_using_memory(const char* path, ABMP_BITMAP* bitmap)
{
    if(path == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    // Open file
    FILE* file = fopen(path, "rb");

    if(file == NULL) return ABMP_FILE_NOT_EXIST;

    ABMP_ERRORS status = abmp_read_file_p_using_memory(file, bitmap);

    fclose(file);

    return status;
}
