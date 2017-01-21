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

#define MAX_SEP_LEN 61

#include "pie_unsharp.h"
#include "pie_kernel.h"
#ifdef _HAS_SSE
# include <nmmintrin.h> /* sse 4.2 */
#endif
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

        if ((sep_len & 0x1) == 0)
        {
                /* Always use an odd number for kernel dimension */
                sep_len++;
        }
        
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

static void pie_unsharp_chan(float* restrict img,
                             const float* restrict blur,
                             const struct pie_unsharp_param* param,
                             int w,
                             int h,
                             int s)
{
        float threshold = param->threshold / 255.0f;

#ifdef _HAS_SSE
        __m128 amountv = _mm_set1_ps(param->amount);
        __m128 thresholdv = _mm_set1_ps(threshold);
        __m128 onev = _mm_set1_ps(1.0f);
        __m128 zerov = _mm_set1_ps(0.0f);
        __m128 sign_maskv = _mm_set1_ps(-0.f); 
#endif
#ifdef _HAS_SIMD
	int rem = w % 4;
	int stop = w - rem;
#else
        int stop = 0;
#endif
        
        for (int y = 0; y < h; y++)
        {
        
#ifdef _HAS_SIMD
# ifdef _HAS_SSE
        
                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * s + x;
                        __m128 imgv = _mm_load_ps(img + p);
                        __m128 blurv = _mm_load_ps(blur + p);
                        __m128 maskv;
                        __m128 newv;
                        __m128 cmpv;
                        __m128 deltav;

                        maskv = _mm_sub_ps(imgv, blurv);
                        maskv = _mm_mul_ps(maskv, amountv);
                        newv = _mm_add_ps(imgv, maskv);

                        /* Threshold stuff */
                        deltav = _mm_sub_ps(imgv, newv);
                        /* Make sure all values are positive */
                        _mm_andnot_ps(sign_maskv, deltav);

                        cmpv = _mm_cmpnlt_ps(deltav, thresholdv);
                        newv = _mm_blendv_ps(imgv, newv, cmpv);
                        
                        /* the ones less than zero will be 0xffffffff */
                        cmpv = _mm_cmplt_ps(newv, zerov);
                        /* d = a,b,m. if m == 0 a else b */
                        newv = _mm_blendv_ps(newv, zerov, cmpv);

                        /* the ones greater than one will be 0xffffffff */
                        cmpv = _mm_cmpnlt_ps(newv, onev);
                        newv = _mm_blendv_ps(newv, onev, cmpv);

                        _mm_store_ps(img + p, newv);                        
                }

# else
#  error ALTIVEC not supported yet
# endif
#endif

                for (int x = stop; x < w; x++)
                {
                        int p = y * s + x;
                        float mask = img[p] - blur[p];
                        float new = img[p] + mask * param->amount;

                        if (fabs(img[p] - new) > threshold)
                        {
                                img[p] = new;
                        }
                        
                        if (img[p] < 0.0f)
                        {
                                img[p] = 0.0f;
                        }
                        else if (img[p] > 1.0f)
                        {
                                img[p] = 1.0f;
                        }
                }
        }        
}
