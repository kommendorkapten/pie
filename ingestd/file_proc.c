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
#include "../lib/fal.h"
#include "../lib/timing.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"
#include "../mq_msg/pie_mq_msg.h"

#define BUF_LEN (1<<14) /* 16kB */

static void* worker(void*);
static ssize_t copy(int, int);

static pthread_t* workers;
static int num_workers;
static struct chan* chan;
static pthread_mutex_t fs_lock = PTHREAD_MUTEX_INITIALIZER;

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
        msg.data = strdup(p);

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
        size_t src_pth_off = strlen(id_cfg.src_path) + 1; /* one extra for the / */

        mdctx = EVP_MD_CTX_create();

        for (;;)
        {
                struct chan_msg chan_msg;
                struct timing t;
                char* path;
                ssize_t len;
                ssize_t nb;
                unsigned int md_len;
                int ok;
                int src_fd;
                int tgt_fd;
                mode_t mode = 0644;

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
                /* path is the absolute path for src file on local disk */
                path = chan_msg.data;
                PIE_LOG("Open %s", path);
                src_fd = open(path, O_RDONLY);
                if (src_fd < 0)
                {
                        perror("open");
                        PIE_WARN("Could not open '%s'", path);
                        free(chan_msg.data);
                        continue;
                }

                timing_start(&t);
                EVP_DigestInit_ex(mdctx, md, impl);
                while ((nb = read(src_fd, buf, BUF_LEN)) > 0)
                {
                        EVP_DigestUpdate(mdctx, buf, nb);
                }

                EVP_DigestFinal_ex(mdctx, sum, &md_len);
                EVP_MD_CTX_destroy(mdctx);
                PIE_TRACE("Digest in %dms", timing_dur_msec(&t));

                /* hex encode */
                for (unsigned int i = 0; i < md_len; i++)
                {
                        sprintf(hex + i * 2, "%02x", sum[i]);
                }
                hex[md_len * 2 + 1] = '\0';
                /* Compare in database for duplicate based on digest */

                /* strip id_cfg.src_path from path */
                PIE_TRACE("Source path: %s", path);
                len = strlen(path) - src_pth_off;
                memmove(path, path + src_pth_off, len);
                path[len] = '\0';
                PIE_TRACE("Stripped path: %s", path);

                if (id_cfg.cp_mode == MODE_COPY_INTO)
                {
                        /* remove preceding dirs */
                        char *p = strrchr(path, '/');

                        if (p != NULL)
                        {
                                len = strlen(p + 1);
                                memmove(path, p + 1, len);
                                path[len] = '\0';
                        }
                }

                /* Create new media message */
                snprintf(new_mmsg.path,
                         PIE_PATH_LEN,
                         "%s%s",
                         id_cfg.dst_path, path);
                new_mmsg.path[PIE_PATH_LEN - 1] = '\0';
                memcpy(new_mmsg.digest, sum, md_len);
                new_mmsg.stg_id = htonl(id_cfg.dst_stg->stg.stg_id);
                new_mmsg.digest_len = htonl(md_len);
                PIE_DEBUG("New media message path: %s", new_mmsg.path);

                /* ensure directory */
                char *dir = strrchr(path, '/');
                if (dir)
                {
                        /* remove filename from path */
                        *dir = '\0';
                        snprintf(buf,
                                 BUF_LEN,
                                 "%s%s",
                                 id_cfg.dst_path,
                                 path);
                }
                else
                {
                        snprintf(buf,
                                 BUF_LEN,
                                 "%s",
                                 id_cfg.dst_path);
                }
                pthread_mutex_lock(&fs_lock);
                PIE_TRACE("Create dir: %s", buf);
                fal_mkdir_tree(id_cfg.dst_stg->mnt_path,
                               buf);
                pthread_mutex_unlock(&fs_lock);

                /* Copy file to target destination */
                snprintf(buf,
                         BUF_LEN,
                         "%s%s",
                         id_cfg.dst_stg->mnt_path,
                         new_mmsg.path);
                PIE_LOG("Copy file to: %s", buf);

                tgt_fd = open(buf, O_WRONLY | O_CREAT | O_EXCL, mode);
                if (tgt_fd < 0)
                {
                        perror("open:tgt_fd");
                        abort();
                }
                /* Reset src fd */
                lseek(src_fd, 0L, SEEK_SET);
                timing_start(&t);
                len = fal_copy_fd(tgt_fd, src_fd);
                time_t ms = timing_dur_msec(&t);

                if (len < 1)
                {
                        perror("fal_copy_fd");
                        abort();
                }
                close(tgt_fd);
                close(src_fd);
                PIE_LOG("Copied %ld bytes in %ldms (%0.2fMB/s)",
                        len,
                        ms,
                        (((float)len) / (1024.0f * 1024.0f) * 1000.0) / (float)ms);

                /* Notify mediad on new media */
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
