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
#include "mediad_cfg.h"
#include "new_media.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../lib/s_queue.h"
#include "../lib/evp_hw.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../cfg/pie_cfg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"

#define Q_INC 0
#define Q_UPD 1
#define NUM_QUEUES 2

/**
 * Signal handler.
 * @param the signum.
 * @return void.
 */
static void sig_h(int);

/**
 * Drain incoming media.
 * @param struct server.
 * @return NULL if successfull.
 */
static void* process_inc(void*);

/**
 * Drain update meta data.
 * @param struct server.
 * @return NULL if successfull.
 */
static void* process_upd(void*);

struct mediad_cfg md_cfg;

int main(void)
{
        pthread_t thr[NUM_QUEUES];
        struct sigaction sa;
        struct q_consumer* q_incoming;
        struct q_consumer* q_update;
        sigset_t b_sigset;
        sigset_t o_sigset;
        int evp_hw = 1;
        int ret = -1;
        int ok;

        evp_enable_hw(evp_hw);

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf\n");
                goto cleanup;
        }

        long lval;
        int num_workers = 4;

        if (pie_cfg_get_long("global:workers", &lval))
        {
                PIE_LOG("Config global:workers not found in config");
        }
        else
        {
                num_workers = (int)lval;
        }
        PIE_LOG("Using %d workers", num_workers);

        if (pie_cfg_get_long("image:thumbnail:size", &lval))
        {
                PIE_LOG("Config image:thumbnail:size not found in config");
                lval = 256;
        }
        else
        {
                md_cfg.max_thumb = (int)lval;
        }
        PIE_LOG("Maximum thumbnail size: %d", md_cfg.max_thumb);
        if (pie_cfg_get_long("image:proxy:size", &lval))
        {
                PIE_LOG("Config image:proxy:size not found in config");
                lval = 1024;
        }
        else
        {
                md_cfg.max_proxy = (int)lval;
        }
        PIE_LOG("Maximum proxy size: %d", md_cfg.max_proxy);

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
                        PIE_LOG("Storage %d at %s",
                                md_cfg.storages->arr[i]->stg.stg_id,
                                md_cfg.storages->arr[i]->mnt_path);
                }
        }

        /* Init queues */
        q_incoming = q_new_consumer(QUEUE_INTRA_HOST);
        if (q_incoming == NULL)
        {
                PIE_ERR("Can not create queue consumer");
                goto cleanup;
        }
        q_update = q_new_consumer(QUEUE_INTRA_HOST);
        if (q_incoming == NULL)
        {
                PIE_ERR("Can not create queue consumer");
                goto cleanup;
        }

        ok = q_incoming->init(q_incoming->this,
                              Q_INCOMING_MEDIA);
        if (ok)
        {
                PIE_ERR("Could not init queue '%s'", Q_INCOMING_MEDIA);
                goto cleanup;
        }
        ok = q_update->init(q_update->this,
                            Q_UPDATE_META);
        if (ok)
        {
                PIE_ERR("Could not init queue '%s'", Q_UPDATE_META);
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
        ok = pthread_create(&thr[Q_INC],
                            NULL,
                            &process_inc,
                            q_incoming);
        if (ok)
        {
                PIE_ERR("Could not create incoming media thread");
                goto cleanup;
        }
        ok = pthread_create(&thr[Q_UPD],
                            NULL,
                            &process_upd,
                            q_update);
        if (ok)
        {
                PIE_ERR("Could not create update meta data thread");
                goto cleanup;
        }

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
        pause();

        q_incoming->close(q_incoming->this);
        q_update->close(q_update->this);
        q_del_consumer(q_incoming);
        q_del_consumer(q_update);
        q_incoming = NULL;
        q_update = NULL;
        ret = 0;

        /* join threads */
        for (int i = 0; i < NUM_QUEUES; i++)
        {
                pthread_join(thr[i], NULL);
        }
        pie_nm_stop_workers();
cleanup:
        if (q_incoming)
        {
                q_incoming->close(q_incoming->this);
                q_del_consumer(q_incoming);
                q_incoming = NULL;
        }
        if (q_update)
        {
                q_update->close(q_update->this);
                q_del_consumer(q_update);
                q_update = NULL;
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

static void* process_inc(void* arg)
{
        struct pie_mq_new_media msg;
        void* ret = NULL;
        struct q_consumer* q = (struct q_consumer*)arg;
        ssize_t br;

        PIE_LOG("Incoming media thread ready for messages");
        while ((br = q->recv(q->this,
                             (char*)&msg,
                             sizeof(struct pie_mq_new_media))) > 0)
        {
                if (pie_nm_add_job(&msg))
                {
                        PIE_WARN("Could not add new media '%s'",
                                 msg.path);
                }
        }

        PIE_LOG("Incoming media thread leaving");

        return ret;
}

static void* process_upd(void* arg)
{
        void* ret = NULL;
        struct q_consumer* q = (struct q_consumer*)arg;
        int i;
        ssize_t br;

        PIE_LOG("Update media thread ready for messages");
        while ((br = q->recv(q->this,
                             (char*)&i,
                             sizeof(int))) > 0)
        {
                PIE_LOG("Read %d from update meta data", i);
        }

        PIE_LOG("Update meta data thread leaving");

        return ret;
}

static void sig_h(int signum)
{
        PIE_LOG("Got signal %d", signum);
}
