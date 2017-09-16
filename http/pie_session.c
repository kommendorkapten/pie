/*
* Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
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
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/time.h>
#include "pie_session.h"
#include "../lib/hmap.h"
#include "../pie_log.h"

struct pie_sess_mgr
{
        struct hmap* store;
};

struct pie_sess* pie_sess_create(struct chan* command, struct chan* response)
{
        struct pie_sess* s;
        char buf[64];
        unsigned char sum[20];
        SHA_CTX ctx;
        struct timeval tv;
        unsigned int n;

        s = malloc(sizeof(struct pie_sess));
        gettimeofday(&tv, NULL);
        n = snprintf(buf, 64, "x&y_z%ld%d", tv.tv_sec, (int)tv.tv_usec);
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, (void*)buf, n);
        SHA1_Final(sum, &ctx);

        /* hex encode */
        for (int i = 0; i < 20; i++)
        {
                sprintf(s->token + i * 2, "%02x", sum[i]);
        }
        s->token[PIE_SESS_TOKEN_LEN-1] = 0;
        s->command = command;
        s->response = response;        
        s->img = NULL;
        s->rgba = NULL;
        s->tx_ready = 0;

        return s;
}

void pie_sess_destroy(struct pie_sess* s)
{
        if (s->rgba)
        {
                free(s->rgba);
        }
}

struct pie_sess_mgr* pie_sess_mgr_create(void)
{
        struct pie_sess_mgr* sm = malloc(sizeof(struct pie_sess_mgr));

        sm->store = hmap_create(NULL, NULL, 16, 0.7f);

        return sm;
}

void pie_sess_mgr_destroy(struct pie_sess_mgr* sm)
{
        struct hmap_entry* elems;
        size_t len;

        elems = hmap_iter(sm->store, &len);
        for (size_t i = 0; i < len; i++)
        {
                free(elems[i].data);
        }
        free(elems);

        hmap_destroy(sm->store);
        free(sm);
}

struct pie_sess* pie_sess_mgr_get(struct pie_sess_mgr* sm, char* token)
{
        struct timeval tv;
        struct pie_sess* s;

        PIE_TRACE("Session: %s", token);
        s = hmap_get(sm->store, token);
        if (s)
        {
                gettimeofday(&tv, NULL);
                s->access_ts = tv.tv_sec;
        }

        return s;
}

void pie_sess_mgr_put(struct pie_sess_mgr* sm, struct pie_sess* s)
{
        struct timeval tv;        

        PIE_TRACE("Session: %s", s->token);
        gettimeofday(&tv, NULL);
        s->access_ts = tv.tv_sec;

        hmap_set(sm->store, s->token, s);
}

int pie_sess_mgr_reap(struct pie_sess_mgr* sm, long threshold)
{
        struct timeval tv;
        struct hmap_entry* elems;
        size_t len;

        gettimeofday(&tv, NULL);
        elems = hmap_iter(sm->store, &len);
        for (size_t i = 0; i < len; i++)
        {
                struct pie_sess* s = elems[i].data;

                if (s->access_ts < tv.tv_sec - threshold)
                {
                        PIE_DEBUG("Destroy session [%s]@%p\n", 
                                  (char*)s->token,
                                  elems[i].data);

                        char token[PIE_SESS_TOKEN_LEN];
                        /* sessions are allocated in cb_http,
                           no references should ever be kept to
                           it except from this hmap. Free the session. */

                        memcpy(token, elems[i].key, PIE_SESS_TOKEN_LEN);
                        pie_sess_destroy((struct pie_sess*)elems[i].data);
                        free(elems[i].data);
                        hmap_del(sm->store, token);
                }
        }
        free(elems);
        
        return 0;
}
