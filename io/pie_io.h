/*
* Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
* This file is part of pie project
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License.
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#ifndef __PIE_IO_H__
#define __PIE_IO_H__

#define PIE_IO_NOT_FOUND       1
#define PIE_IO_IO_ERR          2
#define PIE_IO_INTERNAL_ERR    3
#define PIE_IO_INV_FMT         4
#define PIE_IO_UNSUPPORTED_FMT 5

#include "pie_io_png.h"
#include "pie_io_jpg.h"
#include "pie_io_raw.h"

struct pie_bitmap_f32rgb;
enum pie_io_opt_qual
{
        PIE_IO_NORM_QUAL,
        PIE_IO_HIGH_QUAL
};

struct pie_io_opt
{
        enum pie_io_opt_qual qual;
};

/**
 * Open a file and load content into an empty bitmap.
 * @param the bitmap to load content into.
 * @param the path to open.
 * @param options to use when loading, or NULL for default.
 * @return 0 on success.
 */
extern int pie_io_load(struct pie_bitmap_f32rgb*,
                       const char*,
                       struct pie_io_opt*);

#endif /* __PIE_IO_H__ */
