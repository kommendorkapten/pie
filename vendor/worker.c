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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include "../prunt/pie_log.h"
#include "worker.h"
#include "chan.h"

static void* thr_fun(void*);

struct worker
{
        struct chan* chan;
        unsigned char me;
        wrk_cb cb;
};

struct wrk_pool
{
        pthread_t* workers;
        struct chan* chan;
        int num_workers;
};

struct wrk_pool* wrk_start_workers(int num, wrk_cb cb)
{
        struct wrk_pool* wp = malloc(sizeof(struct wrk_pool));
        int i;

        wp->chan = chan_create();
        wp->num_workers = num;

        if (wp->chan == NULL)
        {
                goto bailout;
        }

        wp->workers = malloc(sizeof(pthread_t) * wp->num_workers);
        for (i = 0; i < wp->num_workers; i++)
        {
                struct worker* wrk = malloc(sizeof(struct worker));

                wrk->chan = wp->chan;
                wrk->me = (unsigned char)(i + 1);
                wrk->cb = cb;
                if (pthread_create(wp->workers + i,
                                   NULL,
                                   thr_fun,
                                   (void*)wrk))
                {
                        PIE_WARN("Failed to create worker");
                        break;
                }
        }
        if (i == 0)
        {
                goto bailout;
        }
        wp->num_workers = i;

        PIE_LOG("%d workers created", wp->num_workers);
        goto done;

bailout:
        if (wp->chan)
        {
                chan_destroy(wp->chan);
                wp->chan = NULL;
        }
        if (wp->workers)
        {
                free(wp->workers);
                wp->workers = NULL;
        }
        free(wp);
        wp = NULL;
done:

        return wp;
}

int wrk_add_job(struct wrk_pool* wp, const void* j, size_t s)
{
        struct chan_msg msg;
        char* d = malloc(s);

        assert(wp);

        memcpy(d, j, s);
        msg.data = d;
        msg.len = s;

        if (chan_write(wp->chan, &msg))
        {
                free(d);
                return -1;
        }

        return 0;
}

void wrk_stop_workers(struct wrk_pool* wp)
{
        assert(wp);

        chan_close(wp->chan);

        for (int i = 0; i < wp->num_workers; i++)
        {
                pthread_join(wp->workers[i], NULL);
        }
        free(wp->workers);
        chan_destroy(wp->chan);
        free(wp);
}

static void* thr_fun(void* arg)
{
        struct chan_msg chan_msg;
        struct worker* me = (struct worker*)arg;
        int ok;

        PIE_LOG("[%d] Starting", me->me);

        for (;;)
        {
                ok = chan_read(me->chan, &chan_msg, -1);
                if (ok)
                {
                        if (ok == EAGAIN)
                        {
                                continue;
                        }
                        break;
                }

                PIE_DEBUG("[%d] Process job", me->me);
                me->cb(chan_msg.data, chan_msg.len);
                PIE_DEBUG("[%d] Job done", me->me);
                free(chan_msg.data);
        }

        PIE_LOG("[%d] Stopping", me->me);
        free(arg);

        return NULL;
}
