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
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int bm_alloc_u8(struct bitmap_u8rgb*);

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int bm_alloc_u16(struct bitmap_u16rgb*);

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int bm_alloc_f32(struct bitmap_f32rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void bm_free_u8(struct bitmap_u8rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void bm_free_u16(struct bitmap_u16rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void bm_free_f32(struct bitmap_f32rgb*);

/**
 * Get a pixel from a bitmap.
 * @param the pixel to store the result in
 * @param the bitmap to extract the pixel from.
 * @param the x coordinate (column).
 * @param the y coordinate (row).
 * @return void
 */
void pixel_u8rgb_get(struct pixel_u8rgb*,
                     const struct bitmap_u8rgb*, 
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
void pixel_u8rgb_set(struct bitmap_u8rgb*, 
                     int, 
                     int,
                     struct pixel_u8rgb*);

/**
 * Convert a bitmap from a specific bitdepth.
 * The destination must point to a bitmap_XXXrgb struct.
 * The destination bitmamp must *NOT* contain any allocated channels,
 * as they will not be freed. Calling with allocated channels will result
 * in a memory leak as they will be overwrittern.
 * @param the target bitmap (uninitialized).
 * @param the target bit depth.
 * @param the source bitmap.
 * @param the source bit dept.
 */
int bm_conv_bd(void* restrict, 
               enum pie_color_bit_depth,
               void* restrict, 
               enum pie_color_bit_depth);
#endif /* __PIE_BM_H__ */
