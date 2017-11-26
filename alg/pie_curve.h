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

#include "../pie_types.h"

struct pie_point_2d;

/**
 * Apply a curve to a bitmap.
 * @param red channel.
 * @param green channel.
 * @param blue channel.
 * @param channel to apply curve to.
 * @param pointer to control points.
 * @param number of control points.
 * @param width of image in pixels.
 * @param height of image in pixls..
 * @param row stride in bytes.
 */
void pie_alg_curve(float* restrict,
                   float* restrict,
                   float* restrict,
                   enum pie_channel,
                   const struct pie_point_2d*,
                   int,
                   int,
                   int,
                   int);


/**
 * Get the Y value for a given X value.
 * Binary search is used to find best match.
 * If X is not inside range nothing is updated.
 * @param pointer to float to store data
 * @param pointer to a sorted (by x) array of points.
 * @param x value to search for.
 * @param number of points
 * @return 0 if value is found.
 */
extern int pie_alg_curve_get(float*,
                             const struct pie_point_2d*,
                             float,
                             int);

/**
 * Get the Y value for a given X value.
 * Linear scanning is executed to find best match.
 * If X is not inside range nothing is updated.
 * @param pointer to float to store data
 * @param pointer to a sorted (by x) array of points.
 * @param x value to search for.
 * @param number of points
 * @return 0 if value is found.
 */
extern int pie_alg_curve_get_scan(float*,
                                  const struct pie_point_2d*,
                                  float,
                                  int);

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
