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

#ifndef __PIE_CATMULL_H__
#define __PIE_CATMULL_H__

#include "pie_point.h"

/**
 * Centripetal Catmull-Rom spline calculation.
 * @param pointer to store generated curve.
 * @param pointer to four control points (order P0, P1, P2, P3).
 *        Curve is drawn between P1 and P2 (including points).
 * @param number of points to draw.
 * @return void
 */
extern void pie_catm_rom(struct pie_point_2d*,
                         const struct pie_point_2d*,
                         int);

/**
 * Chain centripetal Catmull-Rom splines.
 * Connecting points will be duplicated.
 * @param pointer to store generated curve. Buffer must be able to store
 *        number of splines * number of points elements.
 *        i.e for p0, p1, p2, p3, p4 and 25 points per segment, 2 * 25 
 *        points will be generated (p0-p1,p1-p2).
 * @param pointer to four control points, including two control points.
 *        The first point is the first control point, and the last point
 *        is the last control point.
 * @param number of points (including control points).
 * @param number of points to draw per segment.
 * @return void
 */
extern void pie_catm_rom_chain(struct pie_point_2d*,
                               const struct pie_point_2d*,
                               int,
                               int);

#endif /* __PIE_CATMULL_H__ */
