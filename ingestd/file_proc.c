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
int pie_fp_process_file(struct pie_mq_new_media*, char*);
/**
 * Calculate a message digest from the provided file descriptor.
 * @param sum to store digest in. Must be able to at least keep
 *        EVP_MAX_MD_SIZE bytes.
 * @param file descriptor.
 * @return the size of the digest in bytes, zero on error.
 */
unsigned int pie_fp_mdigest(unsigned char[], int);

static pthread_t* workers;
static int num_workers;
static struct chan* chan;
static pthread_mutex_t fs_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cp_lock = PTHREAD_MUTEX_INITIALIZER;

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
                PIE_ERR("Path is to long (%ld>%d)", msg.len, PIE_PATH_LEN);
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
        struct chan* chan = (struct chan*)arg;

        for (;;)
        {
                struct pie_mq_new_media new_mmsg;
                struct chan_msg chan_msg;
                char* path;
                int ok;

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

                if (pie_fp_process_file(&new_mmsg, path))
                {
                        PIE_WARN("Processing of %s failed", path);
                        free(chan_msg.data);
                        continue;
                }

                /* Notify mediad on new media */
                ssize_t nb;
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

int pie_fp_process_file(struct pie_mq_new_media* new_mmsg, char* path)
{
        char tmp_path[PIE_PATH_LEN];
        char hex[2 * EVP_MAX_MD_SIZE + 1];
        unsigned char sum[EVP_MAX_MD_SIZE];
        size_t src_pth_off = strlen(id_cfg.src_path) + 1; /* one extra for the / */
        struct timing t;
        ssize_t len;
        unsigned int md_len;
        int src_fd;
        int tgt_fd;
        mode_t mode = 0644;

        PIE_LOG("Process %s", path);
        src_fd = open(path, O_RDONLY);
        if (src_fd < 0)
        {
                perror("open");
                PIE_WARN("Could not open '%s'", path);
                return 1;
        }

        timing_start(&t);
        md_len = pie_fp_mdigest(sum, src_fd);
        PIE_DEBUG("Digest in %ldms", timing_dur_msec(&t));

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
                /* this is basically a simple (and destructive) 
                   implementation of basename(3C), as basename(3C) is not 
                   required to be reentrant in POSIX.1-2001 
                   (IEEE 1003.1-2001 or SUSv3) */
                char *p = strrchr(path, '/');

                if (p != NULL)
                {
                        len = strlen(p + 1);
                        memmove(path, p + 1, len);
                        path[len] = '\0';
                }
        }

        /* Create new media message */
        snprintf(new_mmsg->path,
                 PIE_PATH_LEN,
                 "%s%s",
                 id_cfg.dst_path, path);
        new_mmsg->path[PIE_PATH_LEN - 1] = '\0';
        memcpy(new_mmsg->digest, sum, md_len);
        new_mmsg->stg_id = htonl(id_cfg.dst_stg->stg.stg_id);
        new_mmsg->digest_len = htonl(md_len);
        PIE_DEBUG("New media message path: %s", new_mmsg->path);

        /* Make sure the target directory exists. */
        /* this is basically a simple (and destructive) implementation 
           of dirname(3C), as dirname(3C) is not required to be reentrant 
           in POSIX.1-2001 (IEEE 1003.1-2001 or SUSv3) */
        char *dir = strrchr(path, '/');
        if (dir)
        {
                /* remove filename from path */
                *dir = '\0';
                snprintf(tmp_path,
                         PIE_PATH_LEN,
                         "%s%s",
                         id_cfg.dst_path,
                         path);
        }
        else
        {
                snprintf(tmp_path,
                         PIE_PATH_LEN,
                         "%s",
                         id_cfg.dst_path);
        }
        pthread_mutex_lock(&fs_lock);
        PIE_TRACE("Create dir: %s", tmp_path);
        fal_mkdir_tree(id_cfg.dst_stg->mnt_path,
                       tmp_path);
        pthread_mutex_unlock(&fs_lock);

        /* Copy file to target destination */
        snprintf(tmp_path,
                 PIE_PATH_LEN,
                 "%s%s",
                 id_cfg.dst_stg->mnt_path,
                 new_mmsg->path);
        tgt_fd = open(tmp_path, O_WRONLY | O_CREAT | O_EXCL, mode);
        if (tgt_fd < 0)
        {
                PIE_WARN("File %s already exists", tmp_path);
                close(src_fd);
                return 1;
        }
        
        /* Reset src fd */
        lseek(src_fd, 0L, SEEK_SET);
        pthread_mutex_lock(&cp_lock);
        PIE_LOG("Copy file to: %s", tmp_path);
        timing_start(&t);
        len = fal_copy_fd(tgt_fd, src_fd);
        time_t ms = timing_dur_msec(&t);
        PIE_LOG("Copied %ld bytes in %ldms (%0.2fMB/s)",
                len,
                ms,
                (((float)len) / (1024.0f * 1024.0f) * 1000.0) / (float)ms);
        pthread_mutex_unlock(&cp_lock);

        if (len < 1)
        {
                perror("fal_copy_fd");
                abort();
        }
        close(tgt_fd);
        close(src_fd);

        return 0;
}

unsigned int pie_fp_mdigest(unsigned char sum[], int fd)
{
        char buf[BUF_LEN];
        EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
        const EVP_MD* md = EVP_sha1();
        ENGINE* impl = NULL;
        ssize_t nb;
        unsigned int md_len;

        EVP_DigestInit_ex(mdctx, md, impl);
        while ((nb = read(fd, buf, BUF_LEN)) > 0)
        {
                EVP_DigestUpdate(mdctx, buf, nb);
        }

        EVP_DigestFinal_ex(mdctx, sum, &md_len);
        EVP_MD_CTX_destroy(mdctx);

        return md_len;
}