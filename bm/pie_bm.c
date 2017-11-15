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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "pie_bm.h"
#include "../pie_log.h"
#include "../math/pie_math.h"

#define MAX_RADIUS 11

static void pie_bm_conv_u8_u16(struct pie_bitmap_u8rgb*,
                               const struct pie_bitmap_u16rgb*);
static void pie_bm_conv_u8_f32(struct pie_bitmap_u8rgb*,
                               const struct pie_bitmap_f32rgb*);
static void pie_bm_conv_u16_u8(struct pie_bitmap_u16rgb*,
                               const struct pie_bitmap_u8rgb*);
static void pie_bm_conv_u16_f32(struct pie_bitmap_u16rgb*,
                                const struct pie_bitmap_f32rgb*);
static void pie_bm_conv_f32_u16(struct pie_bitmap_f32rgb*,
                                const struct pie_bitmap_u16rgb*);
static void pie_bm_conv_f32_u8(struct pie_bitmap_f32rgb*,
                               const struct pie_bitmap_u8rgb*);

static float gauss_avg(const float* restrict img,
                       const float* restrict gauss,
                       int x,
                       int y,
                       int w,
                       int h,
                       int s,
                       int r);

/*
 * TODO: Verify all malloc calls.
 */

int pie_bm_alloc_u8(struct pie_bitmap_u8rgb* bm)
{
        int rem = bm->width % 4;
        size_t s;

        assert(bm->color_type != PIE_COLOR_TYPE_INVALID);

        /* Make sure each row has good alignment */
        if (rem == 0)
        {
                bm->row_stride = bm->width;
        }
        else
        {
                bm->row_stride = bm->width + (4 - rem);
        }

        s = bm->row_stride * bm->height * sizeof(uint8_t);
        bm->bit_depth = PIE_COLOR_8B;
        bm->c_red = malloc(s);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(s);
                bm->c_blue = malloc(s);
        }

        return 0;
}

int pie_bm_alloc_u16(struct pie_bitmap_u16rgb* bm)
{
        int rem = bm->width % 4;
        size_t s;

        assert(bm->color_type != PIE_COLOR_TYPE_INVALID);

        /* Make sure each row has good alignment */
        if (rem == 0)
        {
                bm->row_stride = bm->width;
        }
        else
        {
                bm->row_stride = bm->width + (4 - rem);
        }

        s = bm->row_stride * bm->height * sizeof(uint16_t);
        bm->bit_depth = PIE_COLOR_16B;
        bm->c_red = malloc(s);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(s);
                bm->c_blue = malloc(s);
        }

        return 0;
}

int pie_bm_alloc_f32(struct pie_bitmap_f32rgb* bm)
{
        int rem = bm->width % 4;
        size_t s;

        assert(bm->color_type != PIE_COLOR_TYPE_INVALID);

        /* Make sure each row has good alignment */
        if (rem == 0)
        {
                bm->row_stride = bm->width;
        }
        else
        {
                bm->row_stride = bm->width + (4 - rem);
        }

        s = bm->row_stride * bm->height * sizeof(float);
        bm->bit_depth = PIE_COLOR_32B;
        bm->c_red = malloc(s);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(s);
                bm->c_blue = malloc(s);
        }

        return 0;
}

void pie_bm_free_u8(struct pie_bitmap_u8rgb* bm)
{
        assert(bm);
        assert(bm->c_red);

        free(bm->c_red);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                assert(bm->c_green);
                assert(bm->c_blue);

                free(bm->c_green);
                free(bm->c_blue);
        }

        bm->c_red = NULL;
        bm->c_green = NULL;
        bm->c_blue = NULL;
}

void pie_bm_free_u16(struct pie_bitmap_u16rgb* bm)
{
        assert(bm);
        assert(bm->c_red);

        free(bm->c_red);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                assert(bm->c_green);
                assert(bm->c_blue);

                free(bm->c_green);
                free(bm->c_blue);
        }

        bm->c_red = NULL;
        bm->c_green = NULL;
        bm->c_blue = NULL;
}

void pie_bm_free_f32(struct pie_bitmap_f32rgb* bm)
{
        assert(bm);
        assert(bm->c_red);

        free(bm->c_red);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                assert(bm->c_green);
                assert(bm->c_blue);

                free(bm->c_green);
                free(bm->c_blue);
        }

        bm->c_red = NULL;
        bm->c_green = NULL;
        bm->c_blue = NULL;
}

void pie_pixel_u8rgb_get(struct pie_pixel_u8rgb* p,
                         const struct pie_bitmap_u8rgb* bm,
                         int x,
                         int y)
{
        int offset = bm->row_stride * y + x;

        p->red = bm->c_red[offset];
        p->green = bm->c_green[offset];
        p->blue = bm->c_blue[offset];
}

void pie_pixel_u8rgb_set(struct pie_bitmap_u8rgb* bm,
                         int x,
                         int y,
                         struct pie_pixel_u8rgb* p)
{
        int offset = bm->row_stride * y + x;

        bm->c_red[offset] = p->red;
        bm->c_green[offset] = p->green;
        bm->c_blue[offset] = p->blue;
}

void pie_pixel_u16rgb_get(struct pie_pixel_u16rgb* p,
                          const struct pie_bitmap_u16rgb* bm,
                          int x,
                          int y)
{
        int offset = bm->row_stride * y + x;

        p->red = bm->c_red[offset];
        p->green = bm->c_green[offset];
        p->blue = bm->c_blue[offset];
}

void pie_pixel_u16rgb_set(struct pie_bitmap_u16rgb* bm,
                          int x,
                          int y,
                          struct pie_pixel_u16rgb* p)
{
        int offset = bm->row_stride * y + x;

        bm->c_red[offset] = p->red;
        bm->c_green[offset] = p->green;
        bm->c_blue[offset] = p->blue;
}

int pie_bm_conv_bd(void* restrict dst,
                   enum pie_color_bit_depth dst_bd,
                   void* restrict src,
                   enum pie_color_bit_depth src_bd)
{
        int ret = -1;

        if (dst_bd == src_bd)
        {
                struct pie_bitmap_u8rgb* u8_src;
                struct pie_bitmap_u8rgb* u8_dst;
                struct pie_bitmap_u16rgb* u16_src;
                struct pie_bitmap_u16rgb* u16_dst;
                struct pie_bitmap_f32rgb* f32_src;
                struct pie_bitmap_f32rgb* f32_dst;
                size_t len;

                /* Create a copy */
                switch (dst_bd)
                {
                case PIE_COLOR_8B:
                        u8_src = src;
                        u8_dst = dst;
                        u8_dst->width = u8_src->width;
                        u8_dst->height = u8_src->height;
                        u8_dst->color_type = u8_src->color_type;
                        pie_bm_alloc_u8(u8_dst);
                        len = u8_src->row_stride * u8_src->height * sizeof(uint8_t);
                        memcpy(u8_dst->c_red, u8_src->c_red, len);
                        if (u8_src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                memcpy(u8_dst->c_green, u8_src->c_green, len);
                                memcpy(u8_dst->c_blue, u8_src->c_blue, len);
                        }
                        break;
                case PIE_COLOR_16B:
                        u16_src = src;
                        u16_dst = dst;
                        u16_dst->width = u16_src->width;
                        u16_dst->height = u16_src->height;
                        u16_dst->color_type = u16_src->color_type;
                        pie_bm_alloc_u16(u16_dst);
                        len = u16_src->row_stride * u16_src->height * sizeof(uint16_t);
                        memcpy(u16_dst->c_red, u16_src->c_red, len);
                        if (u16_src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                memcpy(u16_dst->c_green, u16_src->c_green, len);
                                memcpy(u16_dst->c_blue, u16_src->c_blue, len);
                        }
                        break;
                case PIE_COLOR_32B:
                        f32_src = src;
                        f32_dst = dst;
                        f32_dst->width = f32_src->width;
                        f32_dst->height = f32_src->height;
                        f32_dst->color_type = f32_src->color_type;
                        pie_bm_alloc_f32(f32_dst);
                        len = f32_src->row_stride * f32_src->height * sizeof(float);
                        memcpy(f32_dst->c_red, f32_src->c_red, len);
                        if (f32_src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                memcpy(f32_dst->c_green, f32_src->c_green, len);
                                memcpy(f32_dst->c_blue, f32_src->c_blue, len);
                        }
                        break;
                default:
                        return -1;
                }
        }
        if (dst_bd == PIE_COLOR_8B)
        {
                switch (src_bd)
                {
                case PIE_COLOR_16B:
                        pie_bm_conv_u8_u16((struct pie_bitmap_u8rgb*)dst,
                                           (struct pie_bitmap_u16rgb*)src);
                        ret = 0;
                        break;
                case PIE_COLOR_32B:
                        pie_bm_conv_u8_f32((struct pie_bitmap_u8rgb*)dst,
                                           (struct pie_bitmap_f32rgb*)src);
                        ret = 0;
                        break;
                default:
                        return -1;
                }
        }
        else if (dst_bd == PIE_COLOR_16B)
        {
                switch (src_bd)
                {
                case PIE_COLOR_8B:
                        pie_bm_conv_u16_u8((struct pie_bitmap_u16rgb*)dst,
                                           (struct pie_bitmap_u8rgb*)src);
                        ret = 0;
                        break;
                case PIE_COLOR_32B:
                        pie_bm_conv_u16_f32((struct pie_bitmap_u16rgb*)dst,
                                            (struct pie_bitmap_f32rgb*)src);
                        ret = 0;
                        break;
                default:
                        return -1;
                }
        }
        else if (dst_bd == PIE_COLOR_32B)
        {
                switch (src_bd)
                {
                case PIE_COLOR_8B:
                        pie_bm_conv_f32_u8((struct pie_bitmap_f32rgb*)dst,
                                           (struct pie_bitmap_u8rgb*)src);
                        ret = 0;
                        break;
                case PIE_COLOR_16B:
                        pie_bm_conv_f32_u16((struct pie_bitmap_f32rgb*)dst,
                                            (struct pie_bitmap_u16rgb*)src);
                        ret = 0;
                        break;
                default:
                        return -1;
                }
        }

        return ret;
}

int pie_bm_dwn_smpl(struct pie_bitmap_f32rgb* restrict dst,
                    const struct pie_bitmap_f32rgb* restrict src,
                    int max_w,
                    int max_h)
{
        float blur[MAX_RADIUS * MAX_RADIUS];
        float ratio;
        float step;
        float sigma;
        int new_w;
        int new_h;
        int status;
        int radius;

        if (max_w < 0 && max_h < 0)
        {
                PIE_WARN("Min width OR min height must be specified");
                return -1;
        }

        ratio = (float)src->width / (float)src->height;

        if (max_w > 0)
        {
                if (max_h > 0)
                {
                        /* Both are provided, chose the smallest.
                         * Start with height. */
                        new_h = max_h;
                        if ((int)((float)new_h * ratio) <= max_w)
                        {
                                /* new width fits inside provided box. */
                                new_w = (int)((float)new_h * ratio);
                        }
                        else
                        {
                                /* New width is "tighter" */
                                new_w = max_w;
                                new_h = (int)((float)new_w / ratio);
                        }
                }
                else
                {
                        new_w = max_w;
                        new_h = (int)((float)new_w / ratio);
                }
        }
        else
        {
                new_h = max_h;
                new_w = (int)((float)new_h * ratio);
        }

        dst->width = new_w;
        dst->height = new_h;
        dst->color_type = src->color_type;
        status = pie_bm_alloc_f32(dst);
        if (status)
        {
                return -1;
        }

        PIE_DEBUG("Downsample from (%d, %d) to (%d, %d)",
                  src->width, src->height, dst->width, dst->height);
        step = 1.0f/((float)dst->width / (float)src->width);
        PIE_DEBUG("Scaling x: %f, y: %f",
                  (float)dst->width / (float)src->width,
                  (float)dst->height / (float)src->height);
        PIE_DEBUG("Step is %f", step);
        radius = (int)(step + 1.0f);

        /* Only deal with odd values for the radius. */
        if ((radius & 0x1) == 0)
        {
                radius++;
        }
        sigma = step;
        if (radius > MAX_RADIUS)
        {
                radius = MAX_RADIUS;
        }
        PIE_DEBUG("Matrix size %d, sigma: %f", radius, sigma);

        /* Create Gaussion blur matrix */
        pie_mth_gauss_matrix(&blur[0], radius, sigma * sigma);
#if DEBUG > 2
        pie_matrix_print(blur, radius);
#endif
        for (int y = 0; y < dst->height; y++)
        {
		/* Test with rounding here */
                int sy = (int)((float)y * step);

                if (sy >= src->height)
                {
                        sy = src->height - 1;
                }

                for (int x = 0; x < dst->width; x++)
                {
			/* Test with rounding here */
                        int sx = (int)((float)x * step);

                        if (sx >= src->width)
                        {
                                sx = src->width - 1;
                        }

                        /* Gaussian average */
                        dst->c_red[y * dst->row_stride + x] =
                                gauss_avg(src->c_red,
                                          &blur[0],
                                          sx,
                                          sy,
                                          src->width,
                                          src->height,
                                          src->row_stride,
                                          radius);
                        dst->c_green[y * dst->row_stride + x] =
                                gauss_avg(src->c_green,
                                          &blur[0],
                                          sx,
                                          sy,
                                          src->width,
                                          src->height,
                                          src->row_stride,
                                          radius);
                        dst->c_blue[y * dst->row_stride + x] =
                                gauss_avg(src->c_blue,
                                          &blur[0],
                                          sx,
                                          sy,
                                          src->width,
                                          src->height,
                                          src->row_stride,
                                          radius);
                }
        }

        return 0;
}

static void pie_bm_conv_u8_u16(struct pie_bitmap_u8rgb* dst,
                               const struct pie_bitmap_u16rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        pie_bm_alloc_u8(dst);

        for (int y = 0; y < src->height; y++)
        {
                for (int x = 0; x < src->width; x++)
                {
                        int o = y * src->row_stride + x;

                        dst->c_red[o] = (uint8_t) ((src->c_red[o] / 65535.0f) * 255.0f);
                        if (src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                dst->c_green[o] = (uint8_t) ((src->c_green[o] / 65535.0f) * 255.0f);
                                dst->c_blue[o] = (uint8_t) ((src->c_blue[o] / 65535.0f) * 255.0f);
                        }
                }
        }
}

static void pie_bm_conv_u8_f32(struct pie_bitmap_u8rgb* dst,
                               const struct pie_bitmap_f32rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        pie_bm_alloc_u8(dst);

        for (int y = 0; y < src->height; y++)
        {
                for (int x = 0; x < src->width; x++)
                {
                        int o = y * src->row_stride + x;

                        dst->c_red[o] = (uint8_t) (src->c_red[o] * 255.0f);
                        if (src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                dst->c_green[o] = (uint8_t) (src->c_green[o] * 255.0f);
                                dst->c_blue[o] = (uint8_t) (src->c_blue[o] * 255.0f);
                        }
                }
        }
}

static void pie_bm_conv_u16_u8(struct pie_bitmap_u16rgb* dst,
                               const struct pie_bitmap_u8rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        pie_bm_alloc_u16(dst);

        for (int y = 0; y < src->height; y++)
        {
                for (int x = 0; x < src->width; x++)
                {
                        int o = y * src->row_stride + x;

                        dst->c_red[o] = (uint16_t) ((src->c_red[o] / 255.0f) * 65535.0f);
                        if (src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                dst->c_green[o] = (uint16_t) ((src->c_green[o] / 255.0f) * 65535.0f);
                                dst->c_blue[o] = (uint16_t) ((src->c_blue[o] / 255.0f) * 65535.0f);
                        }
                }
        }
}

static void pie_bm_conv_u16_f32(struct pie_bitmap_u16rgb* dst,
                                const struct pie_bitmap_f32rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        pie_bm_alloc_u16(dst);

        for (int y = 0; y < src->height; y++)
        {
                for (int x = 0; x < src->width; x++)
                {
                        int o = y * src->row_stride + x;

                        dst->c_red[o] = (uint16_t) (src->c_red[o] * 65535.0f);
                        if (src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                dst->c_green[o] = (uint16_t) (src->c_green[o] * 65535.0f);
                                dst->c_blue[o] = (uint16_t) (src->c_blue[o] * 65535.0f);
                        }
                }
        }
}

static void pie_bm_conv_f32_u8(struct pie_bitmap_f32rgb* dst,
                               const struct pie_bitmap_u8rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        pie_bm_alloc_f32(dst);

        for (int y = 0; y < src->height; y++)
        {
                for (int x = 0; x < src->width; x++)
                {
                        int o = y * src->row_stride + x;

                        dst->c_red[o] = src->c_red[o] / 255.0f;
                        if (src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                dst->c_green[o] = src->c_green[o] / 255.0f;
                                dst->c_blue[o] = src->c_blue[o] / 255.0f;
                        }
                }
        }
}

static void pie_bm_conv_f32_u16(struct pie_bitmap_f32rgb* dst,
                                const struct pie_bitmap_u16rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        pie_bm_alloc_f32(dst);

        for (int y = 0; y < src->height; y++)
        {
                for (int x = 0; x < src->width; x++)
                {
                        int o = y * src->row_stride + x;

                        dst->c_red[o] = src->c_red[o] / 65535.0f;
                        if (src->color_type == PIE_COLOR_TYPE_RGB)
                        {
                                dst->c_green[o] = src->c_green[o] / 65535.0f;
                                dst->c_blue[o] = src->c_blue[o] / 65535.0f;
                        }
                }
        }
}

static float gauss_avg(const float* restrict img,
                       const float* restrict gauss,
                       int x,
                       int y,
                       int w,
                       int h,
                       int s,
                       int r)
{
        float avg = 0.0f;
        int step = r / 2;

        for (int ky = 0; ky < r; ky++)
        {
                int py = y + ky - step;

                if (py < 0)
                {
                        py = 0;
                }
                else if (py > h - 1)
                {
                        py = h - 1;
                }

                for (int kx = 0; kx < r; kx++)
                {
                        int px = x + kx - step;

                        if (px < 0)
                        {
                                px = 0;
                        }
                        else if (px > w - 1)
                        {
                                px = w - 1;
                        }

                        avg += img[py * s + px] * gauss[ky * r + kx];
                }
        }

        return avg;
}
