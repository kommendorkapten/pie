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
#include <assert.h>
#include "pie_curve.h"
#include "../math/pie_catmull.h"
#include "../math/pie_point.h"
#include "../pie_log.h"

#define SEGMENT_LEN 75
#define LUT_SIZE  (1 << 16)

void pie_alg_curve(float* restrict r,
                   float* restrict g,
                   float* restrict b,
                   enum pie_channel chan,
                   const struct pie_point_2d* p,
                   int np,
                   int w,
                   int h,
                   int stride)
{
        struct pie_point_2d* c;
        float* restrict lut;
        int num_seg = np - 3;
        const float scale = (LUT_SIZE - 1);

        lut = malloc(LUT_SIZE * sizeof(float));
        c = malloc(LUT_SIZE * sizeof(struct pie_point_2d));

        pie_mth_catm_rom_chain(c, p, np, SEGMENT_LEN);

        for (int i = 0; i < LUT_SIZE; i++)
        {
                float out;
                int ret;

                ret = pie_alg_curve_get(&out,
                                        &c[0],
                                        (float)i / scale,
                                        num_seg * SEGMENT_LEN);
                assert(ret == 0);
                lut[i] = out;
        }

        for (int y = 0; y < h; y++)
        {
#ifdef __powerpc__
                int rem = w % 4;
                int stop = w - rem;
#else
                int stop = 0;
#endif /* __powerpc__ */

                /* Slightly better cache locality if channels are
                   modified independently */
#ifdef  __powerpc__
                if (chan & PIE_CHANNEL_RED)
                {
                        for (int x = 0; x < stop; x += 4)
                        {
                                int p = y * stride + x;

                                r[p]   = lut[(int)(r[p] * scale)];
                                r[p+1] = lut[(int)(r[p+1] * scale)];
                                r[p+2] = lut[(int)(r[p+2] * scale)];
                                r[p+3] = lut[(int)(r[p+3] * scale)];
                        }
                }

                if (chan & PIE_CHANNEL_GREEN)
                {
                        for (int x = 0; x < stop; x += 4)
                        {
                                int p = y * stride + x;

                                g[p]   = lut[(int)(g[p] * scale)];
                                g[p+1] = lut[(int)(g[p+1] * scale)];
                                g[p+2] = lut[(int)(g[p+2] * scale)];
                                g[p+3] = lut[(int)(g[p+3] * scale)];
                        }
                }

                if (chan & PIE_CHANNEL_GREEN)
                {
                        for (int x = 0; x < stop; x += 4)
                        {
                                int p = y * stride + x;

                                b[p]   = lut[(int)(b[p] * scale)];
                                b[p+1] = lut[(int)(b[p+1] * scale)];
                                b[p+2] = lut[(int)(b[p+2] * scale)];
                                b[p+3] = lut[(int)(b[p+3] * scale)];
                        }
                }

#endif /* __powerpc__ */

                if (chan & PIE_CHANNEL_RED)
                {
                        for (int x = stop; x < w; x++)
                        {
                                int p = y * stride + x;

                                r[p] = lut[(int)(r[p] * scale)];
                        }
                }

                if (chan & PIE_CHANNEL_GREEN)
                {
                        for (int x = stop; x < w; x++)
                        {
                                int p = y * stride + x;

                                g[p] = lut[(int)(g[p] * scale)];
                        }
                }

                if (chan & PIE_CHANNEL_BLUE)
                {
                        for (int x = stop; x < w; x++)
                        {
                                int p = y * stride + x;

                                b[p] = lut[(int)(b[p] * scale)];
                        }
                }
        }

        free(c);
        free(lut);
}

int pie_alg_curve_get(float* out,
                      const struct pie_point_2d* p,
                      float x,
                      int len)
{
        float new;
        int l = 0;
        int r = len - 1;
        int f = -1;

        /* Binary search for best match */
        for (;;)
        {
                if (l > r)
                {
                        /* no match */
                        break;
                }

                int pivot = (l + r) / 2;
                float el;

                if (pivot == 0)
                {
                        el = 0.0f;
                }
                else
                {
                        el = p[pivot - 1].x;
                }

                if (el <= x && x <= p[pivot].x)
                {
                        f = pivot;
                        break;
                }

                if (p[pivot].x < x)
                {
                        l = pivot + 1;
                        continue;
                }
                if (p[pivot].x > x)
                {
                        r = pivot - 1;
                        continue;
                }
        }

        if (f < 0)
        {
                if (x >= p[len - 1].x)
                {
                        new = p[len - 1].y;
                        f = len - 1;
                }
                else
                {
                        PIE_WARN("Could not find %f in[%f, %f]", x,
                                 p[0].x, p[len-1].x);
                        new = 0.0f;
                }
        }
        else if (f == 0)
        {
                new = p[0].y;
        }
        else
        {
                float dy = p[f].y - p[f-1].y;
                float dx = p[f].x - p[f-1].x;
                float phi = (x - p[f-1].x) / dx;

                new = p[f-1].y + phi * dy;

                if (new > 1.0f)
                {
                        new = 1.0f;
                }
                if (new < 0.0f)
                {
                        new = 0.0f;
                }
        }

        *out = new;
        if (f < 0)
        {
                return -1;
        }

        return 0;
}


int pie_alg_curve_get_scan(float* out,
                           const struct pie_point_2d* p,
                           float x,
                           int len)
{
        for (int i = 1; i < len; i++)
        {
                if (x <= p[i].x)
                {
                        /* Linear interpolate between i-1 and i. */
                        float dy = p[i].y - p[i-1].y;
                        float dx = p[i].x - p[i-1].x;
                        float phi = (x - p[i-1].x) / dx;
                        float new = p[i-1].y + phi * dy;

                        *out = new;
                        return 0;
                }
        }

        /* check for x ~ 1 */
        if (x > p[len - 1].x)
        {
                *out = p[len - 1].y;
                return 0;
        }

        PIE_WARN("No curve defined for input %f", x);
        PIE_WARN("Valid range are [%f, %f]", p[0].x, p[len - 1].x);

        return 1;
}

void pie_alg_curve_intp(struct pie_point_2d* restrict o,
                        const struct pie_point_2d* restrict b,
                        const struct pie_point_2d* restrict e,
                        int count,
                        float phi)
{
        for (int i = 0; i < count; i++)
        {
                float delta;

                delta = e[i].x - b[i].x;
                o[i].x = b[i].x + phi * delta;

                delta = e[i].y - b[i].y;
                o[i].y = b[i].y + phi * delta;
        }
}
