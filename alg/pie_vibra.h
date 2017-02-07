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

#ifndef __PIE_VIBRA_H__
#define __PIE_VIBRA_H__

/**
 * Alter the saturation of the image.
 * 0.0 creates a bw image
 * 0.5 reduces the vibrance to half.
 * 1.0 no changes.
 * 2.0 doubles the vibrance.
 * @param the red channel.
 * @param the green channel.
 * @param the blue channel
 * @param desired vibrance in range [0,2].
 * @param image width.
 * @param image height.
 * @param image row stride.
 * @reurn void.
 */
extern void pie_alg_vibra(float* restrict,
                          float* restrict,
                          float* restrict,
                          float,
                          int,
                          int,
                          int);

#endif /* __PIE_VIBRA_H__ */
