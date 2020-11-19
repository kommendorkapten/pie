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
#include "pie_bm_cspace.h"

/* sRGB gamma exponent */
#define SRGB_G_EXP 2.4f
#define APPROX 1

float pie_bm_gamma(float b, float g)
{
        return powf(b, g);
}

void pie_bm_gammav(float* b, float g, size_t size)
{
        for (size_t i = 0; i < size; i++)
        {
                b[i] = pie_bm_gamma(b[i], g);
        }
}

float pie_bm_srgb_to_linear(float g)
{
        float l;

        if (g < 0.04045f)
        {
                l = g / 12.92f;
        }
        else
        {
                l = powf((g + 0.055f) / 1.055f, SRGB_G_EXP);
        }

        return l;
}

float pie_bm_srgb_to_linearp(float g)
{
        float p1 = -0.04895813f;
        float p2 = 0.20727534f;
        float p3 = -0.40904131f;
        float p4 = 0.70083753f;
        float p5 = 0.51648981f;
        float p6 = 0.03244261f;
        float p7 = 0.00094529f;

        float g2 = g * g;
        float g3 = g * g2;
        float g4 = g * g3;
        float g5 = g * g4;
        float g6 = g * g5;

        if (g < 0.04045f)
        {
                return g / 12.92f;
        }

        return p1 * g6 +
                p2 * g5 +
                p3 * g4 +
                p4 * g3 +
                p5 * g2 +
                p6 * g +
                p7;
}

void pie_bm_srgb_to_linearv(float* l, size_t size)
{
        for (size_t i = 0; i < size; i++)
        {
#if APPROX
                l[i] = pie_bm_srgb_to_linearp(l[i]);
#else
                l[i] = pie_bm_srgb_to_linear(l[i]);
#endif
        }
}

float pie_bm_linear_to_srgb(float l)
{
        float g;

        if (l < 0.0031308f)
        {
                g = l * 12.92f;
        }
        else
        {
                g = 1.055f * powf(l, 1.0f / SRGB_G_EXP) - 0.055f;
        }

        return g;
}

float pie_bm_linear_to_srgbp(float l)
{
        float p1 =35.340155f;
        float p2 =-137.128546f;
        float p3 =216.887479f;
        float p4 =-180.266583f;
        float p5 =85.050919f;
        float p6 =-23.472036f;
        float p7 =4.528103f;
        float p8 =0.064798f;

        float l2 = l * l;
        float l3 = l * l2;
        float l4 = l * l3;
        float l5 = l * l4;
        float l6 = l * l5;
        float l7 = l * l6;

        if (l < 0.0031308f)
        {
                return l * 12.92f;
        }

        return p1 * l7 +
                p2 * l6 +
                p3 * l5 +
                p4 * l4 +
                p5 * l3 +
                p6 * l2 +
                p7 * l +
                p8;
}

void pie_bm_linear_to_srgbv(float* l, size_t size)
{
        for (size_t i = 0; i < size; i++)
        {
                l[i] = pie_bm_linear_to_srgb(l[i]);
        }
}
