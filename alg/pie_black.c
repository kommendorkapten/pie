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

#ifdef _HAS_ALTIVEC
# include <altivec.h>
#endif
#include <math.h>
#include <assert.h>

#define ACTION 0.142f

static float pie_alg_black_val(float x, float v, float s, float k);

void pie_alg_black(float* restrict r,
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
                x0 += d;
        }
        else
        {
                y0 += d;
        }

        k = (y1 - y0) / (x1 - x0);

        for (int y = 0; y < h; y++)
        {
#ifdef __powerpc__
                int rem = w % 4;
                int stop = w - rem;
#else
                int stop = 0;
#endif

#ifdef __powerpc__
                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * s + x;

                        r[p]   = pie_alg_black_val(r[p], v, d, k);
                        r[p+1] = pie_alg_black_val(r[p+1], v, d, k);
                        r[p+2] = pie_alg_black_val(r[p+2], v, d, k);
                        r[p+3] = pie_alg_black_val(r[p+3], v, d, k);
                }

                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * s + x;

                        g[p]   = pie_alg_black_val(g[p], v, d, k);
                        g[p+1] = pie_alg_black_val(g[p+1], v, d, k);
                        g[p+2] = pie_alg_black_val(g[p+2], v, d, k);
                        g[p+3] = pie_alg_black_val(g[p+3], v, d, k);
                }

                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * s + x;

                        b[p]   = pie_alg_black_val(b[p], v, d, k);
                        b[p+1] = pie_alg_black_val(b[p+1], v, d, k);
                        b[p+2] = pie_alg_black_val(b[p+2], v, d, k);
                        b[p+3] = pie_alg_black_val(b[p+3], v, d, k);
                }
#endif

                for (int x = stop; x < w; x++)
                {
                        int p = y * s + x;

                        r[p] = pie_alg_black_val(r[p], v, d, k);
                        g[p] = pie_alg_black_val(g[p], v, d, k);
                        b[p] = pie_alg_black_val(b[p], v, d, k);
                }
        }
}

static float pie_alg_black_val(float x, float v, float s, float k)
{
        float y;
        
        if (v < 0.0f)
        {
                if (x < s)
                {
                        y = 0.0f;
                }
                else
                {
                        y = (x - s) * k;
                }
        }
        else
        {
               y = k * x + s; 
        }
        
        return y;
}
