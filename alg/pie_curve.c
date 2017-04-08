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

#include "pie_curve.h"
#include "../math/pie_point.h"
#include "../pie_log.h"

int pie_alg_curve_get(float* out,
                      const struct pie_point_2d* p,
                      float x,
                      size_t len)
{
        for (size_t i = 0; i < len - 1; i++)
        {
                if (p[i].x <= x && p[i + 1].x >= x)
                {
                        *out = p[i + 1].y;
                        return 0;
                }
        }

        /* check for x ~ 1 */
        if (x > p[len - 1].x && (x - p[len - 1].x) < 0.01)
        {
                *out = p[len - 1].y;
                return 0;
        }

        PIE_WARN("No curve defined for input %f", x);
        PIE_WARN("Valid range are [%f, %f]", p[0].x, p[len - 1].x);
        
        return 1;
}

void pie_alg_curve_intp(struct pie_point_2d* restrict o,
                        const struct pie_point_2d* restrict b,
                        const struct pie_point_2d* restrict e,
                        int count,
                        float phi)
{
        for (int i = 0; i < count; i++)
        {
                float delta;

                delta = e[i].x - b[i].x;
                o[i].x = b[i].x + phi * delta;
                
                delta = e[i].y - b[i].y;
                o[i].y = b[i].y + phi * delta;
        }        
}
