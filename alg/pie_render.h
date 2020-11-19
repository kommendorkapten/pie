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

struct pie_bitmap_f32rgb;
struct pie_dev_settings;
struct pie_curve;

/**
 * Init settings to default values.
 * @param settings.
 * @param width of image.
 * @param height of image.
 * @return void.
 */
extern void pie_bm_init_settings(struct pie_dev_settings*, int, int);

/**
 * Init a curve to linear from (0, 0) to (1, 1).
 * @param pointer to a pie curve struct.
 * @return void.
 */
extern void pie_bm_init_curve(struct pie_curve*);

/**
 * Convert pie_dev_settings to internal format, used when rendering.
 * @param settings struct.
 * @return void
 */
extern void pie_bm_set_to_int_fmt(struct pie_dev_settings*);

/**
 * Convert pie_dev_settings to canonical format. Used for exchanging
 * data (to webapp or persist in database).
 * @param settings struct.
 * @return void
 */
extern void pie_bm_set_to_can_fmt(struct pie_dev_settings*);

/**
 * Render a bitmap.
 * @param bitmap to render.
 * @param temporary buffer, large enough to hold a single channel.
 * @param settings to apply.
 * @return 0 on success.
 */
extern int pie_bm_render(struct pie_bitmap_f32rgb*,
                         float*,
                         const struct pie_dev_settings*);

#endif /* __PIE_RENDER_H__ */
