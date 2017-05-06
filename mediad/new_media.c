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
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include "mediad_cfg.h"
#include "../pie_log.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../lib/chan.h"
#include "../cfg/pie_cfg.h"

#define BUF_LEN (1<<14) /* 16kB */

static void* worker(void*);
static struct pie_mq_new_media* alloc_msg(void);
static void free_msg(struct pie_mq_new_media*);

static pthread_t* workers;
static int num_workers;
static struct chan* chan;

int pie_nm_start_workers(int w)
{
        int i;

        assert(workers == NULL);
        assert(chan == NULL);

        chan = chan_create();

        if (chan == NULL)
        {
                return -1;
        }

        num_workers = w;
        workers = malloc(sizeof(pthread_t) * num_workers);

        for (i = 0; i < num_workers; i++)
        {
                if (pthread_create(workers + i,
                                   NULL,
                                   &worker,
                                   (void*)chan))
                {
                        break;
                }
        }
        if (i == 0)
        {
                return -1;
        }
        num_workers = i;

        PIE_LOG("%d workers created", num_workers);

        return 0;
}

int pie_nm_add_job(const struct pie_mq_new_media* m)
{
        struct pie_mq_new_media* new = alloc_msg();
        struct chan_msg msg;

        assert(chan);
        memcpy(new, m, sizeof(struct pie_mq_new_media));
        msg.data = new;
        msg.len = sizeof(struct pie_mq_new_media);

        if(chan_write(chan, &msg))
        {
                return -1;
        }

        return 0;
}

void pie_nm_stop_workers(void)
{
        if (chan == NULL)
        {
                return;
        }

        chan_close(chan);

        for (int i = 0; i < num_workers; i++)
        {
                pthread_join(workers[i], NULL);
        }
        free(workers);
        chan_destroy(chan);

        chan = NULL;
        workers = NULL;
}

static void* worker(void* arg)
{
        char buf[BUF_LEN];
        char path[PIE_PATH_LEN];
        char hex[2 * PIE_MAX_DIGEST + 1];
        struct chan* chan = (struct chan*)arg;

        for (;;)
        {
                struct chan_msg chan_msg;
                struct pie_mq_new_media* new;
                struct pie_stg_mnt* stg;
                ssize_t br;
                int ok;
                int fd;

                ok = chan_read(chan, &chan_msg, -1);
                if (ok)
                {
                        if (ok == EAGAIN)
                        {
                                continue;
                        }
                        else
                        {
                                break;
                        }
                }
                new = chan_msg.data;
                new->stg_id = ntohl(new->stg_id);
                new->digest_len = ntohl(new->digest_len);

                stg = PIE_CFG_GET_STORAGE(md_cfg.storages, new->stg_id);

                if (stg == NULL)
                {
                        PIE_WARN("Unknown storage: %d\n", new->stg_id);
                        free_msg(new);
                        continue;
                }

                snprintf(path, PIE_PATH_LEN, "%s%s", stg->mnt_path, new->path);
                PIE_LOG("Storage: %d", new->stg_id);
                PIE_LOG("New media: %s", new->path);
                PIE_LOG("Digest len: %d", new->digest_len);


                /* hex encode */
                for (unsigned int i = 0; i < new->digest_len; i++)
                {
                        sprintf(hex + i * 2, "%02x", new->digest[i]);
                }
                hex[new->digest_len * 2 + 1] = '\0';
                PIE_LOG("sha1sum: [%s] %s", path, hex);

                fd = open(path, O_RDONLY);
                if (fd < 0)
                {
                        perror("open");
                }
                else
                {
                        close(fd);
                }

                free_msg(new);
        }

        return NULL;
}

static struct pie_mq_new_media* alloc_msg(void)
{
        return malloc(sizeof(struct pie_mq_new_media));
}

static void free_msg(struct pie_mq_new_media* m)
{
        free(m);
}
