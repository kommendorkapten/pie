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
#include <netinet/in.h>
#include <string.h>
#include "../bm/pie_bm.h"
#include "pie_rgba.h"

void pie_enc_bm_rgba(unsigned char* restrict buf,
                     const struct pie_bm_f32rgb* restrict img,
                     enum pie_image_type type)
{
        uint32_t w = htonl(img->width);
        uint32_t h = htonl(img->height);
        uint32_t t = htonl((int)type);

        memcpy(buf, &t, sizeof(uint32_t));
        buf += sizeof(uint32_t);
        memcpy(buf, &w, sizeof(uint32_t));
        buf += sizeof(uint32_t);
        memcpy(buf, &h, sizeof(uint32_t));
        buf += sizeof(uint32_t);

#if _HAS_SIMD4
        int rem = img->width % 4;
        int stop = img->width - rem;
        float or[4];
        float og[4];
        float ob[4];
#else
        int stop = 0;
#endif

#if _HAS_SSE42
        __m128 scalev = _mm_set1_ps(255.0f);
#endif
#if _HAS_ALTIVEC
        vector float scalev = (vector float){255.0f, 255.0f, 255.0f, 255.0f};
        vector float zerov = (vector float){0.0f, 0.0f, 0.0f, 0.0f};
#endif

        for (int y = 0; y < img->height; y++)
        {

#if _HAS_SSE42
                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * img->row_stride + x;
                        __m128 rv = _mm_load_ps(&img->c_red[p]);
                        __m128 gv = _mm_load_ps(&img->c_green[p]);
                        __m128 bv = _mm_load_ps(&img->c_blue[p]);

                        rv = _mm_mul_ps(rv, scalev);
                        gv = _mm_mul_ps(gv, scalev);
                        bv = _mm_mul_ps(bv, scalev);

                        _mm_store_ps(or, rv);
                        _mm_store_ps(og, gv);
                        _mm_store_ps(ob, bv);

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

#elif _HAS_ALTIVEC

                for (int x = 0; x < stop; x += 4)
                {
                        int p = y * img->row_stride + x;
                        vector float rv = vec_ld(p, img->c_red);
                        vector float gv = vec_ld(p, img->c_green);
                        vector float bv = vec_ld(p, img->c_blue);

                        rv = vec_madd(rv, scalev, zerov);
                        gv = vec_madd(gv, scalev, zerov);
                        bv = vec_madd(bv, scalev, zerov);

                        vec_st(rv, 0, &or[0]);
                        vec_st(gv, 0, &og[0]);
                        vec_st(bv, 0, &ob[0]);

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

#endif

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
