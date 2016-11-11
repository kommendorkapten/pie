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

#ifndef __PIE_MATH_H__
#define __PIE_MATH_H__

/**
 * Calculate a value for a Gauss 2d distribution.
 * @param distance in x.
 * @param distance in y.
 * @param variance of the distribution.
 * @return the Gauss density value.
 */
extern float pie_gauss_2d(float dx, float dy, float);

#endif /* __PIE_MATH_H__ */
