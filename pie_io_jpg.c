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

#include "pie_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

/*
  TODO fix scan lines
  TODO fix support for b/w
*/

int jpg_f32_read(struct bitmap_f32rgb* bm, const char* path)
{
        return -1;
}

int jpg_8rgb_write(const char* path, struct bitmap_8rgb* bm, int quality)
{
        struct jpeg_compress_struct cinfo;
        /* Error handling stuff, default is to exit() */
        struct jpeg_error_mgr jerr;
        FILE* fp;
        JSAMPLE* row;
        int y = 0;

        fp = fopen(path, "wb");
        if (fp == NULL)
        {
                return PIE_IO_NOT_FOUND;
        }

        /* Set error handler (before init the compress struct */
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, fp);

        cinfo.image_width = bm->width; 
        cinfo.image_height = bm->height;

        switch(bm->color_type)
        {
        case PIE_COLOR_TYPE_GRAY:
                cinfo.input_components = 1;
                cinfo.in_color_space = JCS_GRAYSCALE;
                break;
        case PIE_COLOR_TYPE_RGB:
                cinfo.input_components = 3;
                cinfo.in_color_space = JCS_RGB;
                break;
        default:
                fclose(fp);
                jpeg_destroy_compress(&cinfo);
                return PIE_IO_UNSUPPORTED_FMT;
        }
        
        /* Set other values */
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE);
        jpeg_start_compress(&cinfo, TRUE);
        
        /* Allocate buffer for row */
        row = malloc(sizeof(JSAMPLE) * 
                     cinfo.image_width * 
                     cinfo.input_components);
        while (cinfo.next_scanline < cinfo.image_height) 
        {
                JSAMPROW rows[1];
                int jpos = 0;
                /* Only send one row at a time */
                for (unsigned int i = 0; i < cinfo.image_width; i++)
                {
                        row[jpos++] = (JSAMPLE) bm->c_red[i + y];
                        row[jpos++] = (JSAMPLE) bm->c_green[i + y];
                        row[jpos++] = (JSAMPLE) bm->c_blue[i + y];
                }
                y += cinfo.image_width;
                rows[0] = row;
                jpeg_write_scanlines(&cinfo, rows, 1);
        }
        free(row);
        /* Finish up */
        jpeg_finish_compress(&cinfo);
        fclose(fp);
        jpeg_destroy_compress(&cinfo);

        return 0;
}
