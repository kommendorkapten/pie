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
#include "file_proc.h"
#include "ingestd_cfg.h"
#include "../prunt/pie_log.h"
#include "../prunt/pie_cfg.h"
#include "../vendor/fswalk.h"
#include "../vendor/s_queue.h"
#include "../vendor/evp_hw.h"
#include "../vendor/llist.h"
#include "../mq_msg/pie_mq_msg.h"
#include "../dm/pie_host.h"
#include "../dm/pie_storage.h"

static void print_usage(void);
static void cb_fun(const char*);
static const struct pie_stg_mnt* stg_from_path(const char*);

struct ingestd_cfg id_cfg;
static unsigned int num_files;
static unsigned int num_coll;
static unsigned int num_ok;

struct llist* lerr;
struct llist* lcoll;

int main(int argc, char** argv)
{
        size_t len;
        int ok;
        int status = -1;
        long evp_hw;
        int c;

        id_cfg.cp_mode = MODE_COPY_INTO;
        while ((c = getopt(argc, argv, "s:d:t")) != -1)
        {
                switch (c)
                {
                case 's':
                        id_cfg.src_path = optarg;
                        break;
                case 'd':
                        id_cfg.dst_path = optarg;
                        break;
                case 't':
                        id_cfg.cp_mode = MODE_COPY_TREE;
                        break;
                }
        }

        if (id_cfg.src_path == NULL || id_cfg.dst_path == NULL)
        {
                print_usage();
                return 1;
        }

        /* Remove trailing slash */
        len = strlen(id_cfg.src_path);
        if (id_cfg.src_path[len - 1] == '/')
        {
                id_cfg.src_path[len - 1] = '\0';
        }
        len = strlen(id_cfg.dst_path);
        if (id_cfg.dst_path[len - 1] != '/')
        {
                PIE_ERR("Destination must end with a '/'");
                return 1;
        }

        if (pie_cfg_load(PIE_CFG_PATH))
        {
                PIE_ERR("Failed to read conf\n");
                goto cleanup;
        }

        /* E V P */
        if (pie_cfg_get_long("ssl:evp:hw", &evp_hw))
        {
                evp_hw = 0;
        }
        evp_enable_hw((int)evp_hw);

        /* H O S T */
        id_cfg.host = pie_cfg_get_host(-1);
        if (id_cfg.host == NULL)
        {
                PIE_ERR("Could not load current host");
                goto cleanup;
        }

        /* S T O R A G E S */
        id_cfg.storages = pie_cfg_get_hoststg(id_cfg.host->hst_id);
        if (id_cfg.storages == NULL)
        {
                PIE_ERR("Could not resolve mount point for storage");
                goto cleanup;
        }

        /* Q U E U E S */
        id_cfg.queue = q_new_producer(QUEUE_INTRA_HOST);
        if (id_cfg.queue == NULL)
        {
                PIE_ERR("Can not create queue");
                goto cleanup;
        }

        ok = id_cfg.queue->init(id_cfg.queue->this, Q_INCOMING_MEDIA);
        if (ok)
        {
                PIE_ERR("Can not connect to '%s'", Q_INCOMING_MEDIA);
                goto cleanup;
        }

        /* Verify target is a valid storage */
        id_cfg.dst_stg = stg_from_path(id_cfg.dst_path);
        if (id_cfg.dst_stg == NULL)
        {
                PIE_ERR("%s is not a configured storage", id_cfg.dst_path);
                goto cleanup;
        }
        /* Remove dest stg's mount point from dst_path */
        size_t offset = strlen(id_cfg.dst_stg->mnt_path);
        len = strlen(id_cfg.dst_path) - offset;
        memmove(id_cfg.dst_path,
                id_cfg.dst_path + offset,
                len);
        id_cfg.dst_path[len] = '\0';

        lerr = llist_create();
        lcoll = llist_create();

        walk_dir(id_cfg.src_path, &cb_fun);
        status = 0;

        PIE_LOG("Processed %d files", num_files);
        PIE_LOG("          %d imported", num_ok);
        PIE_LOG("          %d duplicates", num_coll);
        PIE_LOG("          %d errors", num_files - (num_ok + num_coll));

        struct lnode* lnode = llist_head(lcoll);
        while (lnode)
        {
                PIE_LOG("dup:%s", lnode->data);
                free(lnode->data);
                lnode = lnode->next;
        }
        llist_destroy(lcoll);
        lnode = llist_head(lerr);
        while (lnode)
        {
                PIE_LOG("err:%s", lnode->data);
                free(lnode->data);
                lnode = lnode->next;
        }
        llist_destroy(lerr);
cleanup:
        if (id_cfg.queue)
        {
                id_cfg.queue->close(id_cfg.queue->this);
                q_del_producer(id_cfg.queue);
        }
        if (id_cfg.host)
        {
                pie_host_free(id_cfg.host);
                id_cfg.host = NULL;
        }
        if (id_cfg.storages)
        {
                pie_cfg_free_hoststg(id_cfg.storages);
                id_cfg.storages = NULL;
        }
        pie_cfg_close();

	return status;
}

static void cb_fun(const char* path)
{
        enum pie_fp_status status = pie_fp_add_file(path);
        char* p;
        size_t l;

        num_files++;
        switch (status)
        {
        case PIE_FP_OK:
                num_ok++;
                break;
        case PIE_FP_COLL:
                num_coll++;
                l = strlen(path) + 1;
                p = malloc(l);
                strncpy(p, path, l);
                llist_pushb(lcoll, p);
                break;
        default:
                PIE_WARN("Processing of %s failed", path);
                l = strlen(path) + 1;
                p = malloc(l);
                strncpy(p, path, l);
                llist_pushb(lerr, p);
                break;
        }
}

const struct pie_stg_mnt* stg_from_path(const char* p)
{
        struct pie_stg_mnt* stg = NULL;

        for (int i = 0; i < id_cfg.storages->len; i++)
        {
                char* pos;

                stg = id_cfg.storages->arr[i];

                if (stg == NULL)
                {
                        continue;
                }

                pos = strstr(p, stg->mnt_path);
                if (pos == NULL || pos != p)
                {
                        continue;
                }

                /* make sure the next pos in p is a '/' */
                if (*(p + strlen(stg->mnt_path)) != '/')
                {
                        continue;
                }

                break;
        }

        return stg;
}

static void print_usage(void)
{
        printf("usage ingestd -s src_path -d target_path -t\n");
        printf("-t preserves drectory structure\n");
}
