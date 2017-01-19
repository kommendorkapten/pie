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

#include "../pie_log.h"
#include "../math/pie_math.h"
#include "pie_bm.h"

#define MAX_RADIUS 11

static float gauss_avg(const float* restrict img,
                       const float* restrict gauss,
                       int x,
                       int y,
                       int w,
                       int h,
                       int s,
                       int r);

int pie_dwn_smpl(struct bitmap_f32rgb* restrict dst,
                 const struct bitmap_f32rgb* restrict src,
                 int min_w,
                 int min_h)
{
        float blur[MAX_RADIUS * MAX_RADIUS];
        float ratio;
        float step;
        float sigma;
        int new_w;
        int new_h;
        int status;
        int radius;

        if (min_w < 0 && min_h < 0)
        {
                PIE_WARN("Min width OR min height must be specified");
                return -1;
        }
        
        ratio = (float)src->width / (float)src->height;
        
        if (min_w > 0)
        {
                if (min_h > 0)
                {
                        // Both are provided, chose the smallest.
                        // Start with height.
                        new_h = min_h;
                        if ((int)(new_h * ratio) <= min_w)
                        {
                                // new width fits inside provided box.
                                new_w = (int)(new_h * ratio);
                        }
                        else
                        {
                                // New width is "tighter"
                                new_w = min_w;
                                new_h = (int)(new_w / ratio);
                        }
                }
                else
                {
                        new_w = min_w;
                        new_h = (int)(new_w / ratio);
                }
        }
        else
        {
                new_h = min_h;
                new_w = (int)(new_h * ratio);
        }

        dst->width = new_w;
        dst->height = new_h;
        dst->color_type = src->color_type;
        status = bm_alloc_f32(dst);
        if (status)
        {
                return -1;
        }
        
        PIE_DEBUG("Downsample from (%d, %d) to (%d, %d)",
                  src->width, src->height, dst->width, dst->height);
        step = 1.0f/(dst->width / (float)src->width);
        PIE_DEBUG("Scaling x: %f, y: %f",
                  dst->width / (float)src->width,
                  dst->height / (float)src->height);
        PIE_DEBUG("Step is %f", step);
        radius = (int)(step + 1.0f);

        // Only deal with odd values for the radius
        if ((radius & 0x1) == 0)
        {
                radius++;
        }
        sigma = step;
        PIE_DEBUG("Matrix size %d, sigma: %f", radius, sigma);
        if (radius > MAX_RADIUS)
        {
                radius = MAX_RADIUS;
        }

        /* Create Gaussion blur matrix */
        pie_gauss_matrix(&blur[0], radius, sigma * sigma);
        pie_matrix_print(blur, radius);

        for (int y = 0; y < dst->height; y++)
        {
                int sy = (int)(y * step);

                if (sy >= src->height)
                {
                        sy = src->height - 1;
                }
                
                for (int x = 0; x < dst->width; x++)
                {
                        int sx = (int)(x * step);

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