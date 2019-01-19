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
#include <sys/types.h>
#include <signal.h>
#include "mediad_cfg.h"
#include "../pie_log.h"
#include "../pie_id.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../lib/chan.h"
#include "../lib/timing.h"
#include "../lib/strutil.h"
#include "../lib/llist.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_collection_member.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_min.h"
#include "../dm/pie_mob.h"
#include "../dm/pie_dev_params.h"
#include "../bm/pie_bm.h"
#include "../bm/pie_render.h"
#include "../exif/pie_exif.h"
#include "../io/pie_io.h"
#include "../io/pie_io_jpg.h"
#include "../alg/pie_cspace.h"
#include "../encoding/pie_json.h"

struct nm_worker
{
        struct chan* chan;
        unsigned char me;
};

static void* worker(void*);
static struct pie_mq_process_media* alloc_msg(void);
static void free_msg(struct pie_mq_process_media*);
/**
 * Get or create a pie_collection in the database.
 * @param path of collection.
 * @param collection id to use if no collection exists.
 * @return pointer to a collection. If a database error occurs, NULL is
 *         returned.
 */
static struct pie_collection* get_or_create_coll(const char*, long);

/**
 * Ingest new media.
 * Create MOB, MIN, proxy and thumbnail. Bind MOB to correct collection.
 * @param id of worker thread.
 * @param proxy storage.
 * @param thumbnail storage.
 * @param io options.
 * @param new media message.
 * @return 0 on success.
 */
static int ingest_new(unsigned char,
                      struct pie_stg_mnt*,
                      struct pie_stg_mnt*,
                      struct pie_io_opts*,
                      struct pie_mq_new_media*);

/**
 * Generate new proxy and thumbnail for a given mob.
 * @param id of worker thread.
 * @param proxy storage.
 * @param thumbnail storage.
 * @param io options.
 * @param update proxy message.
 * @return 0 on success.
 */
static int generate_proxy(unsigned char,
                          struct pie_stg_mnt*,
                          struct pie_stg_mnt*,
                          struct pie_io_opts*,
                          struct pie_mq_upd_proxy*);

/**
 * Write a downsampled version to disk.
 * @param the bitmap to downsample.
 * @param maximum size (in both w and h). Set to negative to keep size.
 * @param complete path to write file to.
 * @param JPEG quality [0,100]
 * @return 0 on success.
 */
static int write_dwn_smpl(struct pie_bitmap_f32rgb*, int, const char*, int);

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
                struct nm_worker* wrk = malloc(sizeof(struct nm_worker));
                wrk->chan = chan;
                wrk->me = (unsigned char)(i + 1);
                if (pthread_create(workers + i,
                                   NULL,
                                   &worker,
                                   (void*)wrk))
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

int pie_nm_add_job(const struct pie_mq_process_media* m)
{
        struct pie_mq_process_media* new = alloc_msg();
        struct chan_msg msg;

        assert(chan);
        memcpy(new, m, sizeof(struct pie_mq_process_media));
        msg.data = new;
        msg.len = sizeof(struct pie_mq_process_media);

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
        struct nm_worker* me = (struct nm_worker*)arg;
        struct pie_stg_mnt* proxy_stg = NULL;
        struct pie_stg_mnt* thumb_stg = NULL;
        struct pie_io_opts io_opts;

        io_opts.qual = PIE_IO_HIGH_QUAL;
#if _PIE_EDIT_LINEAR
        io_opts.cspace = PIE_IO_LINEAR;
#else
        io_opts.cspace = PIE_IO_SRGB;
#endif

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
                        default:
                                break;
                        }
                }
        }

        PIE_LOG("[%d]Worker ready", me->me);

        for (;;)
        {
                struct chan_msg chan_msg;
                struct pie_mq_process_media* envelope;
                int ok;
                struct timing t;

                ok = chan_read(me->chan, &chan_msg, -1);
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
                envelope = chan_msg.data;
                switch (envelope->type)
                {
                case PIE_MQ_MEDIA_NEW:
                        ok = ingest_new(me->me,
                                        proxy_stg,
                                        thumb_stg,
                                        &io_opts,
                                        &envelope->msg.new);
                        if (ok)
                        {
                                abort();
                        }
                        break;
                case PIE_MQ_MEDIA_PROXY:
                        timing_start(&t);
                        ok = generate_proxy(me->me,
                                            proxy_stg,
                                            thumb_stg,
                                            &io_opts,
                                            &envelope->msg.proxy);
                        if (ok)
                        {
                                abort();
                        }
                        PIE_LOG("Generated proxy for %ld in %ldms",
                                envelope->msg.proxy.mob_id,
                                timing_dur_msec(&t));
                        break;
                default:
                        PIE_ERR("Invalid msg type: %d", envelope->type);
                        abort();
                }

        next_msg:
                free_msg(envelope);
        }

        PIE_LOG("[%d]Worker done", me->me);
        free(me);

        return NULL;
}

static struct pie_mq_process_media* alloc_msg(void)
{
        return malloc(sizeof(struct pie_mq_process_media));
}

static void free_msg(struct pie_mq_process_media* m)
{
        free(m);
}

static int ingest_new(unsigned char me,
                      struct pie_stg_mnt* proxy_stg,
                      struct pie_stg_mnt* thumb_stg,
                      struct pie_io_opts* io_opts,
                      struct pie_mq_new_media* new)
{
        char src_path[PIE_PATH_LEN];
        char tgt_path[PIE_PATH_LEN];
        struct pie_exif_data ped;
        struct pie_bitmap_f32rgb bm_src;
        struct pie_mob mob;
        struct pie_min min;
        struct timing t;
        struct pie_stg_mnt* stg;
        char* p;
        int ok;
        pie_id mob_id;
        time_t now_ms;
        now_ms = timing_current_millis();

        new->stg_id = ntohl(new->stg_id);
        new->digest_len = ntohl(new->digest_len);

        stg = PIE_CFG_GET_STORAGE(md_cfg.storages, new->stg_id);
        if (stg == NULL)
        {
                PIE_WARN("[%d]Unknown storage: %d\n",
                         me,
                         new->stg_id);
                return -1;
        }

        snprintf(src_path,
                 PIE_PATH_LEN,
                 "%s%s",
                 stg->mnt_path,
                 new->path);
        PIE_DEBUG("[%d]Begin loading %s...",
                  me,
                  src_path);

        timing_start(&t);
        ok = pie_io_load(&bm_src, src_path, io_opts);
        PIE_DEBUG("[%d]Loaded %s in %ldms",
                  me,
                  src_path,
                  timing_dur_msec(&t));
        if (ok)
        {
                PIE_ERR("[%d]Could not load '%s'", me, src_path);
                return -1;
        }
        /* Create new ID */
        mob_id = pie_id_create((unsigned char)md_cfg.host->hst_id,
                               me,
                               PIE_ID_TYPE_MOB);
        PIE_LOG("[%d]New media: [%ld] %s", me, mob_id, src_path);

        /* Create MOB & MIN */
        p = strrchr(new->path, '/');
        if (p)
        {
                p++;
        }
        else
        {
                p = new->path;
        }

        /* Create MOB but do not persist it, we need data from
           EXIF for capture time */
        mob.mob_id = mob_id;
        mob.mob_parent_mob_id = 0;
        strncpy(mob.mob_name, p, MOB_NAME_LEN);
        mob.mob_capture_ts_millis = now_ms;
        mob.mob_added_ts_millis = now_ms;
        mob.mob_format = 0;
        mob.mob_color = 0;
        mob.mob_rating = 0;
        mob.mob_orientation = 1;

        /* Create MIN */
        min.min_id = pie_id_create((unsigned char)md_cfg.host->hst_id,
                                   me,
                                   PIE_ID_TYPE_MIN);
        min.min_added_ts_millis = now_ms;
        min.min_mob_id = mob_id;
        min.min_size = pie_ntohll(new->file_size);
        min.min_stg_id = new->stg_id;
        strncpy(min.min_path, new->path, MIN_PATH_LEN);
        /* hex encode digest into MIN */
        if (new->digest_len * 2 + 1 > MIN_HASH_LEN)
        {
                PIE_ERR("Too larget digest received %d, max is %d",
                        new->digest_len * 2 + 1,
                        MIN_HASH_LEN);
                abort();
        }
        for (unsigned int i = 0; i < new->digest_len; i++)
        {
                snprintf(min.min_sha1_hash + i * 2,
                         MIN_HASH_LEN,
                         "%02x",
                         new->digest[i]);
        }
        min.min_sha1_hash[new->digest_len * 2] = '\0';

        ok = pie_min_create(pie_cfg_get_db(), &min);
        if (ok)
        {
                PIE_ERR("[%d]Could not create min row for mob %ld",
                        me,
                        mob_id);
        }

        /* Extract EXIF data and store in meta_data */
        timing_start(&t);
        ok = pie_exif_load(&ped, src_path);
        PIE_DEBUG("[%d]Extracted EXIF for %s in %ldms",
                  me,
                  src_path,
                  timing_dur_msec(&t));
        if (ok)
        {
                PIE_ERR("[%d]Could not extract EXIF data from %s",
                        me, src_path);
        }
        else
        {
                if (ped.ped_date_time)
                {
                        mob.mob_capture_ts_millis = pie_exif_date_to_millis(
                                ped.ped_date_time,
                                ped.ped_sub_sec_time);
                }
                /* Grab orientation from exif */
                if (ped.ped_orientation)
                {
                        mob.mob_orientation = (char)ped.ped_orientation;
                }

                ped.ped_mob_id = mob_id;
                ok = pie_exif_data_create(pie_cfg_get_db(), &ped);
                if (ok)
                {
                        PIE_ERR("[%d]Could not create pie exif data row for mob %ld",
                                me,
                                mob_id);
                }
                pie_exif_data_release(&ped);
        }

        /* Perist MOB */
        ok = pie_mob_create(pie_cfg_get_db(), &mob);
        if (ok)
        {
                PIE_ERR("[%d]Could not create mob row for mob %ld",
                        me,
                        mob_id);
        }

        /* Write thumbnail */
        snprintf(tgt_path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 thumb_stg->mnt_path,
                 mob_id);
        PIE_DEBUG("[%d]Thumb: %s", me, tgt_path);
        if (write_dwn_smpl(&bm_src, md_cfg.max_thumb, tgt_path, 100))
        {
                pie_bm_free_f32(&bm_src);
                return -1;
        }

        /* Write proxy image */
        snprintf(tgt_path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 proxy_stg->mnt_path,
                 mob_id);
        PIE_DEBUG("[%d]Proxy: %s", me, tgt_path);
        if (write_dwn_smpl(&bm_src,
                           md_cfg.max_proxy,
                           tgt_path,
                           md_cfg.proxy_qual))
        {
                pie_bm_free_f32(&bm_src);
                return -1;
        }

        /* Update target collection */
        strncpy(tgt_path, new->path, PIE_PATH_LEN);
        p = strrchr(tgt_path, '/');
        if (p)
        {
                struct pie_collection* coll;
                struct pie_collection_member coll_memb;
                pie_id coll_id = pie_id_create((unsigned char)md_cfg.host->hst_id,
                                               me,
                                               PIE_ID_TYPE_COLL);

                /* Trim filename */
                if ((p - tgt_path) == 0L)
                {
                        /* only one slash (the first is present) */
                        p++;
                }
                *p = '\0';

                coll = get_or_create_coll(tgt_path,
                                          coll_id);
                if (coll == NULL)
                {
                        abort();
                }
                PIE_LOG("[%d]Target collection for %ld is '%s', %ld",
                        me,
                        mob_id,
                        coll->col_path,
                        coll->col_id);

                coll_memb.cmb_col_id = coll->col_id;
                coll_memb.cmb_mob_id = mob_id;

                if (pie_collection_member_create(pie_cfg_get_db(),
                                                 &coll_memb))
                {
                        abort();
                }

                pie_collection_free(coll);
        }
        else
        {
                /* new->path has been overwritten, the preceding slash
                   is gone, take us down. */
                PIE_ERR("[%d]Inconsistent state, new->path has been altered", me);
                PIE_ERR("[%d]Value of new->path '%s'", me, new->path);
                kill(getpid(), SIGINT);
        }

        pie_bm_free_f32(&bm_src);

        return 0;
}

static int generate_proxy(unsigned char me,
                          struct pie_stg_mnt* proxy_stg,
                          struct pie_stg_mnt* thumb_stg,
                          struct pie_io_opts* io_opts,
                          struct pie_mq_upd_proxy* upd)
{
        char path[PIE_PATH_LEN];
        struct pie_bitmap_f32rgb bm;
        struct pie_dev_settings dp;
        struct llist* mins = pie_min_find_mob(pie_cfg_get_db(), upd->mob_id);
        struct pie_min* min = NULL;
        struct lnode* n = llist_head(mins);
        struct pie_stg_mnt* stg;
        struct pie_dev_params* dp_json;
        struct timing t;
        int ok;
        int ret = -1;

        PIE_LOG("[%d]Generate proxy for %ld", me, upd->mob_id);

        /* Just take the first MIN */
        if (n)
        {
                min = n->data;
        }
        else
        {
                PIE_ERR("[%d]No MIN found for MOB %ld", me, upd->mob_id);
                llist_destroy(mins);
                return -1;
        }

        stg = PIE_CFG_GET_STORAGE(md_cfg.storages, min->min_stg_id);
        if (stg == NULL)
        {
                PIE_WARN("[%d]Unknown storage: %d\n",
                         me,
                         min->min_stg_id);
                llist_destroy(mins);
                return -1;
        }

        snprintf(path, PIE_PATH_LEN,
                 "%s%s",
                 stg->mnt_path,
                 min->min_path);
        timing_start(&t);
        ok = pie_io_load(&bm, path, io_opts);
        if (ok)
        {
                PIE_ERR("[%d]Could not load '%s'", me, path);
                llist_destroy(mins);
                return -1;
        }
        PIE_LOG("Loaded %s in %ldmsec", path, timing_dur_msec(&t));

        /* Apply development settings */
        dp_json = pie_dev_params_alloc();
        dp_json->pdp_mob_id = upd->mob_id;
        ok = pie_dev_params_read(pie_cfg_get_db(), dp_json);
        if (ok < 0)
        {
                PIE_ERR("[%d]Could not read development parameters", me);
                goto done;
        }
        if (ok == 0)
        {
                ok = pie_dec_json_settings(&dp,
                                           dp_json->pdp_settings);
                if (ok)
                {
                        PIE_ERR("[%d]Broken dev settings in db for MOB %ld",
                                me,
                                upd->mob_id);
                        goto done;
                }

                pie_bm_set_to_int_fmt(&dp);
                pie_bm_render(&bm, NULL, &dp);
        }

        /* Write proxy and thumbnail */
        snprintf(path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 thumb_stg->mnt_path,
                 upd->mob_id);
        PIE_DEBUG("[%d]Thumb: %s", me, path);
        if (write_dwn_smpl(&bm, md_cfg.max_thumb, path, 100))
        {
                goto done;
        }

        snprintf(path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 proxy_stg->mnt_path,
                 upd->mob_id);
        PIE_DEBUG("[%d]Proxy: %s", me, path);
        if (write_dwn_smpl(&bm, md_cfg.max_proxy, path, md_cfg.proxy_qual))
        {
                goto done;
        }
        ret = 0;
done:
        pie_bm_free_f32(&bm);
        pie_dev_params_free(dp_json);
        for (n = llist_head(mins); n; n = n->next)
        {
                pie_min_free(n->data);
        }
        llist_destroy(mins);

        return 0;
}

static int write_dwn_smpl(struct pie_bitmap_f32rgb* src,
                          int max_size,
                          const char* p,
                          int qual)
{
        struct pie_bitmap_f32rgb bm_dwn;
        struct pie_bitmap_u8rgb bm_out;
        struct pie_bitmap_f32rgb* bm_src = src;
        struct timing t;
        int ok;

        if (max_size >= 0 && max_size < 100)
        {
                max_size = 100;
        }

        if (max_size > 0)
        {
                timing_start(&t);
                if (pie_bm_dwn_smpl(&bm_dwn,
                                    src,
                                    max_size,
                                    max_size))
                {
                        PIE_ERR("Failed to downsample");
                        return -1;
                }
                PIE_DEBUG("Downsampled tp %dpx in %ldms",
                          max_size,
                          timing_dur_msec(&t));
                bm_src = &bm_dwn;
        }

#if _PIE_EDIT_LINEAR
        /* Go back to sRGB */
        timing_start(&t);
        pie_alg_linear_to_srgbv(bm_src->c_red,
                                bm_src->height * bm_src->row_stride);
        pie_alg_linear_to_srgbv(bm_src->c_green,
                                bm_src->height * bm_src->row_stride);
        pie_alg_linear_to_srgbv(bm_src->c_blue,
                                bm_src->height * bm_src->row_stride);
        PIE_DEBUG("Converted to sRGB in %ldms", timing_dur_msec(&t));
#endif /* _PIE_EDIT_LINEAR */

        timing_start(&t);
        if (pie_bm_conv_bd(&bm_out,
                           PIE_COLOR_8B,
                           bm_src,
                           PIE_COLOR_32B))
        {
                PIE_ERR("Could not convert to 8b");
                pie_bm_free_f32(&bm_dwn);
                return -1;
        }
        PIE_DEBUG("Converted to 8b in %ldms", timing_dur_msec(&t));

        if (max_size > 0)
        {
                pie_bm_free_f32(&bm_dwn);
        }

        timing_start(&t);
        ok = pie_io_jpg_u8rgb_write(p, &bm_out, qual);
        PIE_DEBUG("Wrote %s in %ldms", p, timing_dur_msec(&t));
        if (ok)
        {
                PIE_ERR("Could not write output '%s', code %d",
                        p,
                        ok);
        }

        pie_bm_free_u8(&bm_out);

        return ok;
}

static struct pie_collection* get_or_create_coll(const char* p, long coll_id)
{
        struct pie_collection* coll;

        /* Avoid RAW and similar race conditions */
        pthread_mutex_lock(&md_cfg.db_lock);

        coll = pie_collection_find_path(pie_cfg_get_db(), p);

        if (coll == NULL)
        {
                /* Create */
                coll = pie_collection_alloc();
                coll->col_id = coll_id;
                strncpy(coll->col_path, p, PIE_PATH_LEN);
                /* Use defaults for now */
                coll->col_usr_id = 0;
                coll->col_grp_id = 0;
                coll->col_acl = 0755;

                PIE_DEBUG("Creating collection@'%s'", p);
                int ok = pie_collection_create(pie_cfg_get_db(), coll);
                if (ok)
                {
                        PIE_ERR("pie_collection_create failed with code %d", ok);
                        pie_collection_free(coll);
                        coll = NULL;
                }
        }
        pthread_mutex_unlock(&md_cfg.db_lock);

        return coll;
}
