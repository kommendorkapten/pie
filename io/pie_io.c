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
#include "../pie_log.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define PIE_MAGIC_MAX 16

enum pie_file_type
{
        PIE_FT_JPG,
        PIE_FT_PNG,
        /* PIE_FT_COUNT must immediate follow the last valid format */
        PIE_FT_COUNT,
        PIE_FT_UNKNOWN
};

struct pie_magic
{
        unsigned char magic[PIE_MAGIC_MAX];
        enum pie_file_type type;
        ssize_t len;
};

static struct pie_magic fmts[PIE_FT_COUNT] = {
        {
                .magic = {0xFF, 0xD8, 0xFF},
                .type = PIE_FT_JPG,
                .len = 3
        },
        {
                .magic = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A},
                .type = PIE_FT_PNG,
                .len = 8
        }
};

static enum pie_file_type pie_get_magic(unsigned char magic[PIE_MAGIC_MAX],
                                        ssize_t len);

int pie_io_load(struct pie_bitmap_f32rgb* bm, const char* path)
{
        /* Read the magic bytes */
        unsigned char magic[PIE_MAGIC_MAX];

        ssize_t br;
        int fd;
        int ret;
        enum pie_file_type ft;

        fd = open(path, O_RDONLY);
        if (fd < 0)
        {
                return PIE_IO_NOT_FOUND;
        }
        br = read(fd, magic, PIE_MAGIC_MAX);
        close(fd);
        if (br < 1)
        {
                return PIE_IO_IO_ERR;
        }
        ft = pie_get_magic(magic, br);

        switch (ft)
        {
        case PIE_FT_JPG:
                ret = pie_io_jpg_f32_read(bm, path);
                break;
        case PIE_FT_PNG:
                ret = pie_io_png_f32_read(bm, path);
                break;
        case PIE_FT_UNKNOWN:
                /* Assume a RAW format, try to load */
                ret = pie_io_raw_f32_read(bm, path);
                break;
        default:
                ret = PIE_IO_UNSUPPORTED_FMT;
        }
        
        return ret;
}


static enum pie_file_type pie_get_magic(unsigned char magic[PIE_MAGIC_MAX],
                                        ssize_t len)
{
        for (int i = 0; i < PIE_FT_COUNT; i++)
        {
                ssize_t stop = len < fmts[i].len ? len : fmts[i].len;
                int eq = 1;

                PIE_TRACE("Format: %d.", fmts[i].type);
                for (ssize_t j = 0; j < stop; j++)
                {
                        PIE_TRACE("%d %d", magic[j], fmts[i].magic[j]);

                        if (magic[j] != fmts[i].magic[j])
                        {
                                eq = 0;
                                break;
                        }
                }

                if (eq)
                {
                        return fmts[i].type;
                }
        }

        return PIE_FT_UNKNOWN;
}
