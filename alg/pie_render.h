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

#ifndef __PIE_RENDER_H__
#define __PIE_RENDER_H__

#include "pie_unsharp.h"
#include "../pie_defs.h"
#include "../math/pie_point.h"

struct pie_curve
{
        struct pie_point_2d cntl_p[PIE_CURVE_MAX_CNTL_P];
        int num_p;
};

struct pie_dev_settings
{
       /* [-0.5, 0.5]  def 0 */
        float color_temp;
       /* [-0.5, 0.5]  def 0 */
        float tint;
        /* [-5, 5]     def 0 */
        float exposure;
        /* [0,2]       def 1 */
        float contrast;
        /* [-1, 1]     def 0 */
        float highlights;
        /* [-1, 1]     def 0 */
        float shadows;
        /* [-1, 1]     def 0 */
        float white;
        /* [-1, 1]     def 0 */
        float black;
        /* unknown def */
        struct pie_unsharp_param clarity;
        /* [-100, 100] def 0 */
        float vibrance;
        /* [0, 2] def 1 */
        float saturation;
        /* [0, 359]    def 0 */
        float rotate;
        /* unknown def */
        struct pie_unsharp_param sharpening;

        /* def linear [0 - 1] */
        struct pie_curve curve_l;
        struct pie_curve curve_r;
        struct pie_curve curve_g;
        struct pie_curve curve_b;

        /* always last */
        int version;
};

struct pie_bm_f32rgb;

/**
 * Init settings to default values.
 * @param settings.
 * @param width of image.
 * @param height of image.
 * @return void.
 */
extern void pie_alg_init_settings(struct pie_dev_settings*, int, int);

/**
 * Init a curve to linear from (0, 0) to (1, 1).
 * @param pointer to a pie curve struct.
 * @return void.
 */
extern void pie_alg_init_curve(struct pie_curve*);

/**
 * Convert pie_dev_settings to internal format, used when rendering.
 * @param settings struct.
 * @return void
 */
extern void pie_alg_set_to_int_fmt(struct pie_dev_settings*);

/**
 * Convert pie_dev_settings to canonical format. Used for exchanging
 * data (to webapp or persist in database).
 * @param settings struct.
 * @return void
 */
extern void pie_alg_set_to_can_fmt(struct pie_dev_settings*);

/**
 * Render a bitmap.
 * @param bitmap to render.
 * @param temporary buffer, large enough to hold a single channel.
 * @param settings to apply.
 * @return 0 on success.
 */
extern int pie_alg_render(struct pie_bm_f32rgb*,
                          float*,
                          const struct pie_dev_settings*);

#endif /* __PIE_RENDER_H__ */
