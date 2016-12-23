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
#include <string.h>
#include "pie_hist.h"

#define LUM_RED   0.2126f
#define LUM_GREEN 0.7152f
#define LUM_BLUE  0.0722f

#ifdef _HAS_SIMD
# ifdef _HAS_SSE

void pie_alg_hist_lum(struct pie_histogram* hist, struct bitmap_f32rgb* bm)
{
	__m128 coeff_red = _mm_set1_ps(LUM_RED);
	__m128 coeff_green = _mm_set1_ps(LUM_GREEN);
	__m128 coeff_blue = _mm_set1_ps(LUM_BLUE);
	__m128 coeff_scale = _mm_set1_ps(255.0f);
	int rem = bm->width % 4;
	int stop = bm->width - rem;

        memset(hist->lum, 0, sizeof(unsigned int) * PIE_HIST_RES);

	for (int y = 0; y < bm->height; y++)
	{
		for (int x = 0; x < stop; x+=4)
		{
                        __m128 red;
                        __m128 green;
                        __m128 blue;
                        float out[4];
			int p = y * bm->row_stride + x;

			red = _mm_load_ps(&bm->c_red[p]);
			green = _mm_load_ps(&bm->c_green[p]);
			blue = _mm_load_ps(&bm->c_blue[p]);

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

		for (int x = stop; x < bm->width; x++)
		{
			int p = y * bm->row_stride + x;
			unsigned char hp;
			float l;

			l = LUM_RED * bm->c_red[p];
			l += LUM_GREEN * bm->c_green[p];
			l += LUM_BLUE * bm->c_blue[p];

			hp = (unsigned char)(l * 255.0f);

			hist->lum[hp]++;
		}
	}
}

void pie_alg_hist_rgb(struct pie_histogram* hist, struct bitmap_f32rgb* bm)
{
	__m128 coeff_scale = _mm_set1_ps(255.0f);
        float or[4];
        float og[4];
        float ob[4];
        int rem = bm->width % 4;
        int stop = bm->width - rem;

        memset(hist->c_red, 0, sizeof(unsigned int) * PIE_HIST_RES);
        memset(hist->c_blue, 0, sizeof(unsigned int) * PIE_HIST_RES);
        memset(hist->c_green, 0, sizeof(unsigned int) * PIE_HIST_RES);

        for (int y = 0; y < bm->height; y++)
        {
                for (int x = 0; x < stop; x += 4)
                {
			unsigned int p = y * bm->row_stride + x;
                        __m128 r = _mm_load_ps(&bm->c_red[p]);
                        __m128 g = _mm_load_ps(&bm->c_green[p]);
                        __m128 b = _mm_load_ps(&bm->c_blue[p]);

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

		for (int x = stop; x < bm->width; x++)
		{
			int p = y * bm->row_stride + x;
			unsigned char hp;
                        float r = bm->c_red[p];
                        float g = bm->c_green[p];
                        float b = bm->c_blue[p];

                        hp = (unsigned char)(r * 255.0f);
                        hist->c_red[hp]++;
                        hp = (unsigned char)(g * 255.0f);
                        hist->c_green[hp]++;
                        hp = (unsigned char)(b * 255.0f);
                        hist->c_blue[hp]++;
                }
        }
}

# elif _HAS_ALTIVEC
#  error ALTIVEC NOT IMPLEMENTED
# endif
#else

void pie_alg_hist_lum(struct pie_histogram* hist, struct bitmap_f32rgb* bm)
{
        memset(hist->lum, 0, sizeof(unsigned int) * PIE_HIST_RES);

	for (int y = 0; y < bm->height; y++)
	{
		for (int x = 0; x < bm->width; x++)
		{
			float l;
			int p = y * bm->row_stride + x;
			unsigned char hp;
#if _USE_GAMMA_CONV > 0
                        float r = linear_to_srgb(bm->c_red[p]);
                        float g = linear_to_srgb(bm->c_green[p]);
                        float b = linear_to_srgb(bm->c_blue[p]);
#else
                        float r = bm->c_red[p];
                        float g = bm->c_green[p];
                        float b = bm->c_blue[p];
#endif

			l = LUM_RED * r + 
                                LUM_GREEN * g +
                                LUM_BLUE * b;
			hp = (unsigned char)(l * 255.0f);
                        hist->lum[hp]++;
                }
	}
}

void pie_alg_hist_rgb(struct pie_histogram* hist, struct bitmap_f32rgb* bm)
{
        memset(hist->c_red, 0, sizeof(unsigned int) * PIE_HIST_RES);
        memset(hist->c_blue, 0, sizeof(unsigned int) * PIE_HIST_RES);
        memset(hist->c_green, 0, sizeof(unsigned int) * PIE_HIST_RES);

	for (int y = 0; y < bm->height; y++)
	{
		for (int x = 0; x < bm->width; x++)
		{
			int p = y * bm->row_stride + x;
			unsigned char hp;
#if _USE_GAMMA_CONV > 0
                        float r = linear_to_srgb(bm->c_red[p]);
                        float g = linear_to_srgb(bm->c_green[p]);
                        float b = linear_to_srgb(bm->c_blue[p]);
#else
                        float r = bm->c_red[p];
                        float g = bm->c_green[p];
                        float b = bm->c_blue[p];
#endif
                        hp = (unsigned char)(r * 255.0f);
                        hist->c_red[hp]++;
                        hp = (unsigned char)(g * 255.0f);
                        hist->c_green[hp]++;
                        hp = (unsigned char)(b * 255.0f);
                        hist->c_blue[hp]++;
                }
	}
}

#endif
