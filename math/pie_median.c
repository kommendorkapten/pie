/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
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

#include "pie_median.h"

/* Implementation based on: http://ndevilla.free.fr/median/median/index.html */

float pie_mth_med4(float* p)
{
        pie_mth_sort2(p + 0, p + 1); pie_mth_sort2(p + 2, p + 3);
        pie_mth_sort2(p + 0, p + 3); pie_mth_sort2(p + 1, p + 2);

        return (p[1] + p[3]) * 0.5;
}

float pie_mth_med6(float* p)
{
        pie_mth_sort2(p + 1, p + 2); pie_mth_sort2(p + 3, p + 4);
        pie_mth_sort2(p + 0, p + 1); pie_mth_sort2(p + 2, p + 3);
        pie_mth_sort2(p + 4, p + 5);

        pie_mth_sort2(p + 1, p + 2); pie_mth_sort2(p + 3, p + 4);
        pie_mth_sort2(p + 0, p + 1); pie_mth_sort2(p + 2, p + 3);
        pie_mth_sort2(p + 4, p + 5);

        pie_mth_sort2(p + 1, p + 2); pie_mth_sort2(p + 3, p + 4);

        return (p[2] + p[3]) * 0.5;
}

float pie_mth_med9(float* p)
{
        pie_mth_sort3(p + 0, p + 1, p + 2);
        pie_mth_sort3(p + 3, p + 4, p + 5);
        pie_mth_sort3(p + 6, p + 7, p + 8);

        p[6] = pie_mth_max3(p[0], p[3], p[6]);
        p[2] = pie_mth_min3(p[2], p[5], p[8]);

        pie_mth_sort3(p + 1, p + 4, p + 7);
        pie_mth_sort3(p + 2, p + 4, p + 6);

        return p[4];
}

void pie_mth_sort2(float* restrict a, float* restrict b)
{
        if (*a > *b)
        {
                float tmp = *b;
                *b = *a;
                *a = tmp;
        }
}

void pie_mth_sort3(float* restrict a, float* restrict b, float* restrict c)
{
        pie_mth_sort2(b, c);
        pie_mth_sort2(a, b);
        pie_mth_sort2(b, c);
}

float pie_mth_min3(float a, float b, float c)
{
        if (a < b)
        {
                return a < c ? a : c;
        }

        return b < c ? b : c;
}

float pie_mth_max3(float a, float b, float c)
{
        if (a > b)
        {
                return a > c ? a : c;
        }

        return b > c ? b : c;
}
