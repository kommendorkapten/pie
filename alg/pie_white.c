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

#if _HAS_ALTIVEC
# include <altivec.h>
#endif
#include <math.h>
#include <assert.h>
#include "pie_white.h"

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
                
#if _HAS_ALTIVEC
                int rem = w % 4;
                int stop = w - rem;
                vector float onev = (vector float){1.0f, 1.0f, 1.0f, 1.0f};
                vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
                vector float kv = (vector float){k, k, k, k};

                for (int x = 0; x < stop; x += 4)
                {
                        int p = sizeof(float) * (y * s + x);
                        vector float rv;
                        vector float gv;
                        vector float bv;
                        vector int bool cmpv;

                        rv = vec_ld(p, r);
                        gv = vec_ld(p, g);
                        bv = vec_ld(p, b);

                        rv = vec_madd(rv, kv, zerov);
                        gv = vec_madd(gv, kv, zerov);
                        bv = vec_madd(bv, kv, zerov);                        
                        
                        /* Max 1.0 */
                        cmpv = vec_cmpgt(rv, onev);
                        rv = vec_sel(rv, onev, cmpv);

                        /* Max 1.0 */
                        cmpv = vec_cmpgt(gv, onev);
                        gv = vec_sel(gv, onev, cmpv);

                        /* Max 1.0 */
                        cmpv = vec_cmpgt(bv, onev);
                        bv = vec_sel(bv, onev, cmpv);
        
                        vec_st(rv, p, r);
                        vec_st(gv, p, g);
                        vec_st(bv, p, b);                        
                }
#else
                int stop = 0;
#endif

                for (int x = stop; x < w; x++)
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
