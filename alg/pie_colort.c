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
#ifdef _HAS_ALTIVEC
# include <altivec.h>
#endif
#include <assert.h>

void pie_alg_color_temp(float* restrict r,
                        float* restrict g,
                        float* restrict b,
                        float colort,
                        float tint,
                        int w,
                        int h,
                        int s)
{
#if _HAS_SSE
        __m128 ctv = _mm_set1_ps(colort);
        __m128 tintv = _mm_set1_ps(tint);
        __m128 onev = _mm_set1_ps(1.0f);
        __m128 zerov = _mm_set1_ps(0.0f);
        __m128 onethirdv = _mm_set1_ps(0.33333f);
#endif
#if _HAS_ALTIVEC
        vector float ctv = (vector float){colort, colort, colort, colort};
        vector float tintv = (vector float){tint, tint, tint, tint};
        vector float onev = (vector float){1.0f, 1.0f, 1.0f, 1.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
        vector float onethirdv = (vector float){0.33333f, 0.33333f, 0.33333f, 0.33333f};
#endif
#if _HAS_SIMD
	int rem = w % 4;
	int stop = w - rem;
#else
        int stop = 0;
#endif
        
        assert(colort >= -0.3f);
        assert(colort <= 0.3f);
        assert(tint >= -0.3f);
        assert(tint <= 0.3f);        

        for (int y = 0; y < h; y++)
        {        
#ifdef _HAS_SIMD
# if _HAS_SSE
                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * s + x;
                        __m128 rv;
                        __m128 gv;
                        __m128 bv;
                        __m128 cmpv;

                        rv = _mm_load_ps(r + p);
                        gv = _mm_load_ps(g + p);
                        bv = _mm_load_ps(b + p);                        

                        rv = _mm_add_ps(rv, ctv);
                        gv = _mm_add_ps(gv, tintv);
                        gv = _mm_add_ps(gv, _mm_mul_ps(ctv, onethirdv));
                        bv = _mm_sub_ps(bv, ctv);
                        
                        /* Truncate red */
                        cmpv = _mm_cmplt_ps(rv, zerov);
                        rv = _mm_blendv_ps(rv, zerov, cmpv);

                        cmpv = _mm_cmpnlt_ps(rv, onev);
                        rv = _mm_blendv_ps(rv, onev, cmpv);

                        _mm_store_ps(r + p, rv);

                        /* Truncate green */                        
                        cmpv = _mm_cmplt_ps(gv, zerov);
                        gv = _mm_blendv_ps(gv, zerov, cmpv);

                        cmpv = _mm_cmpnlt_ps(gv, onev);
                        gv = _mm_blendv_ps(gv, onev, cmpv);

                        _mm_store_ps(g + p, gv);

                        /* Truncate green */
                        cmpv = _mm_cmplt_ps(bv, zerov);
                        bv = _mm_blendv_ps(bv, zerov, cmpv);

                        cmpv = _mm_cmpnlt_ps(bv, onev);
                        bv = _mm_blendv_ps(bv, onev, cmpv);

                        _mm_store_ps(b + p, bv);
                }
# elif _HAS_ALTIVEC
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

                        rv = vec_add(rv, ctv);
                        gv = vec_add(gv, vec_madd(ctv, onethirdv, tintv));
                        bv = vec_sub(bv, ctv);
                        
                        /* Truncate red */
                        cmpv = vec_cmpgt(rv, onev);
                        rv = vec_sel(rv, onev, cmpv);

                        cmpv = vec_cmplt(rv, zerov);
                        rv = vec_sel(rv, zerov, cmpv);
        
                        vec_st(rv, p, r);

                        /* Truncate red */
                        cmpv = vec_cmpgt(gv, onev);
                        gv = vec_sel(gv, onev, cmpv);

                        cmpv = vec_cmplt(gv, zerov);
                        gv = vec_sel(gv, zerov, cmpv);
        
                        vec_st(gv, p, g);

                        /* Truncate red */
                        cmpv = vec_cmpgt(bv, onev);
                        bv = vec_sel(bv, onev, cmpv);

                        cmpv = vec_cmplt(bv, zerov);
                        bv = vec_sel(bv, zerov, cmpv);
        
                        vec_st(bv, p, b);                        
                }
# else
# error invalid SIMD mode        
# endif
#endif
        
                for (int x = stop; x < w; x++)
                {
                        int p = y * s + x;

                        r[p] += colort;
                        g[p] += tint + colort * 0.33333f;
                        b[p] -= colort;

                        if (r[p] > 1.0f)
                        {
                                r[p] = 1.0f;
                        }
                        if (r[p] < 0.0f)
                        {
                                r[p] = 0.0f;
                        }
                        if (g[p] > 1.0f)
                        {
                                g[p] = 1.0f;
                        }
                        if (g[p] < 0.0f)
                        {
                                g[p] = 0.0f;
                        }                        
                        if (b[p] > 1.0f)
                        {
                                b[p] = 1.0f;
                        }
                        if (b[p] < 0.0f)
                        {
                                b[p] = 0.0f;
                        }     
                }
        }
}
