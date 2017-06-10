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

#include "../pie_types.h"

#define Q_INCOMING_MEDIA "/tmp/pie_new_media.sock"
#define Q_UPDATE_META    "/tmp/pie_update_meta.sock"
#define PIE_MAX_DIGEST 64 /* large enough for SHA-512 */

struct pie_mq_new_media
{
        /* path starts with / and is relative to the storage's mount point */
        char path[PIE_PATH_LEN];
        unsigned char digest[PIE_MAX_DIGEST];
        int stg_id;
        int digest_len;
};

#endif /* __PIE_MQ_MSG_H__ */
