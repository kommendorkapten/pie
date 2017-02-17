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

#ifndef __PIE_COLORT_H__
#define __PIE_COLORT_H__

/**
 * Simple color temperature adjustments.
 * Adj and tint are in ranges [-0.5, 0.5].
 * @param the red channel.
 * @param the green channel.
 * @param the blue channel.
 * @param color temperature adjustment.
 * @param ting adjustment.
 * @param image width.
 * @param image height.
 * @param image row stride.
 * @reurn void.
 */
extern void pie_alg_color_temp(float* restrict,
                               float* restrict,
                               float* restrict,
                               float,
                               float,
                               int,
                               int,
                               int);

#endif /* __PIE_COLORT_H__ */
