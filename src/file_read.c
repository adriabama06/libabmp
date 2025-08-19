#include "abmp.h"

ABMP_ERRORS abmp_file_read_header(FILE* file, ABMP_BITMAP_HEADER* header)
{
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

ABMP_ERRORS abmp_file_read_data(FILE* file, ABMP_BITMAP* bitmap)
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

    fseek(file, ftell(file) + bitmap->header.dataoffset, SEEK_SET);

    fread(bitmap->pixel_data, 1, bitmap->header.imagesize, file);

    return ABMP_OK;
}

ABMP_ERRORS abmp_file_read_file_p(FILE* file, ABMP_BITMAP* bitmap)
{
    ABMP_ERRORS status;

    long file_start = ftell(file);

    // Get the file size
    fseek(file, 0, SEEK_END);

    long file_size = ftell(file) - file_start;

    if(file_size < ABMP_HEADER_SIZE) return ABMP_FILE_SIZE_IS_LOWER_THAN_HEADER_SIZE;
    
    fseek(file, file_start, SEEK_SET);

    // Read header & pixel_data
    status = abmp_file_read_header(file, &bitmap->header);

    if(status != ABMP_OK) return status;

    // Reset position
    fseek(file, file_start, SEEK_SET);

    status = abmp_file_read_data(file, bitmap);

    return status;
}

ABMP_ERRORS abmp_file_read_file(char* path, ABMP_BITMAP* bitmap)
{
    // Open file
    FILE* file = fopen(path, "rb");

    if(file == NULL) return ABMP_FILE_NOT_EXIST;

    ABMP_ERRORS status = abmp_file_read_file_p(file, bitmap);

    fclose(file);

    return status;
}
