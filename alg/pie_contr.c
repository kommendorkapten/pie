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

/* 
 * c is amount of contrast int [0, 2]
 * c [0, 1] less contrast
 * c [1,+] more contrast
 */

void pie_alg_contr(float* img,
                   float c,
                   int w,
                   int h,
                   int stride)
{
#ifdef _HAS_SSE
        __m128 sv = _mm_set1_ps(0.5f);
        __m128 av = _mm_set1_ps(c);
        __m128 onev = _mm_set1_ps(1.0f);
        __m128 zerov = _mm_set1_ps(0.0f);
#endif
#ifdef _HAS_ALTIVEC
        vector float sv = (vector float){0.5f, 0.5f, 0.5f, 0.5f};
        vector float av = (vector float){c, c, c, c};
        vector float onev = (vector float){1.0f, 1.0f, 1.0f, 1.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
#endif
#ifdef _HAS_SIMD
	int rem = w % 4;
	int stop = w - rem;
#else
        int stop = 0;
#endif
        
        assert(c >= 0.0f);
        assert(c <= 2.0f);

        for (int y = 0; y < h; y++)
        {

#ifdef _HAS_SIMD
# ifdef _HAS_SSE
                
                for (int x = 0; x < stop; x += 4)
                {
                        __m128 data;
                        __m128 cmpv;
                        int p = y * stride + x;

                        data = _mm_load_ps(img + p);
                        data = _mm_sub_ps(data, sv);
                        data = _mm_mul_ps(data, av);
                        data = _mm_add_ps(data, sv);

                        /* the ones less than zero will be 0xffffffff */
                        cmpv = _mm_cmplt_ps(data, zerov);
                        /* d = a,b,m. if m == 0 a else b */
                        data = _mm_blendv_ps(data, zerov, cmpv);

                        /* the ones greater than one will be 0xffffffff */
                        cmpv = _mm_cmpnlt_ps(data, onev);
                        data = _mm_blendv_ps(data, onev, cmpv);

                        _mm_store_ps(img + p, data);
                }

# else

                for (int x = 0; x < stop; x += 4)
                {
                        vector float datav;
                        vector int bool cmpv;
                        int p = sizeof(float) * (y * stride + x);

                        datav = vec_ld(p, img);
                        datav = vec_sub(datav, sv);
                        datav = vec_madd(datav, av, sv);

                        /* Max 1.0 */
                        cmpv = vec_cmpgt(datav, onev);
                        datav = vec_sel(datav, onev, cmpv);
                        /* Min 0.0 */
                        cmpv = vec_cmplt(datav, zerov);
                        datav = vec_sel(datav, zerov, cmpv);
        
                        vec_st(datav, p, img);
                }

# endif
#endif
                
                for (int x = stop; x < w; x++)
                {
                        float* p = img + y * stride + x;

                        *p = c * (*p - 0.5f) + 0.5f;

                        if (*p > 1.0f)
                        {
                                *p = 1.0f;
                        } 
                        else if (*p < 0.0f)
                        {
                                *p = 0.0f;
                        }
                }
        }
}
