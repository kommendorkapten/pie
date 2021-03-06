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

#include "../pie_types.h"

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int pie_bm_alloc_u8(struct pie_bitmap_u8rgb*);

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int pie_bm_alloc_u16(struct pie_bitmap_u16rgb*);

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int pie_bm_alloc_f32(struct pie_bitmap_f32rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void pie_bm_free_u8(struct pie_bitmap_u8rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void pie_bm_free_u16(struct pie_bitmap_u16rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void pie_bm_free_f32(struct pie_bitmap_f32rgb*);

/**
 * Get a pixel from a bitmap.
 * @param the pixel to store the result in
 * @param the bitmap to extract the pixel from.
 * @param the x coordinate (column).
 * @param the y coordinate (row).
 * @return void
 */
void pie_bm_pixel_u8rgb_get(struct pie_pixel_u8rgb*,
                            const struct pie_bitmap_u8rgb*,
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
void pie_bm_pixel_u8rgb_set(struct pie_bitmap_u8rgb*,
                            int,
                            int,
                            struct pie_pixel_u8rgb*);

/**
 * Get a pixel from a bitmap.
 * @param the pixel to store the result in
 * @param the bitmap to extract the pixel from.
 * @param the x coordinate (column).
 * @param the y coordinate (row).
 * @return void
 */
void pie_bm_pixel_u16rgb_get(struct pie_pixel_u16rgb*,
                             const struct pie_bitmap_u16rgb*,
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
void pie_bm_pixel_u16rgb_set(struct pie_bitmap_u16rgb*,
                             int,
                             int,
                             struct pie_pixel_u16rgb*);

/**
 * Convert a bitmap from a specific bitdepth.
 * The destination must point to a pie_bitmap_XXXrgb struct.
 * The destination bitmamp must *NOT* contain any allocated channels,
 * as they will not be freed. Calling with allocated channels will result
 * in a memory leak as they will be overwrittern.
 * If invalid parameters are present, target bitmap is not modified.
 * @param the target bitmap (uninitialized).
 * @param the target bit depth.
 * @param the source bitmap.
 * @param the source bit dept.
 * @return 0 on success.
 */
int pie_bm_conv_bd(void* restrict,
                   enum pie_color_bit_depth,
                   void* restrict,
                   enum pie_color_bit_depth);

/**
 * Down sample an bitmap. Downscaling is performed by a Gaussian blur
 * across the neighbours. The new size is provided by maximum new width
 * and height. The new size is the smallest of the new parameters that
 * can be achieved by maintaining the original aspect ratio.
 * A dimension can be set to don't care by providing -1. It is an error
 * to set both directions as don't care.
 * @param output bitmap. Must be an uninitialized bitmap structure.
 *        If initialized, memory leak will occur.
 * @param source bitmap to downsample.
 * @param maximum new width in pixels, -1 if don't care.
 * @param maximum new height in pixels, -1 if dont' care.
 * @return 0 on success. Non zero otherwise.
 */
int pie_bm_dwn_smpl(struct pie_bitmap_f32rgb* restrict,
                    const struct pie_bitmap_f32rgb* restrict,
                    int,
                    int);

#endif /* __PIE_BM_H__ */
