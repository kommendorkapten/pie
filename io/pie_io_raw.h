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

#ifndef __PIE_IO_RAW_H__
#define __PIE_IO_RAW_H__

#include "../pie_types.h"

struct pie_io_opt;

/**
 * Reads an RAW image into the provided bitmap.
 * @param the bitmap to store image in.
 * @param path to the file on disk.
 * @param options to use when loading, or NULL for default.
 * @return 0 on success, error code otherwise.
 */
extern int pie_io_raw_f32_read(struct pie_bitmap_f32rgb*,
                               const char*,
                               struct pie_io_opt*);

#endif /* __PIE_IO_RAW_H__ */
