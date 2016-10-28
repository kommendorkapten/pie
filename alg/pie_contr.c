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

#include <assert.h>

/* 
   c is amount of contrast int [0, 2]
   c [0, 1] less contrast
   c [1,+] more contrast
*/
void pie_alg_contr(float* img,
                   float c,
                   unsigned int w,
                   unsigned int h,
                   unsigned int stride)
{
        assert(c >= 0.0f);
        assert(c <= 2.0f);

/*        return c(in - 0.5f) + 0.5f + b; */
        for (unsigned int y = 0; y < h; y++)
        {
                for (unsigned int x = 0; x < w; x++)
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
