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

#include "pie_bm.h"
#include <stdlib.h>

static void bm_conv_u8_u16(struct bitmap_u8rgb*, 
                           const struct bitmap_u16rgb*);
static void bm_conv_u8_f32(struct bitmap_u8rgb*, 
                           const struct bitmap_f32rgb*);
static void bm_conv_u16_u8(struct bitmap_u16rgb*, 
                           const struct bitmap_u8rgb*);
static void bm_conv_u16_f32(struct bitmap_u16rgb*, 
                            const struct bitmap_f32rgb*);
static void bm_conv_f32_u16(struct bitmap_f32rgb*, 
                            const struct bitmap_u16rgb*);
static void bm_conv_f32_u8(struct bitmap_f32rgb*, 
                           const struct bitmap_u8rgb*);

int bm_alloc_u8(struct bitmap_u8rgb* bm)
{
        unsigned int rem = bm->width % 4;
        size_t s;

        /* Make sure each row has good alignment */
        if (rem == 0)
        {
                bm->row_stride = bm->width;
        }
        else
        {
                bm->row_stride = bm->width + (4 - rem);
        }

        s = bm->row_stride * bm->height * sizeof(uint8_t);
        bm->bit_depth = PIE_COLOR_8B;
        bm->c_red = malloc(s);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(s);
                bm->c_blue = malloc(s);
        }

        return 0;
}

int bm_alloc_u16(struct bitmap_u16rgb* bm)
{
        unsigned int rem = bm->width % 4;
        size_t s;

        /* Make sure each row has good alignment */
        if (rem == 0)
        {
                bm->row_stride = bm->width;
        }
        else
        {
                bm->row_stride = bm->width + (4 - rem);
        }

        s = bm->row_stride * bm->height * sizeof(uint16_t);
        bm->bit_depth = PIE_COLOR_16B;
        bm->c_red = malloc(s);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(s);
                bm->c_blue = malloc(s);
        }

        return 0;
}

int bm_alloc_f32(struct bitmap_f32rgb* bm)
{
        unsigned int rem = bm->width % 4;
        size_t s;

        /* Make sure each row has good alignment */
        if (rem == 0)
        {
                bm->row_stride = bm->width;
        }
        else
        {
                bm->row_stride = bm->width + (4 - rem);
        }

        s = bm->row_stride * bm->height * sizeof(float);
        bm->bit_depth = PIE_COLOR_32B;
        bm->c_red = malloc(s);

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(s);
                bm->c_blue = malloc(s);
        }

        return 0;
}

void bm_free_u8(struct bitmap_u8rgb* bm)
{
        free(bm->c_red);
        
        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                free(bm->c_green);
                free(bm->c_blue);
        }

        bm->c_red = NULL;
        bm->c_green = NULL;
        bm->c_blue = NULL;
}

void bm_free_u16(struct bitmap_u16rgb* bm)
{
        free(bm->c_red);
        
        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                free(bm->c_green);
                free(bm->c_blue);
        }

        bm->c_red = NULL;
        bm->c_green = NULL;
        bm->c_blue = NULL;
}

void bm_free_f32(struct bitmap_f32rgb* bm)
{
        free(bm->c_red);
        
        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                free(bm->c_green);
                free(bm->c_blue);
        }

        bm->c_red = NULL;
        bm->c_green = NULL;
        bm->c_blue = NULL;
}

void pixel_u8rgb_get(struct pixel_u8rgb* p, 
                     const struct bitmap_u8rgb* bm,
                     int x,
                     int y)
{
        unsigned int offset = bm->row_stride * y + x;

        p->red = bm->c_red[offset];
        p->green = bm->c_green[offset];
        p->blue = bm->c_blue[offset];
}

void pixel_u8rgb_set(struct bitmap_u8rgb* bm,
                     int x,
                     int y,
                     struct pixel_u8rgb* p)
{
        unsigned int offset = bm->row_stride * y + x;

        bm->c_red[offset] = p->red;
        bm->c_green[offset] = p->green;
        bm->c_blue[offset] = p->blue;
}

int bm_conv_bd(void* restrict dst,
               enum pie_color_bit_depth dst_bd,
               void* restrict src,
               enum pie_color_bit_depth src_bd)
{
        int ret = -1;
        
        if (dst_bd == src_bd)
        {
                /* Create a copy */
        }
        if (dst_bd == PIE_COLOR_8B)
        {
                switch (src_bd)
                {
                case PIE_COLOR_16B:
                        bm_conv_u8_u16((struct bitmap_u8rgb*)dst,
                                       (struct bitmap_u16rgb*)src);
                        ret = 0;
                        break;
                case PIE_COLOR_32B:
                        bm_conv_u8_f32((struct bitmap_u8rgb*)dst,
                                       (struct bitmap_f32rgb*)src);
                        ret = 0;
                        break;
                default:
                        return -1;
                }
        }
        else if (dst_bd == PIE_COLOR_16B)
        {
                switch (src_bd)
                {
                case PIE_COLOR_8B:
                        bm_conv_u16_u8((struct bitmap_u16rgb*)dst,
                                       (struct bitmap_u8rgb*)src);
                        ret = 0;
                        break;
                case PIE_COLOR_32B:
                        bm_conv_u16_f32((struct bitmap_u16rgb*)dst,
                                        (struct bitmap_f32rgb*)src);
                        ret = 0;
                        break;
                default:
                        return -1;
                }                
        }
        else if (dst_bd == PIE_COLOR_32B)
        {
                switch (src_bd)
                {
                case PIE_COLOR_8B:
                        bm_conv_f32_u8((struct bitmap_f32rgb*)dst,
                                       (struct bitmap_u8rgb*)src);
                        ret = 0;
                        break;
                case PIE_COLOR_16B:
                        bm_conv_f32_u16((struct bitmap_f32rgb*)dst,
                                        (struct bitmap_u16rgb*)src);
                        ret = 0;
                        break;
                default:
                        return -1;
                }                
        }

        return ret;
}

static void bm_conv_u8_u16(struct bitmap_u8rgb* dst,
                           const struct bitmap_u16rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        bm_alloc_u8(dst);

        for (unsigned int y = 0; y < src->height; y++)
        {
                for (unsigned int x = 0; x < src->width; x++)
                {
                        unsigned int o = y * src->row_stride + x;

                        dst->c_red[o] = (uint8_t) ((src->c_red[o] / 65535.0f) * 255.0f);
                        dst->c_green[o] = (uint8_t) ((src->c_green[o] / 65535.0f) * 255.0f);
                        dst->c_blue[o] = (uint8_t) ((src->c_blue[o] / 65535.0f) * 255.0f);
                }
        }
}

static void bm_conv_u8_f32(struct bitmap_u8rgb* dst, 
                           const struct bitmap_f32rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        bm_alloc_u8(dst);

        for (unsigned int y = 0; y < src->height; y++)
        {
                for (unsigned int x = 0; x < src->width; x++)
                {
                        unsigned int o = y * src->row_stride + x;
                  
                        dst->c_red[o] = (uint8_t) (src->c_red[o] * 255.0f);
                        dst->c_green[o] = (uint8_t) (src->c_green[o] * 255.0f);
                        dst->c_blue[o] = (uint8_t) (src->c_blue[o] * 255.0f);
                }
        }
}

static void bm_conv_u16_u8(struct bitmap_u16rgb* dst, 
                           const struct bitmap_u8rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        bm_alloc_u16(dst);

        for (unsigned int y = 0; y < src->height; y++)
        {
                for (unsigned int x = 0; x < src->width; x++)
                {
                        unsigned int o = y * src->row_stride + x;
                  
                        dst->c_red[o] = (uint16_t) ((src->c_red[o] / 255.0f) * 65535.0f);
                        dst->c_green[o] = (uint16_t) ((src->c_green[o] / 255.0f) * 65535.0f);
                        dst->c_blue[o] = (uint16_t) ((src->c_blue[o] / 255.0f) * 65535.0f);
                }
        }        
}

static void bm_conv_u16_f32(struct bitmap_u16rgb* dst, 
                            const struct bitmap_f32rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        bm_alloc_u16(dst);

        for (unsigned int y = 0; y < src->height; y++)
        {
                for (unsigned int x = 0; x < src->width; x++)
                {
                        unsigned int o = y * src->row_stride + x;
                  
                        dst->c_red[o] = (uint8_t) (src->c_red[o] * 65535.0f);
                        dst->c_green[o] = (uint8_t) (src->c_green[o] * 65535.0f);
                        dst->c_blue[o] = (uint8_t) (src->c_blue[o] * 65535.0f);
                }
        }
}

static void bm_conv_f32_u8(struct bitmap_f32rgb* dst, 
                           const struct bitmap_u8rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        bm_alloc_f32(dst);

        for (unsigned int y = 0; y < src->height; y++)
        {
                for (unsigned int x = 0; x < src->width; x++)
                {
                        unsigned int o = y * src->row_stride + x;
                  
                        dst->c_red[o] = src->c_red[o] / 255.0f;
                        dst->c_green[o] = src->c_green[o] / 255.0f;
                        dst->c_blue[o] = src->c_blue[o] / 255.0f;
                }
        }
}

static void bm_conv_f32_u16(struct bitmap_f32rgb* dst,
                            const struct bitmap_u16rgb* src)
{
        dst->width = src->width;
        dst->height = src->height;
        dst->color_type = src->color_type;

        bm_alloc_f32(dst);

        for (unsigned int y = 0; y < src->height; y++)
        {
                for (unsigned int x = 0; x < src->width; x++)
                {
                        unsigned int o = y * src->row_stride + x;
                  
                        dst->c_red[o] = src->c_red[o] / 65535.0f;
                        dst->c_green[o] = src->c_green[o] / 65535.0f;
                        dst->c_blue[o] = src->c_blue[o] / 65535.0f;
                }
        }
}
