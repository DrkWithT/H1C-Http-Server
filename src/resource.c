/**
 * @file resource.c
 * @author Derek Tan
 * @brief Implements only static resources for now.
 * @date 2023-09-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/resource.h"

/* Helper Funcs. */

MimeType filename_get_mime(const char *fname)
{
    size_t fname_len = strlen(fname);
    size_t count = 0;
    const char *cursor = fname;
    char temp = *cursor;

    if (temp == '.' || temp == '\0')
        return ANY_ANY;

    // skip separating dot if any
    while (temp != '\0')
    {
        if (temp == '.')
            break;

        cursor++;
        temp = *cursor;
        count++;
    }

    if (strncmp(cursor, FILE_EXT_TXT, 4) == 0)
        return TXT_PLAIN;

    if (strncmp(cursor, FILE_EXT_HTML, 5) == 0)
        return TXT_HTML;

    if (strncmp(cursor, FILE_EXT_CSS, 4) == 0)
        return TXT_CSS;

    if (strncmp(cursor, FILE_EXT_JS, 3) == 0)
        return TXT_JS;

    return ANY_ANY;
}

char *file_read_all(const char *fname, size_t *read_count_ref)
{
    char *data = NULL;
    size_t buf_size = 0;
    size_t buf_end = 0;
    FILE *fs = fopen(fname, "r");

    if (!fs)
    {
        *read_count_ref = buf_size;
        return data;
    }

    // Get file size
    fseek(fs, 0, SEEK_END);
    buf_size = ftell(fs);
    fseek(fs, 0, SEEK_SET);

    // Make and fill buffer as c-string
    data = calloc(buf_size + 1, sizeof(char));

    if (data != NULL)
    {
        buf_end = fread(data, 1, buf_size, fs);
        data[buf_end] = '\0';
        buf_size = buf_end;
    }

    *read_count_ref = buf_size;
    return data;
}

/* StaticResource Funcs. */

void statsrc_init(StaticResource *statsrc, const char *fname)
{
    statsrc->fname = fname;
    statsrc->type = filename_get_mime(fname);

    size_t temp_clen = 0;
    char *raw_data = file_read_all(fname, &temp_clen);

    statsrc->data = raw_data;
    statsrc->clen = temp_clen;
}

void statsrc_dispose(StaticResource *statsrc)
{
    if (!statsrc->data)
        return;

    free(statsrc->data);
    statsrc->data = NULL;
    statsrc->clen = 0;
}

MimeType statsrc_get_type(const StaticResource *statsrc)
{
    return statsrc->type;
}

int statsrc_get_length(const StaticResource *statsrc)
{
    return statsrc->clen;
}

const char *statsrc_view_data(const StaticResource *statsrc)
{
    return statsrc->data;
}
