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

#ifndef __PIE_WHITE_H__
#define __PIE_WHITE_H__

/**
 * Adjust whiteness.
 * White is adjustet in steps from -1 to 1. 0 is unchagned.
 * @param the red channel.
 * @param the green channel.
 * @param the blue channel.
 * @param the exposure adjustment.
 * @param width of image.
 * @param height of image.
 * @param row stride in memory.
 */
extern void pie_alg_white(float* restrict,
                          float* restrict,
                          float* restrict,
                          float,
                          int,
                          int,
                          int);

#endif /* __PIE_WHITE_H__ */
