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

#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include "../pie_types.h"
#include "../bm/pie_bm.h"
#include "../bm/pie_render.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_min.h"
#include "../dm/pie_dev_params.h"
#include "../doml/pie_doml_stg.h"
#include "../io/pie_io.h"
#include "../lib/llist.h"
#include "../lib/s_queue.h"
#include "../lib/timing.h"
#include "../lib/worker.h"
#include "../lib/fal.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../pie_log.h"
#include "../encoding/pie_json.h"
#include "../alg/pie_unsharp.h"

/**
 * Signal handler.
 * @param the signum.
 * @return void.
 */
static void sig_h(int);

/**
 * Drain incoming export events.
 */
static int process(struct q_consumer*);

/**
 * Process one export request
 * @param pointer to pie_mq_export_media struct
 * @param sizeof pie_mq_export_media struct (not used).
 * @return void.
 */
static void export(void*, size_t);

/**
 * Resize a bitmap.
 * Set a dimension to < 1 to don't care. If both are < 1, no resizing
 * happens.
 * @param bitmap to resize.
 * @param new max x size.
 * @param new max y size.
 * @return void.
 */
static void resize(struct pie_bitmap_f32rgb*, int, int);

static struct config
{
        struct pie_host* host;
        struct pie_stg_mnt_arr* storages;
        struct wrk_pool* wrk;
} cfg;
static sqlite3* db;

int main(void)
{
        struct sigaction sa;
        struct q_consumer* q = NULL;
        sigset_t b_sigset;
        sigset_t o_sigset;
        long lval;
        int num_workers = 4;
        int ret = -1;
        int ok;

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf");
                goto cleanup;
        }

        db = pie_cfg_get_db();
        if (db == NULL)
        {
                goto cleanup;
        }

        if (pie_cfg_get_long("global:workers", &lval))
        {
                PIE_WARN("Config global:workers not found in config");
        }
        else
        {
                num_workers = (int)lval;
        }
        PIE_LOG("Using %d workers", num_workers);

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
        for (int i = 0; i < cfg.storages->len; i++)
        {
                if (cfg.storages->arr[i])
                {
                        PIE_LOG("Storage %d (%s) at %s",
                                cfg.storages->arr[i]->stg.stg_id,
                                pie_storage_type(cfg.storages->arr[i]->stg.stg_type),
                                cfg.storages->arr[i]->mnt_path);
                }
        }

        /* Init queues */
        q = q_new_consumer(QUEUE_INTRA_HOST);
        if (q == NULL)
        {
                PIE_ERR("Can not create queue consumer");
                goto cleanup;
        }
        ok = q->init(q->this, Q_EXPORT_MEDIA);
        if (ok)
        {
                PIE_ERR("Could not init queue '%s'", Q_EXPORT_MEDIA);
                goto cleanup;
        }

        /* Block all signals */
        sigfillset(&b_sigset);
        pthread_sigmask(SIG_BLOCK, &b_sigset, &o_sigset);

        /* init workers */
        cfg.wrk = wrk_start_workers(num_workers, &export);
        if (cfg.wrk == NULL)
        {
                goto cleanup;
        }

        /* Restore sigmask and wait for termination */
        pthread_sigmask(SIG_SETMASK, &o_sigset, NULL);
        sa.sa_handler = &sig_h;
        sa.sa_flags = 0;
        sigfillset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL))
        {
                PIE_ERR("Could not set signal handler");
                goto cleanup;
        }

        PIE_LOG("All set, wait for incoming messages");
        ret = process(q);

        q->close(q->this);
        q_del_consumer(q);
        q = NULL;

cleanup:
        if (cfg.wrk)
        {
                wrk_stop_workers(cfg.wrk);
                cfg.wrk = NULL;
        }

        if (q)
        {
                q->close(q->this);
                q_del_consumer(q);
                q = NULL;
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
        pie_cfg_close();

        return ret;
}

static void sig_h(int signum)
{
        PIE_LOG("Got signal %d", signum);
}

static int process(struct q_consumer* q)
{
        struct pie_mq_export_media msg;
        ssize_t br;

        while((br = q->recv(q->this,
                            (char*)&msg,
                            sizeof(msg))) > 0)
        {
                if (wrk_add_job(cfg.wrk, &msg, sizeof(msg)))
                {
                        PIE_ERR("Failed to add job to queue");
                }
        }

        return 0;
}

static void export(void* a, size_t len)
{
        if (len != sizeof(struct pie_mq_export_media))
        {
                PIE_ERR("Received invalid job of len: %ld", len);
                return;
        }
        char path[PIE_PATH_LEN];
        struct pie_io_opt io_opts;
        struct pie_bitmap_f32rgb bm_src;
        struct timing t;
        struct timing t_tot;
        struct pie_mq_export_media* msg = a;
        struct pie_min* min;
        int st;

        timing_start(&t_tot);
        io_opts.qual = PIE_IO_HIGH_QUAL;

        PIE_DEBUG("Path %s", msg->path);
        PIE_DEBUG("Mob id: %ld", msg->mob_id);
        PIE_DEBUG("Stg id: %d", msg->stg_id);
        PIE_DEBUG("Max x:  %d", msg->max_x);
        PIE_DEBUG("Max y:  %d", msg->max_y);
        PIE_DEBUG("Sharp: %d", msg->sharpen);
        PIE_DEBUG("d exif: %d", msg->disable_exif);
        PIE_DEBUG("qualit: %d", msg->quality);
        PIE_DEBUG("e type: %d", msg->type);

        min = pie_doml_min_for_mob(db,
                                   cfg.storages->arr,
                                   cfg.storages->len,
                                   msg->mob_id);
        if (!min)
        {
                PIE_WARN("No MIN found for MOB %ld", msg->mob_id);
                return;
        }
        else
        {
                PIE_LOG("Use MIN %ld for MOB %ld", min->min_id, msg->mob_id);
        }
        if (msg->stg_id >= cfg.storages->len || msg->stg_id < 1)
        {
                PIE_ERR("Unknown storage: %d", msg->stg_id);
                goto cleanup;
        }
        snprintf(path,
                 PIE_PATH_LEN,
                 "%s%s",
                 cfg.storages->arr[min->min_stg_id]->mnt_path,
                 min->min_path);

        PIE_DEBUG("Src path %s", path);
        timing_start(&t);
        if (pie_io_load(&bm_src, path, &io_opts))
        {
                PIE_ERR("Failed to load %s", path);
                goto cleanup;
        }
        PIE_DEBUG("Loaded %s in %ldms",
                  path,
                  timing_dur_msec(&t));

        /* Resize */
        resize(&bm_src, msg->max_x, msg->max_y);

        /* Apply development settings */
        struct pie_dev_params dev_json;
        dev_json.pdp_mob_id = msg->mob_id;
        st = pie_dev_params_read(db, &dev_json);
        if (st == 0)
        {
                struct pie_dev_settings dev;

                st = pie_dec_json_settings(&dev, dev_json.pdp_settings);
                if (st)
                {
                        PIE_ERR("Failed to parse development parameters for %ld",
                                msg->mob_id);
                }
                else
                {
                        pie_bm_set_to_int_fmt(&dev);
                        timing_start(&t);
                        float* buf = malloc(bm_src.row_stride * bm_src.height * sizeof(float));
                        pie_bm_render(&bm_src, buf, &dev);
                        free(buf);
                        PIE_DEBUG("Rendered %s in %ldms",
                                  path,
                                  timing_dur_msec(&t));
                }
        }
        else if (st < 0)
        {
                PIE_ERR("Failed to load development parameters for %ld",
                        msg->mob_id);
        }

        /* Post rescale sharpening */
        if (msg->sharpen)
        {
                struct pie_unsharp_param p =
                        {
                                .radius = 0.3f,
                                .amount = 3.0f,
                                .threshold = 2.0f,
                        };
                PIE_DEBUG("Apply post resize sharpening");
                pie_alg_unsharp(bm_src.c_red,
                                bm_src.c_green,
                                bm_src.c_blue,
                                &p,
                                bm_src.width,
                                bm_src.height,
                                bm_src.row_stride);
        }

        /* Export */
        struct pie_bitmap_u8rgb u8;
        struct pie_bitmap_u16rgb u16;
        char* p = strrchr(min->min_path, '/');
        if (!p)
        {
                p = min->min_path;
        }

        /* Make sure path exists */
        if (fal_mkdir_tree(cfg.storages->arr[msg->stg_id]->mnt_path,
                           msg->path))
        {
                abort();
        }

        st = snprintf(path,
                      PIE_PATH_LEN,
                      "%s%s",
                      cfg.storages->arr[msg->stg_id]->mnt_path,
                      msg->path);
        if (st >= PIE_PATH_LEN)
        {
                /* Path to large */
                abort();
        }
        strncat(path, p, PIE_PATH_LEN - st);

        PIE_DEBUG("Export to path %s", path);

        timing_start(&t);
        switch (msg->type)
        {
        case PIE_MQ_EXP_JPG:
                assert(msg->quality <= 100);
                assert(msg->quality > 0);

                pie_bm_conv_bd(&u8, PIE_COLOR_8B, &bm_src, PIE_COLOR_32B);
                pie_io_jpg_u8rgb_write(path, &u8, msg->quality);
                pie_bm_free_u8(&u8);
                break;
        case PIE_MQ_EXP_PNG8:
                pie_bm_conv_bd(&u8, PIE_COLOR_8B, &bm_src, PIE_COLOR_32B);
                pie_io_png_u8rgb_write(path, &u8);
                pie_bm_free_u8(&u8);
                break;
        case PIE_MQ_EXP_PNG16:
                pie_bm_conv_bd(&u16, PIE_COLOR_16B, &bm_src, PIE_COLOR_32B);
                pie_io_png_u16rgb_write(path, &u16);
                pie_bm_free_u16(&u16);
                break;
        default:
                PIE_ERR("Invalid export type: %d", msg->type);
        }
        PIE_DEBUG("Exported %s in %ldms", path, timing_dur_msec(&t));

        pie_bm_free_f32(&bm_src);
cleanup:
        pie_min_free(min);
        PIE_DEBUG("Processed MOB %ld in %ldms",
                  msg->mob_id,
                  timing_dur_msec(&t_tot));
}

static void resize(struct pie_bitmap_f32rgb* bm, int max_x, int max_y)
{
        struct pie_bitmap_f32rgb new;
        struct timing t;

        if (max_x < 1 && max_y < 1)
        {
                return;
        }

        timing_start(&t);
        if (pie_bm_dwn_smpl(&new, bm, max_x, max_y))
        {
                return;
        }

        /* Free old, copy new to old and free new */
        pie_bm_free_f32(bm);
        pie_bm_conv_bd(bm, PIE_COLOR_32B, &new, PIE_COLOR_32B);
        pie_bm_free_f32(&new);
        PIE_DEBUG("Downsampled to %dpx, %dpx in %ldms",
                  bm->width, bm->height, timing_dur_msec(&t));
}
