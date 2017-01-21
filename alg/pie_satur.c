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

#ifdef _HAS_SSE
# include <nmmintrin.h> /* sse 4.2 */
#endif
#include <math.h>
#include <assert.h>

#define P_R  0.299f
#define P_G  0.587f
#define P_B  0.114f

/* Public domain algorithm, by Darel Rex Finley */
void pie_alg_satur(float* restrict r,
                   float* restrict g,
                   float* restrict b,
                   float v,
                   int w,
                   int h,
                   int s)
{
        assert(v >= 0.0f);
        assert(v <= 2.0f);

#ifdef _HAS_SSE
        __m128 prv = _mm_set1_ps(P_R);
        __m128 pgv = _mm_set1_ps(P_G);
        __m128 pbv = _mm_set1_ps(P_B);
        __m128 vv = _mm_set1_ps(v);
        __m128 onev = _mm_set1_ps(1.0f);
        __m128 zerov = _mm_set1_ps(0.0f);
#endif
        
#ifdef _HAS_SIMD
	int rem = w % 4;
	int stop = w - rem;
#else
        int stop = 0;
#endif

        for (int y = 0; y < h; y++)
        {
                
#ifdef _HAS_SIMD        
# ifdef _HAS_SSE
                
                for (int x = 0; x < stop; x+=4)
                {
                        __m128 rv;
                        __m128 gv;
                        __m128 bv;
                        __m128 rsv;
                        __m128 gsv;
                        __m128 bsv;
                        __m128 accv;
                        __m128 sqv;
                        __m128 dv;
                        __m128 cmpv;
                        int p = y * s + x;

                        rv = _mm_load_ps(r + p);
                        gv = _mm_load_ps(g + p);
                        bv = _mm_load_ps(b + p);

                        rsv = _mm_mul_ps(rv, rv);
                        gsv = _mm_mul_ps(gv, gv);
                        bsv = _mm_mul_ps(bv, bv);
                        rsv = _mm_mul_ps(rsv, prv);
                        gsv = _mm_mul_ps(gsv, pgv);
                        bsv = _mm_mul_ps(bsv, pbv);

                        accv = _mm_add_ps(rsv, gsv);
                        accv = _mm_add_ps(accv, bsv);

                        sqv = _mm_sqrt_ps(accv);

                        /* red */
                        dv = _mm_sub_ps(rv, sqv);
                        dv = _mm_mul_ps(dv, vv);
                        accv = _mm_add_ps(sqv, dv);
                        /* maintain values in range [0,1] */
                        cmpv = _mm_cmplt_ps(accv, zerov);
                        accv = _mm_blendv_ps(accv, zerov, cmpv);
                        cmpv = _mm_cmpnlt_ps(accv, onev);
                        accv = _mm_blendv_ps(accv, onev, cmpv);
                        _mm_store_ps(r + p, accv);

                        /* green */
                        dv = _mm_sub_ps(gv, sqv);
                        dv = _mm_mul_ps(dv, vv);
                        accv = _mm_add_ps(sqv, dv);
                        /* maintain value in range [0,1] */
                        cmpv = _mm_cmplt_ps(accv, zerov);
                        accv = _mm_blendv_ps(accv, zerov, cmpv);
                        cmpv = _mm_cmpnlt_ps(accv, onev);
                        accv = _mm_blendv_ps(accv, onev, cmpv);
                        _mm_store_ps(g + p, accv);

                        /* blue */
                        dv = _mm_sub_ps(bv, sqv);
                        dv = _mm_mul_ps(dv, vv);
                        accv = _mm_add_ps(sqv, dv);
                        /* maintain value in range [0,1] */
                        cmpv = _mm_cmplt_ps(accv, zerov);
                        accv = _mm_blendv_ps(accv, zerov, cmpv);
                        cmpv = _mm_cmpnlt_ps(accv, onev);
                        accv = _mm_blendv_ps(accv, onev, cmpv);
                        _mm_store_ps(b + p, accv);
                }

# else
#  error ALTIVEC not supported yet
# endif
#endif
                
                for (int x = stop; x < w; x++)
                {
                        int o = y * s + x;
                        float p = sqrtf(r[o] * r[o] * P_R +
                                        g[o] * g[o] * P_G +
                                        b[o] * b[o] * P_B);

                        r[o] = p + (r[o] - p) * v;
                        g[o] = p + (g[o] - p) * v;
                        b[o] = p + (b[o] - p) * v;

                        if (r[o] < 0.0f)
                        {
                                r[o] = 0.0f;
                        }
                        else if (r[o] > 1.0f)
                        {
                                r[o] = 1.0f;
                        }

                        if (g[o] < 0.0f)
                        {
                                g[o] = 0.0f;
                        }
                        else if (g[o] > 1.0f)
                        {
                                g[o] = 1.0f;
                        }

                        if (b[o] < 0.0f)
                        {
                                b[o] = 0.0f;
                        }
                        else if (b[o] > 1.0f)
                        {
                                b[o] = 1.0f;
                        }                        
                }
        }
}
