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
struct pie_img_settings;

/**
 * Init settings to default values.
 * @param settings.
 * @param width of image.
 * @param height of image.
 * @return void.
 */
extern void pie_img_init_settings(struct pie_img_settings*, int, int);

/**
 * Render a bitmap.
 * @param bitmap to render.
 * @param temporary buffer, large enough to hold a single channel.
 * @param settings to apply.
 * @return 0 on success.
 */
extern int pie_img_render(struct pie_bitmap_f32rgb*,
                          float*,
                          const struct pie_img_settings*);

#endif /* __PIE_RENDER_H__ */
