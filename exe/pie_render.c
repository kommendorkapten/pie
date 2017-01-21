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

#include "pie_render.h"
#include "../pie_types.h"
#include "../alg/pie_black.h"
#include "../alg/pie_contr.h"
#include "../alg/pie_expos.h"
#include "../alg/pie_satur.h"
#include "../alg/pie_white.h"
#include "../alg/pie_shado.h"
#include "../alg/pie_highl.h"
#include "../alg/pie_unsharp.h"
#include "../lib/timing.h"
#include "../pie_log.h"

void pie_img_init_settings(struct pie_img_settings* s)
{
        s->exposure = 0.0f;
        s->contrast = 1.0f;
        s->highlights = 0.0f;
        s->shadows = 0.0f;
        s->white = 0.0f;
        s->black = 0.0f;
        s->clarity.amount = 0.0f;
        s->clarity.radius = 50.0f;
        s->clarity.threshold = 4.0f;        
        s->vibrance = 0.0f;
        s->saturation = 1.0f;
        s->sharpening.amount = 0.0f;
        s->sharpening.radius = 1.0f;
        s->sharpening.threshold = 4.0f;
        s->rotate = 0.0f;
}

int pie_img_render(struct bitmap_f32rgb* img,
                   float* buf,
                   const struct pie_img_settings* s)
{
        struct timing t1;
        struct timing t2;

        (void)buf;
        timing_start(&t1);

        /* E X P O S U R E */
        timing_start(&t2);
        pie_alg_expos(img->c_red,
                      img->c_green,
                      img->c_blue,
                      s->exposure,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render exposure:       %8ldusec", timing_dur_usec(&t2));

        /* C O N T R A S T */
        timing_start(&t2);
        pie_alg_contr(img->c_red,
                      s->contrast,
                      img->width,
                      img->height,
                      img->row_stride);
        pie_alg_contr(img->c_green,
                      s->contrast,
                      img->width,
                      img->height,
                      img->row_stride);
        pie_alg_contr(img->c_blue,
                      s->contrast,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render contrast:       %8ldusec", timing_dur_usec(&t2));

        /* H I G H L I G H T S */
        timing_start(&t2);
        pie_alg_highl(img->c_red,
                      img->c_green,
                      img->c_blue,
                      s->highlights,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render highlights:     %8ldusec", timing_dur_usec(&t2));

        /* S H A D O W S */
        timing_start(&t2);
        pie_alg_shado(img->c_red,
                      img->c_green,
                      img->c_blue,
                      s->shadows,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render shadows:        %8ldusec", timing_dur_usec(&t2));

        /* W H I T E */
        timing_start(&t2);
        pie_alg_white(img->c_red,
                      img->c_green,
                      img->c_blue,
                      s->white,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render white:          %8ldusec", timing_dur_usec(&t2));

        /* B L A C K */
        timing_start(&t2);
        pie_alg_black(img->c_red,
                      img->c_green,
                      img->c_blue,
                      s->black,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render black:          %8ldusec", timing_dur_usec(&t2));

        /* C L A R I T Y */
        timing_start(&t2);        
        pie_unsharp(img, &s->clarity);
        PIE_DEBUG("Render clarity:        %8ldusec", timing_dur_usec(&t2));
#if 0
        s->vibrance = 0.0f;
#endif 
        /* S A T U R A T I O N */        
        timing_start(&t2);
        pie_alg_satur(img->c_red,
                      img->c_green,
                      img->c_blue,
                      s->saturation,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render saturation:     %8ldusec", timing_dur_usec(&t2));
#if 0
        s->rotate = 0.0f;        
#endif

        /* S H A R P E N I N G */
        timing_start(&t2);        
        pie_unsharp(img, &s->sharpening);
        PIE_DEBUG("Render sharpening:     %8ldusec", timing_dur_usec(&t2));
        
        PIE_DEBUG("Render total:          %8ldusec", timing_dur_usec(&t1));

        return 0;
}
