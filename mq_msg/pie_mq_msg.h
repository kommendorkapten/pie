/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
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

#ifndef __PIE_MQ_MSG_H__
#define __PIE_MQ_MSG_H__

#include <stddef.h>
#include "../pie_types.h"
#include "../pie_id.h"

#define pie_mq_swap64(x) ( ((x) & 0x00000000000000ff) << 56 | \
                           ((x) & 0x000000000000ff00) << 40 | \
                           ((x) & 0x0000000000ff0000) << 24 | \
                           ((x) & 0x00000000ff000000) <<  8 | \
                           ((x) & 0x000000ff00000000) >>  8 | \
                           ((x) & 0x0000ff0000000000) >> 24 | \
                           ((x) & 0x00ff000000000000) >> 40 | \
                           ((x) & 0xff00000000000000) >> 56 )

#if defined (__BYTE_ORDER__)
# if __BYTE_ORDER__ == 4321
/* Big endian */
#  define pie_ntohll(x) (x)
#  define pie_htonll(x) (x)
# else
/* Small endian */
#  define pie_ntohll(x) pie_mq_swap64(x)
#  define pie_htonll(x) pie_mq_swap64(x)
# endif
#elif defined (__BIG_ENDIAN__)
# define ntohll(x) (x)
# define htonll(x) (x)
#elif defined (__SMALL_ENDIAN__)
# define pie_ntohll(x) pie_mq_swap64(x)
# define pie_htonll(x) pie_mq_swap64(x)
#else
# error __BYTE_ORDER__, __BIG_ENDIAN__  or __SMALL_ENDIAN__ not defined
#endif

#define Q_INCOMING_MEDIA "/tmp/pie_new_media.sock"
#define Q_UPDATE_META    "/tmp/pie_update_meta.sock"
#define Q_EXPORT_MEDIA   "/tmp/pie_export_media.sock"
#define PIE_MQ_MAX_DIGEST 64 /* large enough for SHA-512 */
#define PIE_MQ_MAX_UPD 4096

enum pie_mq_upd_media_type
{
        PIE_MQ_UPD_MEDIA_MOB,
        PIE_MQ_UPD_MEDIA_EXIF,
        PIE_MQ_UPD_MEDIA_SETTINGS,
        PIE_MQ_UPD_MEDIA_COUNT
};

struct pie_mq_new_media
{
        /* path starts with / and is relative to the storage's mount point */
        char path[PIE_PATH_LEN];
        unsigned char digest[PIE_MQ_MAX_DIGEST];
        size_t file_size;
        int stg_id;
        unsigned int digest_len;
};

struct pie_mq_upd_media
{
        enum pie_mq_upd_media_type type;
        pie_id mob_id;
        /* JSON encoded data of the development settings */
        char msg[PIE_MQ_MAX_UPD];
};

enum pie_mq_export_type
{
        PIE_MQ_EXP_INV,
        PIE_MQ_EXP_JPG,
        PIE_MQ_EXP_PNG8,
        PIE_MQ_EXP_PNG16
};

struct pie_mq_export_media
{
        /* Complete path/filename relative storage mount point */
        char path[PIE_PATH_LEN];
        pie_id mob_id;
        int stg_id;
        /* -1 to don't care */
        int max_x;
        /* -1 to don't care */
        int max_y;
        enum pie_mq_export_type type;
        /* Only valid for JPG */
        int quality;
        unsigned char sharpen;
        unsigned char disable_exif;


};

#endif /* __PIE_MQ_MSG_H__ */
