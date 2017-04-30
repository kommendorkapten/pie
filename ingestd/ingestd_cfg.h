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

#ifndef __INGESTD_CFG_H__
#define __INGESTD_CFG_H__

struct pie_host;
struct pie_stg_mnt_arr;
struct q_producer;

struct ingestd_cfg
{
        struct pie_host* host;
        struct pie_stg_mnt_arr* storages;
        struct q_producer* queue;
        int* stg_mnt_len;
};

extern struct ingestd_cfg id_cfg;

#endif /* __INGESTD_CFG_H__ */
