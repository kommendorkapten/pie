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
#include <netinet/in.h>
#include <string.h>
#include "../pie_types.h"

#ifdef _HAS_SIMD
# ifdef _HAS_SSE

void encode_rgba(unsigned char* restrict buf,
                 const struct bitmap_f32rgb* restrict img)
{
        __m128 coeff_scale = _mm_set1_ps(255.0f);
        int rem = img->width % 4;
        int stop = img->width - rem;
        uint32_t w = htonl(img->width);
        uint32_t h = htonl(img->height);
        float or[4];
        float og[4];
        float ob[4];

        memcpy(buf, &w, sizeof(uint32_t));
        buf += sizeof(uint32_t);
        memcpy(buf, &h, sizeof(uint32_t));
        buf += sizeof(uint32_t);

        for (int y = 0; y < img->height; y++)
        {
                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * img->row_stride + x;
                        __m128 r = _mm_load_ps(&img->c_red[p]);
                        __m128 g = _mm_load_ps(&img->c_green[p]);
                        __m128 b = _mm_load_ps(&img->c_blue[p]);

                        r = _mm_mul_ps(r, coeff_scale);
                        g = _mm_mul_ps(g, coeff_scale);
                        b = _mm_mul_ps(b, coeff_scale);

                        _mm_store_ps(or, r);
                        _mm_store_ps(og, g);
                        _mm_store_ps(ob, b);

                        *buf++ = (unsigned char)or[0];
                        *buf++ = (unsigned char)og[0];
                        *buf++ = (unsigned char)ob[0];
                        *buf++ = 255;
                        *buf++ = (unsigned char)or[1];
                        *buf++ = (unsigned char)og[1];
                        *buf++ = (unsigned char)ob[1];
                        *buf++ = 255;
                        *buf++ = (unsigned char)or[2];
                        *buf++ = (unsigned char)og[2];
                        *buf++ = (unsigned char)ob[2];
                        *buf++ = 255;
                        *buf++ = (unsigned char)or[3];
                        *buf++ = (unsigned char)og[3];
                        *buf++ = (unsigned char)ob[3];
                        *buf++ = 255;
                }

                for (int x = stop; x < img->width; x++)
                {
                        int p = y * img->row_stride + x;
                        unsigned char r, g, b;

                        r = (unsigned char)(img->c_red[p] * 255.0f);
                        g = (unsigned char)(img->c_green[p] * 255.0f);
                        b = (unsigned char)(img->c_blue[p] * 255.0f);
                        
                        *buf++ = r;
                        *buf++ = g;
                        *buf++ = b;
                        *buf++ = 255;
                }
        }        
        
}

# elif _HAS_ALTIVEC
#  error ALTIVET NOT IMPLEMENTED
# endif

#else

void encode_rgba(unsigned char* restrict buf,
                 const struct bitmap_f32rgb* restrict img)
{
        int stride = img->row_stride;
        uint32_t w = htonl(img->width);
        uint32_t h = htonl(img->height);

        memcpy(buf, &w, sizeof(uint32_t));
        buf += sizeof(uint32_t);
        memcpy(buf, &h, sizeof(uint32_t));
        buf += sizeof(uint32_t);

        for (int y = 0; y < img->height; y++)
        {
#if _USE_GAMMA_CONV > 0
                /* Convert to sRGB */
                linear_to_srgbv(img->c_red + y * stride,
                                img->width);
                linear_to_srgbv(img->c_green + y * stride,
                                img->width);
                linear_to_srgbv(img->c_blue + y * stride,
                                img->width);
#endif
                for (int x = 0; x < img->width; x++)
                {
                        int p = y * stride + x;
                        unsigned char r, g, b;
                        
                        r = (unsigned char)(img->c_red[p] * 255.0f);
                        g = (unsigned char)(img->c_green[p] * 255.0f);
                        b = (unsigned char)(img->c_blue[p] * 255.0f);
                        
                        *buf++ = r;
                        *buf++ = g;
                        *buf++ = b;
                        *buf++ = 255;
                }
        }        
}

#endif /* _HAS_SIMD */
