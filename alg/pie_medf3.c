/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
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

#include <string.h>
#include "pie_medf3.h"
#include "../math/pie_median.h"

void pie_alg_medf3(float* restrict c,
                   float a,
                   float* restrict b,
                   int w,
                   int h,
                   int s)
{
        float p[9];
        (void)a;

        /* Top and bottom row */
        for (int x = 1; x < w - 1; x++)
        {
                /* y = 0 */
                p[0] = c[0 * s + x - 1];
                p[1] = c[0 * s + x];
                p[2] = c[0 * s + x + 1];
                p[3] = c[1 * s + x - 1];
                p[4] = c[1 * s + x];
                p[5] = c[1 * s + x + 1];
                b[x] = pie_mth_med6(p);

                /* y = h - 1 */
                p[0] = c[(h - 2) * s + x - 1];
                p[1] = c[(h - 2) * s + x];
                p[2] = c[(h - 2) * s + x + 1];
                p[3] = c[(h - 1) * s + x - 1];
                p[4] = c[(h - 1) * s + x];
                p[5] = c[(h - 1) * s + x + 1];
                b[(h - 1) * s + x] = pie_mth_med6(p);
        }

        for (int y = 1; y < h - 1; y++)
        {
                /* First column */
                p[0] = c[(y - 1) * s + 0];
                p[1] = c[(y - 1) * s + 1];
                p[2] = c[y * s + 0];
                p[3] = c[y * s + 1];
                p[4] = c[(y + 1) * s + 0];
                p[5] = c[(y + 1) * s + 1];
                b[y * s] = pie_mth_med6(p);


                for (int x = 1; x < w - 1; x++)
                {

                        p[0] = c[(y - 1) * s + x - 1];
                        p[1] = c[(y - 1) * s + x];
                        p[2] = c[(y - 1) * s + x + 1];
                        p[3] = c[y * s + x - 1];
                        p[4] = c[y * s + x];
                        p[5] = c[y * s + x + 1];
                        p[6] = c[(y + 1) * s + x - 1];
                        p[7] = c[(y + 1) * s + x];
                        p[8] = c[(y + 1) * s + x + 1];
                        b[y * s + x] = pie_mth_med9(p);
                }

                /* last column */
                p[0] = c[(y - 1) * s + w - 2];
                p[1] = c[(y - 1) * s + w - 1];
                p[2] = c[y * s + w - 2];
                p[3] = c[y * s + w - 1];
                p[4] = c[(y + 1) * s + w - 2];
                p[5] = c[(y + 1) * s + w - 1];
                b[y * s + w - 1] = pie_mth_med6(p);
        }

        /* Four corners */

        p[0] = c[0];
        p[1] = c[1];
        p[2] = c[s];
        p[3] = c[s + 1];
        b[0] = pie_mth_med4(p);

        p[0] = c[w - 2];
        p[1] = c[w - 1];
        p[2] = c[s + w - 2];
        p[3] = c[s + w - 1];
        b[w - 1] = pie_mth_med4(p);

        p[0] = c[(h - 2) * s];
        p[1] = c[(h - 2) * s + 1];
        p[2] = c[(h - 1) * s];
        p[3] = c[(h - 1) * s + 1];
        b[(h - 1) * s] = pie_mth_med4(p);

        p[0] = c[(h - 2) * s + w - 2];
        p[1] = c[(h - 2) * s + w - 1];
        p[2] = c[(h - 1) * s + w - 2];
        p[3] = c[(h - 1) * s + w - 1];
        b[(h - 1) * s + w - 1] = pie_mth_med4(p);

        /* Copy result back into c */
        memcpy(c, b, h * s * sizeof(float));
}
