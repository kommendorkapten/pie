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

#ifndef __PIE_CSPACE_H__
#define __PIE_CSPACE_H__
#include <stddef.h>

/**
 * Apply gamma correction.
 * @param the channel value.
 * @param the gamma exponent.
 * @return the corrected value.
 */
extern float pie_alg_gamma(float, float);
/**
 * Apply gamma correction to an array of values.
 * @param the channel values.
 * @param the gamma exponent.
 * @param the number of samples in the array.
 * @return void.
 */
extern void pie_alg_gammav(float*, float, size_t);

/**
 * Convert from sRGB color space to linear color space.
 * @param sRGB channel value.
 * @return a linear channel value.
 */
extern float pie_alg_srgb_to_linear(float);
/**
 * Convert from sRGB color space to linear color space, vector.
 * @param sRGB channel values.
 * @param the number of samples in the array.
 * @return void.
 */
extern void pie_alg_srgb_to_linearv(float*, size_t);

/**
 * Convert from linear color space to sRGB color space.
 * @param a linear channel value.
 * @return sRGB channel value.
 */
extern float pie_alg_linear_to_srgb(float);

/**
 * Convert from linear color space to sRGB color space, vector.
 * @param a linear channel value.
 * @param the number of samples in the array.
 * @return void.
 */
extern void pie_alg_linear_to_srgbv(float*, size_t);

#endif /* __PIE_CSPACE_H__ */
