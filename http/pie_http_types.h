/*
* Copyright (C) 2018 Fredrik Skogman, skogman - at - gmail.com.
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

#ifndef __PIE_HTTP_TYPES_H__
#define __PIE_HTTP_TYPES_H__

#include "../pie_defs.h"

#define HTTP_HOSTNAME_LEN 128

struct llist;

struct pie_http_export_request
{
        char path[PIE_PATH_LEN];
        /* list of mob ids */
        struct llist* mobs;
        int max_x;
        int max_y;
        unsigned char sharpen;
        unsigned char disable_exif;
};

struct pie_http_storage_item
{
        int id;
        char name[HTTP_HOSTNAME_LEN];
        char type[HTTP_HOSTNAME_LEN];
        char hostname[HTTP_HOSTNAME_LEN];
        char fqdn[HTTP_HOSTNAME_LEN];
};

struct pie_http_storages_resp
{
        /* list of pie_http_storage_items */
        struct llist* storages;
};

#endif /* __PIE_HTTP_TYPES_H__ */
