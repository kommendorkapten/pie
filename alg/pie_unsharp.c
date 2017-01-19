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

#define MAX_SEP_LEN 60

#include "pie_unsharp.h"
#include "pie_kernel.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * Apply unsharp mask to a single channel.
 * @param channel to be sharpened.
 * @param blurred copy of the channel.
 * @param unsharp param.
 * @param width of bitmap.
 * @param height of bitmap.
 * @param row stride of bitmap.
 * @return void.
 */
static void pie_unsharp_chan(float* restrict,
                             const float* restrict,
                             const struct pie_unsharp_param*,
                             int,
                             int,
                             int);

int pie_unsharp(struct bitmap_f32rgb* img,
                const struct pie_unsharp_param* param)
{
        float kernel[MAX_SEP_LEN];
        size_t size;
        float* buf;
        float* cpy;
        /* Dimension of kernel should be ceil(6*sigma) */
        int sep_len = 6 * (param->radius + 0.5f);

        if (sep_len > MAX_SEP_LEN)
        {
                sep_len = MAX_SEP_LEN;
        }
        
        size = img->row_stride * img->height * sizeof(float);
        buf = malloc(size);
        if (buf == NULL)
        {
                return -1;
        }
        cpy = malloc(size);
        if (cpy == NULL)
        {
                free(buf);
                return -1;
        }
        /* Create a separable gauss kernel */
        pie_kernel_sep_gauss(kernel, sep_len, param->radius * param->radius);

        /* Red channel */
        memcpy(cpy, img->c_red, size);
        pie_kernel_sep_apply(cpy,
                             kernel,
                             sep_len,
                             buf,
                             img->width,
                             img->height,
                             img->row_stride);
        pie_unsharp_chan(&img->c_red[0],
                         cpy,
                         param,
                         img->width,
                         img->height,
                         img->row_stride);
        
        /* Green channel */
        memcpy(cpy, img->c_green, size);        
        pie_kernel_sep_apply(cpy,
                             kernel,
                             sep_len,
                             buf,
                             img->width,
                             img->height,
                             img->row_stride);
        pie_unsharp_chan(&img->c_green[0],
                         cpy,
                         param,
                         img->width,
                         img->height,
                         img->row_stride);

        /* Blue channel */        
        memcpy(cpy, img->c_blue, size);        
        pie_kernel_sep_apply(cpy,
                             kernel,
                             sep_len,
                             buf,
                             img->width,
                             img->height,
                             img->row_stride);
        pie_unsharp_chan(&img->c_blue[0],
                         cpy,
                         param,
                         img->width,
                         img->height,
                         img->row_stride);
        
        free(buf);
        free(cpy);
        
        return 0;
}

static void pie_unsharp_chan(float* restrict o,
                             const float* restrict blur,
                             const struct pie_unsharp_param* param,
                             int w,
                             int h,
                             int s)
{
        float threshold = param->threshold / 255.0f;
        
        for (int y = 0; y < h; y++)
        {
                for (int x = 0; x < w; x++)
                {
                        int p = y * s + x;
                        float mask = o[p] - blur[p];
                        float new = o[p] + mask * param->amount;

                        if (fabs(o[p] - new) > threshold)
                        {
                                o[p] = new;
                        }
                        
                        if (o[p] < 0.0f)
                        {
                                o[p] = 0.0f;
                        }
                        else if (o[p] > 1.0f)
                        {
                                o[p] = 1.0f;
                        }
                }
        }        
}
