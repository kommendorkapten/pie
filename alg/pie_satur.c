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

#if _HAS_SSE42
# include <nmmintrin.h> /* sse 4.2 */
#endif
#if _HAS_ALTIVEC
# include <altivec.h>
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

#if _HAS_SSE42
        __m128 prv = _mm_set1_ps(P_R);
        __m128 pgv = _mm_set1_ps(P_G);
        __m128 pbv = _mm_set1_ps(P_B);
        __m128 vv = _mm_set1_ps(v);
        __m128 onev = _mm_set1_ps(1.0f);
        __m128 zerov = _mm_set1_ps(0.0f);
#endif
#if _HAS_ALTIVEC
        vector float prv = (vector float){P_R, P_R, P_R, P_R};
        vector float pgv = (vector float){P_G, P_G, P_G, P_G};
        vector float pbv = (vector float){P_B, P_B, P_B, P_B};
        vector float vv = (vector float){v, v, v, v};
        vector float onev = (vector float){1.0f, 1.0f, 1.0f, 1.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
        vector float halfv = (vector float){0.5f, 0.5f, 0.5f, 0.5f};
        vector float three_halfv = (vector float){1.5f, 1.5f, 1.5f, 1.5f};
#endif
        
#if _HAS_SIMD4
	int rem = w % 4;
	int stop = w - rem;
#else
        int stop = 0;
#endif

        for (int y = 0; y < h; y++)
        {
                
#if _HAS_SSE42
                
                for (int x = 0; x < stop; x += 4)
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

#elif _HAS_ALTIVEC

                for (int x = 0; x < stop; x += 4)
                {
                        vector float rv;
                        vector float gv;
                        vector float bv;
                        vector float rsv;
                        vector float gsv;
                        vector float bsv;
                        vector float accv;
                        vector float sqv;
                        vector float dv;
                        vector int bool cmpv;
                        vector float input_halfv;
                        vector float prodv;                        
                        int p = sizeof(float) * (y * s + x);

                        rv = vec_ld(p, r);
                        gv = vec_ld(p, g);
                        bv = vec_ld(p, b);                        
                        
                        rsv = vec_madd(rv, rv, zerov);
                        gsv = vec_madd(gv, gv, zerov);
                        bsv = vec_madd(bv, bv, zerov);

                        accv = vec_madd(rsv, prv, zerov);
                        accv = vec_madd(gsv, pgv, accv);
                        accv = vec_madd(bsv, pbv, accv);

                        /* Square root apprixmation. Relative error
                           should be less than 0.0001 */
                        input_halfv = vec_madd(halfv, accv, zerov);
                        accv        = vec_rsqrte(accv);

                        /* One iteration of  Netwon Raphson */
                        /* x = approximation of rqrt(input)*/
                        /* output = x * (threehalfs - (input / 2) * x * x); */
                        prodv = vec_madd(accv, accv, zerov);
                        prodv = vec_madd(input_halfv, prodv, zerov);
                        prodv = vec_sub(three_halfv, prodv);
                        prodv = vec_madd(accv, prodv, zerov);
                        /* Invert to get sqrt approx */
                        sqv    = vec_re(prodv);

                        dv = vec_sub(rv, sqv);
                        accv = vec_madd(dv, vv, sqv);
                        /* Max 1.0 */
                        cmpv = vec_cmpgt(accv, onev);
                        accv = vec_sel(accv, onev, cmpv);
                        /* Min 0.0 */
                        cmpv = vec_cmplt(accv, zerov);
                        accv = vec_sel(accv, zerov, cmpv);
                        vec_st(accv, p, r);

                        dv   = vec_sub(gv, sqv);
                        accv = vec_madd(dv, vv, sqv);
                        /* Max 1.0 */
                        cmpv = vec_cmpgt(accv, onev);
                        accv = vec_sel(accv, onev, cmpv);
                        /* Min 0.0 */
                        cmpv = vec_cmplt(accv, zerov);
                        accv = vec_sel(accv, zerov, cmpv);
                        vec_st(accv, p, g);

                        dv   = vec_sub(bv, sqv);
                        accv = vec_madd(dv, vv, sqv);
                        /* Max 1.0 */
                        cmpv = vec_cmpgt(accv, onev);
                        accv = vec_sel(accv, onev, cmpv);
                        /* Min 0.0 */
                        cmpv = vec_cmplt(accv, zerov);
                        accv = vec_sel(accv, zerov, cmpv);
                        vec_st(accv, p, b);                        
                }

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
