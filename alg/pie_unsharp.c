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

#define MAX_KERNEL_LEN 281
#define USE_BOX_BLUR 1

#if _HAS_AVX
# include <immintrin.h>
# include "../avx_cmp.h"
#elif _HAS_SSE42
# include <nmmintrin.h>
#elif _HAS_ALTIVEC
# include <altivec.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pie_unsharp.h"
#include "../math/pie_kernel.h"
#include "../math/pie_blur.h"

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

/**
 * Helper function that dispatches to the most effective
 * blur operation.
 * @param the channel to blur.
 * @param temporary buffer large enough to hold the channel.
 * @param width of the channel.
 * @param height of the channel.
 * @param row stride of the channel.
 * @param sigma (radius) for the blur.
 * @param kernel to apply.
 * @param size of kernel.
 * @return void.
 */
static void pie_uns_blur_chan(float* restrict,
                              float* restrict,
                              int,
                              int,
                              int,
                              float,
                              float* restrict,
                              int);

int pie_alg_unsharp(float* restrict r,
                    float* restrict g,
                    float* restrict b,
                    const struct pie_unsharp_param* param,
                    int w,
                    int h,
                    int s)
{
        float kernel[MAX_KERNEL_LEN];
        size_t size;
        float* buf;
        float* blur;
        /* Dimension of kernel should be ceil(6*sigma) */
        int kernel_len = (int)((6.0f * param->radius) + 0.5f);

        if ((kernel_len & 0x1) == 0)
        {
                /* Always use an odd number for kernel dimension */
                kernel_len++;
        }

        if (kernel_len > MAX_KERNEL_LEN)
        {
                kernel_len = MAX_KERNEL_LEN;
        }
        if (kernel_len < 5)
        {
                kernel_len = 5;
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
        pie_mth_kernel_sep_gauss(kernel,
                                 kernel_len,
                                 param->radius * param->radius);

        /* Red channel */
        memcpy(blur, r, size);
        pie_uns_blur_chan(blur,
                          buf,
                          w,
                          h,
                          s,
                          param->radius,
                          kernel,
                          kernel_len);
        pie_unsharp_chan(r,
                         blur,
                         param,
                         w,
                         h,
                         s);

        /* Green channel */
        memcpy(blur, g, size);
        pie_uns_blur_chan(blur,
                          buf,
                          w,
                          h,
                          s,
                          param->radius,
                          kernel,
                          kernel_len);
        pie_unsharp_chan(g,
                         blur,
                         param,
                         w,
                         h,
                         s);

        /* Blue channel */
        memcpy(blur, b, size);
        pie_uns_blur_chan(blur,
                          buf,
                          w,
                          h,
                          s,
                          param->radius,
                          kernel,
                          kernel_len);
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

#if _HAS_AVX
        __m256 amountv = _mm256_set1_ps(param->amount);
        __m256 thresholdv = _mm256_set1_ps(threshold);
        __m256 onev = _mm256_set1_ps(1.0f);
        __m256 zerov = _mm256_set1_ps(0.0f);
        __m256 sign_maskv = _mm256_set1_ps(-0.f);
#elif _HAS_SSE42
        __m128 amountv = _mm_set1_ps(param->amount);
        __m128 thresholdv = _mm_set1_ps(threshold);
        __m128 onev = _mm_set1_ps(1.0f);
        __m128 zerov = _mm_set1_ps(0.0f);
        __m128 sign_maskv = _mm_set1_ps(-0.f);
#elif _HAS_ALTIVEC
        vector float amountv = (vector float){param->amount, param->amount, param->amount, param->amount};
        vector float thresholdv = (vector float){threshold, threshold, threshold, threshold};
        vector float onev = (vector float){1.0f, 1.0f, 1.0f, 1.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
#endif
#if _HAS_SIMD8
	int rem = w % 8;
	int stop = w - rem;
#elif _HAS_SIMD4
	int rem = w % 4;
	int stop = w - rem;
#else
        int stop = 0;
#endif
        for (int y = 0; y < h; y++)
        {
#if _HAS_AVX
                for (int x = 0; x < stop; x += 8)
                {
                        int p = y * s + x;
                        __m256 imgv = _mm256_load_ps(img + p);
                        __m256 blurv = _mm256_load_ps(blur + p);
                        __m256 maskv;
                        __m256 newv;
                        __m256 cmpv;
                        __m256 deltav;

                        maskv = _mm256_sub_ps(imgv, blurv);
                        maskv = _mm256_mul_ps(maskv, amountv);
                        newv = _mm256_add_ps(imgv, maskv);

                        /* Threshold stuff */
                        deltav = _mm256_sub_ps(imgv, newv);
                        /* Make sure all values are positive */
                        _mm256_andnot_ps(sign_maskv, deltav);

                        cmpv = _mm256_cmp_ps(deltav, thresholdv, _CMP_NLT_UQ);
                        newv = _mm256_blendv_ps(imgv, newv, cmpv);

                        /* the ones less than zero will be 0xffffffff */
                        cmpv = _mm256_cmp_ps(newv, zerov, _CMP_LT_OQ);
                        /* d = a,b,m. if m == 0 a else b */
                        newv = _mm256_blendv_ps(newv, zerov, cmpv);

                        /* the ones greater than one will be 0xffffffff */
                        cmpv = _mm256_cmp_ps(newv, onev, _CMP_NLT_UQ);
                        newv = _mm256_blendv_ps(newv, onev, cmpv);

                        _mm256_store_ps(img + p, newv);
                }

#elif _HAS_SSE42

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

#elif _HAS_ALTIVEC

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

static void pie_uns_blur_chan(float* restrict chan,
                              float* restrict buf,
                              int w,
                              int h,
                              int s,
                              float sigma,
                              float* restrict kernel,
                              int kernel_len)
{
#if USE_BOX_BLUR
        /* Determine which method to use */
# ifdef __sparc
        /* always use box blur */
        pie_mth_box_blur6(chan, buf, sigma, w, h, s);
# else
        if (sigma < 4.1f)
        {
                /* use kernel convolution */
                pie_mth_kernel_sep_apply(chan,
                                         kernel,
                                         kernel_len,
                                         buf,
                                         w,
                                         h,
                                         s);
        }
        else
        {
                /* Use box blur */
                pie_mth_box_blur6(chan, buf, sigma, w, h, s);
        }
# endif
#else
        pie_mth_kernel_sep_apply(chan,
                                 kernel,
                                 kernel_len,
                                 buf,
                                 w,
                                 h,
                                 s);
#endif
}
