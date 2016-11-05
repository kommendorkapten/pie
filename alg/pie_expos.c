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
#ifdef _HAS_SSE
# include <nmmintrin.h> /* sse 4.2 */
#endif

#define LUM_RED   0.2126f
#define LUM_GREEN 0.7152f
#define LUM_BLUE  0.0722f
#define GAMMA_C   2.33f     

void pie_alg_expos(float* r,
                   float* g,
                   float* b,
                   float e,
                   unsigned int width,
                   unsigned int height,
                   unsigned int stride)
{
        float scale[256];
        float l;

        /* Percieved brightness is not linear, but follow a gamma of 
           2.33. Calculate the linear factor to apply.
           The relationship is virtually linear across the complete 
           luma scale [0,1], so just take a number and calculate. */
        e = powf(2.0f, e);
        l = powf(0.5f, GAMMA_C);
        l *= e;
        l = powf(l, (1.0f / GAMMA_C));
        /* Get the factor the color increased */
        l = l / 0.5f;
        /* Calculate a table per luminosity level to get a smoother
           exposure compensation, that avoids blowing out highlights */
        if (e >= 1.0f) 
        {
                for (int i = 0; i < 256; i++)
                {
                        if (i < 40)
                        {
                                scale[i] = i / 40.0f;
                        }
                        else if (i > 130 && i < 181)
                        {
                                scale[i] = (255 - i) / 125.0f;
                        }
                        else if (i > 180)
                        {
                                scale[i] = (255 - i) / 125.0f;
                                scale[i] = powf(scale[i], 1.35f);
                        }
                        else
                        {
                                scale[i] = 1.0f;
                        }
                        float delta = l - 1.0f;
                        scale[i] = 1.0 + delta * scale[i];
                        /* printf("%d: %f\n", i, scale[i]); */
                }                
        }
        else
        {
                l = l / 2.5f;
                for (int i = 0; i < 256; i++)
                {
                        if (i < 40)
                        {
                                scale[i] = i / 40.0f;
                        }
                        else
                        {
                                scale[i] = 1.0f;
                        }
                        float delta = l - 1.0f;
                        scale[i] = 1.0 + delta * scale[i];
                        /* printf("%d: %f\n", i, scale[i]); */
                }                                
        }

#ifdef _HAS_SIMD
# ifdef _HAS_SSE
        __m128 coeff_red = _mm_set1_ps(LUM_RED);
        __m128 coeff_green = _mm_set1_ps(LUM_GREEN);
        __m128 coeff_blue = _mm_set1_ps(LUM_BLUE);
	__m128 coeff_scale = _mm_set1_ps(255.0f);
        unsigned int rem = width % rem;
        unsigned int stop = width - stop;

        for (unsigned int y = 0; y < height; y++)
        {
                for (unsigned int x = 0; x < stop; x+=4)
                {
                        __m128 red;
                        __m128 green;
                        __m128 blue;
                        float out[4];
                        unsigned int p = y * stride + x;

                        red = _mm_load_ps(r + p);
                        green = _mm_load_ps(g + p);
                        blue = _mm_load_ps(b + p);

                        red = _mm_mul_ps(red, coeff_red);
                        green = _mm_mul_ps(green, coeff_green);
                        blue = _mm_mul_ps(blue, coeff_blue);

                        red = _mm_add_ps(red, green);
                        red = _mm_add_ps(red, blue);
                        red = _mm_mul_ps(red, coeff_scale);
                        
                        _mm_store_ps(out, red);

                        r[p] *= scale[(int)out[0]];
                        r[p+1] *= scale[(int)out[1]];
                        r[p+2] *= scale[(int)out[2]];
                        r[p+3] *= scale[(int)out[3]];

                        g[p] *= scale[(int)out[0]];
                        g[p+1] *= scale[(int)out[1]];
                        g[p+2] *= scale[(int)out[2]];
                        g[p+3] *= scale[(int)out[3]];

                        b[p] *= scale[(int)out[0]];
                        b[p+1] *= scale[(int)out[1]];
                        b[p+2] *= scale[(int)out[2]];
                        b[p+3] *= scale[(int)out[3]];
                }

                for (unsigned x = stop; x < width; x++)
                {
                        unsigned int p = y * stride + x;
                        float lum;
                        lum = r[p] * LUM_RED + 
                                g[p] * LUM_GREEN +
                                b[p] * LUM_BLUE;

                        int s = (int)(255.0f * lum);

                        r[p] *= scale[s];
                        g[p] *= scale[s];
                        b[p] *= scale[s];                        
                }
        }
# elif _HAS_ALTIVEC
#  error ALTIVET NOT IMPLEMENTED
# endif
#else
        for (unsigned int y = 0; y < height; y++)
        {
                for (unsigned int x = 0; x < width; x++)
                {
                        int p = y * stride + x;
                        float lum;
                        lum = r[p] * LUM_RED + 
                                g[p] * LUM_GREEN +
                                b[p] * LUM_BLUE;

                        int s = (int)(255.0f * lum);

                        r[p] *= scale[s];
                        g[p] *= scale[s];
                        b[p] *= scale[s];
                }
        }
#endif /* _HAS_SIMD */
}
