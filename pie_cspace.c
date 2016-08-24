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
#include "pie_cspace.h"

/* sRGB gamma exponent */
#define SRGB_G_EXP 2.4f

float gamma(float b, float g)
{
        return powf(b, g);
}

void gammav(float* b, float g, size_t size)
{
        for (int i = 0; i < size; i++)
        {
                b[i] = gamma(b[i], g);
        }
}

float srgb_to_linear(float g)
{
        float l;

        if (g < 0.04045f)
        {
                l = g / 12.09f;
        }
        else
        {
                l = powf((g + 0.055f) / 1.055f, SRGB_G_EXP);
        }

        return l;
}

void srgb_to_linearv(float* l, size_t size)
{
        for (size_t i = 0; i < size; i++)
        {
                l[i] = srgb_to_linear(l[i]);
        }
}

float linear_to_srgb(float l)
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

void linear_to_srgbv(float* l, size_t size)
{
        for (size_t i = 0; i < size; i++)
        {
                l[i] = linear_to_srgb(l[i]);
        }
}
