/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
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

#ifndef __PIE_MEDF3_H__
#define __PIE_MEDF3_H__

/**
 * Apply a 3x3 median filter to a channel.
 * @param pointer to channel data.
 * @param amount [0-1].
 * @param temporary buffer large enough to hold the entire channel.
 * @param width of image.
 * @param height of image.
 * @param row stride.
 * @return void.
 */
extern void pie_alg_medf3(float* restrict,
                          float,
                          float* restrict,
                          int,
                          int,
                          int);

#endif /* __PIE_MEDF3_H__ */
