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
#include "pie_vibra.h"

void pie_alg_vibra(float* restrict r,
                   float* restrict g,
                   float* restrict b,
                   float v,
                   int w,
                   int h,
                   int s)
{
        assert(v <= 1.0f);
        assert(v >= -1.0f);

        /* Negate vibrance adjustment, alg is 1 = bw */
        v *= -1.0f;
        for (int y = 0; y < h; y++)
        {
                for (int x = 0; x < w; x++)
                {
                        int p = y * s + x;
                        float max = r[p] > g[p] ? r[p] : g[p];
                        float avg = r[p] + g[p] + b[p];
                        float pv;
                        
                        max = max > b[p] ? max : b[p];
                        avg *= 0.333333f;
                        pv = (max - avg) * 2.0f * v;

                        if (r[p] < max)
                        {
                                float a = (max - r[p]) * pv;

                                r[p] += a;
                        }

                        if (g[p] < max)
                        {
                                float a = (max - g[p]) * pv;

                                g[p] += a;
                        }

                        if (b[p] < max)
                        {
                                float a = (max - b[p]) * pv;

                                b[p] += a;
                        }
                }
        }
}
