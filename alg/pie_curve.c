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

#include "../math/pie_point.h"
#include "pie_curve.h"

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
        if (x > p[len - 1].x && (x - p[len - 1].x) < 0.005)
        {
                *out = p[len - 1].y;
                return 0;
        }
        
        return 1;
}
