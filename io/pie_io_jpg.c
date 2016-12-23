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
#include "../pie_bm.h"
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <setjmp.h>

/*
  TODO fix scan lines
  TODO fix support for b/w
*/
struct pie_jpg_error_mgr
{
        /* Jpeg error mgr fields */
        struct jpeg_error_mgr pub; 
        jmp_buf setjmp_buffer;
};

/*
 * Called when an error occurs during decoding.
 * Log the error and then return.
 */
static void pie_jpg_error_exit(j_common_ptr cinfo)
{
        /* cinfo->err is the pie_jpg_error_mgr we provisioned */
        struct pie_jpg_error_mgr* err = (struct pie_jpg_error_mgr*) cinfo->err;
        /* Log message */
        (*cinfo->err->output_message) (cinfo);
        longjmp(err->setjmp_buffer, 1);
}

int jpg_f32_read(struct bitmap_f32rgb* bm, const char* path)
{
        struct jpeg_decompress_struct cinfo;
        FILE* fp;
        JSAMPLE* row;
        int y;
        struct pie_jpg_error_mgr jerr;

        fp = fopen(path, "rb");
        if (fp == NULL)
        {
                return PIE_IO_NOT_FOUND;
        }

        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = pie_jpg_error_exit;
        
        /* Provision error handling */
        if (setjmp(jerr.setjmp_buffer)) 
        {
                /*
                 * Error during encoding, clean up and return.
                 */

                jpeg_destroy_decompress(&cinfo);
                fclose(fp);

                return PIE_IO_INTERNAL_ERR;
        }

        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, fp);
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        switch (cinfo.output_components)
        {
        case 1:
                bm->color_type = PIE_COLOR_TYPE_GRAY;
                break;
        case 3:
                bm->color_type = PIE_COLOR_TYPE_RGB;
                break;
        default:
                return PIE_IO_UNSUPPORTED_FMT;
        }

        bm->width = (int)cinfo.output_width;
        bm->height = (int)cinfo.output_height;
        bm_alloc_f32(bm);
        y = 0;
        row = malloc(sizeof(JSAMPLE) * 
                     cinfo.output_width * 
                     cinfo.output_components);

        while (cinfo.output_scanline < cinfo.output_height) 
        {
                int jpos = 0;
                /* Read one line at a time */
                jpeg_read_scanlines(&cinfo, &row, 1);

                for (int i = 0; i < (int)cinfo.output_width; i++)
                {
                        bm->c_red[y + i] = row[jpos++] / 255.0f;
                        bm->c_green[y + i] = row[jpos++] / 255.0f;
                        bm->c_blue[y + i] = row[jpos++] / 255.0f;
                }
                
                y += bm->row_stride;
        }
        free(row);

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);

       /* If error handling is configured, check *.num_warnings.
        * Should be zero if no warnings were found.
        */

        return 0;
}

int jpg_u8rgb_write(const char* path, struct bitmap_u8rgb* bm, int quality)
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
                for (int i = 0; i < (int)cinfo.image_width; i++)
                {
                        row[jpos++] = (JSAMPLE) bm->c_red[y + i];
                        row[jpos++] = (JSAMPLE) bm->c_green[y + i];
                        row[jpos++] = (JSAMPLE) bm->c_blue[y + i];
                }
                y += bm->row_stride;
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
