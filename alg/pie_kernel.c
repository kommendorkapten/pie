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

#include "pie_kernel.h"
#include "../math/pie_math.h"
#include <string.h>

void pie_kernel3x3_apply(float* c,
                         struct pie_kernel3x3* k,
                         float* buf,
                         unsigned int w,
                         unsigned int h,
                         unsigned int s)
{
        /* Repeat the edge pixels */

        /* Top and bottom row */
        for (unsigned int x = 1; x < w - 1; x++)
        {
                /* y = 0 */
                buf[x] = 
                        k->v[0] * c[0 * s + x - 1] +
                        k->v[1] * c[0 * s + x] +
                        k->v[2] * c[0 * s + x + 1] +
                        k->v[3] * c[0 * s + x - 1] +
                        k->v[4] * c[0 * s + x] +
                        k->v[5] * c[0 * s + x + 1] +
                        k->v[6] * c[1 * s + x - 1] +
                        k->v[7] * c[1 * s + x] +
                        k->v[8] * c[1 * s + x + 1];

                /* y = h - 1 */
                buf[(h - 1) * s + x] = 
                        k->v[0] * c[(h - 2) * s + x - 1] +
                        k->v[1] * c[(h - 2) * s + x] +
                        k->v[2] * c[(h - 2) * s + x + 1] +
                        k->v[3] * c[(h - 1) * s + x - 1] +
                        k->v[4] * c[(h - 1) * s + x] +
                        k->v[5] * c[(h - 1) * s + x + 1] +
                        k->v[6] * c[(h - 1) * s + x - 1] +
                        k->v[7] * c[(h - 1) * s + x] +
                        k->v[8] * c[(h - 1) * s + x + 1];
        }

        for (unsigned int y = 1; y < h - 1; y++)
        {
                /* First column */
                buf[y * s] = 
                        k->v[0] * c[(y - 1) * s + 0] +
                        k->v[1] * c[(y - 1) * s + 0] +
                        k->v[2] * c[(y - 1) * s + 1] +
                        k->v[3] * c[y * s + 0] +
                        k->v[4] * c[y * s + 0] +
                        k->v[5] * c[y * s + 1] +
                        k->v[6] * c[(y + 1) * s + 0] +
                        k->v[7] * c[(y + 1) * s + 0] +
                        k->v[8] * c[(y + 1) * s + 1];

                for (unsigned int x = 1; x < w - 1; x++)
                {
                        buf[y * s + x] = 
                                k->v[0] * c[(y - 1) * s + x - 1] +
                                k->v[1] * c[(y - 1) * s + x] +
                                k->v[2] * c[(y - 1) * s + x + 1] +
                                k->v[3] * c[y * s + x - 1] +
                                k->v[4] * c[y * s + x] +
                                k->v[5] * c[y * s + x + 1] +
                                k->v[6] * c[(y + 1) * s + x - 1] +
                                k->v[7] * c[(y + 1) * s + x] +
                                k->v[8] * c[(y + 1) * s + x + 1];
                }

                /* last column */
                buf[y * s + w - 1] = 
                        k->v[0] * c[(y - 1) * s + w - 2] +
                        k->v[1] * c[(y - 1) * s + w - 1] +
                        k->v[2] * c[(y - 1) * s + w - 1] +
                        k->v[3] * c[y * s + w - 2] +
                        k->v[4] * c[y * s + w - 1] +
                        k->v[5] * c[y * s + w - 1] +
                        k->v[6] * c[(y + 1) * s + w - 2] +
                        k->v[7] * c[(y + 1) * s + w - 1] +
                        k->v[8] * c[(y + 1) * s + w - 1];
        }
        
        /* Four corners */
        buf[0] = 
                k->v[0] * c[0] +
                k->v[1] * c[0] +
                k->v[2] * c[1] +
                k->v[3] * c[0] +
                k->v[4] * c[0] +
                k->v[5] * c[1] +
                k->v[6] * c[s] +
                k->v[7] * c[s] +
                k->v[8] * c[s + 1];

        buf[w - 1] = 
                k->v[0] * c[w - 2] +
                k->v[1] * c[w - 1] +
                k->v[2] * c[w - 1] +
                k->v[3] * c[w - 2] +
                k->v[4] * c[w - 1] +
                k->v[5] * c[w - 1] +
                k->v[6] * c[s + w - 2] +
                k->v[7] * c[s + w - 1] +
                k->v[8] * c[s + w - 1];

        buf[(h - 1) * s] = 
                k->v[0] * c[(h - 2) * s] +
                k->v[1] * c[(h - 2) * s] +
                k->v[2] * c[(h - 2) * s + 1] +
                k->v[3] * c[(h - 1) * s] +
                k->v[4] * c[(h - 1) * s] +
                k->v[5] * c[(h - 1) * s + 1] +
                k->v[6] * c[(h - 1) * s] +
                k->v[7] * c[(h - 1) * s] +
                k->v[8] * c[(h - 1) * s + 1];

        buf[(h - 1) * s + w - 1] = 
                k->v[0] * c[(h - 2) * s + w - 2] +
                k->v[1] * c[(h - 2) * s + w - 1] +
                k->v[2] * c[(h - 2) * s + w - 1] +
                k->v[3] * c[(h - 1) * s + w - 2] +
                k->v[4] * c[(h - 1) * s + w - 1] +
                k->v[5] * c[(h - 1) * s + w - 1] +
                k->v[6] * c[(h - 1) * s + w - 2] +
                k->v[7] * c[(h - 1) * s + w - 1] +
                k->v[8] * c[(h - 1) * s + w - 1];

        /* Copy result back into c */
        memcpy(c, buf, h * s * sizeof(float));
}

void pie_kernel5x5_apply(float* c,
                         struct pie_kernel5x5* k,
                         float* buf,
                         unsigned int w,
                         unsigned int h,
                         unsigned int s)
{
        /* Repeat the edge pixels */
        for (unsigned int x = 2; x < w - 2; x++)
        {
                /* y = 0 */
                buf[x] = 
                        k->v[0] * c[0 * s + x - 2] +
                        k->v[1] * c[0 * s + x - 1] +
                        k->v[2] * c[0 * s + x] +    
                        k->v[3] * c[0 * s + x + 1] + 
                        k->v[4] * c[0 * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[0 * s + x - 2] +
                        k->v[6] * c[0 * s + x - 1] +
                        k->v[7] * c[0 * s + x] +    
                        k->v[8] * c[0 * s + x + 1] +
                        k->v[9] * c[0 * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[0 * s + x - 2] +
                        k->v[11] * c[0 * s + x - 1] +
                        k->v[12] * c[0 * s + x] +    
                        k->v[13] * c[0 * s + x + 1] +
                        k->v[14] * c[0 * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[1 * s + x - 2] +
                        k->v[16] * c[1 * s + x - 1] +
                        k->v[17] * c[1 * s + x] +
                        k->v[18] * c[1 * s + x + 1] +
                        k->v[19] * c[1 * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[2 * s + x - 2] +
                        k->v[21] * c[2 * s + x - 1] +
                        k->v[22] * c[2 * s + x] +    
                        k->v[23] * c[2 * s + x + 1] +
                        k->v[24] * c[2 * s + x + 2];

                /* y = 1 */
                buf[s + x] = 
                        k->v[0] * c[0 * s + x - 2] +
                        k->v[1] * c[0 * s + x - 1] +
                        k->v[2] * c[0 * s + x] +    
                        k->v[3] * c[0 * s + x + 1] + 
                        k->v[4] * c[0 * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[0 * s + x - 2] +
                        k->v[6] * c[0 * s + x - 1] +
                        k->v[7] * c[0 * s + x] +    
                        k->v[8] * c[0 * s + x + 1] +
                        k->v[9] * c[0 * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[1 * s + x - 2] +
                        k->v[11] * c[1 * s + x - 1] +
                        k->v[12] * c[1 * s + x] +    
                        k->v[13] * c[1 * s + x + 1] +
                        k->v[14] * c[1 * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[2 * s + x - 2] +
                        k->v[16] * c[2 * s + x - 1] +
                        k->v[17] * c[2 * s + x] +
                        k->v[18] * c[2 * s + x + 1] +
                        k->v[19] * c[2 * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[3 * s + x - 2] +
                        k->v[21] * c[3 * s + x - 1] +
                        k->v[22] * c[3 * s + x] +    
                        k->v[23] * c[3 * s + x + 1] +
                        k->v[24] * c[3 * s + x + 2];

                /* y = h - 2 */
                buf[(h - 2) * s + x] =                 
                        k->v[0] * c[(h - 4) * s + x - 2] +
                        k->v[1] * c[(h - 4) * s + x - 1] +
                        k->v[2] * c[(h - 4) * s + x] +    
                        k->v[3] * c[(h - 4) * s + x + 1] + 
                        k->v[4] * c[(h - 4) * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[(h - 3) * s + x - 2] +
                        k->v[6] * c[(h - 3) * s + x - 1] +
                        k->v[7] * c[(h - 3) * s + x] +    
                        k->v[8] * c[(h - 3) * s + x + 1] +
                        k->v[9] * c[(h - 3) * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[(h - 2) * s + x - 2] +
                        k->v[11] * c[(h - 2) * s + x - 1] +
                        k->v[12] * c[(h - 2) * s + x] +    
                        k->v[13] * c[(h - 2) * s + x + 1] +
                        k->v[14] * c[(h - 2) * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[(h - 1) * s + x - 2] +
                        k->v[16] * c[(h - 1) * s + x - 1] +
                        k->v[17] * c[(h - 1) * s + x] +
                        k->v[18] * c[(h - 1) * s + x + 1] +
                        k->v[19] * c[(h - 1) * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[(h - 1) * s + x - 2] +
                        k->v[21] * c[(h - 1) * s + x - 1] +
                        k->v[22] * c[(h - 1) * s + x] +    
                        k->v[23] * c[(h - 1) * s + x + 1] +
                        k->v[24] * c[(h - 1) * s + x + 2];
                
                /* y = h - 1 */
                buf[(h - 1) * s + x] = 
                        k->v[0] * c[(h - 3) * s + x - 2] +
                        k->v[1] * c[(h - 3) * s + x - 1] +
                        k->v[2] * c[(h - 3) * s + x] +    
                        k->v[3] * c[(h - 3) * s + x + 1] + 
                        k->v[4] * c[(h - 3) * s + x + 2] +
                        /* 1 */
                        k->v[5] * c[(h - 2) * s + x - 2] +
                        k->v[6] * c[(h - 2) * s + x - 1] +
                        k->v[7] * c[(h - 2) * s + x] +    
                        k->v[8] * c[(h - 2) * s + x + 1] +
                        k->v[9] * c[(h - 2) * s + x + 2] +
                        /* 2 */
                        k->v[10] * c[(h - 1) * s + x - 2] +
                        k->v[11] * c[(h - 1) * s + x - 1] +
                        k->v[12] * c[(h - 1) * s + x] +    
                        k->v[13] * c[(h - 1) * s + x + 1] +
                        k->v[14] * c[(h - 1) * s + x + 2] +
                        /* 3 */
                        k->v[15] * c[(h - 1) * s + x - 2] +
                        k->v[16] * c[(h - 1) * s + x - 1] +
                        k->v[17] * c[(h - 1) * s + x] +
                        k->v[18] * c[(h - 1) * s + x + 1] +
                        k->v[19] * c[(h - 1) * s + x + 2] +
                        /* 4 */
                        k->v[20] * c[(h - 1) * s + x - 2] +
                        k->v[21] * c[(h - 1) * s + x - 1] +
                        k->v[22] * c[(h - 1) * s + x] +    
                        k->v[23] * c[(h - 1) * s + x + 1] +
                        k->v[24] * c[(h - 1) * s + x + 2];
        }

        for (unsigned int y = 2; y < h - 2; y++)
        {
                /* First column, x = 0 */
                buf[y * s] = 
                        k->v[0] * c[(y - 2) * s + 0] +
                        k->v[1] * c[(y - 2) * s + 0] +
                        k->v[2] * c[(y - 2) * s + 0] +    
                        k->v[3] * c[(y - 2) * s + 1] + 
                        k->v[4] * c[(y - 2) * s + 2] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + 0] +
                        k->v[6] * c[(y - 1) * s + 0] +
                        k->v[7] * c[(y - 1) * s + 0] +    
                        k->v[8] * c[(y - 1) * s + 1] +
                        k->v[9] * c[(y - 1) * s + 2] +
                        /* 2 */
                        k->v[10] * c[y * s + 0] +
                        k->v[11] * c[y * s + 0] +
                        k->v[12] * c[y * s + 0] +    
                        k->v[13] * c[y * s + 1] +
                        k->v[14] * c[y * s + 2] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + 0] +
                        k->v[16] * c[(y + 1) * s + 0] +
                        k->v[17] * c[(y + 1) * s + 0] +
                        k->v[18] * c[(y + 1) * s + 1] +
                        k->v[19] * c[(y + 1) * s + 2] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + 0] +
                        k->v[21] * c[(y + 2) * s + 0] +
                        k->v[22] * c[(y + 2) * s + 0] +    
                        k->v[23] * c[(y + 2) * s + 1] +
                        k->v[24] * c[(y + 2) * s + 2];
                
                /* Second column, x = 1 */
                buf[y * s + 1] = 
                        k->v[0] * c[(y - 2) * s + 0] +
                        k->v[1] * c[(y - 2) * s + 0] +
                        k->v[2] * c[(y - 2) * s + 1] +    
                        k->v[3] * c[(y - 2) * s + 2] + 
                        k->v[4] * c[(y - 2) * s + 3] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + 0] +
                        k->v[6] * c[(y - 1) * s + 0] +
                        k->v[7] * c[(y - 1) * s + 1] +    
                        k->v[8] * c[(y - 1) * s + 2] +
                        k->v[9] * c[(y - 1) * s + 3] +
                        /* 2 */
                        k->v[10] * c[y * s + 0] +
                        k->v[11] * c[y * s + 0] +
                        k->v[12] * c[y * s + 1] +    
                        k->v[13] * c[y * s + 2] +
                        k->v[14] * c[y * s + 3] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + 0] +
                        k->v[16] * c[(y + 1) * s + 0] +
                        k->v[17] * c[(y + 1) * s + 1] +
                        k->v[18] * c[(y + 1) * s + 2] +
                        k->v[19] * c[(y + 1) * s + 3] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + 0] +
                        k->v[21] * c[(y + 2) * s + 0] +
                        k->v[22] * c[(y + 2) * s + 1] +    
                        k->v[23] * c[(y + 2) * s + 2] +
                        k->v[24] * c[(y + 2) * s + 3];

                for (unsigned int x = 2; x < w - 2; x++)
                {
                        buf[y * s + x] = 
                                k->v[0] * c[(y - 2) * s + x - 2] +
                                k->v[1] * c[(y - 2) * s + x - 1] +
                                k->v[2] * c[(y - 2) * s + x] +    
                                k->v[3] * c[(y - 2) * s + x + 1] + 
                                k->v[4] * c[(y - 2) * s + x + 2] +
                                /* 1 */
                                k->v[5] * c[(y - 1) * s + x - 2] +
                                k->v[6] * c[(y - 1) * s + x - 1] +
                                k->v[7] * c[(y - 1) * s + x] +    
                                k->v[8] * c[(y - 1) * s + x + 1] +
                                k->v[9] * c[(y - 1) * s + x + 2] +
                                /* 2 */
                                k->v[10] * c[y * s + x - 2] +
                                k->v[11] * c[y * s + x - 1] +
                                k->v[12] * c[y * s + x] +    
                                k->v[13] * c[y * s + x + 1] +
                                k->v[14] * c[y * s + x + 2] +
                                /* 3 */
                                k->v[15] * c[(y + 1) * s + x - 2] +
                                k->v[16] * c[(y + 1) * s + x - 1] +
                                k->v[17] * c[(y + 1) * s + x] +
                                k->v[18] * c[(y + 1) * s + x + 1] +
                                k->v[19] * c[(y + 1) * s + x + 2] +
                                /* 4 */
                                k->v[20] * c[(y + 2) * s + x - 2] +
                                k->v[21] * c[(y + 2) * s + x - 1] +
                                k->v[22] * c[(y + 2) * s + x] +    
                                k->v[23] * c[(y + 2) * s + x + 1] +
                                k->v[24] * c[(y + 2) * s + x + 2];
                }

                /* second last column, x = w - 2 */
                buf[y * s + w - 2] = 
                        k->v[0] * c[(y - 2) * s + w - 4] +
                        k->v[1] * c[(y - 2) * s + w - 3] +
                        k->v[2] * c[(y - 2) * s + w - 2] +    
                        k->v[3] * c[(y - 2) * s + w - 1] + 
                        k->v[4] * c[(y - 2) * s + w - 1] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + w - 4] +
                        k->v[6] * c[(y - 1) * s + w - 3] +
                        k->v[7] * c[(y - 1) * s + w - 2] +    
                        k->v[8] * c[(y - 1) * s + w - 1] +
                        k->v[9] * c[(y - 1) * s + w - 1] +
                        /* 2 */
                        k->v[10] * c[y * s + w - 4] +
                        k->v[11] * c[y * s + w - 3] +
                        k->v[12] * c[y * s + w - 2] +    
                        k->v[13] * c[y * s + w - 1] +
                        k->v[14] * c[y * s + w - 1] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + w - 4] +
                        k->v[16] * c[(y + 1) * s + w - 3] +
                        k->v[17] * c[(y + 1) * s + w - 2] +
                        k->v[18] * c[(y + 1) * s + w - 1] +
                        k->v[19] * c[(y + 1) * s + w - 1] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + w - 4] +
                        k->v[21] * c[(y + 2) * s + w - 3] +
                        k->v[22] * c[(y + 2) * s + w - 2] +
                        k->v[23] * c[(y + 2) * s + w - 1] +
                        k->v[24] * c[(y + 2) * s + w - 1];
                
                /* last column, x = w - 1 */
                buf[y * s + w - 1] = 
                        k->v[0] * c[(y - 2) * s + w - 3] +
                        k->v[1] * c[(y - 2) * s + w - 2] +
                        k->v[2] * c[(y - 2) * s + w - 1] +    
                        k->v[3] * c[(y - 2) * s + w - 1] + 
                        k->v[4] * c[(y - 2) * s + w - 1] +
                        /* 1 */
                        k->v[5] * c[(y - 1) * s + w - 3] +
                        k->v[6] * c[(y - 1) * s + w - 2] +
                        k->v[7] * c[(y - 1) * s + w - 1] +    
                        k->v[8] * c[(y - 1) * s + w - 1] +
                        k->v[9] * c[(y - 1) * s + w - 1] +
                        /* 2 */
                        k->v[10] * c[y * s + w - 3] +
                        k->v[11] * c[y * s + w - 2] +
                        k->v[12] * c[y * s + w - 1] +    
                        k->v[13] * c[y * s + w - 1] +
                        k->v[14] * c[y * s + w - 1] +
                        /* 3 */
                        k->v[15] * c[(y + 1) * s + w - 3] +
                        k->v[16] * c[(y + 1) * s + w - 2] +
                        k->v[17] * c[(y + 1) * s + w - 1] +
                        k->v[18] * c[(y + 1) * s + w - 1] +
                        k->v[19] * c[(y + 1) * s + w - 1] +
                        /* 4 */
                        k->v[20] * c[(y + 2) * s + w - 3] +
                        k->v[21] * c[(y + 2) * s + w - 2] +
                        k->v[22] * c[(y + 2) * s + w - 1] +    
                        k->v[23] * c[(y + 2) * s + w - 1] +
                        k->v[24] * c[(y + 2) * s + w - 1];
        }
        
        /* "Corners", use the naive algoritm here */
        int ay[4] = {0, 1, h - 2, h - 1};
        int ax[4] = {0, 1, w - 2, w - 1};
        for (int iy = 0; iy < 4; iy++)
        {
                int y = ay[iy];
                
                for (int ix = 0; ix < 4; ix++)
                {
                        int x = ax[ix];
                        
                        buf[y * s + x] = 0;
                        
                        for (int ky = 0; ky < 5; ky++)
                        {
                                int py = y + ky - 2;

                                if (py < 0)
                                {
                                        py = 0;
                                }
                                else if (py > h - 1)
                                {
                                        py = h - 1;
                                }
                                
                                for (int kx = 0; kx < 5; kx++)
                                {
                                        int px = x + kx - 2;

                                        if (px < 0)
                                        {
                                                px = 0;
                                        }
                                        else if (px > w - 1)
                                        {
                                                px = w - 1;
                                        }

                                        buf[y * s + x] +=
                                                c[py * s + px] *
                                                k->v[ky * 5 + kx];
                                }
                        }
                }
        }

        /* Copy result back into c */
        memcpy(c, buf, h * s * sizeof(float));
}

/* Generic kernel a x a */
void pie_kernel5x5_apply1(float* c,
                         struct pie_kernel5x5* k,
                         float* buf,
                         unsigned int w,
                         unsigned int h,
                         unsigned int s)
{
        for (int y = 0; y < h; y++)
        {
                for (int x = 0; x < w; x++)
                {
                        buf[y * s + x] = 0;
                        
                        for (int ky = 0; ky < 5; ky++)
                        {
                                int py = y + ky - 2;

                                if (py < 0)
                                {
                                        py = 0;
                                }
                                else if (py > h - 1)
                                {
                                        py = h - 1;
                                }
                                
                                for (int kx = 0; kx < 5; kx++)
                                {
                                        int px = x + kx - 2;

                                        if (px < 0)
                                        {
                                                px = 0;
                                        }
                                        else if (px > w - 1)
                                        {
                                                px = w - 1;
                                        }

                                        buf[y * s + x] +=
                                                c[py * s + px] *
                                                k->v[ky * 5 + kx];
                                }
                        }
                }
        }
        /* Copy result back into c */
        memcpy(c, buf, h * s * sizeof(float));        
}

void pie_kernel3x3_gauss(struct pie_kernel3x3* k,
                         float var)
{
        float sum = 0;
        
        for (int y = 0; y < 3; y++)
        {
                for (int x = 0; x < 3; x++)
                {
                        float dx = (float)(x - 1);
                        float dy = (float)(y - 1);

                        k->v[y * 3 + x] = pie_gauss_2d(dx, dy, var);
                        sum += k->v[y * 3 + x];
                }
        }

        for (int i = 0; i < 9; i++)
        {
                k->v[i] /= sum;
        }        
}

void pie_kernel5x5_gauss(struct pie_kernel5x5* k,
                         float var)
{
        float sum = 0;

        for (int y = 0; y < 5; y++)
        {
                for (int x = 0; x < 5; x++)
                {
                        float dx = (float)(x - 2);
                        float dy = (float)(y - 2);

                        k->v[y * 5 + x] = pie_gauss_2d(dx, dy, var);
                        sum += k->v[y * 5 + x];
                }
        }

        /* Normalize */
        for (int i = 0; i < 25; i++)
        {
                k->v[i] /= sum;
        }
}

