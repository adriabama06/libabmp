#include "abmp.h"

ABMP_ERRORS abmp_write_header_to_file(FILE* file, ABMP_BITMAP_HEADER* header)
{
    if(header->signature[0] != 'B' || header->signature[1] != 'M')
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
        // 1 * sizeof(ABMP_BITMAP_HEADER) = sizeof(ABMP_BITMAP_HEADER)
        if(fwrite(header, 1, sizeof(ABMP_BITMAP_HEADER), file) != sizeof(ABMP_BITMAP_HEADER)) return ABMP_ERROR_WRITING_FILE;
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;

        // fwrite automatically moves the position -> no need of use fseek

        if(fwrite(&header->signature,        1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->filesize,         1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->reserved,         1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->dataoffset,       1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->size,             1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->width,            1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->height,           1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->planes,           1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->bits_per_pixel,   1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->compression,      1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->imagesize,        1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->y_pixels_per_m,   1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->x_pixels_per_m,   1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->colors_used,      1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
        if(fwrite(&header->important_colors, 1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count++]) return ABMP_ERROR_WRITING_FILE;
    }

    return ABMP_OK;
}

ABMP_ERRORS abmp_write_pixeldata_to_file(FILE* file, ABMP_BITMAP* bitmap)
{
    if(fwrite(bitmap->pixel_data, 1, bitmap->header.imagesize, file) != bitmap->header.imagesize) return ABMP_ERROR_WRITING_FILE;

    return ABMP_OK;
}

// TODO: This function is not taking in account the header->dataoffset, so is writing an invalid file if header->dataoffset is different from ABMP_HEADER_SIZE
ABMP_ERRORS abmp_write_file_p_using_direct(FILE* file, ABMP_BITMAP* bitmap)
{
    ABMP_ERRORS status;

    status = abmp_write_header_to_file(file, &bitmap->header);

    if(status != ABMP_OK) return status;

    // TODO: Here do the seek of dataoffset ^

    status = abmp_write_pixeldata_to_file(file, bitmap);

    return status;
}

ABMP_ERRORS abmp_write_filepath_using_direct(char* path, ABMP_BITMAP* bitmap)
{
    // Open file
    FILE* file = fopen(path, "wb");

    if(file == NULL) return ABMP_ERROR_OPENING_FILE;

    ABMP_ERRORS status = abmp_write_file_p_using_direct(file, bitmap);

    fclose(file);

    return status;
}
