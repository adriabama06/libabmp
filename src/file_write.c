#include "abmp.h"

ABMP_ERRORS abmp_write_header_to_file(FILE* file, const ABMP_BITMAP_HEADER* header)
{
    if(file == NULL || header == NULL) return ABMP_INVALID_PARAMETERS;

    ABMP_ERRORS status;

    if((status = abmp_check_header(header)) != ABMP_OK) return status;

    if(sizeof(ABMP_BITMAP_HEADER) == ABMP_HEADER_SIZE) 
    {
        // 1 * sizeof(ABMP_BITMAP_HEADER) = sizeof(ABMP_BITMAP_HEADER)
        if(fwrite(header, 1, sizeof(ABMP_BITMAP_HEADER), file) != sizeof(ABMP_BITMAP_HEADER)) return ABMP_ERROR_WRITING_FILE;
    }
    else // This means __attribute__((__packed__)) is not working, leaving to a manual read
    {
        size_t count = 0;

        // fwrite automatically moves the position -> no need of use fseek

#define ABMP_WRITE_FIELD(field) \
            if(fwrite(&header->field, 1, __BMP_MEMORY_SIZES[count], file) != __BMP_MEMORY_SIZES[count]) return ABMP_ERROR_WRITING_FILE; \
            count++;

        ABMP_WRITE_FIELD(signature)
        ABMP_WRITE_FIELD(filesize)
        ABMP_WRITE_FIELD(reserved)
        ABMP_WRITE_FIELD(dataoffset)
        ABMP_WRITE_FIELD(size)
        ABMP_WRITE_FIELD(width)
        ABMP_WRITE_FIELD(height)
        ABMP_WRITE_FIELD(planes)
        ABMP_WRITE_FIELD(bits_per_pixel)
        ABMP_WRITE_FIELD(compression)
        ABMP_WRITE_FIELD(imagesize)
        ABMP_WRITE_FIELD(y_pixels_per_m)
        ABMP_WRITE_FIELD(x_pixels_per_m)
        ABMP_WRITE_FIELD(colors_used)
        ABMP_WRITE_FIELD(important_colors)

#undef ABMP_WRITE_FIELD
    }

    return ABMP_OK;
}

ABMP_ERRORS abmp_write_pixeldata_to_file(FILE* file, const ABMP_BITMAP* bitmap)
{
    if(file == NULL || bitmap == NULL || bitmap->pixel_data == NULL) return ABMP_INVALID_PARAMETERS;

    if(fwrite(bitmap->pixel_data, 1, bitmap->header.imagesize, file) != bitmap->header.imagesize) return ABMP_ERROR_WRITING_FILE;

    return ABMP_OK;
}

// TODO: This function is not taking in account the header->dataoffset, so is writing an invalid file if header->dataoffset is different from ABMP_HEADER_SIZE
ABMP_ERRORS abmp_write_file_p_using_direct(FILE* file, const ABMP_BITMAP* bitmap)
{
    if(file == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    ABMP_ERRORS status;

    long file_start = ftell(file);

    if(file_start == -1) return ABMP_ERROR_FTELLING_FILE;

    status = abmp_write_header_to_file(file, &bitmap->header);

    if(status != ABMP_OK) return status;

    // Unsafe, could leave to unexpected behaviour: fseek(file, file_start + bitmap->header.dataoffset)
    // Doit by hand

    long current_pos = ftell(file);

    if(current_pos == -1) return ABMP_ERROR_FTELLING_FILE;

    if(current_pos - file_start != ABMP_HEADER_SIZE) return ABMP_ERROR_WRITING_FILE;

    // Manually fill zeros
    if(bitmap->header.dataoffset > ABMP_HEADER_SIZE)
    {
        size_t padding = bitmap->header.dataoffset - ABMP_HEADER_SIZE;
        for (size_t i = 0; i < padding; i++)
        {
            if(fputc(0, file) == EOF) return ABMP_ERROR_WRITING_FILE;
        }
    }

    status = abmp_write_pixeldata_to_file(file, bitmap);

    return status;
}

ABMP_ERRORS abmp_write_filepath_using_direct(const char* path, const ABMP_BITMAP* bitmap)
{
    if(path == NULL || bitmap == NULL) return ABMP_INVALID_PARAMETERS;

    // Open file
    FILE* file = fopen(path, "wb");

    if(file == NULL) return ABMP_ERROR_OPENING_FILE;

    ABMP_ERRORS status = abmp_write_file_p_using_direct(file, bitmap);

    fclose(file);

    return status;
}
