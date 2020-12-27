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

#ifndef __PIE_HIST_H__
#define __PIE_HIST_H__

#define PIE_HIST_RES 256

struct pie_alg_histogram
{
        unsigned int lum[PIE_HIST_RES];
        unsigned int c_red[PIE_HIST_RES];
        unsigned int c_green[PIE_HIST_RES];
        unsigned int c_blue[PIE_HIST_RES];
};
struct pie_bm_f32rgb;

/**
 * Calculate the luminance histogram for a bitmap.
 * @param the histogram to store data in.
 * @param the the bitmap to read from.
 * @return void
 */
extern void pie_alg_hist_lum(struct pie_alg_histogram*, struct pie_bm_f32rgb*);

/**
 * Calculate the crominance (RGB) histogram for a bitmap.
 * @param the histogram to store data in.
 * @param the the bitmap to read from.
 * @return void
 */
extern void pie_alg_hist_rgb(struct pie_alg_histogram*, struct pie_bm_f32rgb*);

#endif /* __PIE_HIST_H__ */
