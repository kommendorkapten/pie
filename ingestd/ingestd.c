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
#include <netinet/in.h>
#include "../pie_types.h"
#include "../pie_log.h"
#include "../lib/fswalk.h"
#include "../lib/s_queue.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"

static void cb_fun(const char*);

static struct
{
        struct pie_host* host;
        struct pie_stg_mnt_arr* storages;
        struct q_producer* queue;
        int* offset;
} cfg;

static struct pie_stg_mnt* stg_from_path(const char*);

int main(int argc, char** argv)
{
	char path[PIE_PATH_LEN];
        int ok;
        int status = -1;

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf\n");
                goto cleanup;
        }
        cfg.host = pie_cfg_get_host(-1);
        if (cfg.host == NULL)
        {
                PIE_ERR("Could not load current host");
                goto cleanup;
        }
        cfg.storages = pie_cfg_get_hoststg(cfg.host->hst_id);
        if (cfg.storages == NULL)
        {
                PIE_ERR("Could not resolve mount point for storage");
                goto cleanup;
        }

        cfg.queue = q_new_producer(QUEUE_INTRA_HOST);
        if (cfg.queue == NULL)
        {
                PIE_ERR("Can not create queue");
                goto cleanup;
        }

        ok = cfg.queue->init(cfg.queue->this, Q_INCOMING_MEDIA);
        if (ok)
        {
                PIE_ERR("Can not connect to '%s'", Q_INCOMING_MEDIA);
                goto cleanup;
        }

        cfg.offset = malloc(sizeof(int) * cfg.storages->len);
        for (int i = 0; i < cfg.storages->len; i++)
        {
                if (cfg.storages->arr[i])
                {
                        PIE_DEBUG("Storage %d at %s",
                                  cfg.storages->arr[i]->stg.stg_id,
                                  cfg.storages->arr[i]->mnt_path);
                        cfg.offset[i] = (int)strlen(cfg.storages->arr[i]->mnt_path);
                }
        }

        if (argc > 1)
        {
                strncpy(path, argv[1], PIE_PATH_LEN);
        }
        else
        {
                strcpy(path, ".");
        }

        walk_dir(path, &cb_fun);
        status = 0;

cleanup:
        if (cfg.queue)
        {
                cfg.queue->close(cfg.queue->this);
                q_del_producer(cfg.queue);
        }
        if (cfg.host)
        {
                pie_host_free(cfg.host);
                cfg.host = NULL;
        }
        if (cfg.storages)
        {
                pie_cfg_free_hoststg(cfg.storages);
                cfg.storages = NULL;
        }
        if (cfg.offset)
        {
                free(cfg.offset);
                cfg.offset = NULL;
        }
        pie_cfg_close();

	return status;
}

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
        p = path + cfg.offset[stg->stg.stg_id];
        /* p starts with '/' */
        if (p[1] == '.')
        {
                /* hidden file */
                return;
        }

        sprintf(msg.path, "%s", p);
        msg.storage = htonl(stg->stg.stg_id);
        bw = cfg.queue->send(cfg.queue->this, (char*)&msg, sizeof(msg));
        if (bw != sizeof(msg))
        {
                perror("queue->send");
                PIE_WARN("Could not send messsage to queue");
                abort();
        }
}

static struct pie_stg_mnt* stg_from_path(const char* p)
{
        struct pie_stg_mnt* stg = NULL;

        for (int i = 0; i < cfg.storages->len; i++)
        {
                char* pos;

                stg = cfg.storages->arr[i];

                if (stg == NULL)
                {
                        continue;
                }

                pos = strstr(p, stg->mnt_path);
                if (pos == NULL || pos != p)
                {
                        continue;
                }

                break;
        }

        return stg;
}
