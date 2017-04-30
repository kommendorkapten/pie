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

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <assert.h>
#include <errno.h>
#include "ingestd_cfg.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../lib/chan.h"
#include "../lib/s_queue.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"
#include "../mq_msg/pie_mq_msg.h"

#define BUF_LEN (1<<14) /* 16kB */

static void* worker(void*);
static const struct pie_stg_mnt* stg_from_path(const char*);

static pthread_t* workers;
static int num_workers;
static struct chan* chan;

int pie_fp_start_workers(int w)
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
                        PIE_WARN("pthread_create failed");
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

int pie_fp_add_job(const char* p)
{
        struct chan_msg msg;

        assert(chan);
        msg.len = strlen(p) + 1;
        if (msg.len > PIE_PATH_LEN)
        {
                PIE_ERR("Path is to long (%d>%d)", msg.len, PIE_PATH_LEN);
                return -1;
        }
        msg.data = malloc(msg.len);
        memcpy(msg.data, p, msg.len);
        
        if(chan_write(chan, &msg))
        {
                return -1;
        }

        return 0;
}

void pie_fp_stop_workers(void)
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
        char hex[2 * EVP_MAX_MD_SIZE + 1];
        unsigned char sum[EVP_MAX_MD_SIZE];
        struct pie_mq_new_media new_mmsg;
        struct chan* chan = (struct chan*)arg;
        EVP_MD_CTX* mdctx;
        const EVP_MD* md = EVP_sha1();
        ENGINE* impl = NULL;

        mdctx = EVP_MD_CTX_create();

        for (;;)
        {
                struct chan_msg chan_msg;
                const struct pie_stg_mnt* stg;
                const char* path;
                ssize_t nb;
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

                path = chan_msg.data;
                stg = stg_from_path(path);
                if (stg == NULL)
                {
                        PIE_WARN("Non storage mounted path '%s'",
                                 path);
                        free(chan_msg.data);
                        continue;
                }

                EVP_DigestInit_ex(mdctx, md, impl);

                fd = open(path, O_RDONLY);
                if (fd < 0)
                {
                        perror("open");
                        PIE_WARN("Could not open '%s'", path);
                        free(chan_msg.data);
                        continue;
                }

                while ((nb = read(fd, buf, BUF_LEN)) > 0)
                {
                        EVP_DigestUpdate(mdctx, buf, nb);
                }
                close(fd);
                EVP_DigestFinal_ex(mdctx, sum, &md_len);
                EVP_MD_CTX_destroy(mdctx);

                /* hex encode */
                for (unsigned int i = 0; i < md_len; i++)
                {
                        sprintf(hex + i * 2, "%02x", sum[i]);
                }
                hex[md_len * 2 + 1] = '\0';
                PIE_LOG("sha1sum: [%s] %s", path, hex);
                printf("%s %d\n", path, stg->stg.stg_id);
                /* Compare in database for duplicate based on digest */

                /* Copy file to target destination */

                /* Notify mediad on new media */
                path = path + id_cfg.stg_mnt_len[stg->stg.stg_id];
                strncpy(new_mmsg.path, path, PIE_PATH_LEN);
                memcpy(new_mmsg.digest, sum, md_len);
                new_mmsg.stg_id = htonl(stg->stg.stg_id);
                new_mmsg.digest_len = htonl(md_len);

                nb = id_cfg.queue->send(id_cfg.queue->this,
                                        (char*)&new_mmsg,
                                        sizeof(new_mmsg));
                if (nb != sizeof(new_mmsg))
                {
                        perror("queue->send");
                        PIE_ERR("Could not send messsage to queue");
                        abort();
                }

                free(chan_msg.data);
        }

        return NULL;
}

static const struct pie_stg_mnt* stg_from_path(const char* p)
{
        struct pie_stg_mnt* stg = NULL;

        for (int i = 0; i < id_cfg.storages->len; i++)
        {
                char* pos;

                stg = id_cfg.storages->arr[i];

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
