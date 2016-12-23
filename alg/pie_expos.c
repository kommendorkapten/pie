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

#include <math.h>
#include <assert.h>
#ifdef _HAS_SSE
# include <nmmintrin.h> /* sse 4.2 */
#endif
#include "../math/pie_catmull.h"
#include "pie_curve.h"
#include "pie_expos.h"

#define GAMMA_C   2.33f     
#define SEGMENT_LEN 150
/* No use to have LUT larger than the curve, 2 * SEGMENT LEN */
#define LUT_SIZE  300

/* Centripetal Catmull-Rom spline parameters for curves matching 
   the desired exposure levels */
static struct pie_point_2d em0[5] = {
        {.x = -1.0f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.75f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  2.0f}
};

static struct pie_point_2d em1[5] = {
        {.x = -1.0f,  .y = -0.4f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.55f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em2[5] = {
        {.x = -1.0f,  .y = -0.2f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.35f},
        {.x =  1.0f,  .y =  0.78f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em3[5] = {
        {.x = -1.0f,  .y = -0.1f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.22f},
        {.x =  1.0f,  .y =  0.54f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em4[5] = {
        {.x = -1.0f,  .y =  0.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.13f},
        {.x =  1.0f,  .y =  0.35f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em5[5] = {
        {.x = -1.0f,  .y =  0.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.074f},
        {.x =  1.0f,  .y =  0.22f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d ep0[5] = {
        {.x = -1.0f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.55f, .y =  0.55f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  2.0f}
};

static struct pie_point_2d ep1[5] = {
        {.x = -0.4f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.55f, .y =  0.75f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.2f}
};

static struct pie_point_2d ep2[5] = {
        {.x = -0.2f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.48f, .y =  0.82f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.0f}
};

static struct pie_point_2d ep3[5] = {
        {.x = -0.2f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.3f,  .y =  0.85f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.0f}
};

static struct pie_point_2d ep4[5] = {
        {.x =  0.0f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.24f, .y =  0.9f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.0f}
};

static struct pie_point_2d ep5[5] = {
        {.x =  0.0f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.18f, .y =  0.9f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.0f}
};

/*
 * Creates a look up table for the chosen exposure value.
 * For each color channel, apply the curve function.
 */
void pie_alg_expos(float* restrict r,
                   float* restrict g,
                   float* restrict b,
                   float e,
                   int width,
                   int height,
                   int stride)
{
        struct pie_point_2d p[5];
        struct pie_point_2d c[2 * SEGMENT_LEN];
        float lut[LUT_SIZE];
        const float scale = LUT_SIZE - 1.0f;        
#ifdef _HAS_SSE
        __m128 scalev = _mm_set1_ps(scale);
#endif

        /* Create LUT */
        pie_alg_expos_curve(p, e);
        pie_catm_rom_chain(c, p, 5, SEGMENT_LEN);

        for (int i = 0; i < LUT_SIZE; i++)
        {
                float out;

                pie_alg_curve_get(&out, &c[0], i / scale, 2 * SEGMENT_LEN);
                lut[i] = out;
        }
        
#ifdef _HAS_SIMD
# ifdef _HAS_SSE
        int rem = width % 4;
        int stop = width - rem;

        /* For smaller image on newer hardware, SSE version seems to take
           more time, most likely due to the extra copying, and the calculation
           is very simple, only a single multiplication */
#if 1
        stop = 0;
#endif
        
        for (int y = 0; y < height; y++)
        {
                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * stride + x;
                        __m128 redv = _mm_load_ps(r + p);
                        __m128 greenv = _mm_load_ps(g + p);
                        __m128 bluev = _mm_load_ps(b + p);
                        float red[4];
                        float green[4];
                        float blue[4];

                        redv = _mm_mul_ps(redv, scalev);
                        greenv = _mm_mul_ps(greenv, scalev);
                        bluev = _mm_mul_ps(bluev, scalev);

                        _mm_store_ps(red, redv);
                        _mm_store_ps(green, greenv);
                        _mm_store_ps(blue, bluev);

                        r[p]     = lut[(int)red[0]];
                        r[p + 1] = lut[(int)red[1]];
                        r[p + 2] = lut[(int)red[2]];
                        r[p + 3] = lut[(int)red[3]];                        

                        g[p]     = lut[(int)green[0]];
                        g[p + 1] = lut[(int)green[1]];
                        g[p + 2] = lut[(int)green[2]];
                        g[p + 3] = lut[(int)green[3]];

                        b[p]     = lut[(int)blue[0]];
                        b[p + 1] = lut[(int)blue[1]];
                        b[p + 2] = lut[(int)blue[2]];
                        b[p + 3] = lut[(int)blue[3]];
                }

                for (int x = stop; x < width; x++)
                {
                        int p = y * stride + x;
                        
                        r[p] = lut[(int)(r[p] * scale)];
                        g[p] = lut[(int)(g[p] * scale)];
                        b[p] = lut[(int)(b[p] * scale)];
                }
        }
# elif _HAS_ALTIVEC
#  error ALTIVET NOT IMPLEMENTED
# endif
#else
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
#endif /* _HAS_SIMD */
}

/*
 * Curves for -5, -4, -3, -2, -1, -0, 0, 1, 2, 3, 4, 5 are pre-calculated.
 * Use the provided exposure value to find the two curves that encloses it,
 * and linear interpolate the new curve parameters.
 */
void pie_alg_expos_curve(struct pie_point_2d o[5], float e)
{
        struct pie_point_2d* beg;
        struct pie_point_2d* end;
        float phi; /* percentage of position from beg to end */
        assert(e <= 5.0f);
        assert(e >= -5.0f);

        if (e < 0.0f)
        {
                if (e >= -1.0f)
                {
                        beg = &em1[0];
                        end = &em0[0];

                        phi = e - (-1.0f);
                }
                else if (e >= -2.0f)
                {
                        beg = &em2[0];
                        end = &em1[0];

                        phi = e - (-2.0f);                        
                }
                else if (e >= -3.0f)
                {
                        beg = &em3[0];
                        end = &em2[0];

                        phi = e - (-3.0f);                        
                }
                else if (e >= -4.0f)
                {
                        beg = &em4[0];
                        end = &em3[0];

                        phi = e - (-4.0f);
                }
                else
                {
                        beg = &em5[0];
                        end = &em4[0];

                        phi = e - (-5.0f);
                }                
        }
        else
        {
                if (e < 1.0f)
                {
                        beg = &ep0[0];
                        end = &ep1[0];

                        phi = e - 0.0f;
                }
                else if (e < 2.0f)
                {
                        beg = &ep1[0];
                        end = &ep2[0];

                        phi = e - 1.0f;
                }
                else if (e < 3.0f)
                {
                        beg = &ep2[0];
                        end = &ep3[0];

                        phi = e - 2.0f;
                }
                else if (e < 4.0f)
                {
                        beg = &ep3[0];
                        end = &ep4[0];

                        phi = e - 3.0f;
                }
                else
                {
                        beg = &ep4[0];
                        end = &ep5[0];

                        phi = e - 4.0f;
                }                                
        }

        /* Linear interpolation */
        for (int i = 0; i < 5; i++)
        {
                float delta;

                delta = end[i].x - beg[i].x;
                o[i].x = beg[i].x + phi * delta;
                
                delta = end[i].y - beg[i].y;
                o[i].y = beg[i].y + phi * delta;
        }
}
