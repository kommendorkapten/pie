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

#include <assert.h>
#include "pie_curve.h"
#include "pie_expos.h"

#define GAMMA_C   2.33f
#define CURVE_LEN 5

/* Centripetal Catmull-Rom spline parameters for curves matching
   the desired exposure levels */
static struct pie_point_2d em0[CURVE_LEN] =
{
        {.x = -1.0f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.75f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  2.0f}
};

static struct pie_point_2d em1[CURVE_LEN] =
{
        {.x = -1.0f,  .y = -0.4f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.55f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em2[CURVE_LEN] =
{
        {.x = -1.0f,  .y = -0.2f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.35f},
        {.x =  1.0f,  .y =  0.78f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em3[CURVE_LEN] =
{
        {.x = -1.0f,  .y = -0.1f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.22f},
        {.x =  1.0f,  .y =  0.54f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em4[CURVE_LEN] =
{
        {.x = -1.0f,  .y =  0.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.13f},
        {.x =  1.0f,  .y =  0.35f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d em5[CURVE_LEN] =
{
        {.x = -1.0f,  .y =  0.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.75f, .y =  0.074f},
        {.x =  1.0f,  .y =  0.22f},
        {.x =  1.0f,  .y =  2.0f}
};

static struct pie_point_2d ep0[CURVE_LEN] =
{
        {.x = -1.0f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.55f, .y =  0.55f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  2.0f}
};

static struct pie_point_2d ep1[CURVE_LEN] =
{
        {.x = -0.4f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.55f, .y =  0.75f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.2f}
};

static struct pie_point_2d ep2[CURVE_LEN] =
{
        {.x = -0.2f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.48f, .y =  0.82f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.0f}
};

static struct pie_point_2d ep3[CURVE_LEN] =
{
        {.x = -0.2f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.3f,  .y =  0.85f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.0f}
};

static struct pie_point_2d ep4[CURVE_LEN] =
{
        {.x =  0.0f,  .y = -1.0f},
        {.x =  0.0f,  .y =  0.0f},
        {.x =  0.24f, .y =  0.9f},
        {.x =  1.0f,  .y =  1.0f},
        {.x =  2.0f,  .y =  1.0f}
};

static struct pie_point_2d ep5[CURVE_LEN] =
{
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
                   int w,
                   int h,
                   int stride)
{
        struct pie_point_2d p[CURVE_LEN];

        /* Create curve */
        pie_alg_expos_curve(p, e);
        /* apply curve */
        pie_alg_curve(r, g, b, PIE_CHANNEL_LUM, p, CURVE_LEN, w, h, stride);
}

/*
 * Curves for -5, -4, -3, -2, -1, -0, 0, 1, 2, 3, 4, 5 are pre-calculated.
 * Use the provided exposure value to find the two curves that encloses it,
 * and linear interpolate the new curve parameters.
 */
void pie_alg_expos_curve(struct pie_point_2d o[CURVE_LEN], float e)
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

                        phi = e + 1.0f;
                }
                else if (e >= -2.0f)
                {
                        beg = &em2[0];
                        end = &em1[0];

                        phi = e + 2.0f;
                }
                else if (e >= -3.0f)
                {
                        beg = &em3[0];
                        end = &em2[0];

                        phi = e + 3.0f;
                }
                else if (e >= -4.0f)
                {
                        beg = &em4[0];
                        end = &em3[0];

                        phi = e + 4.0f;
                }
                else
                {
                        beg = &em5[0];
                        end = &em4[0];

                        phi = e + 5.0f;
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
        pie_alg_curve_intp(&o[0], beg, end, CURVE_LEN, phi);
}
