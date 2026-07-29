#include "abmp.h"

ABMP_ERRORS abmp_read_header_from_file(FILE* file, ABMP_BITMAP_HEADER* header)
{
    if(file == NULL || header == NULL) return ABMP_INVALID_PARAMETERS;

    if(sizeof(ABMP_BITMAP_HEADER) == ABMP_HEADER_SIZE) 
    {
        // 1 * sizeof(ABMP_BITMAP_HEADER) = sizeof(ABMP_BITMAP_HEADER)
        if(fread(header, 1, sizeof(ABMP_BITMAP_HEADER), file) != sizeof(ABMP_BITMAP_HEADER)) return ABMP_ERROR_READING_FILE;
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;

        // fread automatically moves the position -> no need of use fseek

        if(fread(&header->signature,        1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->filesize,         1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->reserved,         1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->dataoffset,       1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->size,             1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->width,            1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->height,           1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->planes,           1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->bits_per_pixel,   1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->compression,      1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->imagesize,        1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->y_pixels_per_m,   1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->x_pixels_per_m,   1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->colors_used,      1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
        if(fread(&header->important_colors, 1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_READING_FILE;
    }

    ABMP_ERRORS status;

    if((status = abmp_check_header(header)) != ABMP_OK) return status;

    return ABMP_OK;
}

/**
 * @param file File must be at the start of the bmp file
 */
ABMP_ERRORS abmp_read_pixeldata_from_file(FILE* file, ABMP_BITMAP* bitmap)
{
    if(file == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

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

    // Can't get current position
    if(ftell(file) == -1)
    {
        free(bitmap->pixel_data);
        return ABMP_ERROR_FTELLING_FILE;
    }

    if(fseek(file, ftell(file) + bitmap->header.dataoffset, SEEK_SET) != 0)
    {
        free(bitmap->pixel_data);
        return ABMP_ERROR_SEEKING_FILE;
    }

    if(fread(bitmap->pixel_data, 1, bitmap->header.imagesize, file) != bitmap->header.imagesize)
    {
        free(bitmap->pixel_data);
        return ABMP_ERROR_READING_FILE;
    }

    return ABMP_OK;
}

ABMP_ERRORS abmp_read_file_p_using_direct(FILE* file, ABMP_BITMAP* bitmap)
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
    
    if(fseek(file, file_start, SEEK_SET) != 0) return ABMP_ERROR_SEEKING_FILE;

    // Read header & pixel_data
    status = abmp_read_header_from_file(file, &bitmap->header);

    if(status != ABMP_OK) return status;

    // Reset position for abmp_read_pixeldata_from_file
    if(fseek(file, file_start, SEEK_SET) != 0) return ABMP_ERROR_SEEKING_FILE;

    status = abmp_read_pixeldata_from_file(file, bitmap);

    return status;
}

ABMP_ERRORS abmp_read_filepath_using_direct(const char* path, ABMP_BITMAP* bitmap)
{
    if(path == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    // Open file
    FILE* file = fopen(path, "rb");

    if(file == NULL) return ABMP_FILE_NOT_EXIST;

    ABMP_ERRORS status = abmp_read_file_p_using_direct(file, bitmap);

    fclose(file);

    return status;
}
