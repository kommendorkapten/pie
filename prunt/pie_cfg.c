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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <assert.h>
#include "pie_cfg.h"
#include "../vendor/hmap.h"
#include "../vendor/strutil.h"
#include "../prunt/pie_log.h"
#include "../dm/pie_host.h"
#include "../dm/pie_mountpoint.h"

#define MAX_LINE 256
#define DB_PATH "db:path"

static struct
{
        sqlite3* db;
        struct hmap* cfg_map;
} pie_cfg;

int pie_cfg_load(const char* p)
{
        char line[MAX_LINE];
        const char* db_path;
        FILE* f;

        f = fopen(p, "r");
        if (f == NULL)
        {
                PIE_ERR("Could not open %s", p);
                return -1;
        }

        pie_cfg.cfg_map = hmap_create(NULL, NULL, 32, 0.7f);

        while (fgets(line, MAX_LINE, f))
        {
                char* d;

                strrstrip(line);
                strlstrip(line);

                if (line[0] == '#')
                {
                        /* Ignore comments */
                        continue;
                }

                if (line[0] == '\0')
                {
                        /* Empty line */
                        continue;
                }

                d = strchr(line, ' ');
                if (d == NULL)
                {
                        printf("Bad line: '%s'\n", line);
                        continue;
                }
                *d++ = '\0';
                strlstrip(d);

                size_t key_len = strlen(line) + 1;
                size_t val_len = strlen(d) + 1;
                char* key = malloc(key_len);
                char* val = malloc(val_len);

                strncpy(key, line, key_len);
                strncpy(val, d, val_len);
                hmap_set(pie_cfg.cfg_map, key, val);
        }

        fclose(f);
        db_path = pie_cfg_get(DB_PATH);
        if (db_path == NULL)
        {
                PIE_ERR("No database defined in %s", p);
                return -1;
        }

        if (sqlite3_open(db_path, &pie_cfg.db))
        {
                PIE_ERR("Could not open database at %s", db_path);
                return -1;
        }

        return 0;
}

int pie_cfg_loaded(void)
{
        return pie_cfg.db != NULL;
}

void pie_cfg_close(void)
{
        if (pie_cfg.cfg_map)
        {
                size_t len;
                struct hmap_entry* e = hmap_iter(pie_cfg.cfg_map, &len);

                for (size_t i = 0; i < len; i++)
                {
                        free(e[i].key);
                        free(e[i].data);
                }
                free(e);
                hmap_destroy(pie_cfg.cfg_map);
                pie_cfg.cfg_map = NULL;
        }
        if (pie_cfg.db)
        {
                sqlite3_close(pie_cfg.db);
                pie_cfg.db = NULL;
        }
}

sqlite3* pie_cfg_get_db(void)
{
        return pie_cfg.db;
}

const char* pie_cfg_get(const char* key)
{
        if (pie_cfg.cfg_map == NULL)
        {
                return NULL;
        }

        return (const char*)hmap_get(pie_cfg.cfg_map, key);
}

int pie_cfg_get_long(const char* key, long* l)
{
        const char* v = hmap_get(pie_cfg.cfg_map, key);
        char* endptr;
        int ret = 1;

        if (v != NULL)
        {
                *l = strtol(v, &endptr, 10);

                if (v != endptr)
                {
                        ret = 0;
                }
        }

        return ret;
}

struct pie_host* pie_cfg_get_host(int host)
{
        struct pie_host* h;

        if (host < 0)
        {
                char hostname[128];

                if (gethostname(hostname, 128))
                {
                        PIE_WARN("Could not get hostname");
                        return NULL;
                }
                PIE_DEBUG("Current host is '%s'", hostname);
                char *p = strchr(hostname, '.');
                if (p)
                {
                        *p = '\0';
                        PIE_DEBUG("Stripping domain information: %s",
                                  hostname);
                }

                h = pie_cfg_get_hostbyname(hostname);
        }
        else
        {
                int ret;
                h = pie_host_alloc();

                h->hst_id = host;
                ret = pie_host_read(pie_cfg.db, h);
                if (ret > 0)
                {
                        pie_host_free(h);
                        h = NULL;
                }
                else if (ret < 0)
                {
                        pie_host_free(h);
                        h = NULL;
                        PIE_ERR("Error communicating with database");
                }
        }

        return h;
}

struct pie_host* pie_cfg_get_hostbyname(const char* host)
{
        struct pie_host* h = pie_host_alloc();
        size_t host_len = strlen(host) + 1;
        int ret;

        strncpy(h->hst_name, host, host_len);
        ret = pie_host_find_name(pie_cfg.db, h);

        if (ret > 0)
        {
                pie_host_free(h);
                h = NULL;
        }
        else if (ret < 0)
        {
                pie_host_free(h);
                h = NULL;
                PIE_ERR("Error communicating with database");
        }

        return h;
}

struct pie_stg_mnt_arr* pie_cfg_get_hoststg(int host)
{
        /* Simple static implementation, assume no more than 8 storages
           AND no storage id is higher than 7 */
        int cnt = 8;
        struct pie_stg_mnt_arr* ret;
        struct pie_mountpoint* mnt[cnt];
        int fail;

        if (host < 0)
        {
                /* Current host is requested */
                struct pie_host* ph = pie_cfg_get_host(host);

                if (ph)
                {
                        host = ph->hst_id;
                        pie_host_free(ph);
                }
                else
                {
                        return NULL;
                }
        }

        /* Fetch all mount points for host */
        fail = pie_mountpoint_find_host(pie_cfg.db, mnt, host, cnt - 1);
        if (fail)
        {
                printf("Failed to get mountpoints\n");
                return NULL;
        }

        /* If more storages is found than cnt,
           it will not be null terminated */
        mnt[cnt - 1] = NULL;
        ret = malloc(sizeof(struct pie_stg_mnt_arr));
        ret->arr = malloc(sizeof(struct pie_stg_mnt*) * cnt);
        ret->len = cnt;
        memset(ret->arr, 0, sizeof(struct pie_stg_mnt*) * cnt);
        /* Safety check, only one online stg is supported now */
        int online = 0;
        for (int i = 0; i < cnt; i++)
        {
                int stg_id;

                if (mnt[i] == NULL)
                {
                        /* mnt is NULL terminated if less than cnt
                           storages where found */
                        break;
                }

                stg_id = mnt[i]->mnt_stg_id;
                if (stg_id >= cnt)
                {
                        PIE_ERR("More storages exists than supported (%d >= %d",
                                stg_id, cnt);
                        fail = 1;
                        break;
                }

                ret->arr[stg_id] = malloc(sizeof(struct pie_stg_mnt));
                ret->arr[stg_id]->stg.stg_id = stg_id;
                fail = pie_storage_read(pie_cfg.db, &ret->arr[stg_id]->stg);
                if (fail)
                {
                        PIE_ERR("Failed to find storage %d", stg_id);
                        break;
                }
                strncpy(ret->arr[stg_id]->mnt_path,
                        mnt[i]->mnt_path,
                        PIE_PATH_LEN);
                if (ret->arr[stg_id]->stg.stg_type == PIE_STG_ONLINE)
                {
                        if (online)
                        {
                                PIE_ERR("Max 1 online storage is currently supported");
                                abort();
                        }
                        online = 1;
                }
        }
        for (int i = 0; i < cnt; i++)
        {
                if (mnt[i] == NULL)
                {
                        break;
                }
                pie_mountpoint_free(mnt[i]);
        }
        if (fail)
        {
                pie_cfg_free_hoststg(ret);
                ret = NULL;
        }

        return ret;
}

void pie_cfg_free_hoststg(struct pie_stg_mnt_arr* arr)
{
        assert(arr);

        for (int i = 0; i < arr->len; i++)
        {
                struct pie_stg_mnt* p = arr->arr[i];

                if (p == NULL)
                {
                        continue;
                }

                pie_storage_release(&p->stg);
                free(p);
        }

        free(arr->arr);
        free(arr);
}
