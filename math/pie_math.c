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

#include <stdio.h>
#include <math.h>
#include "pie_math.h"

float pie_mth_gauss(float dx, float var)
{
        float ret;
        float numerator = dx * dx;
        float denominator = 2 * var;
        float c1;
        float c2;

        c1 = expf(- (numerator / denominator));
        c2 = sqrtf(2.0f * (float)M_PI * var);

        ret = c1 / c2;

        return ret;
}

float pie_mth_gauss_2d(float dx, float dy, float var)
{
        float ret;
        float numerator = (dx * dx) + (dy * dy);
        float denominator = 2 * var;
        float c1;
        float c2;

        c1 = expf(- (numerator / denominator));
        c2 = (2.0f * (float)M_PI * var);

        ret = c1 / c2;

        return ret;
}

void pie_mth_gauss_matrix(float* m, size_t l, float var)
{
        float sum = 0;
        float delta = (float)(l - 1) / 2.0f;

        for (size_t y = 0; y < l; y++)
        {
                for (size_t x = 0; x < l; x++)
                {
                        float dx = (float)((float)x - delta);
                        float dy = (float)((float)y - delta);

                        m[y * l + x] = pie_mth_gauss_2d(dx, dy, var);
                        sum += m[y * l + x];
                }
        }

        for (size_t i = 0; i < (l * l); i++)
        {
                m[i] /= sum;
        }
}

void pie_mth_matrix_print(float* m, size_t l)
{
        for (size_t y = 0; y < l; y++)
        {
                for (size_t x = 0; x < l; x++)
                {
                        printf("%f ", m[y * l + x]);
                }
                printf("\n");
        }
}
