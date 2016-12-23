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

#ifndef __PIE_EXPOS_H__
#define __PIE_EXPOS_H__

#include "../math/pie_point.h"

/**
 * Adjust exposure.
 * Exposure is adjusted in complete steps, i.e 0 is unchanged.
 * @param the red channel.
 * @param the green channel.
 * @param the blue channel.
 * @param the exposure adjustment.
 * @param width of image.
 * @param height of image.
 * @param row stride in memory.
 */
extern void pie_alg_expos(float*,
                          float*,
                          float*,
                          float,
                          int,
                          int,
                          int);

/**
 * Create an exposure correction curve. Used internally by pie_alg_expos.
 * @param array of points to store the control points in.
 * @param the desired exposure level, from -5 to 5.
 * @return void.
 */
extern void pie_alg_expos_curve(struct pie_point_2d[5],
                                float);

#endif /* __PIE_EXPOS_H__ */
