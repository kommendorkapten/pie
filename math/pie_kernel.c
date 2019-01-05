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

#include <string.h>
#if _HAS_SSE42
# include <nmmintrin.h> /* sse 4.2 */
#endif
#include "pie_kernel.h"
#include "pie_math.h"

static void pie_mth_kernel_sep_apply_x(float* restrict c,
                                       float* restrict k,
                                       int len,
                                       float* restrict buf,
                                       int w,
                                       int h,
                                       int s);
static void pie_mth_kernel_sep_apply_y(float* restrict c,
                                       float* restrict k,
                                       int len,
                                       float* restrict buf,
                                       int w,
                                       int h,
                                       int s);

void pie_mth_kernel3x3_apply(float* restrict c,
                             struct pie_kernel3x3* k,
                             float* restrict buf,
                             int w,
                             int h,
                             int s)
{
        /* Repeat the edge pixels */

        /* Top and bottom row */
        for (int x = 1; x < w - 1; x++)
        {
                /* y = 0 */
                buf[x] =
                        k->v[0] * c[0 * s + x - 1] +
                        k->v[1] * c[0 * s + x] +
                        k->v[2] * c[0 * s + x + 1] +
                        k->v[3] * c[0 * s + x - 1] +
                        k->v[4] * c[0 * s + x] +
                        k->v[5] * c[0 * s + x + 1] +
                        k->v[6] * c[1 * s + x - 1] +
                        k->v[7] * c[1 * s + x] +
                        k->v[8] * c[1 * s + x + 1];

                /* y = h - 1 */
                buf[(h - 1) * s + x] =
                        k->v[0] * c[(h - 2) * s + x - 1] +
                        k->v[1] * c[(h - 2) * s + x] +
                        k->v[2] * c[(h - 2) * s + x + 1] +
                        k->v[3] * c[(h - 1) * s + x - 1] +
                        k->v[4] * c[(h - 1) * s + x] +
                        k->v[5] * c[(h - 1) * s + x + 1] +
                        k->v[6] * c[(h - 1) * s + x - 1] +
                        k->v[7] * c[(h - 1) * s + x] +
                        k->v[8] * c[(h - 1) * s + x + 1];
        }

        for (int y = 1; y < h - 1; y++)
        {
                /* First column */
                buf[y * s] =
                        k->v[0] * c[(y - 1) * s + 0] +
                        k->v[1] * c[(y - 1) * s + 0] +
                        k->v[2] * c[(y - 1) * s + 1] +
                        k->v[3] * c[y * s + 0] +
                        k->v[4] * c[y * s + 0] +
                        k->v[5] * c[y * s + 1] +
                        k->v[6] * c[(y + 1) * s + 0] +
                        k->v[7] * c[(y + 1) * s + 0] +
                        k->v[8] * c[(y + 1) * s + 1];

                for (int x = 1; x < w - 1; x++)
                {
                        buf[y * s + x] =
                                k->v[0] * c[(y - 1) * s + x - 1] +
                                k->v[1] * c[(y - 1) * s + x] +
                                k->v[2] * c[(y - 1) * s + x + 1] +
                                k->v[3] * c[y * s + x - 1] +
                                k->v[4] * c[y * s + x] +
                                k->v[5] * c[y * s + x + 1] +
                                k->v[6] * c[(y + 1) * s + x - 1] +
                                k->v[7] * c[(y + 1) * s + x] +
                                k->v[8] * c[(y + 1) * s + x + 1];
                }

                /* last column */
                buf[y * s + w - 1] =
                        k->v[0] * c[(y - 1) * s + w - 2] +
                        k->v[1] * c[(y - 1) * s + w - 1] +
                        k->v[2] * c[(y - 1) * s + w - 1] +
                        k->v[3] * c[y * s + w - 2] +
                        k->v[4] * c[y * s + w - 1] +
                        k->v[5] * c[y * s + w - 1] +
                        k->v[6] * c[(y + 1) * s + w - 2] +
                        k->v[7] * c[(y + 1) * s + w - 1] +
                        k->v[8] * c[(y + 1) * s + w - 1];
        }

        /* Four corners */
        buf[0] =
                k->v[0] * c[0] +
                k->v[1] * c[0] +
                k->v[2] * c[1] +
                k->v[3] * c[0] +
                k->v[4] * c[0] +
                k->v[5] * c[1] +
                k->v[6] * c[s] +
                k->v[7] * c[s] +
                k->v[8] * c[s + 1];

        buf[w - 1] =
                k->v[0] * c[w - 2] +
                k->v[1] * c[w - 1] +
                k->v[2] * c[w - 1] +
                k->v[3] * c[w - 2] +
                k->v[4] * c[w - 1] +
                k->v[5] * c[w - 1] +
                k->v[6] * c[s + w - 2] +
                k->v[7] * c[s + w - 1] +
                k->v[8] * c[s + w - 1];

        buf[(h - 1) * s] =
                k->v[0] * c[(h - 2) * s] +
                k->v[1] * c[(h - 2) * s] +
                k->v[2] * c[(h - 2) * s + 1] +
                k->v[3] * c[(h - 1) * s] +
                k->v[4] * c[(h - 1) * s] +
                k->v[5] * c[(h - 1) * s + 1] +
                k->v[6] * c[(h - 1) * s] +
                k->v[7] * c[(h - 1) * s] +
                k->v[8] * c[(h - 1) * s + 1];

        buf[(h - 1) * s + w - 1] =
                k->v[0] * c[(h - 2) * s + w - 2] +
                k->v[1] * c[(h - 2) * s + w - 1] +
                k->v[2] * c[(h - 2) * s + w - 1] +
                k->v[3] * c[(h - 1) * s + w - 2] +
                k->v[4] * c[(h - 1) * s + w - 1] +
                k->v[5] * c[(h - 1) * s + w - 1] +
                k->v[6] * c[(h - 1) * s + w - 2] +
                k->v[7] * c[(h - 1) * s + w - 1] +
                k->v[8] * c[(h - 1) * s + w - 1];

        /* Copy result back into c */
        memcpy(c, buf, h * s * sizeof(float));

        for (int y = 0; y < h; y++)
        {
                for (int x = 0; x < w; x++)
                {
                        int p = y * s + x;

                        if (c[p] > 1.0f)
                        {
                                c[p] = 1.0f;
                        }
                        else if (c[p] < 0.0f)
                        {
                                c[p] = 0.0f;
                        }
                }
        }
}

void pie_mth_kernel5x5_apply(float* restrict c,
                             struct pie_kernel5x5* k,
                             float* restrict buf,
                             int w,
                             int h,
                             int s)
{
        /* Repeat the edge pixels */
        for (int x = 2; x < w - 2; x++)
        {
                /* y = 0 */
                buf[x] =
                        k->v[0] * c[0 * s + x - 2] +
                        k->v[1] * c[0 * s + x - 1] +
                        k->v[2] * c[0 * s + x] +
                        k->v[3] * c[0 * s + x + 1] +
                        k->v[4] * c[0 * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[0 * s + x - 2] +
                        k->v[6] * c[0 * s + x - 1] +
                        k->v[7] * c[0 * s + x] +
                        k->v[8] * c[0 * s + x + 1] +
                        k->v[9] * c[0 * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[0 * s + x - 2] +
                        k->v[11] * c[0 * s + x - 1] +
                        k->v[12] * c[0 * s + x] +
                        k->v[13] * c[0 * s + x + 1] +
                        k->v[14] * c[0 * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[1 * s + x - 2] +
                        k->v[16] * c[1 * s + x - 1] +
                        k->v[17] * c[1 * s + x] +
                        k->v[18] * c[1 * s + x + 1] +
                        k->v[19] * c[1 * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[2 * s + x - 2] +
                        k->v[21] * c[2 * s + x - 1] +
                        k->v[22] * c[2 * s + x] +
                        k->v[23] * c[2 * s + x + 1] +
                        k->v[24] * c[2 * s + x + 2];

                /* y = 1 */
                buf[s + x] =
                        k->v[0] * c[0 * s + x - 2] +
                        k->v[1] * c[0 * s + x - 1] +
                        k->v[2] * c[0 * s + x] +
                        k->v[3] * c[0 * s + x + 1] +
                        k->v[4] * c[0 * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[0 * s + x - 2] +
                        k->v[6] * c[0 * s + x - 1] +
                        k->v[7] * c[0 * s + x] +
                        k->v[8] * c[0 * s + x + 1] +
                        k->v[9] * c[0 * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[1 * s + x - 2] +
                        k->v[11] * c[1 * s + x - 1] +
                        k->v[12] * c[1 * s + x] +
                        k->v[13] * c[1 * s + x + 1] +
                        k->v[14] * c[1 * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[2 * s + x - 2] +
                        k->v[16] * c[2 * s + x - 1] +
                        k->v[17] * c[2 * s + x] +
                        k->v[18] * c[2 * s + x + 1] +
                        k->v[19] * c[2 * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[3 * s + x - 2] +
                        k->v[21] * c[3 * s + x - 1] +
                        k->v[22] * c[3 * s + x] +
                        k->v[23] * c[3 * s + x + 1] +
                        k->v[24] * c[3 * s + x + 2];

                /* y = h - 2 */
                buf[(h - 2) * s + x] =
                        k->v[0] * c[(h - 4) * s + x - 2] +
                        k->v[1] * c[(h - 4) * s + x - 1] +
                        k->v[2] * c[(h - 4) * s + x] +
                        k->v[3] * c[(h - 4) * s + x + 1] +
                        k->v[4] * c[(h - 4) * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[(h - 3) * s + x - 2] +
                        k->v[6] * c[(h - 3) * s + x - 1] +
                        k->v[7] * c[(h - 3) * s + x] +
                        k->v[8] * c[(h - 3) * s + x + 1] +
                        k->v[9] * c[(h - 3) * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[(h - 2) * s + x - 2] +
                        k->v[11] * c[(h - 2) * s + x - 1] +
                        k->v[12] * c[(h - 2) * s + x] +
                        k->v[13] * c[(h - 2) * s + x + 1] +
                        k->v[14] * c[(h - 2) * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[(h - 1) * s + x - 2] +
                        k->v[16] * c[(h - 1) * s + x - 1] +
                        k->v[17] * c[(h - 1) * s + x] +
                        k->v[18] * c[(h - 1) * s + x + 1] +
                        k->v[19] * c[(h - 1) * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[(h - 1) * s + x - 2] +
                        k->v[21] * c[(h - 1) * s + x - 1] +
                        k->v[22] * c[(h - 1) * s + x] +
                        k->v[23] * c[(h - 1) * s + x + 1] +
                        k->v[24] * c[(h - 1) * s + x + 2];

                /* y = h - 1 */
                buf[(h - 1) * s + x] =
                        k->v[0] * c[(h - 3) * s + x - 2] +
                        k->v[1] * c[(h - 3) * s + x - 1] +
                        k->v[2] * c[(h - 3) * s + x] +
                        k->v[3] * c[(h - 3) * s + x + 1] +
                        k->v[4] * c[(h - 3) * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[(h - 2) * s + x - 2] +
                        k->v[6] * c[(h - 2) * s + x - 1] +
                        k->v[7] * c[(h - 2) * s + x] +
                        k->v[8] * c[(h - 2) * s + x + 1] +
                        k->v[9] * c[(h - 2) * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[(h - 1) * s + x - 2] +
                        k->v[11] * c[(h - 1) * s + x - 1] +
                        k->v[12] * c[(h - 1) * s + x] +
                        k->v[13] * c[(h - 1) * s + x + 1] +
                        k->v[14] * c[(h - 1) * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[(h - 1) * s + x - 2] +
                        k->v[16] * c[(h - 1) * s + x - 1] +
                        k->v[17] * c[(h - 1) * s + x] +
                        k->v[18] * c[(h - 1) * s + x + 1] +
                        k->v[19] * c[(h - 1) * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[(h - 1) * s + x - 2] +
                        k->v[21] * c[(h - 1) * s + x - 1] +
                        k->v[22] * c[(h - 1) * s + x] +
                        k->v[23] * c[(h - 1) * s + x + 1] +
                        k->v[24] * c[(h - 1) * s + x + 2];
        }

        for (int y = 2; y < h - 2; y++)
        {
                /* First column, x = 0 */
                buf[y * s] =
                        k->v[0] * c[(y - 2) * s + 0] +
                        k->v[1] * c[(y - 2) * s + 0] +
                        k->v[2] * c[(y - 2) * s + 0] +
                        k->v[3] * c[(y - 2) * s + 1] +
                        k->v[4] * c[(y - 2) * s + 2] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + 0] +
                        k->v[6] * c[(y - 1) * s + 0] +
                        k->v[7] * c[(y - 1) * s + 0] +
                        k->v[8] * c[(y - 1) * s + 1] +
                        k->v[9] * c[(y - 1) * s + 2] +
                        /* 2 */
                        k->v[10] * c[y * s + 0] +
                        k->v[11] * c[y * s + 0] +
                        k->v[12] * c[y * s + 0] +
                        k->v[13] * c[y * s + 1] +
                        k->v[14] * c[y * s + 2] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + 0] +
                        k->v[16] * c[(y + 1) * s + 0] +
                        k->v[17] * c[(y + 1) * s + 0] +
                        k->v[18] * c[(y + 1) * s + 1] +
                        k->v[19] * c[(y + 1) * s + 2] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + 0] +
                        k->v[21] * c[(y + 2) * s + 0] +
                        k->v[22] * c[(y + 2) * s + 0] +
                        k->v[23] * c[(y + 2) * s + 1] +
                        k->v[24] * c[(y + 2) * s + 2];

                /* Second column, x = 1 */
                buf[y * s + 1] =
                        k->v[0] * c[(y - 2) * s + 0] +
                        k->v[1] * c[(y - 2) * s + 0] +
                        k->v[2] * c[(y - 2) * s + 1] +
                        k->v[3] * c[(y - 2) * s + 2] +
                        k->v[4] * c[(y - 2) * s + 3] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + 0] +
                        k->v[6] * c[(y - 1) * s + 0] +
                        k->v[7] * c[(y - 1) * s + 1] +
                        k->v[8] * c[(y - 1) * s + 2] +
                        k->v[9] * c[(y - 1) * s + 3] +
                        /* 2 */
                        k->v[10] * c[y * s + 0] +
                        k->v[11] * c[y * s + 0] +
                        k->v[12] * c[y * s + 1] +
                        k->v[13] * c[y * s + 2] +
                        k->v[14] * c[y * s + 3] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + 0] +
                        k->v[16] * c[(y + 1) * s + 0] +
                        k->v[17] * c[(y + 1) * s + 1] +
                        k->v[18] * c[(y + 1) * s + 2] +
                        k->v[19] * c[(y + 1) * s + 3] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + 0] +
                        k->v[21] * c[(y + 2) * s + 0] +
                        k->v[22] * c[(y + 2) * s + 1] +
                        k->v[23] * c[(y + 2) * s + 2] +
                        k->v[24] * c[(y + 2) * s + 3];

                for (int x = 2; x < w - 2; x++)
                {
/* SSE Does not appear to be any faster */
#if _HAS_SSE42
                        __m128 v1 = _mm_load_ps(&k->v[0]);
                        __m128 v2 = _mm_load_ps(&k->v[4]);
                        __m128 v3 = _mm_load_ps(&k->v[8]);
                        __m128 v4 = _mm_load_ps(&k->v[12]);
                        __m128 v5 = _mm_load_ps(&k->v[16]);
                        __m128 v6 = _mm_load_ps(&k->v[20]);
                        __m128 c1;
                        __m128 c2;
                        __m128 c3;
                        __m128 c4;
                        __m128 c5;
                        __m128 c6;
#ifdef _USE_MEMCPY
                        float f[24];
#endif
                        float f1[4];
                        float f2[4];
                        float f3[4];
                        float f4[4];
                        float f5[4];
                        float f6[4];

#ifdef _USE_MEMCPY
                        memcpy(f, &c[(y - 2) * s + x - 2],
                               5 * sizeof(float));
                        memcpy(f + 5, &c[(y - 1) * s + x - 2],
                               5 * sizeof(float));
                        memcpy(f + 10, &c[y * s + x - 2],
                               5 * sizeof(float));
                        memcpy(f + 15, &c[(y + 1) * s + x - 2],
                               5 * sizeof(float));
                        memcpy(f + 20, &c[(y + 2) * s + x - 2],
                               4 * sizeof(float));
                        memcpy(f1, f, 4 * sizeof(float));
                        memcpy(f2, f + 4, 4 * sizeof(float));
                        memcpy(f3, f + 8, 4 * sizeof(float));
                        memcpy(f4, f + 12, 4 * sizeof(float));
                        memcpy(f5, f + 16, 4 * sizeof(float));
                        memcpy(f6, f + 20, 4 * sizeof(float));

#else

                        f1[0] = c[(y - 2) * s + x - 2];
                        f1[1] = c[(y - 2) * s + x - 1];
                        f1[2] = c[(y - 2) * s + x];
                        f1[3] = c[(y - 2) * s + x + 1];
                        f2[0] = c[(y - 2) * s + x + 2];
                        f2[1] = c[(y - 1) * s + x - 2];
                        f2[2] = c[(y - 1) * s + x - 1];
                        f2[3] = c[(y - 1) * s + x];
                        f3[0] = c[(y - 1) * s + x + 1];
                        f3[1] = c[(y - 1) * s + x + 2];
                        f3[2] = c[y * s + x - 2];
                        f3[3] = c[y * s + x - 1];
                        f4[0] = c[y * s + x];
                        f4[1] = c[y * s + x + 1];
                        f4[2] = c[y * s + x + 2];
                        f4[3] = c[(y + 1) * s + x - 2];
                        f5[0] = c[(y + 1) * s + x - 1];
                        f5[1] = c[(y + 1) * s + x];
                        f5[2] = c[(y + 1) * s + x + 1];
                        f5[3] = c[(y + 1) * s + x + 2];
                        f6[0] = c[(y + 2) * s + x - 2];
                        f6[1] = c[(y + 2) * s + x - 1];
                        f6[2] = c[(y + 2) * s + x];
                        f6[3] = c[(y + 2) * s + x + 1];

#endif

                        c1 = _mm_load_ps(f1);
                        c2 = _mm_load_ps(f2);
                        c3 = _mm_load_ps(f3);
                        c4 = _mm_load_ps(f4);
                        c5 = _mm_load_ps(f5);
                        c6 = _mm_load_ps(f6);

                        /* Store result in least significant addr */
                        v1 = _mm_dp_ps(v1, c1, 0xf1);
                        v2 = _mm_dp_ps(v2, c2, 0xf1);
                        v3 = _mm_dp_ps(v3, c3, 0xf1);
                        v4 = _mm_dp_ps(v4, c4, 0xf1);
                        v5 = _mm_dp_ps(v5, c5, 0xf1);
                        v6 = _mm_dp_ps(v6, c6, 0xf1);

                        c1 = _mm_add_ps(v1, v2);
                        c2 = _mm_add_ps(v3, v4);
                        c3 = _mm_add_ps(v5, v6);
                        c4 = _mm_add_ps(c1, c2);
                        c5 = _mm_add_ps(c3, c4);

                        _mm_store_ps(f1, c5);

                        /* add k->v[24] */
                        buf[y * s + x] =
                                f1[0] + k->v[24] * c[(y + 2) * s + x + 2];
#elif _HAS_ALTIVEC
                        /* TODO implement me */
#else
                        buf[y * s + x] =
                                k->v[0] * c[(y - 2) * s + x - 2] +
                                k->v[1] * c[(y - 2) * s + x - 1] +
                                k->v[2] * c[(y - 2) * s + x] +
                                k->v[3] * c[(y - 2) * s + x + 1] +
                                k->v[4] * c[(y - 2) * s + x + 2] +
                                /* 1 */
                                k->v[5] * c[(y - 1) * s + x - 2] +
                                k->v[6] * c[(y - 1) * s + x - 1] +
                                k->v[7] * c[(y - 1) * s + x] +
                                k->v[8] * c[(y - 1) * s + x + 1] +
                                k->v[9] * c[(y - 1) * s + x + 2] +
                                /* 2 */
                                k->v[10] * c[y * s + x - 2] +
                                k->v[11] * c[y * s + x - 1] +
                                k->v[12] * c[y * s + x] +
                                k->v[13] * c[y * s + x + 1] +
                                k->v[14] * c[y * s + x + 2] +
                                /* 3 */
                                k->v[15] * c[(y + 1) * s + x - 2] +
                                k->v[16] * c[(y + 1) * s + x - 1] +
                                k->v[17] * c[(y + 1) * s + x] +
                                k->v[18] * c[(y + 1) * s + x + 1] +
                                k->v[19] * c[(y + 1) * s + x + 2] +
                                /* 4 */
                                k->v[20] * c[(y + 2) * s + x - 2] +
                                k->v[21] * c[(y + 2) * s + x - 1] +
                                k->v[22] * c[(y + 2) * s + x] +
                                k->v[23] * c[(y + 2) * s + x + 1] +
                                k->v[24] * c[(y + 2) * s + x + 2];
#endif
                }

                /* second last column, x = w - 2 */
                buf[y * s + w - 2] =
                        k->v[0] * c[(y - 2) * s + w - 4] +
                        k->v[1] * c[(y - 2) * s + w - 3] +
                        k->v[2] * c[(y - 2) * s + w - 2] +
                        k->v[3] * c[(y - 2) * s + w - 1] +
                        k->v[4] * c[(y - 2) * s + w - 1] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + w - 4] +
                        k->v[6] * c[(y - 1) * s + w - 3] +
                        k->v[7] * c[(y - 1) * s + w - 2] +
                        k->v[8] * c[(y - 1) * s + w - 1] +
                        k->v[9] * c[(y - 1) * s + w - 1] +
                        /* 2 */
                        k->v[10] * c[y * s + w - 4] +
                        k->v[11] * c[y * s + w - 3] +
                        k->v[12] * c[y * s + w - 2] +
                        k->v[13] * c[y * s + w - 1] +
                        k->v[14] * c[y * s + w - 1] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + w - 4] +
                        k->v[16] * c[(y + 1) * s + w - 3] +
                        k->v[17] * c[(y + 1) * s + w - 2] +
                        k->v[18] * c[(y + 1) * s + w - 1] +
                        k->v[19] * c[(y + 1) * s + w - 1] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + w - 4] +
                        k->v[21] * c[(y + 2) * s + w - 3] +
                        k->v[22] * c[(y + 2) * s + w - 2] +
                        k->v[23] * c[(y + 2) * s + w - 1] +
                        k->v[24] * c[(y + 2) * s + w - 1];

                /* last column, x = w - 1 */
                buf[y * s + w - 1] =
                        k->v[0] * c[(y - 2) * s + w - 3] +
                        k->v[1] * c[(y - 2) * s + w - 2] +
                        k->v[2] * c[(y - 2) * s + w - 1] +
                        k->v[3] * c[(y - 2) * s + w - 1] +
                        k->v[4] * c[(y - 2) * s + w - 1] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + w - 3] +
                        k->v[6] * c[(y - 1) * s + w - 2] +
                        k->v[7] * c[(y - 1) * s + w - 1] +
                        k->v[8] * c[(y - 1) * s + w - 1] +
                        k->v[9] * c[(y - 1) * s + w - 1] +
                        /* 2 */
                        k->v[10] * c[y * s + w - 3] +
                        k->v[11] * c[y * s + w - 2] +
                        k->v[12] * c[y * s + w - 1] +
                        k->v[13] * c[y * s + w - 1] +
                        k->v[14] * c[y * s + w - 1] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + w - 3] +
                        k->v[16] * c[(y + 1) * s + w - 2] +
                        k->v[17] * c[(y + 1) * s + w - 1] +
                        k->v[18] * c[(y + 1) * s + w - 1] +
                        k->v[19] * c[(y + 1) * s + w - 1] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + w - 3] +
                        k->v[21] * c[(y + 2) * s + w - 2] +
                        k->v[22] * c[(y + 2) * s + w - 1] +
                        k->v[23] * c[(y + 2) * s + w - 1] +
                        k->v[24] * c[(y + 2) * s + w - 1];
        }

        /* "Corners", use the naive algoritm here */
        int ay[4] = {0, 1, h - 2, h - 1};
        int ax[4] = {0, 1, w - 2, w - 1};
        for (int iy = 0; iy < 4; iy++)
        {
                int y = ay[iy];

                for (int ix = 0; ix < 4; ix++)
                {
                        int x = ax[ix];

                        buf[y * s + x] = 0;

                        for (int ky = 0; ky < 5; ky++)
                        {
                                int py = (int)(y + ky - 2);

                                if (py < 0)
                                {
                                        py = 0;
                                }
                                else if (py > h - 1)
                                {
                                        py = h - 1;
                                }

                                for (int kx = 0; kx < 5; kx++)
                                {
                                        int px = x + kx - 2;

                                        if (px < 0)
                                        {
                                                px = 0;
                                        }
                                        else if (px > w - 1)
                                        {
                                                px = w - 1;
                                        }

                                        buf[y * s + x] +=
                                                c[py * s + px] *
                                                k->v[ky * 5 + kx];
                                }
                        }
                }
        }

        /* Copy result back into c */
        memcpy(c, buf, h * s * sizeof(float));
}

#if 0
/* Generic kernel a x a */
void pie_mth_kernel5x5_apply(float* restrict c,
                             struct pie_kernel5x5* k,
                             float* restrict buf,
                             int w,
                             int h,
                             int s)
{
        for (int y = 0; y < h; y++)
        {
                for (int x = 0; x < w; x++)
                {
                        buf[y * s + x] = 0;

                        for (int ky = 0; ky < 5; ky++)
                        {
                                int py = y + ky - 2;

                                if (py < 0)
                                {
                                        py = 0;
                                }
                                else if (py > h - 1)
                                {
                                        py = h - 1;
                                }

                                for (int kx = 0; kx < 5; kx++)
                                {
                                        int px = x + kx - 2;

                                        if (px < 0)
                                        {
                                                px = 0;
                                        }
                                        else if (px > w - 1)
                                        {
                                                px = w - 1;
                                        }

                                        buf[y * s + x] +=
                                                c[py * s + px] *
                                                k->v[ky * 5 + kx];
                                }
                        }
                }
        }
        /* Copy result back into c */
        memcpy(c, buf, h * s * sizeof(float));
}
#endif

void pie_mth_kernel_sep_apply(float* restrict c,
                              float* restrict k,
                              int len,
                              float* restrict buf,
                              int w,
                              int h,
                              int s)
{
        pie_mth_kernel_sep_apply_x(c, k, len, buf, w, h, s);
        pie_mth_kernel_sep_apply_y(buf, k, len, c, w, h, s);
}

static void pie_mth_kernel_sep_apply_x(float* restrict c,
                                       float* restrict k,
                                       int len,
                                       float* restrict buf,
                                       int w,
                                       int h,
                                       int s)
{
        int half = len >> 1;
        int rem = h % 4;
        int ystop = h - rem;

        for (int y = 0; y < ystop; y+=4)
        {
                int row1 = y * s;
                int row2 = (y+1) * s;
                int row3 = (y+2) * s;
                int row4 = (y+3) * s;

                for (int x = 0; x < w; x++)
                {
                        int start = x - half;
                        int stop = x + half + 1;
                        int count = stop > w ? w - start : stop - start;
                        float sum1 = 0.0f;
                        float sum2 = 0.0f;
                        float sum3 = 0.0f;
                        float sum4 = 0.0f;

                        start = start < 0 ? 0 : start;

                        for (int i = 0; i < count; i++)
                        {
                                sum1 += c[row1 + start + i] * k[i];
                                sum2 += c[row2 + start + i] * k[i];
                                sum3 += c[row3 + start + i] * k[i];
                                sum4 += c[row4 + start + i] * k[i];
                        }

                        buf[row1 + x] = sum1;
                        buf[row2 + x] = sum2;
                        buf[row3 + x] = sum3;
                        buf[row4 + x] = sum4;
                }
        }

        for (int y = ystop; y < h; y++)
        {
                int row1 = y * s;

                for (int x = 0; x < w; x++)
                {
                        int start = x - half;
                        int stop = x + half + 1;
                        int count = stop > w ? w - start : stop - start;
                        float sum1 = 0.0f;

                        start = start < 0 ? 0 : start;

                        for (int i = 0; i < count; i++)
                        {
                                sum1 += c[row1 + start + i] * k[i];
                        }

                        buf[row1 + x] = sum1;
                }
        }
}

static void pie_mth_kernel_sep_apply_y(float* restrict c,
                                       float* restrict k,
                                       int len,
                                       float* restrict buf,
                                       int w,
                                       int h,
                                       int s)
{
        int half = len >> 1;
        int rem = w % 4;
        int xstop = w - rem;

        for (int x = 0; x < w; x+=4)
        {
                for (int y = 0; y < h; y++)
                {
                        int start = y - half;
                        int stop = y + half + 1;
                        int count = stop > h ? h - start : stop - start;
                        float sum1 = 0.0f;
                        float sum2 = 0.0f;
                        float sum3 = 0.0f;
                        float sum4 = 0.0f;

                        start = start < 0 ? 0 : start;

                        for (int i = 0; i < count; i++)
                        {
                                sum1 += c[(start + i) * s + x] * k[i];
                                sum2 += c[(start + i) * s + x + 1] * k[i];
                                sum3 += c[(start + i) * s + x + 2] * k[i];
                                sum4 += c[(start + i) * s + x + 3] * k[i];
                        }

                        buf[y * s + x] = sum1;
                        buf[y * s + x + 1] = sum2;
                        buf[y * s + x + 2] = sum3;
                        buf[y * s + x + 3] = sum4;
                }
        }

        for (int x = xstop; x < w; x++)
        {
                for (int y = 0; y < h; y++)
                {
                        int start = y - half;
                        int stop = y + half + 1;
                        int count = stop > h ? h - start : stop - start;
                        float sum1 = 0.0f;

                        start = start < 0 ? 0 : start;

                        for (int i = 0; i < count; i++)
                        {
                                sum1 += buf[(start + i) *  s + x] * k[i];
                        }

                        c[y * s + x] = sum1;
                }
        }
}

void pie_mth_kernel3x3_gauss(struct pie_kernel3x3* k,
                             float var)
{
        pie_mth_gauss_matrix(&k->v[0], 3, var);
}

void pie_mth_kernel5x5_gauss(struct pie_kernel5x5* k,
                             float var)
{
        pie_mth_gauss_matrix(&k->v[0], 5, var);
}

void pie_mth_kernel_sep_gauss(float* r,
                              int len,
                              float var)
{
        float sum = 0;
        int h = len >> 1;

        for (int i = 0; i < len; i++)
        {
                float g = pie_mth_gauss((float)(i - h), var);

                r[i] = g;
                sum += g;
        }

        /* Normalize */
        for (int i = 0; i < len; i++)
        {
                r[i] /= sum;
        }
}
