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

int pie_io_raw_f32_read(struct pie_bitmap_f32rgb* bm, const char* path)
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
        lrd->params.use_camera_wb = 1;
        lrd->params.user_flip = 0;
        lrd->params.output_color = 1; /* sRGB */
        lrd->params.user_qual = 0;
        /* user_qual 0 - linear interpolation
                     1 - VNG interpolation
                     2 - PPG interpolation
                     3 - AHD interpolation
                     4 - DCB interpolation
        */
        
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
