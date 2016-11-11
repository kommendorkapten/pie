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

#ifndef __PIE_CONTR_H__
#define __PIE_CONTR_H__

/**
 * Apply contrast.
 * Contrast amount is in range [0,2]. 0 -> 1 is less contrast,
 * 1 is no contrast and 1 -> 2 is more contrast.
 * @param channel to apply contrast to.
 * @param amount of contrast, in range [0,2].
 * @param image width.
 * @param image height.
 * @param image row stride.
 * @reurn void.
 */
extern void pie_alg_contr(float* restrict,
                          float,
                          unsigned int,
                          unsigned int,
                          unsigned int);

#endif /* __PIE_CONTR_H__ */
