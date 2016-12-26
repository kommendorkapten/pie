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

#define ACTION 0.142f

void pie_alg_white(float* restrict r,
                   float* restrict g,
                   float* restrict b,
                   float v,
                   int w,
                   int h,
                   int s)
{
        float x0 = 0.0f;
        float x1 = 1.0f;
        float y0 = 0.0f;
        float y1 = 1.0f;
        float d = fabsf(v) * ACTION;
        float k;

        assert(v >= -1.0f);
        assert(v <= 1.0f);
        
        if (v < 0.0f)
        {
                y1 = y1 - d;                
        }
        else
        {
                x1 = x1 - d;              
        }

        k = (y1 - y0) / (x1 - x0);

        for (int y = 0; y < h; y++)
        {
                for (int x = 0; x < w; x++)
                {
                        int p = y * s + x;

                        r[p] *= k;
                        g[p] *= k;
                        b[p] *= k;

                        if (r[p] > 1.0f)
                        {
                                r[p] = 1.0f;
                        }
                        if (g[p] > 1.0f)
                        {
                                g[p] = 1.0f;
                        }
                        if (b[p] > 1.0f)
                        {
                                b[p] = 1.0f;
                        }
                }
        }

}
