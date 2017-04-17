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
#include "../lib/strutil.h"
#include "../pie_log.h"
#include "../dm/pie_host.h"
#include "../dm/pie_mountpoint.h"

#define MAX_LINE 256

static struct 
{
        char* dbpath;
        sqlite3* db;
        
} pie_cfg;

int pie_cfg_load(const char* p)
{
        FILE* f;
        char line[MAX_LINE];
        
        f = fopen(p, "r");
        if (f == NULL)
        {
                PIE_ERR("Could not open %s", p);
                return -1;
        }

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
                
                /* This is the lamest thing ever, but good enough for now */
                if (strcmp("db:path", line) == 0)
                {
                        pie_cfg.dbpath = malloc(strlen(d) + 1);
                        strcpy(pie_cfg.dbpath, d);
                }
        }

        fclose(f);

        if (pie_cfg.dbpath == NULL)
        {
                PIE_ERR("No database defined in %s", p);
                return -1;
        }
        
        if (sqlite3_open(pie_cfg.dbpath, &pie_cfg.db))
        {
                PIE_ERR("Could not open database at %s", pie_cfg.dbpath);
                pie_cfg.dbpath = NULL;
                return -1;
        }

        return 0;
}

void pie_cfg_close(void)
{
        if (pie_cfg.dbpath)
        {
                free(pie_cfg.dbpath);
                pie_cfg.dbpath = NULL;
        }
        if (pie_cfg.db)
        {
                sqlite3_close(pie_cfg.db);
                pie_cfg.db = NULL;
        }
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

                return pie_cfg_get_hostbyname(hostname);
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
        int ret;

        h->hst_name = malloc(strlen(host) + 1);
        strcpy(h->hst_name, host);
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
        
        /* Fetch all mount points for host */
        fail = pie_mountpoint_find_host(pie_cfg.db, mnt, host, cnt);
        if (fail)
        {
                printf("Failed to get mountpoints\n");
                return NULL;
        }

        ret = malloc(sizeof(struct pie_stg_mnt_arr));
        ret->arr = malloc(sizeof(struct pie_stg_mnt*) * cnt);
        ret->len = cnt;
        memset(ret->arr, 0, sizeof(struct pie_stg_mnt*) * cnt);

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
                PIE_LOG("i=%d, stg_id=%d", i, stg_id);

                ret->arr[stg_id] = malloc(sizeof(struct pie_stg_mnt));
                ret->arr[stg_id]->stg.stg_id = stg_id;
                fail = pie_storage_read(pie_cfg.db, &ret->arr[stg_id]->stg);
                if (fail)
                {
                        PIE_ERR("Failed to find storage %d", stg_id);
                        break;
                }
                strcpy(ret->arr[stg_id]->mnt_path, mnt[i]->mnt_path);
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
