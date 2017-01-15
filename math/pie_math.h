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

#include <stddef.h>

/**
 * Calculate a value for a Gauss distribution.
 * @param distance in x.
 * @param variance of the distribution.
 * @return the Gauss density value.
 */
extern float pie_gauss(float, float);

/**
 * Calculate a value for a Gauss 2d distribution.
 * @param distance in x.
 * @param distance in y.
 * @param variance of the distribution.
 * @return the Gauss density value.
 */
extern float pie_gauss_2d(float, float, float);

/**
 * Create a square Gaussian blur matrix. Matrix will be row dominant.
 * The created matrix will be normalized (sum of all elements equal. one).
 * @param output matrix to store data in, must be at least (len*len) in size.
 * @param the size for a row/column.
 * @param the variance of the distribution.
 * @return void.
 */
extern void pie_gauss_matrix(float*,
                             size_t,
                             float);

/**
 * Print a square matrix to std out.
 * @param matrix to print.
 * @param num elements for each row/column.
 * @return void.
 */
extern void pie_matrix_print(float*,
                             size_t);

#endif /* __PIE_MATH_H__ */
