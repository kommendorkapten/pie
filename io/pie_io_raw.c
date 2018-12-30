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

#include <libraw/libraw.h>
#include "pie_io.h"
#include "pie_io_raw.h"
#include "../pie_log.h"
#include "../pie_types.h"
#include "../bm/pie_bm.h"

int pie_io_raw_f32_read(struct pie_bitmap_f32rgb* bm,
                        const char* path,
                        struct pie_io_opts* opts)
{
        libraw_data_t* lrd;
        libraw_processed_image_t* mem_img;
        unsigned short* data;
        int ret;

        lrd = libraw_init(0);
        if (lrd == NULL)
        {
                return PIE_IO_INTERNAL_ERR;
        }

        /* Prepare image reading */
        lrd->params.output_bps = 16;
        lrd->params.use_camera_matrix = 1;
        lrd->params.use_camera_wb = 1;
        lrd->params.user_flip = 0;
        lrd->params.med_passes = 0; /* med 3x3 after interpolation */
        lrd->params.output_color = 1; /* sRGB */
        lrd->params.no_auto_bright = 0;
        lrd->params.highlight = 5; /* blend in high lights */
        /* lrd->params.auto_bright_thr = 0.00003f; 0.001 to 0.00003 */
        lrd->params.fbdd_noiserd = 0;

        /* exposure correction */
        lrd->params.exp_correc = 1;
        lrd->params.exp_shift = 2.5f; /* 0.25 = 2stop dark, 8 = 3 stop light */
        lrd->params.exp_preser = 1.0f; /* 0 - 1.0 */

        lrd->params.user_qual = 2;
        /* user_qual 0 - linear interpolation
                     1 - VNG interpolation
                     2 - PPG interpolation <--- Best qual/perf
                     3 - AHD interpolation
                     4 - DCB interpolation
                    11 - DHT interpolation <--- Best performance 2x slower
        */

        /* Default to sRGB */
        lrd->params.gamm[0] = 1.0 / 2.4f;
        lrd->params.gamm[1] = 12.92f;

        /* Load any options */
        if (opts)
        {
                switch (opts->qual)
                {
                case PIE_IO_HIGH_QUAL:
                        lrd->params.user_qual = 11;
                        break;
                default:
                        break;
                }

                switch (opts->cspace)
                {
                case PIE_IO_LINEAR:
                        lrd->params.gamm[0] = 1.0f;
                        lrd->params.gamm[1] = 1.0f;
                        break;
                case PIE_IO_SRGB:
                        break;
                default:
                        PIE_WARN("Invalid target color space: %d", opts->cspace);
                        ret = PIE_IO_INV_OPT;
                        goto done;
                }
        }

        ret = libraw_open_file(lrd, path);
        if (ret)
        {
                PIE_DEBUG("libraw_open_file: %d", ret);
                switch(ret)
                {
                case LIBRAW_FILE_UNSUPPORTED:
                        ret = PIE_IO_INV_FMT;
                        break;
                case LIBRAW_IO_ERROR:
                        ret = PIE_IO_NOT_FOUND;
                        break;
                default:
                        ret = PIE_IO_INTERNAL_ERR;
                }

                goto done;
        }

        if (libraw_unpack(lrd))
        {
                PIE_ERR("libraw_unpack");
                ret = PIE_IO_INTERNAL_ERR;
                goto done;
        }
        if (libraw_dcraw_process(lrd))
        {
                PIE_ERR("libraw_dcraw_process");
                ret = PIE_IO_INTERNAL_ERR;
                goto done;
        }

        mem_img = libraw_dcraw_make_mem_image(lrd, &ret);
        if (mem_img == NULL)
        {
                PIE_ERR("libraw_dcraw_make_mem_image");
                ret = PIE_IO_INTERNAL_ERR;
                goto done;
        }

        /* Prepare copy of pixels */
        bm->width = mem_img->width;
        bm->height = mem_img->height;
        bm->color_type = PIE_COLOR_TYPE_RGB;
        pie_bm_alloc_f32(bm);

        /* Copy pixels */
        data = (unsigned short*)mem_img->data;
        for (int y = 0; y < bm->height; y++)
        {
                for (int x = 0; x < bm->width; x++)
                {
                        int p = y * bm->row_stride + x;

                        bm->c_red[p] = ((float)*data++) / 65535.0f;
                        bm->c_green[p] = ((float)*data++) / 65535.0f;
                        bm->c_blue[p] = ((float)*data++) / 65535.0f;
                }
        }
        libraw_dcraw_clear_mem(mem_img);
        ret = 0;

done:
        libraw_close(lrd);
        return ret;
}
