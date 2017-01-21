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

#ifndef __PIE_UNSHARP_H__
#define __PIE_UNSHARP_H__

#include "../pie_types.h"

/*
 * Common values for sharpening:
 * screen:      Amount:      50% Radius: 0.5 Threshold: 2
 * portrait:    Amount:      75% Radius: 2.0 Threshold: 3
 * all-purpose: Amount:      85% Radius: 1.0 Threshold: 4
 * web:         Amount: 200-400% Radius: 0.3 Threshold: 0
 *
 * For local contrast enchancements:
 *              Amount:    5-20% Radius: 30-100 Threshold 2-4
 */

#define PIE_CLARITY_SCALE 0.01f

/**
 * Apply an unsharp mask to a bitmap.
 * @param the red channel.
 * @param the green channel.
 * @param the blue channel
 * @param unsharp parameters.
 * @param image width.
 * @param image height.
 * @param image row stride.
 * @return 0 on success. Non zero otherwise.
 */
extern int pie_unsharp(float* restrict,
                       float* restrict,
                       float* restrict,
                       const struct pie_unsharp_param*,
                       int,
                       int,
                       int);


#endif /* __PIE_UNSHARP_H__ */
