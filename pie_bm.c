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

int bm_alloc_f32(struct bitmap_f32rgb* bm)
{
        bm->c_red = malloc(bm->width * bm->height * sizeof(float));

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(bm->width * bm->height * sizeof(float));
                bm->c_blue = malloc(bm->width * bm->height * sizeof(float));
        }

        return 0;
}

int bm_alloc_8(struct bitmap_8rgb* bm)
{
        bm->c_red = malloc(bm->width * bm->height * sizeof(uint8_t));

        if (bm->color_type == PIE_COLOR_TYPE_RGB)
        {
                bm->c_green = malloc(bm->width * bm->height * sizeof(uint8_t));
                bm->c_blue = malloc(bm->width * bm->height * sizeof(uint8_t));
        }

        return 0;
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

void bm_free_8(struct bitmap_8rgb* bm)
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

void pixel_8rgb_get(struct pixel_8rgb* p, 
                    const struct bitmap_8rgb* bm,
                    int x,
                    int y)
{
        unsigned int offset = bm->width * y + x;

        p->red = bm->c_red[offset];
        p->green = bm->c_green[offset];
        p->blue = bm->c_blue[offset];
}

void pixel_8rgb_set(struct bitmap_8rgb* bm,
                    int x,
                    int y,
                    struct pixel_8rgb* p)
{
        unsigned int offset = bm->width * y + x;

        bm->c_red[offset] = p->red;
        bm->c_green[offset] = p->green;
        bm->c_blue[offset] = p->blue;
}

