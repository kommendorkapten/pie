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
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include "mediad_cfg.h"
#include "new_media.h"
#include "../bm/pie_bm.h"
#include "../prunt/pie_log.h"
#include "../prunt/pie_cfg.h"
#include "../vendor/s_queue.h"
#include "../vendor/evp_hw.h"
#include "../vendor/timing.h"
#include "../vendor/btree.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"
#include "../dm/pie_dev_params.h"

#define Q_ING 0
#define Q_UPD 1
#define NUM_QUEUES 2
#define FLUSH_INT 5000 /* 5 sec */

static int mob_id_cmp(const void*, const void*);

/**
 * Signal handler.
 * @param the signum.
 * @return void.
 */
static void sig_h(int);

/**
 * Drain incoming media.
 * @param q to read from
 * @return void
 */
static void process_inc(struct q_consumer*);

/**
 * Drain update meta data.
 * @param q to read from
 * @return mob_id processed, or -1 if error occured.
 */
static pie_id process_upd(struct q_consumer*);

/**
 * Serve incoming requests.
 * @param Queus to listen on.
 * @return void.
 */
static void serve(struct q_consumer*[], int);

static void flush_upd(struct btree*);

struct mediad_cfg md_cfg;
static sig_atomic_t run;

int main(void)
{
        struct sigaction sa;
        struct q_consumer* mqs[NUM_QUEUES];
        sigset_t b_sigset;
        sigset_t o_sigset;
        int evp_hw = 1;
        int ret = -1;
        int ok;

        setvbuf(stdout, NULL, _IOLBF, 0);

        evp_enable_hw(evp_hw);

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf");
                goto cleanup;
        }

        long lval;
        int num_workers = 4;

        if (pie_cfg_get_long("global:workers", &lval))
        {
                PIE_WARN("Config global:workers not found in config");
        }
        else
        {
                num_workers = (int)lval;
        }
        PIE_LOG("Using %d workers", num_workers);

        if (pie_cfg_get_long("image:thumbnail:size", &lval))
        {
                PIE_WARN("Config image:thumbnail:size not found in config");
                lval = 256;
        }
        else
        {
                md_cfg.max_thumb = (int)lval;
        }
        PIE_LOG("Maximum thumbnail size: %d", md_cfg.max_thumb);

        if (pie_cfg_get_long("image:proxy:size", &lval))
        {
                PIE_WARN("Config image:proxy:size not found in config");
                lval = 1024;
        }
        else
        {
                md_cfg.max_proxy = (int)lval;
        }
        PIE_LOG("Maximum proxy size: %d", md_cfg.max_proxy);

        if (pie_cfg_get_long("image:proxy:fullsize", &lval))
        {
                PIE_WARN("Config image:proxy:fullsize not found in config");
                lval = 0;
        }
        else
        {
                md_cfg.proxy_fullsize = lval ? 1 : 0;
        }
        PIE_LOG("Proxy fullsize: %d", md_cfg.proxy_fullsize);
        if (md_cfg.proxy_fullsize)
        {
                md_cfg.max_proxy = -1;
                PIE_LOG("Changing maximum proxy size: %d", md_cfg.max_proxy);
        }

        if (pie_cfg_get_long("image:proxy:quality", &lval))
        {
                PIE_WARN("Config image:proxy:quality not found in config");
                lval = 90;
        }
        else
        {
                md_cfg.proxy_qual = (int)lval;

                if (md_cfg.proxy_qual > 100)
                {
                        md_cfg.proxy_qual = 100;
                }
                else if (md_cfg.proxy_qual < 10)
                {
                        md_cfg.proxy_qual = 10;
                }
        }
        PIE_LOG("Proxy quality: %d", md_cfg.proxy_qual);

        md_cfg.host = pie_cfg_get_host(-1);
        if (md_cfg.host == NULL)
        {
                PIE_ERR("Could not load current host");
                goto cleanup;
        }
        md_cfg.storages = pie_cfg_get_hoststg(md_cfg.host->hst_id);
        if (md_cfg.storages == NULL)
        {
                PIE_ERR("Could not resolve mount point for storage");
                goto cleanup;
        }
        for (int i = 0; i < md_cfg.storages->len; i++)
        {
                if (md_cfg.storages->arr[i])
                {
                        PIE_LOG("Storage %d (%s) at %s",
                                md_cfg.storages->arr[i]->stg.stg_id,
                                pie_storage_type(md_cfg.storages->arr[i]->stg.stg_type),
                                md_cfg.storages->arr[i]->mnt_path);
                }
        }

        /* Init queues */
        mqs[Q_ING] = q_new_consumer(QUEUE_INTRA_HOST);
        if (mqs[Q_ING] == NULL)
        {
                PIE_ERR("Can not create ingest queue consumer");
                goto cleanup;
        }
        mqs[Q_UPD] = q_new_consumer(QUEUE_INTRA_HOST);
        if (mqs[Q_UPD] == NULL)
        {
                PIE_ERR("Can not create update queue consumer");
                goto cleanup;
        }

        ok = mqs[Q_ING]->init(mqs[Q_ING]->this,
                              Q_INCOMING_MEDIA);
        if (ok)
        {
                PIE_ERR("Could not init queue '%s' %d", Q_INCOMING_MEDIA, ok);
                goto cleanup;
        }
        ok = mqs[Q_UPD]->init(mqs[Q_UPD]->this,
                              Q_UPDATE_META);
        if (ok)
        {
                PIE_ERR("Could not init queue '%s' %d", Q_UPDATE_META, ok);
                goto cleanup;
        }

        /* Init db w-lock */
        if (pthread_mutex_init(&md_cfg.db_lock, NULL))
        {
                perror("pthread_mutex_init");
                PIE_ERR("Could not init db w-lock");
                goto cleanup;
        }

        /* Block all signals */
        sigfillset(&b_sigset);
        pthread_sigmask(SIG_BLOCK, &b_sigset, &o_sigset);

        if (pie_nm_start_workers(num_workers))
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

        run = 1;
        serve(mqs, NUM_QUEUES);

        ret = 0;

        PIE_LOG("Waiting for worker threads");
        pie_nm_stop_workers();
        PIE_LOG("Done");

cleanup:
        for (int i = 0; i < NUM_QUEUES; i++)
        {
                if (mqs[i])
                {
                        mqs[i]->close(mqs[i]->this);
                        q_del_consumer(mqs[i]);
                        mqs[i] = NULL;
                }
        }

        if (md_cfg.host)
        {
                pie_host_free(md_cfg.host);
                md_cfg.host = NULL;
        }

        if (md_cfg.storages)
        {
                pie_cfg_free_hoststg(md_cfg.storages);
                md_cfg.storages = NULL;
        }
        pie_cfg_close();

        return ret;
}

static void serve(struct q_consumer* mqs[], int num)
{
        struct pollfd poll_fds[num];
        struct timing t;
        struct btree* mob_set = btree_create(&mob_id_cmp);

        for (int i = 0; i < num; i++)
        {
                poll_fds[i].fd = mqs[i]->fd(mqs[i]->this);
                poll_fds[i].events = POLLIN;
        }

        timing_start(&t);
        while (run)
        {
                int s = poll(poll_fds, num, FLUSH_INT);

                if (s < 0)
                {
                        if (errno != EAGAIN && errno != EINTR)
                        {
                                perror("poll");
                                abort();
                        }
                }

                if (poll_fds[Q_ING].revents & POLLIN)
                {
                        process_inc(mqs[Q_ING]);
                }
                if (poll_fds[Q_UPD].revents & POLLIN)
                {
                        pie_id mob_id = process_upd(mqs[Q_UPD]);
                        assert(mob_id > 0);
                        btree_insert(mob_set, (void*)mob_id);
                }

                for (int i = 0; i < num; i++)
                {
                        if (poll_fds[i].revents & POLLHUP)
                        {
                                poll_fds[i].fd = -1;
                                poll_fds[i].events = 0;
                                poll_fds[i].revents = 0;
                        }
                }

                if (timing_dur_msec(&t) >= FLUSH_INT)
                {
                        flush_upd(mob_set);
                        timing_start(&t);
                }
        }

        flush_upd(mob_set);
        btree_destroy(mob_set);
}

static void process_inc(struct q_consumer* q)
{
        struct pie_mq_process_media envelope;
        ssize_t br;

        envelope.type = PIE_MQ_MEDIA_NEW;
        if ((br = q->recv(q->this,
                          (char*)&envelope.msg.new,
                          sizeof(envelope))) > 0)
        {
                if (pie_nm_add_job(&envelope))
                {
                        PIE_ERR("Could not add new media '%s'",
                                envelope.msg.new.path);
                }
        }
        else
        {
                perror("recv");
                PIE_ERR("Failed to process incoming media");
        }
}

static pie_id process_upd(struct q_consumer* q)
{
        struct pie_dev_params dp;
        struct pie_mq_upd_media msg;
        ssize_t br;
        pie_id mob_id = -1;

        if ((br = q->recv(q->this,
                          (char*)&msg,
                          sizeof(msg))) > 0)
        {
                int status;
                int db_ok = 0;

                assert(msg.type == PIE_MQ_MEDIA_SETTINGS);

                mob_id = pie_ntohll(msg.mob_id);
                dp.pdp_mob_id = mob_id;

                PIE_DEBUG("Update mob: %ld with msg type %d",
                          dp.pdp_mob_id,
                          msg.type);
                strncpy(dp.pdp_settings,
                        msg.msg,
                        PIE_DEV_PARAMS_LEN - 1);

                status = pie_dev_params_exist(pie_cfg_get_db(), dp.pdp_mob_id);
                if (status == 0)
                {
                        db_ok = pie_dev_params_create(pie_cfg_get_db(), &dp);
                }
                else if (status < 0)
                {
                        PIE_ERR("Database error: %d", status);
                }
                else
                {
                        db_ok = pie_dev_params_update(pie_cfg_get_db(), &dp);
                }

                if (db_ok)
                {
                        PIE_ERR("Database error: %d", db_ok);
                }
        }
        else
        {
                perror("recv");
                PIE_ERR("Failed to process update media");
        }

        return mob_id;
}

static void sig_h(int signum)
{
        PIE_LOG("Got signal %d", signum);
        run = 0;
}

static int mob_id_cmp(const void* a, const void* b)
{
        const pie_id l = (const pie_id)a;
        const pie_id r = (const pie_id)b;

        if (l < r)
        {
                return -1;
        }
        else if (l > r)
        {
                return 1;
        }
        return 0;
}

static void flush_upd(struct btree* set)
{
        void** mob_ids = btree_bf(set);
        int i = 0;

        while (mob_ids[i])
        {
                struct pie_mq_process_media envelope;
                pie_id mob_id = (pie_id)mob_ids[i++];


                envelope.type = PIE_MQ_MEDIA_PROXY;
                envelope.msg.proxy.mob_id = mob_id;

                PIE_LOG("Request new proxy for %ld", envelope.msg.proxy.mob_id);
                if (pie_nm_add_job(&envelope))
                {
                        PIE_ERR("Could not add new proxy %ld",
                                envelope.msg.proxy.mob_id);
                }
        }

        free(mob_ids);
        btree_clear(set);
}
