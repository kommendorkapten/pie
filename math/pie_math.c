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

#include "pie_math.h"
#include <math.h>

float pie_gauss_2d(float dx, float dy, float var)
{
        float ret;
        float numerator = (dx * dx) + (dy * dy);
        float denominator = 2 * var;
        float c1;
        float c2;

        c1 = expf(- (numerator / denominator));
        c2 = (2 * M_PI * var);

        ret = c1 / c2;

        return ret;
}
