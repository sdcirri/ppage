#ifndef __FORMATTER_H
#define __FORMATTER_H

#include <stddef.h>

#include "file_io.h"

char *format(filematr_t *fileinfo, size_t num_col, size_t col_len, size_t col_lines, size_t col_space);

#endif

