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

#ifndef __PIE_CURVE_H__
#define __PIE_CURVE_H__

#include <stddef.h>

struct pie_point_2d;

/**
 * Get the Y value for a given X value.
 * If X is not inside range nothing is updated.
 * @param pointer to float to store data
 * @param pointer to a sorted (by x) array of points.
 * @param number of points
 * @return 0 if value is found.
 */
extern int pie_alg_curve_get(float*,
                             const struct pie_point_2d*,
                             float,
                             size_t);

/**
 * Linear interpolate curve parameters.
 * @param output parameters.
 * @param start parameters.
 * @param end parameters.
 * @param number of points in curve.
 * @param percentage from beginning to end.
 * @return void.
 */
void pie_alg_curve_intp(struct pie_point_2d* restrict,
                        const struct pie_point_2d* restrict,
                        const struct pie_point_2d* restrict,
                        int,
                        float);

#endif /* __PIE_CURVE_H__ */
