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

#include "pie_downs.h"
#include "../bm/pie_bm.h"
#include "../math/pie_math.h"
#include "../prunt/pie_log.h"

#define MAX_RADIUS 11
#define MIN_RADIUS 7

static float gauss_avg(const float* restrict img,
                       const float* restrict gauss,
                       int x,
                       int y,
                       int w,
                       int h,
                       int s,
                       int r);

int pie_alg_dwn_smpl(struct pie_bm_f32rgb* restrict dst,
                     const struct pie_bm_f32rgb* restrict src,
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
        step = (float)src->width / (float)dst->width;
        PIE_DEBUG("Scaling x: %f, y: %f",
                  (float)dst->width / (float)src->width,
                  (float)dst->height / (float)src->height);
        PIE_DEBUG("Step is %f", step);
        radius = (int)(step * 2.0f + 0.5f);

        /* Only deal with odd values for the radius. */
        if ((radius & 0x1) == 0)
        {
                radius++;
        }
        sigma = step / 3.0f;
        if (radius > MAX_RADIUS)
        {
                radius = MAX_RADIUS;
        }
        if (radius < MIN_RADIUS)
        {
                radius = MIN_RADIUS;
        }
        PIE_DEBUG("Matrix size %d, sigma: %f", radius, sigma);

        /* Create Gaussion blur matrix */
        pie_mth_gauss_matrix(&blur[0], radius, sigma * sigma);
#if DEBUG > 1
        pie_mth_matrix_print(blur, radius);
#endif
        for (int y = 0; y < dst->height; y++)
        {
		/* Test with rounding here */
                int sy = (int)((float)y * step + 0.5f);

                if (sy >= src->height)
                {
                        sy = src->height - 1;
                }

                for (int x = 0; x < dst->width; x++)
                {
			/* Test with rounding here */
                        int sx = (int)((float)x * step + 0.5f);

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
