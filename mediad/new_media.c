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
#include "../lib/timing.h"
#include "../lib/strutil.h"
#include "../cfg/pie_cfg.h"
#include "../bm/pie_bm.h"
#include "../bm/pie_dwn_smpl.h"
#include "../io/pie_io.h"
#include "../io/pie_io_jpg.h"

static void* worker(void*);
static struct pie_mq_new_media* alloc_msg(void);
static void free_msg(struct pie_mq_new_media*);
/**
 * Write a downsampled version to disk.
 * @param the bitmap to downsample.
 * @param maximum size (in both w and h).
 * @param complete path to write file to.
 * @return 0 on success.
 */
static int write_dwn_smpl(struct pie_bitmap_f32rgb*, int, const char*);

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
        struct chan* chan = (struct chan*)arg;
        struct pie_stg_mnt* proxy_stg = NULL;
        struct pie_stg_mnt* thumb_stg = NULL;

        for (int i = 0; i < md_cfg.storages->len; i++)
        {
                if (md_cfg.storages->arr[i])
                {
                        struct pie_stg_mnt* stg;

                        stg = md_cfg.storages->arr[i];
                        switch(stg->stg.stg_type)
                        {
                        case PIE_STG_THUMB:
                                thumb_stg = stg;
                                break;
                        case PIE_STG_PROXY:
                                proxy_stg = stg;
                                break;
                        }
                }
        }
        
        for (;;)
        {
                char src_path[PIE_PATH_LEN];
                char tgt_path[PIE_PATH_LEN];
                char hex[2 * PIE_MAX_DIGEST + 1];
                struct pie_bitmap_f32rgb bm_src;
                struct chan_msg chan_msg;
                struct timing t;
                struct pie_mq_new_media* new;
                struct pie_stg_mnt* stg;
                ssize_t br;
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
                new = chan_msg.data;
                new->stg_id = ntohl(new->stg_id);
                new->digest_len = ntohl(new->digest_len);

                stg = PIE_CFG_GET_STORAGE(md_cfg.storages, new->stg_id);

                if (stg == NULL)
                {
                        PIE_WARN("Unknown storage: %d\n", new->stg_id);
                        goto next_msg;
                }

                snprintf(src_path,
                         PIE_PATH_LEN,
                         "%s%s",
                         stg->mnt_path,
                         new->path);
                /* hex encode digest */
                for (unsigned int i = 0; i < new->digest_len; i++)
                {
                        sprintf(hex + i * 2, "%02x", new->digest[i]);
                }
                hex[new->digest_len * 2 + 1] = '\0';
                PIE_LOG("New media: [%s] %s", hex, src_path);                

                timing_start(&t);                
                ok = pie_io_load(&bm_src, src_path);
                PIE_DEBUG("Loaded %s in %ldms", src_path, timing_dur_msec(&t));
                if (ok)
                {
                        PIE_ERR("Could not load %s", src_path);
                        goto next_msg;
                }
                
                snprintf(tgt_path,
                         PIE_PATH_LEN,
                         "%s/%s.jpg",
                         thumb_stg->mnt_path,
                         hex);
                PIE_DEBUG("Thumb: %s", tgt_path);
                if (write_dwn_smpl(&bm_src, md_cfg.max_thumb, tgt_path))
                {
                        pie_bm_free_f32(&bm_src);
                        goto next_msg;                        
                }
                snprintf(tgt_path,
                         PIE_PATH_LEN,
                         "%s/%s.jpg",
                         proxy_stg->mnt_path,
                         hex);
                PIE_DEBUG("Proxy: %s", tgt_path);
                if (write_dwn_smpl(&bm_src, md_cfg.max_proxy, tgt_path))
                {
                        pie_bm_free_f32(&bm_src);
                        goto next_msg;                        
                }
          
                pie_bm_free_f32(&bm_src);
        next_msg:
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

static int write_dwn_smpl(struct pie_bitmap_f32rgb* src, int max, const char* p)
{
        struct pie_bitmap_f32rgb bm_dwn;
        struct pie_bitmap_u8rgb bm_out;
        struct timing t;
        int ok;

        timing_start(&t);
        if (pie_bm_dwn_smpl(&bm_dwn,
                            src,
                            max,
                            max))
        {
                PIE_ERR("Failed to downsample");
                return -1;
        }
        PIE_DEBUG("Downsampeld tp %dpx in %ldms",
                  max,
                  timing_dur_msec(&t));
        
        timing_start(&t);
        if (pie_bm_conv_bd(&bm_out,
                           PIE_COLOR_8B,
                           &bm_dwn,
                           PIE_COLOR_32B))
        {
                PIE_ERR("Could not convert to 8b");
                pie_bm_free_f32(&bm_dwn);                        
                return -1;
        }
        PIE_DEBUG("Converted to 8b in %ldms", timing_dur_msec(&t));

        timing_start(&t);
        ok = jpg_u8rgb_write(p, &bm_out, 90);
        PIE_DEBUG("Wrote %s in %ldms", p, timing_dur_msec(&t));
        if (ok)
        {
                PIE_ERR("Could not write output '%s', code %d",
                        p,
                        ok);
        }
                
        pie_bm_free_u8(&bm_out);
        pie_bm_free_f32(&bm_dwn);

        return ok;
}
