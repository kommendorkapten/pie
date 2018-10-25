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
#include <unistd.h>
#include <string.h>
#include "pie_doml_mob.h"
#include "pie_stg.h"
#include "../pie_types.h"
#include "../pie_log.h"
#include "../dm/pie_host.h"
#include "../dm/pie_min.h"
#include "../dm/pie_mob.h"
#include "../dm/pie_exif_data.h"
#include "../dm/pie_dev_params.h"
#include "../dm/pie_collection.h"
#include "../dm/pie_collection_member.h"
#include "../lib/llist.h"
#include "../lib/fal.h"
#include "../cfg/pie_cfg.h"

/**
 * Unlink a file.
 * @param Complete and absolute path to file.
 * @param Storage file is located on.
 * @param 0 on success.
 */
static int pie_doml_unlink(const char*, int);

/**
 * Move a file from one storage to another.
 * @param Target host.
 * @param Target storage.
 * @param Target absolute path.
 * @param Source host.
 * @param Source storage.
 * @param Source absolute path.
 * @param 0 on success.
 */
static int pie_doml_move(int, int, const char*, int, int, const char*);

int pie_doml_mob_delete(sqlite3* db, pie_id mob_id)
{
        char path[PIE_PATH_LEN];
        struct pie_stg_mnt_arr* stgs = NULL;
        struct llist* l = NULL;
        struct lnode* n;
        int ret = -1;
        int proxy = -1;
        int thumb = -1;

        PIE_DEBUG("Mob: %ld", mob_id);
        stgs = pie_cfg_get_hoststg(-1);
        for (int i = 0; i < stgs->len; i++)
        {
                if (stgs->arr[i])
                {
                        switch (stgs->arr[i]->stg.stg_type)
                        {
                        case PIE_STG_THUMB:
                                thumb = i;
                                break;
                        case PIE_STG_PROXY:
                                proxy = i;
                                break;
                        default:
                                break;
                        }
                }
        }
        if (proxy < 0 || thumb < 0)
        {
                PIE_WARN("Proxy or thumb is not mounted");
                goto done;
        }

        /* Find all MINs and their storages, unlink files */
        l = pie_min_find_mob(db, mob_id);

        if (l == NULL)
        {
                PIE_WARN("Error fetching MINs for MOB %ld", mob_id);
                goto done;
        }

        /* Verify all MINs are on storages mounted by this host */
        for (n = llist_head(l); n != NULL; n = n->next)
        {
                struct pie_min* min = n->data;

                if (min->min_stg_id > stgs->len ||
                    stgs->arr[min->min_stg_id] == NULL)
                {
                        PIE_WARN("Storage %d is not mounted", min->min_stg_id);
                        goto done;
                }
        }

        /* unlink files */
        snprintf(path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 stgs->arr[proxy]->mnt_path,
                 mob_id);
        pie_doml_unlink(path, PIE_STG_PROXY);
        snprintf(path,
                 PIE_PATH_LEN,
                 "%s/%ld.jpg",
                 stgs->arr[thumb]->mnt_path,
                 mob_id);
        pie_doml_unlink(path, PIE_STG_THUMB);
        for (n = llist_head(l); n != NULL; n = n->next)
        {
                struct pie_min* min = n->data;
                int type = stgs->arr[min->min_stg_id]->stg.stg_type;

                PIE_DEBUG("Remove MIN %ld at storage %d",
                          min->min_id,
                          min->min_stg_id);

                snprintf(path,
                         PIE_PATH_LEN,
                         "%s%s",
                         stgs->arr[min->min_stg_id]->mnt_path,
                         min->min_path);
                pie_doml_unlink(path, type);

                /* Delete MIN from database */
                pie_min_delete(db, min);
        }

        /* Delete any EXIF data */
        struct pie_exif_data ped = {.ped_mob_id = mob_id};
        pie_exif_data_delete(db, &ped);

        /* Delete any development parameters */
        struct pie_dev_params pdp = {.pdp_mob_id = mob_id};
        pie_dev_params_delete(db, &pdp);

        /* Delete collection membership */
        pie_collection_member_delete_mob(db, mob_id);

        /* Delete MOB from database */
        struct pie_mob mob = {.mob_id = mob_id};
        pie_mob_delete(db, &mob);

        ret = 0;
done:
        if (stgs)
        {
                pie_cfg_free_hoststg(stgs);
        }
        if (l)
        {
                for (n = llist_head(l); n != NULL; n = n->next)
                {
                        pie_min_free(n->data);
                }
                llist_destroy(l);
        }

        return ret;
}

/*
  Move a MOB from one collection to another.
  1: verify target file in new collection does not exist.
  2: Update database (should be done in a transaction):
    2.1: remove old collection member.
    2.2: create new collection member.
    2.3: update min path.
  3: move file.
 */
int pie_doml_mob_move(sqlite3* db,
                      pie_id tgt_col_id,
                      pie_id src_col_id,
                      pie_id mob_id)
{
        char tgt_path[PIE_PATH_LEN];
        char filename[PIE_PATH_LEN];
        struct pie_collection tgt_col;
        struct pie_host* host = NULL;
        struct llist* l = NULL;
        struct pie_min* min = NULL;
        char* p;
        int cnt = 0;
        int ret = -1;
        int ok;

        if (tgt_col_id == src_col_id)
        {
                return 0;
        }

        host = pie_cfg_get_host(-1);
        l = pie_min_find_mob(db, mob_id);

        if (l == NULL)
        {
                PIE_WARN("Could not find any MINs for mob %ld", mob_id);
                goto done;
        }
        if (host == NULL)
        {
                PIE_WARN("Could not resolve local host");
                goto done;
        }

        /* Only a single MIN per mob is currently supported */
        for (struct lnode* n = llist_head(l); n != NULL; n = n->next)
        {
                if (cnt > 0)
                {
                        PIE_WARN("Found multiple MINs for mob %ld", mob_id);
                        goto done;
                }
                cnt++;
                min = n->data;
        }

        if (min == NULL)
        {
                PIE_WARN("No min found for MOB %ld", mob_id);
                return -1;
        }

        /* Get the target collection */
        tgt_col.col_id = tgt_col_id;
        ok = pie_collection_read(db, &tgt_col);
        if (ok < 0)
        {
                PIE_ERR("Database error when reading collection: %d", ok);
                goto done;
        }
        if (ok > 0)
        {
                PIE_WARN("Collection %ld does not exist", tgt_col.col_id);
                goto done;
        }

        p = strrchr(min->min_path, '/');
        if (p == NULL)
        {
                PIE_ERR("Invalid file '%s' for MIN %ld",
                        min->min_path,
                        min->min_id);
                goto done;
        }
        strncpy(filename, p + 1, PIE_PATH_LEN);
        snprintf(tgt_path,
                 PIE_PATH_LEN,
                 "%s/%s",
                 tgt_col.col_path,
                 filename);

        /* See that file does not already exists */
        if (pie_doml_file_exists(min->min_stg_id, tgt_path))
        {
                PIE_WARN("Target filename exists");
                goto done;
        }

        /* Move mob from to new collection */
        struct pie_collection_member cmb;
        cmb.cmb_col_id = src_col_id;
        cmb.cmb_mob_id = mob_id;

        PIE_DEBUG("Delete MOB %ld from collection %ld", mob_id, cmb.cmb_col_id);
        if (pie_collection_member_delete(db, &cmb))
        {
                PIE_ERR("Could not delete MOB %ld from collection %ld",
                        mob_id,
                        src_col_id);
                goto done;
        }
        cmb.cmb_col_id = tgt_col_id;
        PIE_DEBUG("Add MOB %ld to collection %ld", mob_id, cmb.cmb_col_id);
        if (pie_collection_member_create(db, &cmb))
        {
                PIE_ERR("Could not move MOB %ld to collection %ld",
                        mob_id,
                        tgt_col_id);
                goto done;
        }

        /* Move file */
        ret = pie_doml_move(host->hst_id,
                            min->min_stg_id,
                            tgt_path,
                            host->hst_id,
                            min->min_stg_id,
                            min->min_path);
        if (ret)
        {
                PIE_ERR("Could not move file");
                /* Move the mob back to original coll... */
                goto done;
        }

        /* Rewrite min_path */
        strncpy(min->min_path, tgt_path, PIE_PATH_LEN);
        if (pie_min_update(db, min))
        {
                PIE_ERR("Could not update MIN %ld", min->min_id);
                goto done;
        }

        ret = 0;
done:

        if (l)
        {
                for (struct lnode* n = llist_head(l); n != NULL; n = n->next)
                {
                        pie_min_free(n->data);
                }
                llist_destroy(l);
                l = NULL;
        }
        if (host)
        {
                pie_host_free(host);
                host = NULL;
        }

        return ret;
}

static int pie_doml_unlink(const char* path, int stg_type)
{
        int valid_stg = 0;

        switch (stg_type)
        {
        case PIE_STG_ONLINE:
        case PIE_STG_THUMB:
        case PIE_STG_PROXY:
                valid_stg = 1;
        }

        if (!valid_stg)
        {
                PIE_ERR("Unsupported storage type: %d", stg_type);
                return -1;
        }

        PIE_DEBUG("Remove '%s' on storage type %d\n", path, stg_type);

        return unlink(path);
}

static int pie_doml_move(int tgt_host,
                         int tgt_stg,
                         const char* tgt_rel_path,
                         int src_host,
                         int src_stg,
                         const char* src_rel_path)
{
        char tgt_path[PIE_PATH_LEN];
        char src_path[PIE_PATH_LEN];
        struct pie_stg_mnt_arr* stgs;
        int ok;

        if (tgt_host != src_host || src_stg != tgt_stg)
        {
                PIE_WARN("Invalid operation");
                return 1;
        }

        stgs = pie_cfg_get_hoststg(-1);
        if (stgs == NULL)
        {
                PIE_WARN("Culd not look up storages");
                return -1;
        }
        snprintf(tgt_path,
                 PIE_PATH_LEN,
                 "%s%s",
                 stgs->arr[tgt_stg]->mnt_path,
                 tgt_rel_path);
        snprintf(src_path,
                 PIE_PATH_LEN,
                 "%s%s",
                 stgs->arr[src_stg]->mnt_path,
                 src_rel_path);
        pie_cfg_free_hoststg(stgs);

        PIE_DEBUG("Move %s from %d@%d to %s %d@%d",
                  src_path, src_stg, src_host,
                  tgt_path, tgt_stg, tgt_host);
        ok = rename(src_path, tgt_path);
        if (ok)
        {
                PIE_ERR("Failed to rename");
                perror("rename");
                return 1;
        }
        return 0;
}
