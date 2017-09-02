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
#include <stdint.h>
#ifdef __sun
# include <note.h>
#else
# define NOTE(X)
#endif
#include "pie_io.h"
#include "../bm/pie_bm.h"
#include "../pie_log.h"

/* PNG is stored in network order */
#ifdef __BYTE_ORDER__
# if __BYTE_ORDER__ == 4321
#  define SWAP_PIXELS 0
# else
#  define SWAP_PIXELS 1
# endif
#else
#  error __BYTE_ORDER__ not defined
#endif

int png_f32_read(struct pie_bitmap_f32rgb* bm, const char* path)
{
        unsigned char header[8];
        FILE* fp = fopen(path, "rb");
        png_structp pngp;
        png_infop infop;
        png_byte** rows;
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
        bm->width = (int)infop->width;
        bm->height = (int)infop->height;
        
        switch (infop->color_type)
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

        switch (infop->bit_depth)
        {
        case 8:
        case 16:
                if (SWAP_PIXELS)
                {
                        png_set_swap(pngp);                        
                }
                break;
        default:
                png_destroy_read_struct(&pngp, &infop, NULL);
                fclose(fp);
                return PIE_IO_UNSUPPORTED_FMT;                
        }

        if (png_get_gAMA(pngp, infop, &gamma))
        {
                /* png_set_gamma(pngp, display_exponent, gamma); */
                NOTE(EMPTY);
                PIE_DEBUG("Read gamma from file: %f.", gamma);
        }
        
        /* Update with any transformations set */
        png_read_update_info(pngp, infop);

        /* Sett restore point */
        if (setjmp(png_jmpbuf(pngp)))
        {
                png_destroy_read_struct(&pngp, &infop, NULL);
                fclose(fp);
                return PIE_IO_IO_ERR;                
        }

        rows = malloc(sizeof(png_byte*) * bm->height);
        for (int y = 0; y < bm->height; y++)
        {
                rows[y] = malloc(infop->rowbytes);
        }

        /* Read data  */        
        png_read_image(pngp, rows);
        pie_bm_alloc_f32(bm);

        /* Copy data to bitmap */
        if (infop->bit_depth == 8)
        {
                for (int y = 0; y < bm->height; y++)
                {
                        png_byte* row = rows[y];

                        for (int x = 0; x < bm->width; x++)
                        {
                                float red = (float)*row++;
                                float green = (float)*row++;
                                float blue = (float)*row++;
                                int offset = y * bm->row_stride + x;

                                bm->c_red[offset] = red / 255.0f;
                                bm->c_green[offset] = green / 255.0f;
                                bm->c_blue[offset] = blue / 255.0f;
                        }

                        /* Free copy buffer */
                        free(rows[y]);
                }                
        }
        else if (infop->bit_depth == 16)
        {
                for (int y = 0; y < bm->height; y++)
                {
                        uint16_t* row = (uint16_t*)rows[y];

                        for (int x = 0; x < bm->width; x++)
                        {
                                float red = (float)*row++;
                                float green = (float)*row++;
                                float blue = (float)*row++;
                                int offset = y * bm->row_stride + x;

                                bm->c_red[offset] = red / 65535.0f;
                                bm->c_green[offset] = green / 65535.0f;
                                bm->c_blue[offset] = blue / 65535.0f;
                        }
                        
                        /* Free copy buffer */
                        free(rows[y]);
                }                
        }
        else
        {
                /* Should never happen, validation of bit depth happens
                   earlier */
                abort();
        }

        free(rows);
        png_destroy_read_struct(&pngp, &infop, NULL);
        fclose(fp);

        return 0;
}

int png_u8rgb_write(const char* path, struct pie_bitmap_u8rgb* bitmap)
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

        png_set_gAMA(pngp, infop, 2.2);
    
        /* Write rows to PNG obj */
        rows = malloc(bitmap->height * sizeof(png_byte*));
        for (int y = 0; y < bitmap->height; y++)
        {
                size_t row_size = depth / 8 * bitmap->width * pixel_size;
                png_byte* row = malloc(row_size);
                rows[y] = row;
                for (int x = 0; x < bitmap->width; ++x)
                {
                        struct pie_pixel_u8rgb p;

                        pie_pixel_u8rgb_get(&p, bitmap, x, y);
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

int png_u16rgb_write(const char* path, struct pie_bitmap_u16rgb* bitmap)
{
        FILE* fp;
        png_structp pngp;
        png_infop infop;
        png_byte** rows;
        int pixel_size = 3; /* RGB */
        int depth = 16; 
    
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

        png_set_gAMA(pngp, infop, 2.2);
    
        /* Write rows to PNG obj */
        rows = malloc(bitmap->height * sizeof(png_byte*));
        for (int y = 0; y < bitmap->height; y++)
        {
                size_t row_size = depth / 8 * bitmap->width * pixel_size;
                png_byte* row = malloc(row_size);
                uint16_t* r16 = (uint16_t*)row;
                
                rows[y] = row;
                for (int x = 0; x < bitmap->width; ++x)
                {
                        struct pie_pixel_u16rgb p;

                        pie_pixel_u16rgb_get(&p, bitmap, x, y);
                        *r16++ = p.red;
                        *r16++ = p.green;
                        *r16++ = p.blue;
                }
        }
    
        /* Write PNG obj to disk */
        int transforms = PNG_TRANSFORM_IDENTITY;
        if (SWAP_PIXELS)
        {
                transforms = PNG_TRANSFORM_SWAP_ENDIAN;
        }
        png_init_io(pngp, fp);
        png_set_rows(pngp, infop, rows);
        png_write_png(pngp, 
                      infop,
                      transforms, 
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

