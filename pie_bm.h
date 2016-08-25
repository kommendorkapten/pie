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

#ifndef __PIE_BM_H__
#define __PIE_BM_H__

#include "pie_types.h"

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int bm_alloc_f32(struct bitmap_f32rgb*);

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int bm_alloc_8(struct bitmap_8rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void bm_free_f32(struct bitmap_f32rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void bm_free_8(struct bitmap_8rgb*);

/**
 * Get a pixel from a bitmap.
 * @param the pixel to store the result in
 * @param the bitmap to extract the pixel from.
 * @param the x coordinate (column).
 * @param the y coordinate (row).
 * @return void
 */
void pixel_8rgb_get(struct pixel_8rgb*,
                    const struct bitmap_8rgb*, 
                    int, 
                    int);

/**
 * Set a pixel int the bitmap.
 * @param the bitmap to extract set pixel int.
 * @param the x coordinate (column).
 * @param the y coordinate (row)
 * @param the pixel to use.
 * @return void
 */
void pixel_8rgb_set(struct bitmap_8rgb*, 
                    int, 
                    int,
                    struct pixel_8rgb*);
#endif /* __PIE_BM_H__ */
