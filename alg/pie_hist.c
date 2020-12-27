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
#include <string.h>
#include "pie_hist.h"
#include "../bm/pie_bm.h"

#define LUM_RED   0.2126f
#define LUM_GREEN 0.7152f
#define LUM_BLUE  0.0722f

void pie_alg_hist_lum(struct pie_alg_histogram* hist, struct pie_bm_f32rgb* bm)
{
#if _HAS_SSE42
	__m128 coeff_red = _mm_set1_ps(LUM_RED);
	__m128 coeff_green = _mm_set1_ps(LUM_GREEN);
	__m128 coeff_blue = _mm_set1_ps(LUM_BLUE);
	__m128 coeff_scale = _mm_set1_ps(255.0f);
#endif
#if _HAS_ALTIVEC
        vector float coeff_red = (vector float){LUM_RED, LUM_RED, LUM_RED, LUM_RED};
        vector float coeff_green = (vector float){LUM_GREEN, LUM_GREEN, LUM_GREEN, LUM_GREEN};
        vector float coeff_blue = (vector float){LUM_BLUE, LUM_BLUE, LUM_BLUE, LUM_BLUE};
        vector float coeff_scale = (vector float){255.0f, 255.0f, 255.0f, 255.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
#endif
#if _HAS_SIMD4
	int rem = bm->width % 4;
	int stop = bm->width - rem;
#else
        int stop = 0;
#endif
        memset(hist->lum, 0, sizeof(unsigned int) * PIE_HIST_RES);

	for (int y = 0; y < bm->height; y++)
	{

#if _HAS_SSE42

		for (int x = 0; x < stop; x+=4)
		{
                        __m128 red;
                        __m128 green;
                        __m128 blue;
                        float out[4];
			int p = y * bm->row_stride + x;

			red = _mm_load_ps(bm->c_red + p);
			green = _mm_load_ps(bm->c_green + p);
			blue = _mm_load_ps(bm->c_blue + p);

			red = _mm_mul_ps(red, coeff_red);
			green = _mm_mul_ps(green, coeff_green);
			blue = _mm_mul_ps(blue, coeff_blue);

			red = _mm_add_ps(red, green);
			red = _mm_add_ps(red, blue);
			red = _mm_mul_ps(red, coeff_scale);

			_mm_store_ps(out, red);

			hist->lum[(unsigned char)out[0]]++;
			hist->lum[(unsigned char)out[1]]++;
			hist->lum[(unsigned char)out[2]]++;
			hist->lum[(unsigned char)out[3]]++;
		}

#elif _HAS_ALTIVEC

                for (int x = 0; x < stop; x+=4)
                {
                        int p = sizeof(float) * (y * bm->row_stride + x);
                        vector float red;
                        vector float green;
                        vector float blue;
                        vector float acc;
                        float out[4];

                        red = vec_ld(p, bm->c_red);
                        green = vec_ld(p, bm->c_green);
                        blue = vec_ld(p, bm->c_blue);

                        acc = vec_madd(red, coeff_red, zerov);
                        acc = vec_madd(green, coeff_green, acc);
                        acc = vec_madd(blue, coeff_blue, acc);

                        acc = vec_madd(acc, coeff_scale, zerov);

                        vec_st(acc, 0, out);

			hist->lum[(unsigned char)out[0]]++;
			hist->lum[(unsigned char)out[1]]++;
			hist->lum[(unsigned char)out[2]]++;
			hist->lum[(unsigned char)out[3]]++;
                }

#endif

		for (int x = stop; x < bm->width; x++)
		{
			int p = y * bm->row_stride + x;
			float l;
                        float r = bm->c_red[p];
                        float g = bm->c_green[p];
                        float b = bm->c_blue[p];
			unsigned char hp;

			l = LUM_RED * r +
                                LUM_GREEN * g +
                                LUM_BLUE * b;

			hp = (unsigned char)(l * 255.0f);
			hist->lum[hp]++;
		}
	}
}

void pie_alg_hist_rgb(struct pie_alg_histogram* hist, struct pie_bm_f32rgb* bm)
{
#if _HAS_SSE42
	__m128 coeff_scale = _mm_set1_ps(255.0f);
#endif
#if _HAS_ALTIVEC
        vector float coeff_scale = (vector float){255.0f, 255.0f, 255.0f, 255.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
#endif
#if _HAS_SIMD4
        float or[4];
        float og[4];
        float ob[4];
        int rem = bm->width % 4;
        int stop = bm->width - rem;
#else
        int stop = 0;
#endif

        memset(hist->c_red, 0, sizeof(unsigned int) * PIE_HIST_RES);
        memset(hist->c_blue, 0, sizeof(unsigned int) * PIE_HIST_RES);
        memset(hist->c_green, 0, sizeof(unsigned int) * PIE_HIST_RES);

        for (int y = 0; y < bm->height; y++)
        {

#if _HAS_SSE42

                for (int x = 0; x < stop; x += 4)
                {
			int p = y * bm->row_stride + x;
                        __m128 r = _mm_load_ps(bm->c_red + p);
                        __m128 g = _mm_load_ps(bm->c_green + p);
                        __m128 b = _mm_load_ps(bm->c_blue + p);

                        r = _mm_mul_ps(r, coeff_scale);
                        g = _mm_mul_ps(g, coeff_scale);
                        b = _mm_mul_ps(b, coeff_scale);

                        _mm_store_ps(or, r);
                        _mm_store_ps(og, g);
                        _mm_store_ps(ob, b);

                        hist->c_red[(unsigned char)or[0]]++;
                        hist->c_red[(unsigned char)or[1]]++;
                        hist->c_red[(unsigned char)or[2]]++;
                        hist->c_red[(unsigned char)or[3]]++;
                        hist->c_green[(unsigned char)og[0]]++;
                        hist->c_green[(unsigned char)og[1]]++;
                        hist->c_green[(unsigned char)og[2]]++;
                        hist->c_green[(unsigned char)og[3]]++;
                        hist->c_blue[(unsigned char)ob[0]]++;
                        hist->c_blue[(unsigned char)ob[1]]++;
                        hist->c_blue[(unsigned char)ob[2]]++;
                        hist->c_blue[(unsigned char)ob[3]]++;
                }

#elif _HAS_ALTIVEC

                for (int x = 0; x < stop; x += 4)
                {
                        int p = sizeof(float) * (y * bm->row_stride + x);
                        vector float r = vec_ld(p, bm->c_red);
                        vector float g = vec_ld(p, bm->c_green);
                        vector float b = vec_ld(p, bm->c_blue);

                        r = vec_madd(r, coeff_scale, zerov);
                        g = vec_madd(g, coeff_scale, zerov);
                        b = vec_madd(b, coeff_scale, zerov);

                        vec_st(r, 0, or);
                        vec_st(g, 0, og);
                        vec_st(b, 0, ob);

                        hist->c_red[(unsigned char)or[0]]++;
                        hist->c_red[(unsigned char)or[1]]++;
                        hist->c_red[(unsigned char)or[2]]++;
                        hist->c_red[(unsigned char)or[3]]++;
                        hist->c_green[(unsigned char)og[0]]++;
                        hist->c_green[(unsigned char)og[1]]++;
                        hist->c_green[(unsigned char)og[2]]++;
                        hist->c_green[(unsigned char)og[3]]++;
                        hist->c_blue[(unsigned char)ob[0]]++;
                        hist->c_blue[(unsigned char)ob[1]]++;
                        hist->c_blue[(unsigned char)ob[2]]++;
                        hist->c_blue[(unsigned char)ob[3]]++;
                }

#endif

		for (int x = stop; x < bm->width; x++)
		{
			int p = y * bm->row_stride + x;
                        float r = bm->c_red[p];
                        float g = bm->c_green[p];
                        float b = bm->c_blue[p];
			unsigned char hp;

                        hp = (unsigned char)(r * 255.0f);
                        hist->c_red[hp]++;
                        hp = (unsigned char)(g * 255.0f);
                        hist->c_green[hp]++;
                        hp = (unsigned char)(b * 255.0f);
                        hist->c_blue[hp]++;
                }
        }
}
