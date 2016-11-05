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

        /* Percieved brightness is not linear, but follow a gamma of 
           2.33. Calculate the linear factor to apply.
           The relationship is virtually linear across the complete 
           luma scale [0,1], so just take a number and calculate. */

        e = powf(2.0f, e);
        float l = powf(0.5f, GAMMA_C);
        l *= e;
        l = powf(l, (1.0f / GAMMA_C));
        /* Get the factor the color increased */
        l = l / 0.5f;

        /* Calculate a table per luminosity level to get a smoother
           exposure compensation, that avoids blowing out highlights */
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
                        if (s > 255)
                        {
                                s = 255;
                        }
                        r[p] *= scale[s];
                        g[p] *= scale[s];
                        b[p] *= scale[s];
                }
        }
}
