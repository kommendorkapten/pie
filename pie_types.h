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

#ifndef __PIE_TYPES_H__
#define __PIE_TYPES_H__

#include <stdint.h>
#include "math/pie_point.h"

#define PIE_HIST_RES 256
#define PIE_PATH_LEN 256
#define PIE_CURVE_MAX_CNTL_P 16
#define PIE_EPOCH 1483228800L /* 20170101T00:00:00Z */

enum pie_color_type
{
        PIE_COLOR_TYPE_INVALID = 0,
        PIE_COLOR_TYPE_GRAY = 1,
        PIE_COLOR_TYPE_RGB = 3
};

enum pie_color_bit_depth
{
        PIE_COLOR_INVALID  = 0,
        PIE_COLOR_8B = 8,   /* unsigned 8 bit int */
        PIE_COLOR_16B = 16, /* unsigned 16 bit int */
        PIE_COLOR_32B = 32  /* single precision 32 bit float */
};

enum pie_channel
{
        PIE_CHANNEL_INVALID = 0,
        PIE_CHANNEL_RED = 0x1,
        PIE_CHANNEL_GREEN = 0x2,
        PIE_CHANNEL_BLUE = 0x4,
        PIE_CHANNEL_RGB = 0x7,
};

struct pie_pixel_u8rgb
{
        uint8_t red;
        uint8_t green;
        uint8_t blue;
};

struct pie_pixel_u16rgb
{
        uint16_t red;
        uint16_t green;
        uint16_t blue;
};

struct pie_pixel_f32rgb
{
        float red;
        float green;
        float blue;
};

struct pie_bitmap_u8rgb
{
        uint8_t* c_red;
        uint8_t* c_green;
        uint8_t* c_blue;
        enum pie_color_type color_type;
        enum pie_color_bit_depth bit_depth;
        int width;
        int height;
        int row_stride;
};

struct pie_bitmap_u16rgb
{
        uint16_t* c_red;
        uint16_t* c_green;
        uint16_t* c_blue;
        enum pie_color_type color_type;
        enum pie_color_bit_depth bit_depth;
        int width;
        int height;
        int row_stride;
};

struct pie_bitmap_f32rgb
{
        float* c_red;
        float* c_green;
        float* c_blue;
        enum pie_color_type color_type;
        enum pie_color_bit_depth bit_depth;
        int width;
        int height;
        int row_stride;
};

struct pie_unsharp_param
{
        float amount;    /* typically between 0.3 and 0.7 (30 to 70%) */
        float radius;    /* typically 0.5 to 2.0 */
        float threshold; /* typically from 3 to 20 */
};

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

struct pie_histogram
{
        unsigned int lum[PIE_HIST_RES];
        unsigned int c_red[PIE_HIST_RES];
        unsigned int c_green[PIE_HIST_RES];
        unsigned int c_blue[PIE_HIST_RES];
};

#endif /* __PIE_TYPES_H__ */
