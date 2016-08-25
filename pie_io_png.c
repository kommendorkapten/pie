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

#include <png.h>
#include <stdlib.h>
#include "pie_io.h"
#include "pie_bm.h"

int png_f32_read(struct bitmap_f32rgb* bm, const char* path)
{
        unsigned char header[8];
        FILE *fp = fopen(path, "rb");
        png_structp pngp;
        png_infop infop;
        png_byte** rows;
        png_byte color_type;
        png_byte bit_depth;
        double gamma;

        if (fp == NULL)
        {
                return PIE_IO_NOT_FOUND;
        }

        fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8))
        {
                return PIE_IO_INV_FMT;
        }

        pngp = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                         NULL,
                                         NULL,
                                         NULL);

        if (pngp == NULL)
        {
                return PIE_IO_INTERNAL_ERR;
        }

        infop = png_create_info_struct(pngp);
        if (infop == NULL)
        {
                fclose(fp);
                png_destroy_read_struct(&pngp, NULL, NULL);
                return PIE_IO_INTERNAL_ERR;
        }

        if (setjmp(png_jmpbuf(pngp)))
        {
                png_destroy_read_struct(&pngp, &infop, NULL);
                fclose(fp);
                return PIE_IO_INTERNAL_ERR;
        }

        png_init_io(pngp, fp);
        /* The first 8 bytes are already checkd */
        png_set_sig_bytes(pngp, 8);
        png_read_info(pngp, infop);
        bm->width = png_get_image_width(pngp, infop);
        bm->height = png_get_image_height(pngp, infop);
        color_type = png_get_color_type(pngp, infop);
        bit_depth = png_get_bit_depth(pngp, infop);

        switch (color_type)
        {
        case PNG_COLOR_TYPE_GRAY:
                bm->color_type = PIE_COLOR_TYPE_GRAY;
                break;
        case PNG_COLOR_TYPE_RGB:
                bm->color_type = PIE_COLOR_TYPE_RGB;
                break;
        default:
                png_destroy_read_struct(&pngp, &infop, NULL);
                fclose(fp);
                return PIE_IO_UNSUPPORTED_FMT;
        }

        switch (bit_depth)
        {
        case 8:
                break;
        default:
                png_destroy_read_struct(&pngp, &infop, NULL);
                fclose(fp);
                return PIE_IO_UNSUPPORTED_FMT;                
        }

        if (png_get_gAMA(pngp, infop, &gamma))
        {
                /* png_set_gamma(pngp, display_exponent, gamma); */
                printf("Read gamma from file: %f\n", gamma);
        }
        
        /* Update with any transformations set */
        png_read_update_info(pngp, infop);

        /* Read data  */
        if (setjmp(png_jmpbuf(pngp)))
        {
                png_destroy_read_struct(&pngp, &infop, NULL);
                fclose(fp);
                return PIE_IO_IO_ERR;                
        }

        rows = malloc(sizeof(png_byte*) * bm->height);
        for (int y = 0; y < bm->height; y++)
        {
                rows[y] = malloc(png_get_rowbytes(pngp, infop));
        }

        png_read_image(pngp, rows);
        bm_alloc_f32(bm);

        /* Copy data to bitmap */
        for (int y = 0; y < bm->height; y++)
        {
                png_byte* row = rows[y];

                for (int x = 0; x < bm->width; x++)
                {
                        float red = (float)*row++;
                        float green = (float)*row++;
                        float blue = (float)*row++;
                        int offset = y * bm->width + x;

                        /* refactor to set methods */
                        bm->c_red[offset] = red / 255.0f;
                        bm->c_green[offset] = blue / 255.0f;
                        bm->c_blue[offset] = blue / 255.0f;
                }

                /* Free copy buffer */
                free(rows[y]);
        }
        free(rows);
        png_destroy_read_struct(&pngp, &infop, NULL);
        fclose(fp);

        return 0;
}

int png_8rgb_write(const char* path, struct bitmap_8rgb* bitmap)
{
        FILE* fp;
        png_structp pngp;
        png_infop infop;
        png_byte** rows;
        int pixel_size = 3; /* RGB */
        int depth = 8; 
    
        fp = fopen(path, "wb");
        if (fp == NULL) {
                return PIE_IO_NOT_FOUND;
        }

        pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                       NULL, /* data for error handler */
                                       NULL, /* error handler */
                                       NULL);
        if (pngp == NULL) {
                fclose(fp);
                return PIE_IO_INTERNAL_ERR;
        }
    
        infop = png_create_info_struct(pngp);
        if (infop == NULL) {
                png_destroy_write_struct(&pngp, NULL);
                fclose(fp);
                return PIE_IO_INTERNAL_ERR;
        }
    
        if (setjmp(png_jmpbuf(pngp))) {
                png_destroy_write_struct(&pngp, &infop);
                fclose(fp);
                return PIE_IO_INTERNAL_ERR;
        }
    
        /* Set image attributes. */
        png_set_IHDR(pngp,
                     infop,
                     bitmap->width,
                     bitmap->height,
                     depth,
                     PNG_COLOR_TYPE_RGB,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);
    
        /* Write rows to PNG obj */
        rows = malloc(bitmap->height * sizeof(png_byte*));
        for (int y = 0; y < bitmap->height; y++)
        {
                size_t row_size = sizeof(uint8_t) * bitmap->width * pixel_size;
                png_byte* row = malloc(row_size);
                rows[y] = row;
                for (int x = 0; x < bitmap->width; ++x)
                {
                        struct pixel_8rgb p;

                        pixel_8rgb_get(&p, bitmap, x, y);
                        *row++ = p.red;
                        *row++ = p.green;
                        *row++ = p.blue;
                }
        }
    
        /* Write PNG obj to disk */
        png_init_io(pngp, fp);
        png_set_rows(pngp, infop, rows);
        png_write_png(pngp, 
                      infop,
                      PNG_TRANSFORM_IDENTITY, /* No transform */
                      NULL); /* not used */
    
        for (int y = 0; y < bitmap->height; y++) 
        {
                free(rows[y]);
        }
        free(rows);
    
        png_destroy_write_struct(&pngp, &infop);
        fclose(fp);

        return 0;
}

