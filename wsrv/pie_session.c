#include "pie_session.h"
#include "../lib/hmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <sha1.h>
#include <sys/time.h>

struct pie_sess_mgr
{
        struct hmap* store;
};

void pie_sess_init(struct pie_sess* s)
{
        char buf[64];
        unsigned char sum[20];
        SHA1_CTX ctx;
        struct timeval tv;
        unsigned int n;

        gettimeofday(&tv, NULL);
        n = snprintf(buf, 64, "x&y_z%ld%ld", tv.tv_sec, tv.tv_usec);
        SHA1Init(&ctx);
        SHA1Update(&ctx, (unsigned char*)buf, n);
        SHA1Final(sum, &ctx);

        /* hex encode */
        for (int i = 0; i < 20; i++)
        {
                sprintf(s->token + i * 2, "%02x", sum[i]);
        }
        s->token[PIE_SESS_TOKEN_LEN-1] = 0;
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
#if DEBUG > 0
                        printf("Destroy session [%s]@%p\n", 
                               (char*)s->token,
                                elems[i].data);
#endif      
                        /* sessions are allocated in cb_http,
                           no references should ever be kept to
                           it except from this hmap. Free the session. */
                        free(elems[i].data);
                        hmap_del(sm->store, elems[i].key);
                }
        }
        free(elems);
        
        return 0;
}
