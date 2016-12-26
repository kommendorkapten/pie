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
#include "pie_types.h"
#include "alg/pie_contr.h"
#include "alg/pie_expos.h"
#include "alg/pie_satur.h"
#include "alg/pie_black.h"
#include "lib/timing.h"
#include "pie_log.h"

void pie_img_init_settings(struct pie_img_settings* s)
{
        s->exposure = 0.0f;
        s->contrast = 1.0f;
        s->highlights = 0.0f;
        s->shadows = 0.0f;
        s->white = 0.0f;
        s->black = 0.0f;
        s->clarity = 0.0f;
        s->vibrance = 0.0f;
        s->saturation = 1.0f;
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
        PIE_DEBUG("Render exposure:       %ldusec", timing_dur_usec(&t2));

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
        PIE_DEBUG("Render contrast:       %ldusec", timing_dur_usec(&t2));

#if 0
        s->highlights = 0.0f;
        s->shadows = 0.0f;
        s->white = 0.0f;
#endif

        /* B L A C K */
        timing_start(&t2);
        pie_alg_black(img->c_red,
                      img->c_green,
                      img->c_blue,
                      s->black,
                      img->width,
                      img->height,
                      img->row_stride);
        PIE_DEBUG("Render black:          %ldusec", timing_dur_usec(&t2));
#if 0
        s->clarity = 0.0f;
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
        PIE_DEBUG("Render saturation:     %ldusec", timing_dur_usec(&t2));
#if 0
        s->rotate = 0.0f;        
#endif

        PIE_DEBUG("Render total:          %ldusec", timing_dur_usec(&t1));

        return 0;
}
