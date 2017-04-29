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
#include <openssl/evp.h>
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

int nm_start_workers(int w)
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

int nm_add_job(const struct pie_mq_new_media* m)
{
        struct pie_mq_new_media* new = alloc_msg();
        struct chan_msg msg;

        memcpy(new, m, sizeof(struct pie_mq_new_media));
        msg.data = new;
        msg.len = sizeof(struct pie_mq_new_media);

        if(chan_write(chan, &msg))
        {
                return -1;
        }

        return 0;
}

void nm_stop_workers(void)
{
        if (chan == NULL)
        {
                return;
        }

        chan_destroy(chan);

        for (int i = 0; i < num_workers; i++)
        {
                pthread_join(workers[i], NULL);
        }
        free(workers);

        chan = NULL;
        workers = NULL;
}

static void* worker(void* arg)
{
        char buf[BUF_LEN];
        char path[PIE_PATH_LEN];
        unsigned char sum[EVP_MAX_MD_SIZE];
        char hex[2 * EVP_MAX_MD_SIZE + 1];
        struct chan* chan = (struct chan*)arg;
        EVP_MD_CTX* mdctx;
        const EVP_MD* md = EVP_sha1();
        ENGINE* impl = NULL;

        mdctx = EVP_MD_CTX_create();

        for (;;)
        {
                struct chan_msg chan_msg;
                struct pie_mq_new_media* new;
                struct pie_stg_mnt* stg;
                ssize_t br;
                unsigned int md_len;
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
                new->storage = ntohl(new->storage);

                stg = PIE_CFG_GET_STORAGE(md_cfg.storages, new->storage);

                if (stg == NULL)
                {
                        PIE_WARN("Unknown storage: %d\n", new->storage);
                }

                snprintf(path, PIE_PATH_LEN, "%s%s", stg->mnt_path, new->path);
                PIE_LOG("Storage: %d", new->storage);
                PIE_LOG("New media: %s", new->path);

                EVP_DigestInit_ex(mdctx, md, impl);

                fd = open(path, O_RDONLY);
                if (fd < 0)
                {
                        perror("open");
                        PIE_WARN("Could not open '%s'", path);
                        continue;
                }

                while ((br = read(fd, buf, BUF_LEN)) > 0)
                {
                        EVP_DigestUpdate(mdctx, buf, br);
                }
                close(fd);
                EVP_DigestFinal_ex(mdctx, sum, &md_len);
                EVP_MD_CTX_destroy(mdctx);

                free_msg(new);

                /* hex encode */
                for (unsigned int i = 0; i < md_len; i++)
                {
                        sprintf(hex + i * 2, "%02x", sum[i]);
                }
                sum[md_len] = '\0';
                PIE_LOG("sha1sum: [%s] %s", path, hex);
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
