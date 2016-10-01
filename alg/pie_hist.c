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

#ifdef _HAS_SSE
static float lum_redv[4] = {LUM_RED, LUM_RED, LUM_RED, LUM_RED};
static float lum_greenv[4] = {LUM_GREEN, LUM_GREEN, LUM_GREEN, LUM_GREEN};
static float lum_bluev[4] = {LUM_BLUE, LUM_BLUE, LUM_BLUE, LUM_BLUE};
static float vec_255[4] = {255.0f, 255.0f, 255.0f, 255.0f};
#endif

#ifdef _HAS_SIMD
# ifdef _HAS_SSE

void pie_hist_lum(struct pie_histogram* hist, struct bitmap_f32rgb* bm)
{
	__m128 coeff_red = _mm_load_ps(lum_redv);
	__m128 coeff_green = _mm_load_ps(lum_greenv);
	__m128 coeff_blue = _mm_load_ps(lum_bluev);
	__m128 coeff_scale = _mm_load_ps(vec_255);
	unsigned int rem = bm->width % 4;
	unsigned int stop = bm->width - rem;

        memset(hist->lum, 0, sizeof(unsigned int) * PIE_HIST_RES);

	for (unsigned int y = 0; y < bm->height; y++)
	{
		for (unsigned int x = 0; x < stop; x+=4)
		{
                        __m128 red;
                        __m128 green;
                        __m128 blue;
                        float out[4];
			unsigned int p = y * bm->row_stride + x;

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

		for (unsigned int x = stop; x < bm->width; x++)
		{
			unsigned int p = y * bm->row_stride + x;
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

# elif _HAS_ALTIVEC
#  error ALTIVEC NOT IMPLEMENTED
# endif
#else

void pie_hist_lum(struct pie_histogram* hist, struct bitmap_f32rgb* bm)
{
        memset(hist->lum, 0, sizeof(unsigned int) * PIE_HIST_RES);
        /* memset(hist->c_red, 0, sizeof(unsigned int) * PIE_HIST_RES); */
        /* memset(hist->c_blue, 0, sizeof(unsigned int) * PIE_HIST_RES); */
        /* memset(hist->c_green, 0, sizeof(unsigned int) * PIE_HIST_RES); */

	for (unsigned int y = 0; y < bm->height; y++)
	{
		for (unsigned int x = 0; x < bm->width; x++)
		{
			float l;
			unsigned int p = y * bm->row_stride + x;
			unsigned char hp;

			l = LUM_RED * bm->c_red[p];
			l += LUM_GREEN * bm->c_green[p];
			l += LUM_BLUE * bm->c_blue[p];

			hp = (unsigned char)(l * 255.0f);
                        hist->lum[hp]++;
                        /* hp = (unsigned char)(bm->c_red[p] * 255.0f); */
                        /* hist->c_red[hp]++; */
                        /* hp = (unsigned char)(bm->c_blue[p] * 255.0f); */
                        /* hist->c_blue[hp]++; */
                        /* hp = (unsigned char)(bm->c_green[p] * 255.0f); */
                        /* hist->c_green[hp]++; */
                }
	}
}

#endif
