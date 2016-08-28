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

enum pie_color_type {
        PIE_COLOR_TYPE_GRAY,
        PIE_COLOR_TYPE_RGB
};

enum pie_color_bit_depth {
        PIE_COLOR_8B = 8,  /* unsigned 8 bit int */
        PIE_COLOR_16B = 16, /* unsigned 16 bit int */
        PIE_COLOR_32B = 32 /* single precision 32 bit float */
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
        unsigned int width;
        unsigned int height;
};

struct bitmap_u16rgb
{
        uint16_t* c_red;
        uint16_t* c_green;
        uint16_t* c_blue;
        enum pie_color_type color_type;
        unsigned int width;
        unsigned int height;
};

struct bitmap_f32rgb
{
        float* c_red;
        float* c_green;
        float* c_blue;
        enum pie_color_type color_type;
        unsigned int width;
        unsigned int height;
};

#endif /* __PIE_TYPES_H__ */
