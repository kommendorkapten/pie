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

/* Six pass box blur, see more here
   http://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf */ 

#include <math.h>
#include <string.h>

/**
 * @param storage to put n boxes in.
 * @param sigma of gauss function to estimate.
 * @param number of boxes to create.
 */
static void gauss_boxes(float*, float, int);
static void pie_box_blur4(float* restrict img,
                          float* restrict buf,
                          float sigma,
                          int w,
                          int h,
                          int s);
static void pie_box_blur_h4(float* restrict tgt,
                            float* restrict src,
                            float sigma,
                            int w,
                            int h,
                            int s);
static void pie_box_blur_t4(float* restrict tgt,
                            float* restrict src,
                            float sigma,
                            int w,
                            int h,
                            int s);

void pie_box_blur6(float* restrict img,
                   float* restrict buf,
                   float sigma,
                   int w,
                   int h,
                   int s)
{
        float boxes[3];

        gauss_boxes(&boxes[0], sigma, 3);
        pie_box_blur4(img, buf, (boxes[0] - 1.0f) / 2.0f, w, h, s);
        pie_box_blur4(img, buf, (boxes[1] - 1.0f) / 2.0f, w, h, s);
        pie_box_blur4(img, buf, (boxes[2] - 1.0f) / 2.0f, w, h, s);
}

static void gauss_boxes(float* boxes, float sigma, int n)
{
        /*Ideal averaging filter width */
        float ideal_width;
        float ideal_m;
        int m;
        int wl;
        int wu;
        float var = sigma * sigma;
        
        ideal_width = sqrtf((12.0f * var / (float)n) + 1.0f);
        wl = (int)ideal_width;
        
        if ((wl & 0x1) == 0)
        {
                wl--;
        }
        wu = (float)wl + 2.0f;
        
        ideal_m = 12.0f * var;
        ideal_m -= (float)(n * wl * wl);
        ideal_m -= (float)(4 * n * wl);
        ideal_m -= (float)(3 * n);
        ideal_m /= (-4.0f * (float)wl - 4.0f);
        
        m = (int)(ideal_m + 0.5f);
        
        for (int i = 0; i < n; i++)
        {
                boxes[i] = ( i < m ? wl : wu);
        }
        
        return;
}

static void pie_box_blur4(float* restrict img,
                          float* restrict buf,
                          float sigma,
                          int w,
                          int h,
                          int s)
{
        pie_box_blur_h4(buf, img, sigma, w, h, s);
        pie_box_blur_t4(img, buf, sigma, w, h, s);
}

static void pie_box_blur_h4(float* restrict tgt,
                            float* restrict src,
                            float sigma,
                            int w,
                            int h,
                            int s)
{
        float iarr = 1.0f / (2.0f * sigma + 1.0f);
#if 1
        int radius = (int)(sigma + 0.5f);
#else
        int radius = (int)sigma;
#endif
    
        for (int i = 0; i < h; i++)
        {
                int ti = i * s; /* int, start of row */
                int li = ti;  /* int, start of row */
                int ri = ti + radius;
                
                float fv = src[ti];
                float lv = src[ti + w - 1];
                float val = (sigma + 1.0f) * fv;
            
                for (int j = 0; j < radius; j++)
                {
                        val += src[ti + j];
                }
                for (int j = 0; j <= radius; j++)
                {
                        val += src[ri++] - fv;
                        tgt[ti++] = val * iarr;
                }
                for (int j = radius + 1; j < w - radius; j++)
                {
                        val += src[ri++] - src[li++];
                        tgt[ti++] = val * iarr;
                }
                for (int j = w - radius; j < w; j++)
                {
                        val += lv - src[li++];
                        tgt[ti++] = val * iarr;
                }
        }
}

static void pie_box_blur_t4(float* restrict tgt,
                            float* restrict src,
                            float sigma,
                            int w,
                            int h,
                            int s)
{
        float iarr = 1.0f / (2.0f * sigma + 1.0f);
#if 1
        int radius = (int)(sigma + 0.5f);
#else
        int radius = (int)sigma;        
#endif
    
        for (int i = 0; i < w; i++)
        {
                int ti = i;
                int li = ti;
                int ri = ti + radius * s;
                float fv = src[ti];
                float lv = src[ti + s * (h - 1)];
                float val = (sigma + 1.0f) * fv;
            
                for (int j = 0; j < radius; j++)
                {
                        val += src[ti + j * s];
                }
                for (int j = 0; j <= radius; j++)
                {
                        val += src[ri] - fv;
                        tgt[ti] = val * iarr;
                        ri += s;
                        ti += s;
                }

                for (int j = radius + 1; j < h - radius; j++)
                {
                        /* this row is unfriendly to the cache */
                        val += src[ri] - src[li];
                        tgt[ti] = val * iarr;
                        li += s;
                        ri += s;
                        ti += s;
                }

                for (int j = h - radius; j < h; j++)
                {
                        val += lv - src[li];
                        tgt[ti] = val * iarr;
                        li += s;
                        ti += s;
                }
        }
}
