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

#include "../pie_types.h"
#include "../pie_log.h"
#include "../io/pie_io.h"
#include "../bm/pie_bm.h"
#include "../lib/timing.h"
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef __sun
# include <note.h>
#else
# define NOTE(X)
#endif

#define FLAG_FREE     0x0
#define FLAG_INACTIVE 0x1
#define FLAG_ACTIVE   0x2

struct entry
{
        struct pie_img_workspace* img;
        time_t ts;
        unsigned int flags;
};

struct pie_wrkspc_mgr
{
        char library[PIE_PATH_LEN];
        struct entry* cache;
        int cap;
};

struct pie_wrkspc_mgr* pie_wrkspc_mgr_create(const char* l, int cap)
{
        struct pie_wrkspc_mgr* mgr = malloc(sizeof(struct pie_wrkspc_mgr));

        strncpy(mgr->library, l, PIE_PATH_LEN);
        mgr->library[PIE_PATH_LEN - 1] = 0;
        mgr->cap = cap;
        mgr->cache = malloc(mgr->cap * sizeof(struct entry));

        for (int i = 0; i < mgr->cap; i++)
        {
                mgr->cache[i].flags = FLAG_FREE;
        }

        return mgr;
}

struct pie_img_workspace* pie_wrkspc_mgr_acquire(struct pie_wrkspc_mgr* mgr,
                                                 const char* path)
{
        struct pie_img_workspace* img = NULL;
        struct timeval tv;

        gettimeofday(&tv, NULL);

        /* Linear scan over entries */
        for (int i = 0; i < mgr->cap; i++)
        {
                if (mgr->cache[i].flags == FLAG_FREE)
                {
                        continue;
                }
                if (strncmp(mgr->cache[i].img->path, path, PIE_PATH_LEN) == 0)
                {
                        img = mgr->cache[i].img;
                        mgr->cache[i].flags = FLAG_ACTIVE;
                        mgr->cache[i].ts = tv.tv_sec;
                        PIE_TRACE("Reuse: %s at pos %d",
                                  mgr->cache[i].img->path,
                                  i);
                        break;
                }
        }
        if (img != NULL)
        {
                goto done;
        }

        /* Set to the first free pos (if empty slots exists */
        int pos = -1;
        /* Set to the slot to evict if cache is full */
        int evict = -1;
        time_t min = tv.tv_sec;

        /* New entry needs to be inserted, scan for free slots */
        /* Cache is LRU */
        for (int i = 0; i < mgr->cap; i++)
        {
                if (mgr->cache[i].flags == FLAG_FREE)
                {
                        pos = i;
                        break;
                }

                /* Scan for objects to evict */
                if (mgr->cache[i].flags == FLAG_INACTIVE &&
                    mgr->cache[i].ts < min)
                {
                        min = mgr->cache[i].ts;
                        evict = i;
                }
        }

        if (pos == -1 && evict != -1)
        {
                /* Cache is full. Free old image */
                img = mgr->cache[evict].img;
                PIE_TRACE("Replace %s at pos %d with %s",
                          img->path,
                          evict,
                          path);

                pie_bm_free_f32(&img->raw);
                /* make sure that proxies are allocated */
                if (img->proxy.c_red)
                {
                        pie_bm_free_f32(&img->proxy);
                }
                if (img->proxy_out.c_red)
                {
                        pie_bm_free_f32(&img->proxy_out);
                }
                free(img);
                img = NULL;
                /* Mark slot as free, as it is not know yet wheter the
                   new image actually exists */
                mgr->cache[evict].flags = FLAG_FREE;
                pos = evict;
        }

        if (pos != -1)
        {
                char buf[PIE_PATH_LEN];
                struct timing t;
                int res;

                img = malloc(sizeof(struct pie_img_workspace));
                memset(img, 0, sizeof(struct pie_img_workspace));
                snprintf(buf, PIE_PATH_LEN, "%s/%s", mgr->library, path);
                timing_start(&t);
                res = pie_io_load(&img->raw, buf);
                PIE_DEBUG("Loaded %s in %ldusec",
                          buf,
                          timing_dur_usec(&t));
                if (res)
                {
                        PIE_ERR("Could not open '%s'",
                                buf);
                        free(img);
                        img = NULL;
                        goto done;
                }

                strncpy(img->path, path, PIE_PATH_LEN);
                mgr->cache[pos].flags = FLAG_ACTIVE;
                mgr->cache[pos].img = img;
                mgr->cache[pos].ts = tv.tv_sec;
                PIE_TRACE("Create: %s at pos %d",
                          mgr->cache[pos].img->path,
                          pos);
        }
        else
        {
                NOTE(EMPTY);
                PIE_TRACE("Cache is full. This is most likely a bug!");
        }

done:
        return img;
}

void pie_wrkspc_mgr_release(struct pie_wrkspc_mgr* mgr,
                            struct pie_img_workspace* img)
{
        for (int i = 0; i < mgr->cap; i++)
        {
                if (mgr->cache[i].img == img)
                {
                        PIE_TRACE("%s at pos %d", mgr->cache[i].img->path, i);
                        mgr->cache[i].flags = FLAG_INACTIVE;
                        break;
                }
        }
}

void pie_wrkspc_mgr_destroy(struct pie_wrkspc_mgr* mgr)
{
        for (int i = 0; i < mgr->cap; i++)
        {
                if (mgr->cache[i].flags != FLAG_FREE)
                {
                        struct pie_img_workspace* img;

                        PIE_TRACE("%s", mgr->cache[i].img->path);

                        img = mgr->cache[i].img;
                        pie_bm_free_f32(&img->raw);
                        /* make sure that proxies are allocated */
                        if (img->proxy.c_red)
                        {
                                pie_bm_free_f32(&img->proxy);
                        }
                        if (img->proxy_out.c_red)
                        {
                                pie_bm_free_f32(&img->proxy_out);
                        }
                        free(img);
                }
        }

        free(mgr->cache);
        free(mgr);
}
