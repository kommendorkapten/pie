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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_proc.h"
#include "ingestd_cfg.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../lib/fswalk.h"
#include "../lib/s_queue.h"
#include "../lib/evp_hw.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"

static void cb_fun(const char*);

struct ingestd_cfg id_cfg;

int main(int argc, char** argv)
{
	char path[PIE_PATH_LEN];
        int ok;
        int status = -1;
        long evp_hw;
        long lval;
        int num_workers = 4;

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf\n");
                goto cleanup;
        }

        /* E V P */
        if (pie_cfg_get_long("ssl:evp:hw", &evp_hw))
        {
                evp_hw = 0;
        }
        evp_enable_hw((int)evp_hw);

        /* H O S T */
        id_cfg.host = pie_cfg_get_host(-1);
        if (id_cfg.host == NULL)
        {
                PIE_ERR("Could not load current host");
                goto cleanup;
        }

        /* S T O R A G E S */
        id_cfg.storages = pie_cfg_get_hoststg(id_cfg.host->hst_id);
        if (id_cfg.storages == NULL)
        {
                PIE_ERR("Could not resolve mount point for storage");
                goto cleanup;
        }

        /* W O R K E R S */
        if (pie_cfg_get_long("global:workers", &lval))
        {
                PIE_LOG("Config global:workers not found in config");
        }
        else
        {
                num_workers = (int)lval;
        }
        pie_fp_start_workers(num_workers);

        PIE_LOG("Using %d workers", num_workers);

        /* Q U E U E S */
        id_cfg.queue = q_new_producer(QUEUE_INTRA_HOST);
        if (id_cfg.queue == NULL)
        {
                PIE_ERR("Can not create queue");
                goto cleanup;
        }

        ok = id_cfg.queue->init(id_cfg.queue->this, Q_INCOMING_MEDIA);
        if (ok)
        {
                PIE_ERR("Can not connect to '%s'", Q_INCOMING_MEDIA);
                goto cleanup;
        }

        id_cfg.stg_mnt_len = malloc(sizeof(int) * id_cfg.storages->len);
        for (int i = 0; i < id_cfg.storages->len; i++)
        {
                if (id_cfg.storages->arr[i])
                {
                        PIE_DEBUG("Storage %d at %s",
                                  id_cfg.storages->arr[i]->stg.stg_id,
                                  id_cfg.storages->arr[i]->mnt_path);
                        id_cfg.stg_mnt_len[i] = (int)strlen(id_cfg.storages->arr[i]->mnt_path);
                }
        }

        if (argc > 1)
        {
                size_t len;
                strncpy(path, argv[1], PIE_PATH_LEN);

                /* Remove trailing slash */
                len = strlen(path);
                if (path[len - 1] == '/')
                {
                        path[len - 1] = '\0';
                }
        }
        else
        {
                strcpy(path, ".");
        }

        walk_dir(path, &cb_fun);
        status = 0;

        /* Stop and wait for workers */
        pie_fp_stop_workers();
cleanup:
        if (id_cfg.queue)
        {
                id_cfg.queue->close(id_cfg.queue->this);
                q_del_producer(id_cfg.queue);
        }
        if (id_cfg.host)
        {
                pie_host_free(id_cfg.host);
                id_cfg.host = NULL;
        }
        if (id_cfg.storages)
        {
                pie_cfg_free_hoststg(id_cfg.storages);
                id_cfg.storages = NULL;
        }
        if (id_cfg.stg_mnt_len)
        {
                free(id_cfg.stg_mnt_len);
                id_cfg.stg_mnt_len = NULL;
        }
        pie_cfg_close();

	return status;
}



static void cb_fun(const char* path)
{
        pie_fp_add_job(path);
}

#if 0
static void cb_fun(const char* path)
{
        struct pie_mq_new_media msg;
        struct pie_stg_mnt* stg;
        const char* p;
        ssize_t bw;

        stg = stg_from_path(path);
        if (stg == NULL)
        {
                PIE_WARN("Non storage mounted path: '%s'", path);
                return;
        }
        p = path + id_cfg.offset[stg->stg.stg_id];
        /* p starts with '/' */
        if (p[1] == '.')
        {
                /* hidden file */
                return;
        }

        sprintf(msg.path, "%s", p);
        msg.storage = htonl(stg->stg.stg_id);
        bw = id_cfg.queue->send(id_cfg.queue->this, (char*)&msg, sizeof(msg));
        if (bw != sizeof(msg))
        {
                perror("queue->send");
                PIE_WARN("Could not send messsage to queue");
                abort();
        }
}
#endif
