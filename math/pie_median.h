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

#ifndef __PIE_MEDIAN_H__
#define __PIE_MEDIAN_H__

/**
 * Return the median of 4 floats.
 * @param pointer to a mutable array of floats.
 * @return median value.
 */
extern float pie_mth_med4(float*);

/**
 * Return the median of 6 floats.
 * @param pointer to a mutable array of floats.
 * @return median value.
 */
extern float pie_mth_med6(float*);

/**
 * Return the median of 9 floats.
 * @param pointer to a mutable array of floats.
 * @return median value.
 */
extern float pie_mth_med9(float*);

extern void pie_mth_sort2(float* restrict, float* restrict);

extern void pie_mth_sort3(float* restrict, float* restrict, float* restrict);

extern float pie_mth_min3(float, float, float);

extern float pie_mth_max3(float, float, float);


#endif /* __PIE_MEDIAN_H__ */
