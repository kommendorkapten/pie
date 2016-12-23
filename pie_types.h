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

#define PIE_HIST_RES 256
#define PIE_PATH_LEN 256
#if defined(LWS_PRE)
# define PROXY_RGBA_OFF LWS_PRE
#else
# define PROXY_RGBA_OFF 16
#endif

enum pie_color_type {
        PIE_COLOR_TYPE_INVALID = 0,
        PIE_COLOR_TYPE_GRAY = 1,
        PIE_COLOR_TYPE_RGB = 3
};

enum pie_color_bit_depth {
        PIE_COLOR_INVALID  = 0,
        PIE_COLOR_8B = 8,   /* unsigned 8 bit int */
        PIE_COLOR_16B = 16, /* unsigned 16 bit int */
        PIE_COLOR_32B = 32  /* single precision 32 bit float */
};

struct pixel_u8rgb
{
        uint8_t red;
        uint8_t green;
        uint8_t blue;
};

struct pixel_u16rgb
{
        uint16_t red;
        uint16_t green;
        uint16_t blue;
};

struct pixel_f32rgb
{
        float red;
        float green;
        float blue;
};

struct bitmap_u8rgb
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

struct bitmap_u16rgb
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

struct bitmap_f32rgb
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

struct pie_img_settings
{
        /* [-5, 5]     def 0 */
        float exposure;
        /* [0,2]       def 1 */
        float contrast;
        /* [-100, 100] def 0 */
        float highlights;
        /* [-100, 100] def 0 */
        float shadows;
        /* [-100, 100] def 0 */
        float white;
        /* [-100, 100] def 0 */
        float black;
        /* [-100, 100] def 0 */
        float clarity;
        /* [-100, 100] def 0 */
        float vibrance;
        /* [-100, 100] def 0 */
        float saturation;
        /* [0, 359]    def 0 */
        float rotate;
};

struct pie_histogram
{
        unsigned int lum[PIE_HIST_RES];
        unsigned int c_red[PIE_HIST_RES];
        unsigned int c_green[PIE_HIST_RES];
        unsigned int c_blue[PIE_HIST_RES];
};

struct pie_img_workspace
{
        char path[PIE_PATH_LEN];
        struct pie_histogram hist;
        struct pie_img_settings settings;
        /* Unmodified full resolution image */
        struct bitmap_f32rgb raw;
        /* Downsampled unmodified proxy image */
        struct bitmap_f32rgb proxy;
        /* Downsampled and rendered proxy image */
        struct bitmap_f32rgb proxy_out;
        /* proxy out coded as unsigned chars, in rgba format.
           With prelude of LWS_PRE header size */
        unsigned char* buf_proxy;
        /* Pointer to start of image in buf */
        unsigned char* proxy_out_rgba;
        int proxy_out_len;
        /* buffer to encode histogram as JSON to */
        unsigned char* buf_hist;
        unsigned char* hist_json;
        int hist_json_len;
};

#endif /* __PIE_TYPES_H__ */
