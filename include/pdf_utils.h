#ifndef PDF_UTILS_H
#define PDF_UTILS_H

#include <poppler.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>

#include "fs_utils.h"

int pdf_to_png(Path *pdf_path, const char *output_dir);

#endif
