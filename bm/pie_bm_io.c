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

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "pie_bm.h"
#include "pie_bm_png.h"
#include "pie_bm_jpg.h"
#include "pie_bm_raw.h"
#include "pie_bm_cspace.h"
#include "../pie_log.h"

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

int pie_bm_load(struct pie_bm_f32rgb* bm,
                const char* path,
                struct pie_bm_opts* opts)
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
                return PIE_BM_IO_NOT_FOUND;
        }
        br = read(fd, magic, PIE_MAGIC_MAX);
        close(fd);
        if (br < 1)
        {
                return PIE_BM_IO_IO_ERR;
        }
        ft = pie_get_magic(magic, br);

        int conv_cspace = 0;
        switch (ft)
        {
        case PIE_FT_JPG:
                ret = pie_bm_jpg_f32_read(bm, path);
                conv_cspace = 1;
                break;
        case PIE_FT_PNG:
                ret = pie_bm_png_f32_read(bm, path);
                conv_cspace = 1;
                break;
        case PIE_FT_UNKNOWN:
                /* Assume a RAW format, try to load */
                ret = pie_bm_raw_f32_read(bm, path, opts);
                break;
        default:
                ret = PIE_BM_IO_UNSUPPORTED_FMT;
        }

        if (conv_cspace && opts)
        {
                switch (opts->cspace)
                {
                case PIE_BM_LINEAR:
                        pie_bm_srgb_to_linearv(bm->c_red,
                                                bm->height * bm->row_stride);
                        pie_bm_srgb_to_linearv(bm->c_green,
                                                bm->height * bm->row_stride);
                        pie_bm_srgb_to_linearv(bm->c_blue,
                                                bm->height * bm->row_stride);
                        break;
                case PIE_BM_SRGB:
                        break;
                default:
                        pie_bm_free_f32(bm);
                        ret = PIE_BM_IO_INV_OPT;
                }
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
