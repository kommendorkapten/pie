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

#include "../math/pie_catmull.h"
#include "pie_curve.h"
#include <assert.h>

#define SEGMENT_LEN 150
#define CURVE_LEN_M 6
#define CURVE_LEN_P 8
/* No use to have LUT larger than the curve, 5 * SEGMENT LEN */
#define LUT_SIZE  (5 * SEGMENT_LEN)

/**
 * Create an shadow adjustment curve. Used internally by pie_alg_shado.
 * @param array of points to store the control points in.
 * @param the desired adjustment level, from -1 to 1.
 * @return number of control points in output curve.
 */
static int pie_alg_shado_curve(struct pie_point_2d*, float);

static struct pie_point_2d sm0[6] =
{
        {.x = -1.0f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.23f,    .y =  0.23f},
        {.x =  0.53725f, .y =  0.53725f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}
};

static struct pie_point_2d sm50[6] =
{
        {.x = -1.0f,     .y =  0.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.23f,    .y =  0.15f},
        {.x =  0.53725f, .y =  0.50588f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}
};

static struct pie_point_2d sm100[6] =
{
        {.x = -1.0f,     .y =  0.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.23f,    .y =  0.1f},
        {.x =  0.53725f, .y =  0.488f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}        
};

static struct pie_point_2d sp0[8] =
{
        {.x = -1.0f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.13f,    .y =  0.13f},
        {.x =  0.4f,     .y =  0.4f},
        {.x =  0.53725f, .y =  0.53725f},
        {.x =  0.6f,     .y =  0.6f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}        
};

static struct pie_point_2d sp50[8] =
{
        {.x =  0.0f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.13f,    .y =  0.23f},
        {.x =  0.4f,     .y =  0.47f},
        {.x =  0.53725f, .y =  0.55686f},
        {.x =  0.6f,     .y =  0.62f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}        
};

static struct pie_point_2d sp100[8] =
{
        {.x =  0.0f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.13f,    .y =  0.38f},
        {.x =  0.4f,     .y =  0.53f},
        {.x =  0.53725f, .y =  0.581f},
        {.x =  0.6f,     .y =  0.64f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}        
};

void pie_alg_shado(float* restrict r,
                   float* restrict g,
                   float* restrict b,
                   float e,
                   int width,
                   int height,
                   int stride)
{
        struct pie_point_2d p[CURVE_LEN_P];
        struct pie_point_2d c[LUT_SIZE];
        float lut[LUT_SIZE];
        const float scale = LUT_SIZE - 1.0f;
        int len;

        /* Create curve */
        len = pie_alg_shado_curve(p, e);
        pie_catm_rom_chain(c, p, len, SEGMENT_LEN);

        for (int i = 0; i < LUT_SIZE; i++)
        {
                float out;
                int ret;

                ret = pie_alg_curve_get(&out,
                                        &c[0],
                                        i / scale,
                                        LUT_SIZE);
                assert(ret == 0);
                lut[i] = out;
        }
        
        for (int y = 0; y < height; y++)
        {
                for (int x = 0; x < width; x++)
                {
                        int p = y * stride + x;
                        
                        r[p] = lut[(int)(r[p] * scale)];
                        g[p] = lut[(int)(g[p] * scale)];
                        b[p] = lut[(int)(b[p] * scale)];
                }
        }
}

/*
 * Curves for -100, -50, -0, 0, 50, 100 are pre-calculated.
 * Use the provided exposure value to find the two curves that encloses it,
 * and linear interpolate the new curve parameters.
 */
static int pie_alg_shado_curve(struct pie_point_2d* o, float e)
{
        struct pie_point_2d* beg;
        struct pie_point_2d* end;
        int len;
        float phi; /* percentage of position from beg to end */

        assert(e <= 1.0f);
        assert(e >= -1.0f);

        if (e < 0.0f)
        {
                len = CURVE_LEN_M;
                if (e >= -0.5f)
                {
                        beg = &sm50[0];
                        end = &sm0[0];

                        phi = (e + 0.5f) * 2.0f;
                }
                else
                {
                        beg = &sm100[0];
                        end = &sm50[0];

                        phi = (e + 1.0f) * 2.0f;
                }                
        }
        else
        {
                len = CURVE_LEN_P;
                if (e < 0.5f)
                {
                        beg = &sp0[0];
                        end = &sp50[0];

                        phi = e * 2.0f;
                }
                else
                {
                        beg = &sp50[0];
                        end = &sp100[0];

                        phi = (e - 0.5f) * 2.0f;
                }                                
        }

        /* Linear interpolation */
        pie_alg_curve_intp(&o[0], beg, end, len, phi);
        return len;
}
