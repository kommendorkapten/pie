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

#ifndef __PIE_RGBA_H__
#define __PIE_RGBA_H__

enum pie_image_type
{
        PIE_IMAGE_TYPE_PRIMARY = 1,
};

struct bitmap_f32rgb;

/**
 * Encode the proxy_out into rgba format.
 * Rgba format is w, h, rgba, rgba etc.
 * w and h are 32 uint, in network order.
 * Min size of buffer is 8b + w * h * 4b.
 * @param buffer to encode to.
 * @param the bitmap to encode.
 * @return void
 */
extern void encode_rgba(unsigned char* restrict,
                        const struct pie_bitmap_f32rgb* restrict,
                        enum pie_image_type);

#endif /* __PIE_RGBA_H__ */
