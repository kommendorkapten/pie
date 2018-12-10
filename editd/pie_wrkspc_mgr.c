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

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "pie_wrkspc_mgr.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../io/pie_io.h"
#include "../bm/pie_bm.h"
#include "../lib/timing.h"
#include "../cfg/pie_cfg.h"
#include "../doml/pie_doml_stg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_min.h"

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
        struct pie_editd_workspace* wrkspc;
        time_t ts;
        unsigned int flags;
};

struct pie_wrkspc_mgr
{
        /* Storage config */
        struct pie_host* host;
        struct pie_stg_mnt_arr* storages;
        sqlite3* db;
        struct entry* cache;
        int cap;
};

struct pie_wrkspc_mgr* pie_wrkspc_mgr_create(sqlite3* db, int cap)
{
        struct pie_wrkspc_mgr* mgr;

        if (!pie_cfg_loaded())
        {
                PIE_ERR("Configuration is not loaded");
                return NULL;
        }

        mgr = malloc(sizeof(struct pie_wrkspc_mgr));
        if (mgr == NULL)
        {
                PIE_ERR("Failed to malloc struct");
                return NULL;
        }

        mgr->db = db;
        mgr->host = pie_cfg_get_host(-1);
        if (mgr->host == NULL)
        {
                free(mgr);
                PIE_ERR("Failed to get host");
                return NULL;
        }

        mgr->storages = pie_cfg_get_hoststg(mgr->host->hst_id);
        if (mgr->storages == NULL)
        {
                PIE_ERR("Could not resolve mount point for storage");
                pie_host_free(mgr->host);
                free(mgr);
                return NULL;
        }

        mgr->cap = cap;
        mgr->cache = malloc(mgr->cap * sizeof(struct entry));
        if (mgr->cache == NULL)
        {
                PIE_ERR("Could not malloc cache");
                pie_host_free(mgr->host);
                pie_cfg_free_hoststg(mgr->storages);
                free(mgr);
                return NULL;
        }

        for (int i = 0; i < mgr->cap; i++)
        {
                mgr->cache[i].flags = FLAG_FREE;
        }

        return mgr;
}

struct pie_editd_workspace* pie_wrkspc_mgr_acquire(struct pie_wrkspc_mgr* mgr,
                                                   pie_id id)
{
        struct pie_editd_workspace* wrkspc = NULL;
        struct timeval tv;

        gettimeofday(&tv, NULL);

        /* Linear scan over entries */
        for (int i = 0; i < mgr->cap; i++)
        {
                if (mgr->cache[i].flags == FLAG_FREE)
                {
                        continue;
                }
                if (mgr->cache[i].wrkspc->mob_id == id)
                {
                        wrkspc = mgr->cache[i].wrkspc;
                        mgr->cache[i].flags = FLAG_ACTIVE;
                        mgr->cache[i].ts = tv.tv_sec;
                        PIE_DEBUG("Found: %ld at pos %d",
                                  mgr->cache[i].wrkspc->mob_id,
                                  i);
                        break;
                }
        }
        if (wrkspc != NULL)
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
                wrkspc = mgr->cache[evict].wrkspc;
                PIE_TRACE("Replace %ld at pos %d with %s",
                          wrkspc->mob_id,
                          evict,
                          path);

                pie_bm_free_f32(&wrkspc->raw);
                /* make sure that proxies are allocated */
                if (wrkspc->proxy.c_red)
                {
                        pie_bm_free_f32(&wrkspc->proxy);
                }
                if (wrkspc->proxy_out.c_red)
                {
                        pie_bm_free_f32(&wrkspc->proxy_out);
                }
                free(wrkspc);
                /* Mark slot as free, as it is not know yet wheter the
                   new image actually exists */
                mgr->cache[evict].flags = FLAG_FREE;
                pos = evict;
        }

        if (pos != -1)
        {
                char buf[PIE_PATH_LEN];
                struct pie_min* min;
                struct timing t;
                int res;

                /* Check for MIN existance before allocating anything */
                min = pie_doml_min_for_mob(mgr->db,
                                           mgr->storages->arr,
                                           mgr->storages->len,
                                           id);
                if (min == NULL)
                {
                        PIE_WARN("Could not find any MIN for MOB %ld", id);
                        return NULL;
                }
                PIE_DEBUG("Got MIN %ld for MOB %ld",
                          min->min_id,
                          id);

                wrkspc = malloc(sizeof(struct pie_editd_workspace));
                if (wrkspc == NULL)
                {
                        goto done;
                }
                memset(wrkspc, 0, sizeof(struct pie_editd_workspace));

                /* Accessing mgr->storages[min->min_stg_id] is safe.
                   pie_stg_min_for_mob verifies storage is mounted. */
                snprintf(buf, PIE_PATH_LEN,
                         "%s%s",
                         mgr->storages->arr[min->min_stg_id]->mnt_path,
                         min->min_path);
                /* Drop min, no longer needed */
                pie_min_free(min);

                timing_start(&t);
                struct pie_io_opt opts;
                opts.qual = PIE_IO_NORM_QUAL;
                res = pie_io_load(&wrkspc->raw, buf, &opts);
                PIE_DEBUG("Loaded '%s'@stg-%d in %ldusec",
                          buf,
                          min->min_stg_id,
                          timing_dur_usec(&t));
                if (res)
                {
                        PIE_ERR("Could not open '%s'",
                                buf);
                        free(wrkspc);
                        wrkspc = NULL;
                        goto done;
                }

                wrkspc->mob_id = id;
                mgr->cache[pos].flags = FLAG_ACTIVE;
                mgr->cache[pos].wrkspc = wrkspc;
                mgr->cache[pos].ts = tv.tv_sec;

                /* Load exif data */
                wrkspc->exif.ped_mob_id = id;
                if (pie_exif_data_read(mgr->db,
                                       &wrkspc->exif))
                {
                        PIE_ERR("Could not load exif data for MOB %ld", id);
                }
                else
                {
                        PIE_LOG("Loaded exif data for MOB %ld", id);
                }

                PIE_TRACE("Create: %ld at pos %d",
                          mgr->cache[pos].wrkspc->mob_id,
                          pos);
        }
        else
        {
                NOTE(EMPTY);
                PIE_TRACE("Cache is full. This is most likely a bug!");
        }

done:
        return wrkspc;
}

void pie_wrkspc_mgr_release(struct pie_wrkspc_mgr* mgr,
                            struct pie_editd_workspace* wrkspc)
{
        for (int i = 0; i < mgr->cap; i++)
        {
                if (mgr->cache[i].wrkspc == wrkspc)
                {
                        PIE_TRACE("%ld at pos %d",
                                  mgr->cache[i].wrkscp->mob_id, i);
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
                        struct pie_editd_workspace* wrkspc;

                        PIE_TRACE("%ld", mgr->cache[i].wrkspc->mob_id);

                        wrkspc = mgr->cache[i].wrkspc;
                        pie_bm_free_f32(&wrkspc->raw);
                        /* make sure that proxies are deallocated */
                        if (wrkspc->proxy.c_red)
                        {
                                pie_bm_free_f32(&wrkspc->proxy);
                        }
                        if (wrkspc->proxy_out.c_red)
                        {
                                pie_bm_free_f32(&wrkspc->proxy_out);
                        }
                        pie_exif_data_release(&wrkspc->exif);
                        free(wrkspc);
                }
        }

        pie_host_free(mgr->host);
        pie_cfg_free_hoststg(mgr->storages);
        free(mgr->cache);
        free(mgr);
}
