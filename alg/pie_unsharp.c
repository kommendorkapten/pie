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

#define MAX_SEP_LEN 81

#include "pie_unsharp.h"
#include "pie_kernel.h"
#ifdef _HAS_SSE
# include <nmmintrin.h> /* sse 4.2 */
#endif
#ifdef _HAS_ALTIVEC
# include <altivec.h>
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

int pie_alg_unsharp(float* restrict r,
                    float* restrict g,
                    float* restrict b,
                    const struct pie_unsharp_param* param,
                    int w,
                    int h,
                    int s)
{
        float kernel[MAX_SEP_LEN];
        size_t size;
        float* buf;
        float* blur;
        /* Dimension of kernel should be ceil(6*sigma) */
        int sep_len = (int)((6.0f * param->radius) + 0.5f);

        if ((sep_len & 0x1) == 0)
        {
                /* Always use an odd number for kernel dimension */
                sep_len++;
        }

        if (sep_len > MAX_SEP_LEN)
        {
                sep_len = MAX_SEP_LEN;
        }

        size = s * h * sizeof(float);
        buf = malloc(size);
        if (buf == NULL)
        {
                return -1;
        }
        blur = malloc(size);
        if (blur == NULL)
        {
                free(buf);
                return -1;
        }
        /* Create a separable gauss kernel */
        pie_kernel_sep_gauss(kernel, sep_len, param->radius * param->radius);
        /* for (int i = 0; i < sep_len; i++) */
        /*         printf("%d: %f\n", i, kernel[i]); */

        /* Red channel */
        memcpy(blur, r, size);
        pie_kernel_sep_apply(blur,
                             kernel,
                             sep_len,
                             buf,
                             w,
                             h,
                             s);
        pie_unsharp_chan(r,
                         blur,
                         param,
                         w,
                         h,
                         s);
        
        /* Green channel */
        memcpy(blur, g, size);        
        pie_kernel_sep_apply(blur,
                             kernel,
                             sep_len,
                             buf,
                             w,
                             h,
                             s);
        pie_unsharp_chan(g,
                         blur,
                         param,
                         w,
                         h,
                         s);

        /* Blue channel */        
        memcpy(blur, b, size);        
        pie_kernel_sep_apply(blur,
                             kernel,
                             sep_len,
                             buf,
                             w,
                             h,
                             s);
        pie_unsharp_chan(b,
                         blur,
                         param,
                         w,
                         h,
                         s);
        
        free(buf);
        free(blur);
        
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
#ifdef _HAS_ALTIVEC
        vector float amountv = (vector float){param->amount, param->amount, param->amount, param->amount};
        vector float thresholdv = (vector float){threshold, threshold, threshold, threshold};
        vector float onev = (vector float){1.0f, 1.0f, 1.0f, 1.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
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

# elif _HAS_ALTIVEC

                for (int x = 0; x < stop; x += 4)
                {
                        int p = sizeof(float) * (y * s + x);
                        vector float imgv;
                        vector float blurv;
                        vector float maskv;
                        vector float newv;
                        vector int bool cmpv;
                        vector float deltav;

                        imgv = vec_ld(p, img);
                        blurv = vec_ld(p, blur);

                        maskv = vec_sub(imgv, blurv);
                        newv = vec_madd(maskv, amountv, imgv);

                        /* Threshold */
                        deltav = vec_abs(vec_sub(imgv, newv));
                        cmpv = vec_cmpgt(deltav, thresholdv);
                        newv = vec_sel(imgv, newv, cmpv);

                        /* Max 1.0 */
                        cmpv = vec_cmpgt(newv, onev);
                        newv = vec_sel(newv, onev, cmpv);

                        /* Min 0.0 */
                        cmpv = vec_cmplt(newv, zerov);
                        newv = vec_sel(newv, zerov, cmpv);

                        vec_st(newv, p, img);
                }

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
