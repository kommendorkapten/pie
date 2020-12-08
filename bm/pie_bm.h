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

#include <stdint.h>

/* Error codes for pie_bm_load */
#define PIE_BM_IO_NOT_FOUND       1
#define PIE_BM_IO_IO_ERR          2
#define PIE_BM_IO_INTERNAL_ERR    3
#define PIE_BM_IO_INV_FMT         4
#define PIE_BM_IO_UNSUPPORTED_FMT 5
#define PIE_BM_IO_INV_OPT         6

enum pie_bm_color_type
{
        PIE_BM_COLOR_TYPE_INVALID = 0,
        PIE_BM_COLOR_TYPE_GRAY = 1,
        PIE_BM_COLOR_TYPE_RGB = 3
};

enum pie_bm_color_bit_depth
{
        PIE_BM_COLOR_INVALID  = 0,
        PIE_BM_COLOR_8B = 8,   /* unsigned 8 bit int */
        PIE_BM_COLOR_16B = 16, /* unsigned 16 bit int */
        PIE_BM_COLOR_32B = 32  /* single precision 32 bit float */
};

struct pie_bm_u8rgb
{
        uint8_t* c_red;
        uint8_t* c_green;
        uint8_t* c_blue;
        enum pie_bm_color_type color_type;
        enum pie_bm_color_bit_depth bit_depth;
        int width;
        int height;
        int row_stride;
};

struct pie_bm_u16rgb
{
        uint16_t* c_red;
        uint16_t* c_green;
        uint16_t* c_blue;
        enum pie_bm_color_type color_type;
        enum pie_bm_color_bit_depth bit_depth;
        int width;
        int height;
        int row_stride;
};

struct pie_bm_f32rgb
{
        float* c_red;
        float* c_green;
        float* c_blue;
        enum pie_bm_color_type color_type;
        enum pie_bm_color_bit_depth bit_depth;
        int width;
        int height;
        int row_stride;
};

struct pie_bm_px_u8rgb
{
        uint8_t red;
        uint8_t green;
        uint8_t blue;
};

struct pie_bm_px_u16rgb
{
        uint16_t red;
        uint16_t green;
        uint16_t blue;
};

struct pie_bm_px_f32rgb
{
        float red;
        float green;
        float blue;
};


enum pie_bm_opt_qual
{
        PIE_BM_NORM_QUAL,
        PIE_BM_HIGH_QUAL
};

enum pie_bm_opt_cspace
{
        PIE_BM_LINEAR,
        PIE_BM_SRGB
};

struct pie_bm_opts
{
        enum pie_bm_opt_qual qual;
        enum pie_bm_opt_cspace cspace;
};

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int pie_bm_alloc_u8(struct pie_bm_u8rgb*);

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int pie_bm_alloc_u16(struct pie_bm_u16rgb*);

/**
 * Alloc data for pixel data. If color type is rgb, only
 * red channel will contain data.
 * Width, height and color_type must be set before allocation
 * can occur.
 * After allocation the channels, bit depth and row_stride are set.
 * @param the bitmap to allocate pixel data for.
 * @return 0 if success, non zero otherwise.
 */
int pie_bm_alloc_f32(struct pie_bm_f32rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void pie_bm_free_u8(struct pie_bm_u8rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void pie_bm_free_u16(struct pie_bm_u16rgb*);

/**
 * Free data for bitmap. If color type is rgb, only
 * red channel will be freed.
 * @param the bitmap to free data for.
 * @return void
 */
void pie_bm_free_f32(struct pie_bm_f32rgb*);

/**
 * Get a pixel from a bitmap.
 * @param the pixel to store the result in
 * @param the bitmap to extract the pixel from.
 * @param the x coordinate (column).
 * @param the y coordinate (row).
 * @return void
 */
void pie_bm_px_u8rgb_get(struct pie_bm_px_u8rgb*,
                         const struct pie_bm_u8rgb*,
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
void pie_bm_px_u8rgb_set(struct pie_bm_u8rgb*,
                         int,
                         int,
                         struct pie_bm_px_u8rgb*);

/**
 * Get a pixel from a bitmap.
 * @param the pixel to store the result in
 * @param the bitmap to extract the pixel from.
 * @param the x coordinate (column).
 * @param the y coordinate (row).
 * @return void
 */
void pie_bm_px_u16rgb_get(struct pie_bm_px_u16rgb*,
                          const struct pie_bm_u16rgb*,
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
void pie_bm_px_u16rgb_set(struct pie_bm_u16rgb*,
                          int,
                          int,
                          struct pie_bm_px_u16rgb*);

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
                   enum pie_bm_color_bit_depth,
                   void* restrict,
                   enum pie_bm_color_bit_depth);

/**
 * Open a file and load content into an empty bitmap.
 * @param the bitmap to load content into.
 * @param the path to open.
 * @param options to use when loading, or NULL for default:
 *        PIE_IO_NORM_QUAL
 *        PIE_IO_SRGB
 * @return 0 on success.
 */
extern int pie_bm_load(struct pie_bm_f32rgb*,
                       const char*,
                       struct pie_bm_opts*);

#endif /* __PIE_BM_H__ */
