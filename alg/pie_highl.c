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
#include "pie_highl.h"
#include "../math/pie_catmull.h"
#include "pie_curve.h"

#define CURVE_LEN_M 6
#define CURVE_LEN_P 7

/**
 * Create an highlight adjustment curve. Used internally by pie_alg_highl.
 * @param array of points to store the control points in.
 * @param the desired adjustment level, from -1 to 1.
 * @return number of control points in output curve.
 */
static int pie_alg_highl_curve(struct pie_point_2d*, float);

static struct pie_point_2d hm0[6] =
{
        {.x = -1.0f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.53725f, .y =  0.53725f},
        {.x =  0.8f,     .y =  0.8f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}
};

static struct pie_point_2d hm50[6] =
{
        {.x = -1.0f,     .y = -0.9f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.53725f, .y =  0.50588f},
        {.x =  0.8f,     .y =  0.7f},
        {.x =  1.0f,     .y =  0.98f},
        {.x =  1.0f,     .y =  2.0f}
};

static struct pie_point_2d hm100[6] =
{
        {.x = -1.0f,     .y = -0.8f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.53725f, .y =  0.48235f},
        {.x =  0.8f,     .y =  0.6f},
        {.x =  1.0f,     .y =  0.94f},
        {.x =  1.0f,     .y =  2.0f}
};

static struct pie_point_2d hp0[7] =
{
        {.x = -1.0f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.53725f, .y =  0.53725f},
        {.x =  0.6f,     .y =  0.6f},
        {.x =  0.8f,     .y =  0.8f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  2.0f}
};

static struct pie_point_2d hp50[7] =
{
        {.x = -0.9f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.53725f, .y =  0.56f},
        {.x =  0.6f,     .y =  0.65f},
        {.x =  0.8f,     .y =  0.88f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  1.0f}
};

static struct pie_point_2d hp100[7] =
{
        {.x = -0.8f,     .y = -1.0f},
        {.x =  0.0f,     .y =  0.0f},
        {.x =  0.53725f, .y =  0.584f},
        {.x =  0.6f,     .y =  0.7f},
        {.x =  0.8f,     .y =  0.94f},
        {.x =  1.0f,     .y =  1.0f},
        {.x =  2.0f,     .y =  1.0f}
};

void pie_alg_highl(float* restrict r,
                   float* restrict g,
                   float* restrict b,
                   float e,
                   int w,
                   int h,
                   int stride)
{
        struct pie_point_2d p[CURVE_LEN_P];
        int len;

        /* Create curve */
        len = pie_alg_highl_curve(p, e);
        /* apply curve */
        pie_alg_curve(r, g, b, PIE_CHANNEL_RGB, p, len, w, h, stride);
}

/*
 * Curves for -100, -50, -0, 0, 50, 100 are pre-calculated.
 * Use the provided exposure value to find the two curves that encloses it,
 * and linear interpolate the new curve parameters.
 */
static int pie_alg_highl_curve(struct pie_point_2d* o, float e)
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
                        beg = &hm50[0];
                        end = &hm0[0];

                        phi = (e + 0.5f) * 2.0f;
                }
                else
                {
                        beg = &hm100[0];
                        end = &hm50[0];

                        phi = (e + 1.0f) * 2.0f;
                }
        }
        else
        {
                len = CURVE_LEN_P;
                if (e < 0.5f)
                {
                        beg = &hp0[0];
                        end = &hp50[0];

                        phi = e * 2.0f;
                }
                else
                {
                        beg = &hp50[0];
                        end = &hp100[0];

                        phi = (e - 0.5f) * 2.0f;
                }
        }

        /* Linear interpolation */
        pie_alg_curve_intp(&o[0], beg, end, len, phi);
        return len;
}
