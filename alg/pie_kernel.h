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

#ifndef __PIE_KERNEL_H__
#define __PIE_KERNEL_H__

/* Kernels are row dominant in memory */

struct pie_kernel3x3 
{
        float v[9];
};

struct pie_kernel5x5 
{
        float v[25];
};

/**
 * Apply a kernel to a channel.
 * @param channel to alter.
 * @param kernel to apply
 * @param temporary buffer. Must be at least as large as the channel.
 * @param row width
 * @param row height
 * @param row stride
 * @return void
 */
extern void pie_kernel3x3_apply(float*,
                                struct pie_kernel3x3*,
                                float*,
                                int,
                                int,
                                int);

/**
 * Apply a kernel to a channel.
 * @param channel to alter.
 * @param kernel to apply
 * @param temporary buffer. Must be at least as large as the channel.
 * @param row width
 * @param row height
 * @param row stride
 * @return void
 */
extern void pie_kernel5x5_apply(float*,
                                struct pie_kernel5x5*,
                                float*,
                                int,
                                int,
                                int);

/**
 * Apply a separable kernel to a channel.
 * @param channel to alter.
 * @param kernel to apply
 * @param length of kernel
 * @param temporary buffer. Must be at least as large as the channel.
 * @param row width
 * @param row height
 * @param row stride
 * @return void
 */
extern void pie_kernel_sep_apply(float*,
                                 float*,
                                 int,
                                 float*,
                                 int,
                                 int,
                                 int);

/**
 * Initialize a kernel with a Gauss distribution (E=1).
 * @param kernel to initialize.
 * @param variance.
 * @return void.
 */
extern void pie_kernel3x3_gauss(struct pie_kernel3x3*,
                                float);

/**
 * Initialize a kernel with a Gauss distribution (E=1).
 * @param kernel to initialize.
 * @param variance.
 * @return void.
 */
extern void pie_kernel5x5_gauss(struct pie_kernel5x5*,
                                float);

/**
 * Initialize a separable Gauss kernel.
 * @param result to store parameters
 * @param number of elements, must be odd.
 * @param variance.
 * @return void.
 */
extern void pie_kernel_sep_gauss(float*,
                                 int,
                                 float);
                                 

#endif /* __PIE_KERNEL_H__ */
