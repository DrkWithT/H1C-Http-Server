#ifndef RESOURCE_H
#define RESOURCE_H

#include "h1c/h1consts.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Magic Macros */

#define FILE_EXT_TXT ".txt"
#define FILE_EXT_HTML ".html"
#define FILE_EXT_CSS ".css"
#define FILE_EXT_JS ".js"

/* Helper Funcs. */

MimeType filename_get_mime(const char *fname);
char *file_read_all(const char *fname, size_t *read_count_ref);

/* StaticResource */

/**
 * @brief Encapusulates data of any static file resource.
 * @note The data is managed and freed within resource functions, but the file name c-string is not managed. Thus, freeing the file name is dangerous.
 */
typedef struct static_resource_t
{
    const char *fname;
    MimeType type;
    size_t clen;
    char *data;
} StaticResource;

/* StaticResource Funcs. */

bool statsrc_init(StaticResource *statsrc, const char *fname);
void statsrc_dispose(StaticResource *statsrc);
MimeType statsrc_get_type(const StaticResource *statsrc);
int statsrc_get_length(const StaticResource *statsrc);
const char *statsrc_view_data(const StaticResource *statsrc);

#endif